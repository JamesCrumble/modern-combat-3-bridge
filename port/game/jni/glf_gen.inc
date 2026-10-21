static void Glf_gen_0(JNIEnv *env, jclass c, jboolean a0, jfloat a1) { (void)env; GEN_SAID("enableAccelerometer(ZF)V");  }
static void Glf_gen_1(JNIEnv *env, jclass c, jboolean a0, jfloat a1) { (void)env; GEN_SAID("enableGyroscope(ZF)V");  }
static void Glf_gen_2(JNIEnv *env, jclass c, jint a0) { (void)env; GEN_SAID("notifyTrophy(I)V");  }
static void Glf_gen_3(JNIEnv *env, jclass c, jstring a0) { (void)env; GEN_SAID("openBrowser(Ljava/lang/String;)V");  }
static void Glf_gen_4(JNIEnv *env, jclass c, jint a0) { (void)env; GEN_SAID("openGLLive(I)V");  }
static void Glf_gen_5(JNIEnv *env, jclass c, jint a0) { (void)env; GEN_SAID("openIGP(I)V");  }
#define GLF_GEN_METHODS \
    ManagedMethod::RegisterStatic<&Glf_gen_0>(CLS::clazz, "enableAccelerometer", "(ZF)V"), \
    ManagedMethod::RegisterStatic<&Glf_gen_1>(CLS::clazz, "enableGyroscope", "(ZF)V"), \
    ManagedMethod::RegisterStatic<&Glf_gen_2>(CLS::clazz, "notifyTrophy", "(I)V"), \
    ManagedMethod::RegisterStatic<&Glf_gen_3>(CLS::clazz, "openBrowser", "(Ljava/lang/String;)V"), \
    ManagedMethod::RegisterStatic<&Glf_gen_4>(CLS::clazz, "openGLLive", "(I)V"), \
    ManagedMethod::RegisterStatic<&Glf_gen_5>(CLS::clazz, "openIGP", "(I)V"), \
    /* end */
