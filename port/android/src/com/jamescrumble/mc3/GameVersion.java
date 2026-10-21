package com.jamescrumble.mc3;

import android.content.Context;
import android.content.pm.PackageInfo;

import java.io.File;

/**
 * Which build of the game the player has put in the folder. The version is the APK's own versionName
 * (the library inside is what runs). The port is made for 1.0.1 (com.gameloft.android.GAND.GloftM3HP);
 * other builds use another package, other entry points and another cache, and are only named here.
 */
final class GameVersion {
    /** The APK found in the game folder, or null. */
    final File apk;
    /** versionName from the APK, or null when it could not be read. */
    final String versionName;
    /** The APK's package, or null. */
    final String packageName;

    private GameVersion(File apk, String versionName, String packageName) {
        this.apk = apk;
        this.versionName = versionName;
        this.packageName = packageName;
    }

    static GameVersion detect(Context c) {
        File apk = Storage.firstWithSuffix(c, ".apk");
        String name = null, pkg = null;
        if (apk != null) {
            PackageInfo info = c.getPackageManager().getPackageArchiveInfo(apk.getPath(), 0);
            if (info != null) {
                name = info.versionName;
                pkg = info.packageName;
            }
        }
        return new GameVersion(apk, name, pkg);
    }

    boolean supported() {
        return "com.gameloft.android.GAND.GloftM3HP".equals(packageName);
    }

    /** The folder (inside game/) the engine keeps its profile in. */
    String saveFolder() {
        return "saves";
    }

    /** The line for the settings screen, or null when there is no APK to speak about. */
    String notice() {
        if (apk == null) return null;
        if (supported()) return "Обнаружена " + (versionName != null ? versionName : "1.0.1") + " — версия, для которой сделан порт.";
        return "Версия игры" + (versionName != null ? " " + versionName : "") + " (" + packageName + ") не поддерживается. "
                + "Порт сделан для 1.0.1 (com.gameloft.android.GAND.GloftM3HP).";
    }
}
