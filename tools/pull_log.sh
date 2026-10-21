#!/bin/sh
# Copies the phone's game log to logs/phone.log
P=${MC3_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}
A="$P/tools/adb"
D=/sdcard/BridgeGames/com.jamescrumble.mc3
[ -n "$($A shell ls $D/mc3.log 2>/dev/null)" ] || D=/sdcard/Android/data/com.jamescrumble.mc3/files
$A shell cat $D/mc3.log | tr -d '\r' > "$P/logs/phone.log"
wc -l "$P/logs/phone.log"
