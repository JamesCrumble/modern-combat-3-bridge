/*
 * com/gameloft/glf/GL2JNILib and <package>/GloftM3HP - the Java half of this
 * game - plus the small classes under the game's own package.
 *
 * Modern Combat 3 1.0.1 (com.gameloft.android.GAND.GloftM3HP). The engine looks
 * its glf callbacks up on the class GL2JNILib.init() is handed and the game's
 * own on <package>/GloftM3HP (static methods of the activity). The names are in
 * the library's .rodata, the descriptors are the ones classes.dex declares.
 *
 * Almost everything answers with the quietest defensible constant and comes
 * from tools/gen_stubs.sh (glf_gen.inc, game_gen.inc, device_gen.inc). The ones
 * written here do real work:
 *
 *   setupPaths               hands the engine its three directories (mc3_set_paths)
 *   getSDFolder              the data directory (the cache tree)
 *   useGeneration0/1/2       the device class: all false = the best profile,
 *                            which is what the game picks for an unknown phone
 *
 * Unlike MC4 there is no SwapBuffer: the GLSurfaceView swaps after every
 * step(), and main.cpp does the same.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <string>

#include "platform.h"
#include "jni.h"
#include "jni_internals.h"
#include "app_exit.h"
#include "io_paths.h"
#include "port_env.h"
#include "so_util.h"
#include "trace.h"

#include "mc3.h"
#include "mc3_classes.h"

#define MC3_PACKAGE "com/gameloft/android/GAND/GloftM3HP"

GloftGL2JNIActivity g_activity;

/* The surface, kept for anything that asks for the window size. */
static int g_width  = 640;
static int g_height = 480;

void mc3_set_view_size(int w, int h)
{
    g_width  = w;
    g_height = h;
}

static const char *cstr(jstring s)
{
    return s ? ((String *)s)->str : "";
}

/* Says once that a generated stub was reached: the first thing to look at when
 * the game behaves as if Java had told it something wrong. */
#define GEN_SAID(what)                                  \
    do {                                                \
        static bool said;                               \
        if (!said) {                                    \
            said = true;                                \
            trace("java stub answered: %s", what);      \
        }                                               \
    } while (0)

#define STR "Ljava/lang/String;"
#define M(fn, name, sig) ManagedMethod::RegisterStatic<&fn>(CLS::clazz, name, sig)

/* ------------------------------------------------------------ the paths */

static void Lib_setupPaths(JNIEnv *env, jclass clazz)
{
    (void)clazz;
    mc3_set_paths(env);
}

/*
 * GL2JNILib.getResource: Class.getResourceAsStream on Android, i.e. a file at
 * the root of the APK (with a leading "." dropped). The wrapper unpacks the
 * APK's assets/ next to the library; a name that is not there is looked up at
 * the root of the game tree. Empty and not NULL when missing.
 */
