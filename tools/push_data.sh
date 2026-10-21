#!/bin/sh
# Copies the player's game files to the phone: the game's APK (1.0.1 under gamebin/) and the unpacked
# cache (mc3data/v101/files -> <app folder>/com.gameloft.android.GAND.GloftM3HP/files). The folders are made
# first and opened to the app (2777): what adb creates belongs to the shell user and is closed to the app otherwise.
# About 1.7 GB. Usage: tools/push_data.sh
P=${MC3_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}
A=$P/tools/adb
D=/sdcard/Android/data/com.jamescrumble.mc3/files
G=com.gameloft.android.GAND.GloftM3HP
APK=$(find "$P"/gamebin -name '*1.0.1*.apk' 2>/dev/null | head -1)
[ -n "$APK" ] || { echo "no 1.0.1 game APK under gamebin/"; exit 1; }
C=$P/mc3data/v101/files
[ -f "$C/data/DATA_1" ] || { echo "no cache in $C"; exit 1; }
open_dirs() { $A shell "chmod 2777 ${D%/files} $D; find $D/$G -type d -exec chmod 2777 {} +"; }
$A shell mkdir -p "$D/$G" || exit 1
open_dirs
$A push "$(wslpath -w "$APK")" "$D/mc3-1.0.1.apk" || exit 1
$A push "$(wslpath -w "$C")" "$D/$G/" || exit 1
# Again after the copy: every folder adb made belongs to the shell user.
open_dirs
$A shell "ls -la $D $D/$G/files | head -30; du -sh $D"
