#include <simplecble/android.h>

#if defined(__ANDROID__)
#include <jni.h>
#include <simpleble/Advanced.h>
#endif

void simpleble_android_set_jvm(void* java_vm) {
#if defined(__ANDROID__)
    SimpleBLE::Advanced::Android::set_jvm(static_cast<JavaVM*>(java_vm));
#else
    (void)java_vm;
#endif
}
