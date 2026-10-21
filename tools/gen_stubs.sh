#!/bin/bash
# Regenerates the quiet Java stubs (port/game/jni/*_gen.inc) from the dex listing and the library's
# strings (extracted/classes.txt, extracted/strings.txt: links to the 1.0.1 listings).
# The names after the class are the methods written by hand in gl2jni_activity.cpp.
P=$(cd "$(dirname "$0")/.." && pwd)
G=com/gameloft/android/GAND/GloftM3HP
python3 "$P/tools/gen_java_stubs.py" "Lcom/gameloft/glf/GL2JNILib;" Glf CLS \
  getResource setupPaths createView setViewSettings setCurrentContext getDeviceLanguage \
  > "$P/port/game/jni/glf_gen.inc"
python3 "$P/tools/gen_java_stubs.py" "L$G/GloftM3HP;" Game GloftM3HP \
  getSDFolder getUserAgent IsAndroidOrientationAvailable onKeyDown onKeyUp \
  > "$P/port/game/jni/game_gen.inc"
python3 "$P/tools/gen_java_stubs.py" "L$G/GLUtils/Device;" Device GloftDevice \
  getUserAgent \
  > "$P/port/game/jni/device_gen.inc"
