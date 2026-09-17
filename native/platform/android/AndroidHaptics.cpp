#define LOG_TAG "AndroidHaptics"
#include "platform/android/AndroidHaptics.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "engine/core/Log.h"

namespace platform::android {

namespace {

// Attaches the calling thread if needed. android_main runs on its own thread
// (native_app_glue does not attach it), so the JavaVM must be attached before
// any JNI call. Sets *attached when this call did the attaching; the thread
// must be detached before it exits or ART aborts the process.
JNIEnv* attach(android_app* app, bool* attached) {
    JNIEnv* env = nullptr;
    JavaVM* vm = app->activity->vm;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return nullptr;
        }
        if (attached != nullptr) {
            *attached = true;
        }
    }
    return env;
}

bool clearException(JNIEnv* env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return true;
    }
    return false;
}

}  // namespace

AndroidHaptics::AndroidHaptics(android_app* app) : app_(app) {}

AndroidHaptics::~AndroidHaptics() {
    JNIEnv* env = attach(app_, &attachedThread_);
    if (env != nullptr) {
        if (vibrator_ != nullptr) {
            env->DeleteGlobalRef(vibrator_);
        }
        if (effectClass_ != nullptr) {
            env->DeleteGlobalRef(effectClass_);
        }
    }
    if (attachedThread_) {
        app_->activity->vm->DetachCurrentThread();
        attachedThread_ = false;
    }
}

bool AndroidHaptics::resolve(JNIEnv* env) {
    resolved_ = true;
    jobject activity = app_->activity->javaGameActivity;

    // vibrator = activity.getSystemService("vibrator")
    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getSystemService =
        env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring serviceName = env->NewStringUTF("vibrator");
    jobject vibrator = env->CallObjectMethod(activity, getSystemService, serviceName);
    env->DeleteLocalRef(serviceName);
    env->DeleteLocalRef(contextClass);
    if (clearException(env) || vibrator == nullptr) {
        LOGW("no vibrator service");
        return false;
    }

    jclass vibratorClass = env->GetObjectClass(vibrator);
    jmethodID hasVibrator = env->GetMethodID(vibratorClass, "hasVibrator", "()Z");
    const bool has = env->CallBooleanMethod(vibrator, hasVibrator) == JNI_TRUE;
    vibrateEffect_ = env->GetMethodID(vibratorClass, "vibrate", "(Landroid/os/VibrationEffect;)V");
    env->DeleteLocalRef(vibratorClass);
    if (clearException(env) || !has || vibrateEffect_ == nullptr) {
        env->DeleteLocalRef(vibrator);
        LOGW("device has no vibrator");
        return false;
    }

    jclass effectClass = env->FindClass("android/os/VibrationEffect");
    createOneShot_ = env->GetStaticMethodID(effectClass, "createOneShot", "(JI)Landroid/os/VibrationEffect;");
    if (clearException(env) || createOneShot_ == nullptr) {
        env->DeleteLocalRef(vibrator);
        env->DeleteLocalRef(effectClass);
        return false;
    }

    vibrator_ = env->NewGlobalRef(vibrator);
    effectClass_ = static_cast<jclass>(env->NewGlobalRef(effectClass));
    env->DeleteLocalRef(vibrator);
    env->DeleteLocalRef(effectClass);
    LOGI("vibrator ready");
    return true;
}

void AndroidHaptics::vibrate(Strength strength) {
    JNIEnv* env = attach(app_, &attachedThread_);
    if (env == nullptr) {
        return;
    }
    if (!resolved_) {
        available_ = resolve(env);
    }
    if (!available_) {
        return;
    }

    // Duration and amplitude per tier; short and crisp so it reads as a tap, not a buzz.
    jlong millis = 12;
    jint amplitude = 90;
    switch (strength) {
    case Strength::Light:
        millis = 10;
        amplitude = 70;
        break;
    case Strength::Medium:
        millis = 22;
        amplitude = 150;
        break;
    case Strength::Heavy:
        millis = 45;
        amplitude = 255;
        break;
    }

    jobject effect = env->CallStaticObjectMethod(effectClass_, createOneShot_, millis, amplitude);
    if (!clearException(env) && effect != nullptr) {
        env->CallVoidMethod(vibrator_, vibrateEffect_, effect);
        clearException(env);
    }
    if (effect != nullptr) {
        env->DeleteLocalRef(effect);
    }
}

}  // namespace platform::android
