#include "android_utils.h"

#include <android/log.h>
#include <fmt/core.h>

void log_error(const std::string& msg) {
    __android_log_write(ANDROID_LOG_ERROR, "SimpleBLE", msg.c_str());
}
void log_info(const std::string& msg) {
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg.c_str());
}
void log_debug(const std::string& msg) {
    __android_log_write(ANDROID_LOG_DEBUG, "SimpleBLE", msg.c_str());
}

jstring to_jstring(JNIEnv* env, const std::string& str){
    return env->NewStringUTF(str.c_str());
}

std::string from_jstring(JNIEnv* env, jstring str){
    const char* c_str = env->GetStringUTFChars(str, nullptr);
    std::string result(c_str);
    env->ReleaseStringUTFChars(str, c_str);
    return result;
}

jbyteArray to_jbyteArray(JNIEnv* env, const std::string& data) {
    jbyteArray result = env->NewByteArray(data.size());
    env->SetByteArrayRegion(result, 0, data.size(), reinterpret_cast<const jbyte*>(data.data()));

    jsize length = env->GetArrayLength(result);
    jbyte* bytes = env->GetByteArrayElements(result, NULL);

    std::string arrayOut = "Array: ";
    for (jsize i = 0; i < length; i++) {
        arrayOut += fmt::format("{:02x} ", bytes[i]);
    }
    log_debug(arrayOut);

    env->ReleaseByteArrayElements(result, bytes, JNI_ABORT);


    return result;
}

jobject jarraylist_new(JNIEnv* env) {
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jobject arrayList = env->NewObject(arrayListClass, arrayListConstructor);
    return arrayList;
}

void jarraylist_add(JNIEnv* env, jobject arrayList, jobject element) {
    jclass arrayListClass = env->GetObjectClass(arrayList);
    jmethodID arrayListAdd = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    env->CallBooleanMethod(arrayList, arrayListAdd, element);
}

void throw_exception(JNIEnv* env, const std::string& msg) {
    log_error(fmt::format("Throwing exception: {}", msg));

    jclass Exception = env->FindClass("java/lang/Exception");
    env->ThrowNew(Exception, msg.c_str());
}