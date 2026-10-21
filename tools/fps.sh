#!/bin/sh
# Waits $1 s (default 0), then prints the last $2 (default 4) bridge stat lines and phase lines.
P=${MC3_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}
A=$P/tools/adb
D=/sdcard/BridgeGames/com.jamescrumble.mc3
[ -n "$($A shell ls $D/mc3.log 2>/dev/null)" ] || D=/sdcard/Android/data/com.jamescrumble.mc3/files
F=$D/mc3.log
[ "${1:-0}" -gt 0 ] && sleep "$1"
$A shell "grep -E 'frames [0-9.]+/s|phase=|qemu exited|died' $F | tail -${2:-4}"