static jbyteArray read_asset(JNIEnv *env, const char *name)
{
    if (name[0] == '.')
        name++;
    while (name[0] == '/')
        name++;
    char path[PATH_MAX];
    FILE *f = NULL;
    if (!strstr(name, "..")) {
        snprintf(path, sizeof(path), "%s/assets/%s", io_game_dir(), name);
        f = fopen(path, "rb");
        if (!f) {
            snprintf(path, sizeof(path), "%s/%s", io_game_dir(), name);
            f = fopen(path, "rb");
        }
    }
    if (!f) {
        trace("resource '%s' not found - answered empty", name);
        return env->NewByteArray(0);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string data((size_t)(size > 0 ? size : 0), '\0');
    size_t got = data.empty() ? 0 : fread(&data[0], 1, data.size(), f);
    fclose(f);
    jbyteArray out = env->NewByteArray((jsize)got);
    if (got)
        env->SetByteArrayRegion(out, 0, (jsize)got, (const jbyte *)data.data());
    trace("resource '%s': %zu bytes", name, got);
    return out;
}

static jbyteArray Lib_getResource(JNIEnv *env, jclass clazz, jstring name)
{
    (void)clazz;
    return read_asset(env, cstr(name));
}

/* ------------------------------------------------------------- the view */

static void Lib_createView(JNIEnv *env, jclass clazz)
{
    (void)env; (void)clazz;
    trace("createView - the loader already owns the window");
}

static void Lib_setViewSettings(JNIEnv *env, jclass clazz,
                                jint pixel, jint depth, jint stencil,
                                jint csaa, jint extra)
{
    (void)env; (void)clazz;
    trace("setViewSettings(pixel=%d depth=%d stencil=%d csaa=%d extra=%d)",
          pixel, depth, stencil, csaa, extra);
}

static jboolean Lib_setCurrentContext(JNIEnv *env, jclass clazz, jint index)
{
    (void)env; (void)clazz;
    /* One context; anything else is refused rather than silently accepted. */
    return index == 0 ? JNI_TRUE : JNI_FALSE;
}

static jobject Lib_getDeviceLanguage(JNIEnv *env, jclass clazz)
{
    (void)clazz;
    /* <PREFIX>_LANG lets the wrapper pass the phone's language; "en" otherwise. */
    const char *lang = getenv("MC3_LANG");
    return (jobject)env->NewStringUTF(lang && *lang ? lang : "en");
}

#include "glf_gen.inc"

#define MC3_GLF_METHODS                                                         \
    M(Lib_getResource,         "getResource",             "(" STR ")[B"),       \
    M(Lib_setupPaths,          "setupPaths",              "()V"),               \
    M(Lib_createView,          "createView",              "()V"),               \
    M(Lib_setViewSettings,     "setViewSettings",         "(IIIII)V"),          \
    M(Lib_setCurrentContext,   "setCurrentContext",       "(I)Z"),              \
    M(Lib_getDeviceLanguage,   "getDeviceLanguage",       "()" STR),            \
    GLF_GEN_METHODS                                                             \
    {NULL}

#define CLS GloftGL2JNIActivity
const ManagedMethod gl2jniActivityMethods[] = { MC3_GLF_METHODS };
#undef CLS
#define CLS GloftGL2JNILib
const ManagedMethod gl2jniLibMethods[] = { MC3_GLF_METHODS };
#undef CLS

Class GloftGL2JNIActivity::clazz = {
    .classpath       = "com/gameloft/glf/GL2JNIActivity",
    .classname       = "GL2JNIActivity",
    .managed_methods = gl2jniActivityMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftGL2JNIActivity),
};

Class GloftGL2JNILib::clazz = {
    .classpath       = "com/gameloft/glf/GL2JNILib",
    .classname       = "GL2JNILib",
    .managed_methods = gl2jniLibMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftGL2JNILib),
};

/* ------------------------------------------------------------ GloftM3HP */

GloftM3HP g_game_activity;

static jobject Game_getSDFolder(JNIEnv *env, jclass c)  { (void)c; return (jobject)env->NewStringUTF(mc3_resource_dir()); }
static jobject Game_getUserAgent(JNIEnv *env, jclass c) { (void)c; return (jobject)env->NewStringUTF("Mozilla/5.0 (Linux; U; Android 2.3.6)"); }
/* SDK >= 9 on the Java side. */
static jint Game_IsAndroidOrientationAvailable(JNIEnv *e, jclass c) { (void)e; (void)c; return 1; }

/*
 * The device class. The Java answers true for useGeneration0 on one Motorola,
 * useGeneration1 on the first Galaxy S, useGeneration2 on the Nexus S - and
 * false for every phone it does not know, which is the best profile. The
 * wrapper's quality setting passes MC3_GENERATION: 0, 1 or 2 makes that one
 * answer true; anything else (3, the default) leaves all three false.
 */
