//
// Created by PC on 2017/7/7.
//


#include "github_com_ffmpegplayer_demo_PosixThread.h"
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"thread",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"thread",FORMAT,##__VA_ARGS__);
#define LOGW(FORMAT,...) __android_log_print(ANDROID_LOG_WARN,"thread",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"thread",FORMAT,##__VA_ARGS__);


JavaVM* javaVM;
jobject uuidutil_jclazz_global;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGI("%s","JNI_OnLoad");
    javaVM = vm;
    return JNI_VERSION_1_4;

}
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved){
    javaVM = NULL;
}



void *fun(void * arg){
    JNIEnv *mEnv = NULL;
    javaVM->AttachCurrentThread(&mEnv,NULL);

    jmethodID  get_jmethodId = mEnv->GetStaticMethodID((jclass) uuidutil_jclazz_global, "get", "()Ljava/lang/String;");







    char * va = (char *) arg;
    for (int i = 0; i <5; ++i) {

        jobject  uuid_jobj = mEnv->CallStaticObjectMethod((jclass) uuidutil_jclazz_global, get_jmethodId, NULL);

        const char * uuid_c =  mEnv->GetStringUTFChars((jstring) uuid_jobj, NULL);


        LOGI("thread:%s ,i: %d, uuid :%s",va,i,uuid_c);
        if(i == 4){
            pthread_exit((void *) 1);
        }
        sleep(1);

        mEnv->ReleaseStringUTFChars((jstring) uuid_jobj, uuid_c);
    }


    javaVM->DetachCurrentThread();

}

//JavaVM（java虚拟机） 每个程序有一个
//每个线程都有独立的JNIEnv，JNIEnv所有操作对象的工作都是从JavaVM开始的

JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_1demo_PosixThread_pthread
(JNIEnv * env, jobject jobj){


//    env->GetJavaVM(&javaVM);

    jclass uuidutil_jclazz = env->FindClass("github/com/util/UUIDUtil");
    uuidutil_jclazz_global = env->NewGlobalRef(uuidutil_jclazz);


    pthread_t tid;

    pthread_create(&tid, NULL,fun, (void *) "Thread_01");

    int re;

    pthread_join(tid, (void **) &re);


    env->DeleteGlobalRef(uuidutil_jclazz_global);

    LOGI("return value is %d",re);

}
