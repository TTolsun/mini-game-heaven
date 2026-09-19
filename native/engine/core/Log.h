#pragma once

#include <android/log.h>

// Thin logging macros so engine code does not spell out __android_log_print.
// Tag is per translation unit: define LOG_TAG before including this header.
#ifndef LOG_TAG
#define LOG_TAG "Wuten"
#endif

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
