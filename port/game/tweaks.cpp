/*
 * Engine tweaks that do not depend on offsets inside the library.
 *
 *   <PREFIX>_FAST_CAS=0   turn off the atomics fix below (on by default). A library built for ARMv5
 *                         calls the kernel's cmpxchg helper at 0xffff0fc0 for its __sync_* functions,
 *                         which qemu-user emulates by trapping out of translated code on every call.
 *                         Each literal holding that address is pointed at a small ldrex/strex routine.
 *   <PREFIX>_SLEEP_TRACE=1   who calls glf::Thread::Sleep, see sleep_trace_hook.
 *   <PREFIX>_MONEY_INFINITE=1, <PREFIX>_MONEY_RESTORE=N   the credits balance, see install_money.
 *   <PREFIX>_UCLAMP=0     leave the scheduler's clamp on the main thread alone (see mc3_sched_tick).
 *   <PREFIX>_UCLAMP_MIN=0..1024 also ask for at least this much (0 by default).
 */
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "so_util.h"
#include "trace.h"
#include "port_env.h"

#include "mc3.h"

static void install_fast_cas(so_module *mod)
{
    /* kuser cmpxchg: r0 = old, r1 = new, r2 = ptr; returns r0 = 0 when it stored. */
    static const uint32_t code[] = {
        0xf57ff05b, /* dmb ish            */
        0xe1923f9f, /* 1: ldrex r3,[r2]   */
        0xe0533000, /* subs r3,r3,r0      */
        0x01823f91, /* strexeq r3,r1,[r2] */
        0x03330001, /* teqeq r3,#1        */
        0x0afffffa, /* beq 1b             */
        0xf57ff05b, /* dmb ish            */
        0xe1a00003, /* mov r0,r3          */
        0xe12fff1e, /* bx lr              */
    };
    void *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (page == MAP_FAILED) {
        trace("tweaks: fast cas: mmap failed");
        return;
    }
    memcpy(page, code, sizeof(code));
    uint32_t *w = (uint32_t *)mod->text_base;
    size_t n = mod->text_size / 4, hits = 0;
    for (size_t i = 0; i < n; i++) {
        if (w[i] == 0xffff0fc0u) {
            w[i] = (uint32_t)(uintptr_t)page;
            hits++;
        }
    }
    trace("tweaks: kuser cmpxchg literal replaced at %zu places (routine %p)", hits, page);
}

/*
 * <PREFIX>_SLEEP_TRACE=1: who calls glf::Thread::Sleep, for how long and from which thread.
 * The function is three instructions (ms * 1000, tail call to usleep), so it is replaced outright.
 * One line per distinct call site, then a count line every 600 calls.
 */
static so_module *g_trace_mod;

