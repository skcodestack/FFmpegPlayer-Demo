/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class github_com_ffmpegplayer_util_VideoUtil */

#ifndef _Included_github_com_ffmpegplayer_util_VideoUtil
#define _Included_github_com_ffmpegplayer_util_VideoUtil
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     github_com_ffmpegplayer_util_VideoUtil
 * Method:    decode
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_decode
  (JNIEnv *, jclass, jstring, jstring);
/*
 * Class:     github_com_ffmpegplayer_util_VideoUtil
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_render
        (JNIEnv *, jclass,jstring,jobject);

JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_audioDecode
        (JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_play
        (JNIEnv *, jclass, jstring,jobject);

#ifdef __cplusplus
}
#endif
#endif
