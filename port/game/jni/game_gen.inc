static void Game_gen_0(JNIEnv *env, jclass c) { (void)env; GEN_SAID("GameLaunchTracking()V");  }
static jint Game_gen_1(JNIEnv *env, jclass c) { (void)env; GEN_SAID("IsScreenLock()I"); return 0; }
static jint Game_gen_2(JNIEnv *env, jclass c) { (void)env; GEN_SAID("IsWifiEnabled()I"); return 0; }
static jint Game_gen_3(JNIEnv *env, jclass c) { (void)env; GEN_SAID("IsWifiOr3GEnabled()I"); return 0; }
static void Game_gen_4(JNIEnv *env, jclass c, jint a0) { (void)env; GEN_SAID("SetAndroidOrientation(I)V");  }
static void Game_gen_5(JNIEnv *env, jclass c, jstring a0, jstring a1, jstring a2) { (void)env; GEN_SAID("ShowAlert(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");  }
static jobject Game_gen_6(JNIEnv *env, jclass c) { (void)env; GEN_SAID("getIMEI()Ljava/lang/String;"); return (jobject)env->NewStringUTF(""); }
static jboolean Game_gen_7(JNIEnv *env, jclass c) { (void)env; GEN_SAID("hasGyroscope()Z"); return 0; }
static void Game_gen_8(JNIEnv *env, jclass c) { (void)env; GEN_SAID("hideloading()V");  }
static jboolean Game_gen_9(JNIEnv *env, jclass c) { (void)env; GEN_SAID("ignoreGyroLowInc()Z"); return 0; }
static jboolean Game_gen_10(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isGyroInvertedX()Z"); return 0; }
static jboolean Game_gen_11(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isGyroPendingToFix()Z"); return 0; }
static jboolean Game_gen_12(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isGyroSensibilityHigh()Z"); return 0; }
static jboolean Game_gen_13(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isGyroSensibilityLow()Z"); return 0; }
static jboolean Game_gen_14(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isHTCDevice()Z"); return 0; }
static jboolean Game_gen_15(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isHoneycombDevice()Z"); return 0; }
static jboolean Game_gen_16(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isSamsung_I9250()Z"); return 0; }
static jboolean Game_gen_17(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isSamsung_P7100()Z"); return 0; }
static jboolean Game_gen_18(JNIEnv *env, jclass c) { (void)env; GEN_SAID("isXperiaPlay()Z"); return 0; }
static void Game_gen_19(JNIEnv *env, jclass c) { (void)env; GEN_SAID("moveToBackground()V");  }
static void Game_gen_20(JNIEnv *env, jclass c, jint a0) { (void)env; GEN_SAID("showloading(I)V");  }
static jboolean Game_gen_21(JNIEnv *env, jclass c) { (void)env; GEN_SAID("useGeneration0()Z"); return 0; }
static jboolean Game_gen_22(JNIEnv *env, jclass c) { (void)env; GEN_SAID("useGeneration1()Z"); return 0; }
static jboolean Game_gen_23(JNIEnv *env, jclass c) { (void)env; GEN_SAID("useGeneration2()Z"); return 0; }
#define GAME_GEN_METHODS \
    ManagedMethod::RegisterStatic<&Game_gen_0>(GloftM3HP::clazz, "GameLaunchTracking", "()V"), \
    ManagedMethod::RegisterStatic<&Game_gen_1>(GloftM3HP::clazz, "IsScreenLock", "()I"), \
    ManagedMethod::RegisterStatic<&Game_gen_2>(GloftM3HP::clazz, "IsWifiEnabled", "()I"), \
    ManagedMethod::RegisterStatic<&Game_gen_3>(GloftM3HP::clazz, "IsWifiOr3GEnabled", "()I"), \
    ManagedMethod::RegisterStatic<&Game_gen_4>(GloftM3HP::clazz, "SetAndroidOrientation", "(I)V"), \
    ManagedMethod::RegisterStatic<&Game_gen_5>(GloftM3HP::clazz, "ShowAlert", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"), \
    ManagedMethod::RegisterStatic<&Game_gen_6>(GloftM3HP::clazz, "getIMEI", "()Ljava/lang/String;"), \
    ManagedMethod::RegisterStatic<&Game_gen_7>(GloftM3HP::clazz, "hasGyroscope", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_8>(GloftM3HP::clazz, "hideloading", "()V"), \
    ManagedMethod::RegisterStatic<&Game_gen_9>(GloftM3HP::clazz, "ignoreGyroLowInc", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_10>(GloftM3HP::clazz, "isGyroInvertedX", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_11>(GloftM3HP::clazz, "isGyroPendingToFix", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_12>(GloftM3HP::clazz, "isGyroSensibilityHigh", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_13>(GloftM3HP::clazz, "isGyroSensibilityLow", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_14>(GloftM3HP::clazz, "isHTCDevice", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_15>(GloftM3HP::clazz, "isHoneycombDevice", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_16>(GloftM3HP::clazz, "isSamsung_I9250", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_17>(GloftM3HP::clazz, "isSamsung_P7100", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_18>(GloftM3HP::clazz, "isXperiaPlay", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_19>(GloftM3HP::clazz, "moveToBackground", "()V"), \
    ManagedMethod::RegisterStatic<&Game_gen_20>(GloftM3HP::clazz, "showloading", "(I)V"), \
    ManagedMethod::RegisterStatic<&Game_gen_21>(GloftM3HP::clazz, "useGeneration0", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_22>(GloftM3HP::clazz, "useGeneration1", "()Z"), \
    ManagedMethod::RegisterStatic<&Game_gen_23>(GloftM3HP::clazz, "useGeneration2", "()Z"), \
    /* end */