static jboolean generation_is(int g)
{
    return port_getenv_long("GENERATION", 3) == g ? JNI_TRUE : JNI_FALSE;
}
static jboolean Game_useGeneration0(JNIEnv *e, jclass c) { (void)e; (void)c; return generation_is(0); }
static jboolean Game_useGeneration1(JNIEnv *e, jclass c) { (void)e; (void)c; return generation_is(1); }
static jboolean Game_useGeneration2(JNIEnv *e, jclass c) { (void)e; (void)c; return generation_is(2); }

/*
 * The GloftDRM handshake nativeMC3Init makes, under one-letter names:
 *
 *   d()   getSystemService("phone"), nothing returned
 *   da()  the activity
 *   db()  res/raw/igli.bin, 256 KB
 *   dc()  res/raw/serialkey.txt (GloftDRM.a())
 *
 * The two files come out of the game's own APK; the wrapper unpacks res/raw
 * next to the library (<game>/res/raw).
 */
static jbyteArray read_game_file(JNIEnv *env, const char *rel)
{
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", io_game_dir(), rel);
    FILE *f = fopen(path, "rb");
    if (!f) {
        trace("%s not found - answered empty", path);
        return env->NewByteArray(0);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string data((size_t)(size > 0 ? size : 0), '\0');
    size_t got = data.empty() ? 0 : fread(&data[0], 1, data.size(), f);
    fclose(f);
    jbyteArray out = env->NewByteArray((jsize)got);
    if (got)
        env->SetByteArrayRegion(out, 0, (jsize)got, (const jbyte *)data.data());
    trace("%s: %zu bytes", rel, got);
    return out;
}

static void Game_d(JNIEnv *e, jclass c)          { (void)e; (void)c; }
static jobject Game_da(JNIEnv *e, jclass c)      { (void)e; (void)c; return (jobject)&g_game_activity; }
static jbyteArray Game_db(JNIEnv *env, jclass c) { (void)c; return read_game_file(env, "res/raw/igli.bin"); }
static jbyteArray Game_dc(JNIEnv *env, jclass c) { (void)c; return read_game_file(env, "res/raw/serialkey.txt"); }

#include "game_gen.inc"

#define CLS GloftM3HP
const ManagedMethod gloftM3HPMethods[] = {
    M(Game_getSDFolder,                   "getSDFolder",                   "()" STR),
    M(Game_getUserAgent,                  "getUserAgent",                  "()" STR),
    M(Game_IsAndroidOrientationAvailable, "IsAndroidOrientationAvailable", "()I"),
    M(Game_useGeneration0,                "useGeneration0",                "()Z"),
    M(Game_useGeneration1,                "useGeneration1",                "()Z"),
    M(Game_useGeneration2,                "useGeneration2",                "()Z"),
    M(Game_d,                             "d",                             "()V"),
    M(Game_da,                            "da",                            "()Landroid/app/Activity;"),
    M(Game_db,                            "db",                            "()[B"),
    M(Game_dc,                            "dc",                            "()[B"),
    GAME_GEN_METHODS
    {NULL},
};
#undef CLS

Class GloftM3HP::clazz = {
    .classpath       = MC3_PACKAGE "/GloftM3HP",
    .classname       = "GloftM3HP",
    .managed_methods = gloftM3HPMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftM3HP),
};

/* ------------------------------------------------------------- Device */

static jobject Device_getUserAgent(JNIEnv *env, jclass c) { (void)c; return (jobject)env->NewStringUTF("Mozilla/5.0 (Linux; U; Android 2.3.6)"); }

#include "device_gen.inc"

#define CLS GloftDevice
const ManagedMethod gloftDeviceMethods[] = {
    M(Device_getUserAgent, "getUserAgent", "()" STR),
    DEVICE_GEN_METHODS
    {NULL},
};
#undef CLS

Class GloftDevice::clazz = {
    .classpath       = MC3_PACKAGE "/GLUtils/Device",
    .classname       = "Device",
    .managed_methods = gloftDeviceMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftDevice),
};

/* ------------------------------------------------------------- SUtils */

static void SUtils_release(JNIEnv *e, jclass c) { (void)e; (void)c; }

