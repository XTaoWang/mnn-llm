#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>

#include "llm.hpp"

static std::unique_ptr<Llm> llm(nullptr);
static std::stringstream response_buffer;

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}

JNIEXPORT jboolean JNICALL Java_com_jdo_imageocr_mnn_Chat_Init(JNIEnv* env, jobject thiz, jstring modelDir) {
    const char* model_dir = env->GetStringUTFChars(modelDir, 0);
    if (!llm.get()) {
        llm.reset(Llm::createLLM(model_dir));
        llm->load();
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_jdo_imageocr_mnn_Chat_Ready(JNIEnv* env, jobject thiz) {
    if (llm.get()) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_com_jdo_imageocr_mnn_Chat_Submit(JNIEnv* env, jobject thiz, jstring inputStr) {
    if (!llm.get()) {
        return env->NewStringUTF("Failed, Chat is not ready!");
    }
    const char* input_str = env->GetStringUTFChars(inputStr, 0);
    auto chat = [&](std::string str) {
        llm->response(str, &response_buffer, "<eop>");
    };
    std::thread chat_thread(chat, input_str);
    chat_thread.detach();
    jstring result = env->NewStringUTF("Submit success!");
    return result;
}

JNIEXPORT jbyteArray JNICALL Java_com_jdo_imageocr_mnn_Chat_Response(JNIEnv* env, jobject thiz) {
    auto len = response_buffer.str().size();
    jbyteArray res = env->NewByteArray(len);
    env->SetByteArrayRegion(res, 0, len, (const jbyte*)response_buffer.str().c_str());
    return res;
}

JNIEXPORT void JNICALL Java_com_jdo_imageocr_mnn_Chat_Done(JNIEnv* env, jobject thiz) {
    response_buffer.str("");
}

JNIEXPORT void JNICALL Java_com_jdo_imageocr_mnn_Chat_Reset(JNIEnv* env, jobject thiz) {
    llm->reset();
}

JNIEXPORT void JNICALL Java_com_jdo_imageocr_mnn_Chat_Stop(JNIEnv* env, jobject thiz) {
    llm->stop();
}


JNIEXPORT void JNICALL Java_com_jdo_imageocr_mnn_Chat_Print(JNIEnv* env, jobject thiz) {
    llm->print_speed();
}

JNIEXPORT jstring JNICALL Java_com_jdo_imageocr_mnn_Chat_Prefill(JNIEnv* env, jobject thiz) {
    if (!llm) {
        return env->NewStringUTF("Llm instance is not initialized.");
    }
    std::string result = llm->get_prefill_speed();
    return env->NewStringUTF(result.c_str()); // 将 std::string 转换为 jstring 返回
}

JNIEXPORT jstring JNICALL Java_com_jdo_imageocr_mnn_Chat_Decode(JNIEnv* env, jobject thiz) {
    if (!llm) {
        return env->NewStringUTF("Llm instance is not initialized.");
    }
    std::string result = llm->get_decode_speed();
    return env->NewStringUTF(result.c_str()); // 将 std::string 转换为 jstring 返回
}


} // extern "C"