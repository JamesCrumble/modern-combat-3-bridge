static jobject Device_gen_0(JNIEnv *env, jclass c) { (void)env; GEN_SAID("getDeviceId()Ljava/lang/String;"); return (jobject)env->NewStringUTF(""); }
static jobject Device_gen_1(JNIEnv *env, jclass c) { (void)env; GEN_SAID("getIMEI()Ljava/lang/String;"); return (jobject)env->NewStringUTF(""); }
#define DEVICE_GEN_METHODS \
    ManagedMethod::RegisterStatic<&Device_gen_0>(GloftDevice::clazz, "getDeviceId", "()Ljava/lang/String;"), \
    ManagedMethod::RegisterStatic<&Device_gen_1>(GloftDevice::clazz, "getIMEI", "()Ljava/lang/String;"), \
    /* end */