#define CLS GloftSUtils
const ManagedMethod gloftSUtilsMethods[] = {
    M(SUtils_release, "release", "()V"),
    {NULL},
};
#undef CLS

Class GloftSUtils::clazz = {
    .classpath       = MC3_PACKAGE "/GLUtils/SUtils",
    .classname       = "SUtils",
    .managed_methods = gloftSUtilsMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftSUtils),
};

/* ------------------------------------------------------ GameInstaller */

String GloftGameInstaller::mPreferencesName("GloftM3HPPrefs");
String GloftGameInstaller::SaveFolder("");
String GloftGameInstaller::sd_folder("");
String GloftGameInstaller::DATA_PATH("");

const FieldId gloftGameInstallerFields[] = {
    REGISTER_STATIC_FIELD(GloftGameInstaller, mPreferencesName),
    REGISTER_STATIC_FIELD(GloftGameInstaller, SaveFolder),
    REGISTER_STATIC_FIELD(GloftGameInstaller, sd_folder),
    REGISTER_STATIC_FIELD(GloftGameInstaller, DATA_PATH),
    {NULL},
};

Class GloftGameInstaller::clazz = {
    .classpath       = MC3_PACKAGE "/installer/GameInstaller",
    .classname       = "GameInstaller",
    .managed_methods = {NULL},
    .native_methods  = {NULL},
    .fields          = gloftGameInstallerFields,
    .instance_size   = sizeof(GloftGameInstaller),
};

/* ------------------------------------------------------ video/MyVideoView */

/*
 * Videos. The engine plays a clip (the mission briefings, data/briefing/*.mp4)
 * the Android way: new Intent(activity, MyVideoView.class), putExtra
 * ("video_name", path), putExtra("language", ...), activity.startActivity().
 * MyVideoView plays it and, when it ends or is skipped, calls the native
 * MyVideoView.SetFinished() - until then the engine waits on a black screen.
 *
 * There is no player here yet: startActivity records the clip and the frame
 * loop (mc3_video_tick) reports it finished on the next frame.
 */
class AndroidIntent : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }
    char video[512];
    char language[32];
};

static char g_video_pending[512];
static bool g_video_started;

static jobject Intent_init(JNIEnv *env, jobject self, jclass clazz, jobject ctx, jclass target)
{
    (void)env; (void)clazz; (void)ctx; (void)target;
    AndroidIntent *in = (AndroidIntent *)self;
    in->video[0] = 0;
    in->language[0] = 0;
    return self;
}

static jobject Intent_setFlags(JNIEnv *e, jobject self, jint f) { (void)e; (void)f; return self; }
static jobject Intent_addCategory(JNIEnv *e, jobject self, jstring c) { (void)e; (void)c; return self; }

static jobject Intent_putExtraStr(JNIEnv *e, jobject self, jstring key, jstring value)
{
    (void)e;
    AndroidIntent *in = (AndroidIntent *)self;
    if (strcmp(cstr(key), "video_name") == 0)
        snprintf(in->video, sizeof(in->video), "%s", cstr(value));
    else if (strcmp(cstr(key), "language") == 0)
        snprintf(in->language, sizeof(in->language), "%s", cstr(value));
    else
        trace("Intent.putExtra('%s', '%s')", cstr(key), cstr(value));
    return self;
}

static jobject Intent_putExtraInt(JNIEnv *e, jobject self, jstring key, jint value)
{
    (void)e;
    trace("Intent.putExtra('%s', %d)", cstr(key), value);
    return self;
}

static jint Intent_getIntExtra(JNIEnv *e, jobject self, jstring key, jint def) { (void)e; (void)self; (void)key; return def; }
static void Intent_removeExtra(JNIEnv *e, jobject self, jstring key) { (void)e; (void)self; (void)key; }

