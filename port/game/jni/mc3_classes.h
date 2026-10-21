#pragma once

#include "jni.h"
#include "jni_internals.h"

/*
 * The Java classes this game asks for (Modern Combat 3 1.0.1, package
 * com.gameloft.android.GAND.GloftM3HP).
 *
 * The glf callbacks are looked up on the class GL2JNILib.init() is handed; the
 * game's own on <package>/GloftM3HP (the activity, static methods), plus
 * <package>/GLUtils/Device and android/media/AudioTrack.
 *
 *   gl2jni_activity.cpp  GL2JNIActivity, GL2JNILib, GloftM3HP, Device, SUtils,
 *                        GameInstaller, video/MyVideoView
 *   audio_track.cpp      android/media/AudioTrack (the game's only audio output)
 *   android_app.cpp      Context, Bundle, Process and the device-id helpers
 *
 * Two rules run through all of them:
 *
 *   - Never return NULL to the engine. It stores what it gets and dereferences
 *     it later.
 *   - Method lookup is an exact strcmp on name AND descriptor. The descriptors
 *     here are the ones the game's own classes.dex declares.
 */

#define MC3_CLASS(name)                       \
    class name : public Object {                \
    public:                                     \
        static Class clazz;                     \
        Class *_getClass() { return &clazz; }   \
    }

/* com/gameloft/glf/GL2JNIActivity and com/gameloft/glf/GL2JNILib. Both carry
 * the same method table. */
MC3_CLASS(GloftGL2JNIActivity);
MC3_CLASS(GloftGL2JNILib);

/* <package>/GloftM3HP - the game's activity. Its callbacks are static methods;
 * GloftM3HP.Init() is an instance method and is given g_game_activity. */
MC3_CLASS(GloftM3HP);

/* <package>/GLUtils/Device, /GLUtils/SUtils, video/MyVideoView */
MC3_CLASS(GloftDevice);
MC3_CLASS(GloftSUtils);
MC3_CLASS(GloftVideoView);

MC3_CLASS(AndroidBundle);
MC3_CLASS(AndroidProcess);
MC3_CLASS(AndroidSystemProperties);
MC3_CLASS(AndroidSettingsSecure);
MC3_CLASS(AndroidTelephonyManager);
MC3_CLASS(AndroidContentResolver);
MC3_CLASS(AndroidViewRoot);
MC3_CLASS(JavaUuid);
MC3_CLASS(JavaObject);

#undef MC3_CLASS

/*
 * The classes the game reads a *field* off, rather than calling.
 *
 * These cannot use the macro above because a field has to exist as a real
 * member to have an address: REGISTER_STATIC_FIELD records &Class::member, and
 * the fake JVM hands the engine that address directly.
 */
class AndroidContext : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }
};

class AndroidBuild : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }

    /* Read by the device-id fallback chain (be9ca0/be9cb4). */
    static String SERIAL;
};

class AndroidBuildVersion : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }

    /* Named at be9c30/be9c4c. Declared, deliberately not registered - see the
     * comment on its Class in android_app.cpp. */
    static int SDK_INT;
};

/*
 * <package>/installer/GameInstaller. The engine reads the name of the
 * preferences file off it (mPreferencesName, a static String) and then asks
 * SUtils.getPreferenceString("SDFolder", <that name>) for the data directory.
 */
class GloftGameInstaller : public Object {
public:
    static Class clazz;
    Class *_getClass() { return &clazz; }

    static String mPreferencesName;
    static String SaveFolder;
    static String sd_folder;
    static String DATA_PATH;
};

/*
 * The singletons the engine is handed.
 *
 * On Android these are distinct objects with state; here they exist so that a
 * method looked up on what getContentResolver() returned resolves to
 * something. They are deliberately global rather than allocated per call: the
 * engine keeps the pointers it is given for the life of the process and
 * compares some of them, and a fresh object per call would break that
 * silently.
 */
extern GloftGL2JNIActivity     g_activity;
extern GloftM3HP               g_game_activity;
extern AndroidTelephonyManager g_telephony_manager;
extern AndroidContentResolver  g_content_resolver;
extern AndroidViewRoot         g_view_root;
