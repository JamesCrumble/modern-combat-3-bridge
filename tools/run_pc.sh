#!/bin/bash
# Runs the port on this PC under the system qemu-arm with software GL (llvmpipe), no window.
# Usage: tools/run_pc.sh [frames]      (default 600; log in logs/pc_run.log)
# A screenshot while it runs:  echo "screenshot NAME" >> ctl/commands   -> ctl/screenshots/NAME.png
P=${MC3_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}
mkdir -p "$P/ctl" "$P/logs"
: > "$P/ctl/commands"
cd "$P/port" || exit 1
LOADER_TRACE=1 MC3_CONTROL_DIR="$P/ctl" SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=1 EGL_PLATFORM=surfaceless \
SDL_AUDIODRIVER=dummy MC3_WIDTH=${MC3_WIDTH:-960} MC3_HEIGHT=${MC3_HEIGHT:-540} MC3_AUTOPILOT=${MC3_AUTOPILOT:-0} \
MC3_FRAME_LIMIT="${1:-600}" \
    qemu-arm -L android/build/sysroot build/mc3 "${MC3_GAMEDIR:-$P/mc3data/v101}" > "$P/logs/pc_run.log" 2>&1
echo "exit code $?"
grep -E "summary|run finished|FATAL|unresolved" "$P/logs/pc_run.log"
