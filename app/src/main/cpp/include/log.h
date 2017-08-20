//
// Created by PC on 2017/7/14.
//

#ifndef FFMPEGPLAYER_DEMO_LOG_H
#define FFMPEGPLAYER_DEMO_LOG_H
#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGW(FORMAT,...) __android_log_print(ANDROID_LOG_WARN,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"ffmpeg",FORMAT,##__VA_ARGS__);
#endif //FFMPEGPLAYER_DEMO_LOG_H