const ManagedMethod androidIntentMethods[] = {
    REGISTER_INIT_METHOD(AndroidIntent, Intent_init, "(Landroid/content/Context;Ljava/lang/Class;)V"),
    ManagedMethod::Register<&Intent_setFlags>(AndroidIntent::clazz, "setFlags", "(I)Landroid/content/Intent;"),
    ManagedMethod::Register<&Intent_addCategory>(AndroidIntent::clazz, "addCategory", "(" STR ")Landroid/content/Intent;"),
    ManagedMethod::Register<&Intent_putExtraStr>(AndroidIntent::clazz, "putExtra", "(" STR STR ")Landroid/content/Intent;"),
    ManagedMethod::Register<&Intent_putExtraInt>(AndroidIntent::clazz, "putExtra", "(" STR "I)Landroid/content/Intent;"),
    ManagedMethod::Register<&Intent_getIntExtra>(AndroidIntent::clazz, "getIntExtra", "(" STR "I)I"),
    ManagedMethod::Register<&Intent_removeExtra>(AndroidIntent::clazz, "removeExtra", "(" STR ")V"),
    {NULL},
};

Class AndroidIntent::clazz = {
    .classpath       = "android/content/Intent",
    .classname       = "Intent",
    .managed_methods = androidIntentMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(AndroidIntent),
};

/* android/app/Activity: what the engine calls on the activity it was given. */
class AndroidActivity : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }
};

static void Activity_startActivity(JNIEnv *e, jobject self, jobject intent)
{
    (void)e; (void)self;
    AndroidIntent *in = (AndroidIntent *)intent;
    if (!in || !in->video[0]) {
        trace("startActivity: no video in the intent - ignored");
        return;
    }
    trace("video: '%s' (language '%s') - no player, reported finished", in->video, in->language);
    snprintf(g_video_pending, sizeof(g_video_pending), "%s", in->video);
    g_video_started = true;
}

static jboolean Activity_hasWindowFocus(JNIEnv *e, jobject self) { (void)e; (void)self; return JNI_TRUE; }

const ManagedMethod androidActivityMethods[] = {
    ManagedMethod::Register<&Activity_startActivity>(AndroidActivity::clazz, "startActivity", "(Landroid/content/Intent;)V"),
    ManagedMethod::Register<&Activity_hasWindowFocus>(AndroidActivity::clazz, "hasWindowFocus", "()Z"),
    {NULL},
};

Class AndroidActivity::clazz = {
    .classpath       = "android/app/Activity",
    .classname       = "Activity",
    .managed_methods = androidActivityMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(AndroidActivity),
};

/* Once per frame from main(): a clip started on the last frame is over. */
void mc3_video_tick(_JNIEnv *env)
{
    if (!g_video_started)
        return;
    g_video_started = false;
    auto set_finished = (void (*)(JNIEnv *, jclass))so_symbol(mc3_module(), "Java_video_MyVideoView_SetFinished");
    if (set_finished) {
        set_finished((JNIEnv *)env, (jclass)&GloftVideoView::clazz);
        trace("video: MyVideoView.SetFinished() for '%s'", g_video_pending);
    }
}

#define CLS GloftVideoView
const ManagedMethod gloftVideoMethods[] = {
    {NULL},
};
#undef CLS

Class GloftVideoView::clazz = {
    .classpath       = "video/MyVideoView",
    .classname       = "MyVideoView",
    .managed_methods = gloftVideoMethods,
    .native_methods  = {NULL},
    .fields          = {NULL},
    .instance_size   = sizeof(GloftVideoView),
};

/* ----------------------------------------------------------- registry */

static const int registered[] = {
    ClassRegistry::register_class(GloftGL2JNIActivity::clazz),
    ClassRegistry::register_class(GloftGL2JNILib::clazz),
    ClassRegistry::register_class(GloftM3HP::clazz),
    ClassRegistry::register_class(GloftDevice::clazz),
    ClassRegistry::register_class(GloftSUtils::clazz),
    ClassRegistry::register_class(GloftGameInstaller::clazz),
    ClassRegistry::register_class(GloftVideoView::clazz),
    ClassRegistry::register_class(AndroidIntent::clazz),
    ClassRegistry::register_class(AndroidActivity::clazz),
};
