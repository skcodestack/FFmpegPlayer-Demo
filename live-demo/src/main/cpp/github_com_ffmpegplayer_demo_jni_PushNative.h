/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class github_com_ffmpegplayer_demo_jni_PushNative */

#ifndef _Included_github_com_ffmpegplayer_demo_jni_PushNative
#define _Included_github_com_ffmpegplayer_demo_jni_PushNative
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    startPush
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_startPush
  (JNIEnv *, jobject, jstring);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    stopPush
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_stopPush
  (JNIEnv *, jobject);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_release
  (JNIEnv *, jobject);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    setVideoOptions
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_setVideoOptions
  (JNIEnv *, jobject, jint, jint, jint, jint);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    setAudioOptions
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_setAudioOptions
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    fireVideo
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_fireVideo
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     github_com_ffmpegplayer_demo_jni_PushNative
 * Method:    fireAudio
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_jni_PushNative_fireAudio
  (JNIEnv *, jobject, jbyteArray, jint);

#ifdef __cplusplus
}
#endif
#endif