static void sleep_trace(uintptr_t lr, unsigned ms)
{
    static struct { uintptr_t lr; unsigned calls; unsigned long total_ms; } sites[32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    static unsigned long calls;
    lr -= g_trace_mod->text_base;

    pthread_mutex_lock(&lock);
    int slot = -1;
    for (int i = 0; i < 32; i++) {
        if (sites[i].lr == lr || !sites[i].lr) {
            slot = i;
            break;
        }
    }
    if (slot >= 0) {
        if (!sites[slot].lr) {
            sites[slot].lr = lr;
            trace("sleep trace: new call site +0x%lx: %u ms, thread %ld%s", (unsigned long)lr, ms,
                  (long)syscall(SYS_gettid), syscall(SYS_gettid) == getpid() ? " (main)" : "");
        }
        sites[slot].calls++;
        sites[slot].total_ms += ms;
    }
    if (++calls % 600 == 0) {
        for (int i = 0; i < 32 && sites[i].lr; i++) {
            if (sites[i].calls)
                trace("sleep trace: +0x%lx: %u calls, %lu ms in total", (unsigned long)sites[i].lr,
                      sites[i].calls, sites[i].total_ms);
            sites[i].calls = 0;
            sites[i].total_ms = 0;
        }
    }
    pthread_mutex_unlock(&lock);
    usleep(ms * 1000u);
}

static void sleep_trace_hook(unsigned ms)
{
    sleep_trace((uintptr_t)__builtin_return_address(0), ms);
}

/* glf::Thread::_Sleep(ms): the same three instructions as a member function. */
static void sleep_trace_hook_member(void *self, unsigned ms)
{
    (void)self;
    sleep_trace((uintptr_t)__builtin_return_address(0), ms);
}

/*
 * <PREFIX>_MONEY_INFINITE=1: the player's credits (the blue ones) read as kMoneyInfinite and spending
 *                            takes nothing.
 * <PREFIX>_MONEY_RESTORE=N:  once the profile is there, the credits are set to N (only with the infinite
 *                            option off).
 *
 * The 1.0.1 library is stripped, so the three functions are known by offset (found from the
 * "NbCreditsLocal" value the menu shows) and each is checked word for word before anything is changed;
 * another build fails the check and is left alone:
 *
 *   +0xceeac  credits getter    the profile's vtable[12]('stor'), lrand48 noise, return [this+0x90]
 *   +0xcf884  add credits       [this+0x90] += n (x50 under a debug flag), capped at 99 999 999
 *   +0xd028c  remove credits    [this+0x90] -= n, never below 0
 *   +0x7c644  the game object   (a global); its +0x27c is the local profile, +0x280 the online one
 *
 * Infinite: the getter becomes "ldr r0,[pc,#0]; bx lr; .word kMoneyInfinite" and remove becomes
 * "bx lr". The add function is untouched: the real balance at +0x90 keeps growing, so switching the
 * option off gives back what the player had plus what was earned. The gold credits are a different
 * field (+0x6f) and are not touched.
 */
static const int kMoneyInfinite = 9999999;
static const uintptr_t kCreditsGet = 0xceeac, kCreditsRemove = 0xd028c, kGameObject = 0x7c644;
static const uint32_t kCreditsGetWords[4]    = { 0xe3071473u, 0xe5903000u, 0xe92d4010u, 0xe347126fu };
static const uint32_t kCreditsRemoveWords[4] = { 0xe92d4070u, 0xe2515000u, 0xe1a04000u, 0xba00000eu };
static const uint32_t kGameObjectWords[3]    = { 0xe59f3004u, 0xe79f0003u, 0xe12fff1eu };
static void *(*g_game_object)(void);
static long g_money_restore = -1;

static bool code_is(so_module *mod, uintptr_t off, const uint32_t *words, int n)
{
    if (off + 4u * n > mod->text_size)
        return false;
    return memcmp((const void *)(mod->text_base + off), words, 4u * n) == 0;
}

/* Code that may already have been translated: make the page writable first (qemu then drops its
 * translations), and say if that failed instead of faulting on the store. */
static bool code_write(so_module *mod, uintptr_t off, const uint32_t *words, int n)
{
    uintptr_t at = mod->text_base + off, page = at & ~(uintptr_t)4095;
    if (mprotect((void *)page, ((at + 4u * n - page) + 4095) & ~(uintptr_t)4095, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        trace("tweaks: money: mprotect at +0x%lx failed (errno %d)", (unsigned long)off, errno);
        return false;
    }
    memcpy((void *)at, words, 4u * n);
    __builtin___clear_cache((char *)at, (char *)(at + 4u * n));
    return true;
}

static void install_money(so_module *mod)
{
    long restore = port_getenv_long("MONEY_RESTORE", -1);
    bool infinite = port_getenv_long("MONEY_INFINITE", 0) != 0;
    if (!infinite && restore < 0)
        return;

    if (!code_is(mod, kCreditsGet, kCreditsGetWords, 4) || !code_is(mod, kCreditsRemove, kCreditsRemoveWords, 4)
            || !code_is(mod, kGameObject, kGameObjectWords, 3)) {
        trace("tweaks: money: the credit functions are not those of 1.0.1 - left alone");
        return;
    }

    if (infinite) {
        const uint32_t get[3] = { 0xe59f0000u /* ldr r0, [pc, #0] */, 0xe12fff1eu /* bx lr */, (uint32_t)kMoneyInfinite };
        const uint32_t rem[1] = { 0xe12fff1eu /* bx lr */ };
        if (code_write(mod, kCreditsGet, get, 3) && code_write(mod, kCreditsRemove, rem, 1))
            trace("tweaks: money: infinite (%d), spending takes nothing", kMoneyInfinite);
        return;
    }

    g_game_object = (void *(*)(void))(mod->text_base + kGameObject);
    g_money_restore = restore > 99999999 ? 99999999 : restore;
    trace("tweaks: money: credits will be set to %ld once the profile is there", g_money_restore);
}

static void money_tick(long frames)
{
    static long ready_at = -1;
    if (g_money_restore < 0 || frames % 30 != 0)
        return;
    char *game = (char *)g_game_object();
    char *profile = game ? *(char **)(game + 0x27c) : NULL;
    if (!profile) {
        ready_at = -1;
        return;
    }
    /* A little after the profile appears, so that a load in progress does not overwrite the value. */
    if (ready_at < 0)
        ready_at = frames;
    if (frames - ready_at < 300)
        return;
    int before = *(int *)(profile + 0x90);
    *(int *)(profile + 0x90) = (int)g_money_restore;
    trace("tweaks: money: credits %d replaced by %ld", before, g_money_restore);
    g_money_restore = -1;
}

void mc3_tweaks_install(so_module *mod)
{
    if (port_getenv_long("FAST_CAS", 1))
        install_fast_cas(mod);

    install_money(mod);

    if (port_getenv_long("SLEEP_TRACE", 0)) {
        g_trace_mod = mod;
        if (so_symbol(mod, "_ZN3glf6Thread5SleepEj")) {
            hook_symbol(mod, "_ZN3glf6Thread5SleepEj", (uintptr_t)sleep_trace_hook, 1);
            trace("tweaks: glf::Thread::Sleep is traced");
            if (so_symbol(mod, "_ZN3glf6Thread6_SleepEj"))
                hook_symbol(mod, "_ZN3glf6Thread6_SleepEj", (uintptr_t)sleep_trace_hook_member, 1);
        } else {
            trace("tweaks: no glf::Thread::Sleep exported by this library - not traced");
        }
    }
}

/*
 * The phone's scheduler puts a utilisation clamp on the game's main thread a few seconds into a
 * mission: uclamp.max 171-199 of 1024, on that one thread and no other. To the frequency governor the
 * thread then never looks more than a fifth loaded, and the core stays at its lowest clock while the
 * thread is in fact saturated (util 1023, 1.0 GHz of 4.3). The clamp is a per-task attribute its owner
 * may set, so the main thread sets it back - again and again, because whatever lowers it does so
 * repeatedly.
 *
 * sched_setattr is not something Android apps normally call, and a system call the seccomp filter
 * does not know kills the process, so the first call is made in a forked child.
 */
struct mc3_sched_attr {
    uint32_t size, sched_policy;
    uint64_t sched_flags;
    int32_t  sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime, sched_deadline, sched_period;
    uint32_t sched_util_min, sched_util_max;
};

static int set_uclamp(uint32_t lo)
{
    mc3_sched_attr a;
    memset(&a, 0, sizeof(a));
    a.size = sizeof(a);
    a.sched_flags = 0x08 /* KEEP_POLICY */ | 0x10 /* KEEP_PARAMS */ | 0x20 /* UTIL_CLAMP_MIN */ | 0x40 /* UTIL_CLAMP_MAX */;
    a.sched_util_min = lo;
    a.sched_util_max = 1024;
    return syscall(SYS_sched_setattr, 0, &a, 0) == 0 ? 0 : errno;
}

void mc3_sched_tick(long frames)
{
    money_tick(frames);

    static int state = -1;      /* -1 not tried, 0 off, 1 working */
    static uint32_t lo;

    if (state == 0 || (state == 1 && frames % 8 != 0))
        return;
    if (state < 0) {
        state = 0;
        if (!port_getenv_long("UCLAMP", 1))
            return;
        long v = port_getenv_long("UCLAMP_MIN", 0);
        lo = v < 0 ? 0 : v > 1024 ? 1024 : (uint32_t)v;
        pid_t child = fork();
        if (child < 0)
            return;
        if (child == 0)
            _exit(set_uclamp(lo) & 0x7f);
        int st = 0;
        if (waitpid(child, &st, 0) != child || !WIFEXITED(st) || WEXITSTATUS(st) != 0) {
            trace("tweaks: uclamp: sched_setattr unusable here (%s %d) - left alone",
                  WIFEXITED(st) ? "errno" : "signal", WIFEXITED(st) ? WEXITSTATUS(st) : WTERMSIG(st));
            return;
        }
        state = 1;
        trace("tweaks: uclamp: main thread kept at min %u, max 1024", lo);
    }
    int e = set_uclamp(lo);
    if (e) {
        trace("tweaks: uclamp: sched_setattr failed (errno %d) - stopped", e);
        state = 0;
    }
}
