#include <jni.h>

#include "github_com_ffmpegplayer_util_VideoUtil.h"


extern  "C"{

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

//重采样
#include "libswresample/swresample.h"


};

#include "libyuv.h"
#include "include/player_queue.h"
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

using namespace libyuv;


#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGW(FORMAT,...) __android_log_print(ANDROID_LOG_WARN,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"ffmpeg",FORMAT,##__VA_ARGS__);

#define QUEUE_SIZE 50
#define STREAM_MAX 2
typedef struct _Player Player;
#define MAX_AUDIO_FRME_SIZE 48000 * 4
#define MIN_SLEEP_TIME_US 1000ll
#define AUDIO_TIME_ADJUST_US -200000ll



struct  _Player{

    AVFormatContext *av_formate_context;
    int  video_stream_index;
    int  audio_stream_index ;

    AVCodecContext * av_codec_context[STREAM_MAX];

    //video
    ANativeWindow* nativeWindow;
    AVFrame *vFrame;
    AVFrame *yuvFrame;
    AVFrame *argeFrame;
    uint8_t *out_buffer;
    struct SwsContext *swsContext;
    ANativeWindow_Buffer outBuffer;

    //audio
    AVFrame *aFrame;
    struct SwrContext *swrContext;
    uint8_t *a_out_buffer;
    int out_channel_nb;
    AVSampleFormat out_sample_fmt;

    pthread_t decode_threads[STREAM_MAX];

    jobject  jAudioTrack_global;
    jmethodID  write_mid;

    //生产者  消费者
    pthread_t pthread_read_from_stream;
    //生产者  消费者 队列
    Queue * audio_queue;
    Queue * video_queue;

    //互斥锁 条件变量
    pthread_mutex_t pthread_mutex;
    pthread_cond_t pthread_cond;

    //播放的开始时间
    int64_t start_time;


    int64_t audio_clock;
};


JavaVM* javaVM;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGI("%s","JNI_OnLoad");
    javaVM = vm;
    return JNI_VERSION_1_4;

}
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved){
    javaVM = NULL;
}



/**
 * 获取视频当前播放时间
 */
int64_t player_get_current_video_time(Player *player) {
    int64_t current_time = av_gettime();
    LOGE("av_gettime is  %d",current_time);
    return current_time - player->start_time;
}

/**
 * 延迟
 */
void player_wait_for_frame(Player *player, int64_t stream_time,
                           int stream_no) {
    pthread_mutex_lock(&player->pthread_mutex);
    for(;;){
        int64_t current_video_time = player_get_current_video_time(player);
        int64_t sleep_time = stream_time - current_video_time;

//        LOGE("stream_time is %l,current_video_time is %l  ,sleep_time %l",stream_time,current_video_time,sleep_time);
        if (sleep_time < -300000ll) {
            // 300 ms late
            int64_t new_value = player->start_time - sleep_time;
            LOGI("player_wait_for_frame[%d] correcting %f to %f because late",
                 stream_no, (av_gettime() - player->start_time) / 1000000.0,
                 (av_gettime() - new_value) / 1000000.0);

            player->start_time = new_value;
            pthread_cond_broadcast(&player->pthread_cond);
        }

        if (sleep_time <= MIN_SLEEP_TIME_US) {
            // We do not need to wait if time is slower then minimal sleep time
            LOGI("this is break ");
            break;
        }

        if (sleep_time > 500000ll) {
            // if sleep time is bigger then 500ms just sleep this 500ms
            // and check everything again
            sleep_time = 500000ll;
        }


        //等待指定时长
        int timeout_ret = pthread_cond_timeout_np(&player->pthread_cond,
                                                  &player->pthread_mutex, sleep_time/1000ll);

        // just go further
        LOGI("player_wait_for_frame[%d] finish", stream_no);
    }
    LOGI("this is end ");
    pthread_mutex_unlock(&player->pthread_mutex);
}





int init_format_context(Player *player,const char * input_cstr){


    //1.注册组件
    av_register_all();
    //2.打开文件

    //2.1  封装格式上下文
    AVFormatContext *ps = avformat_alloc_context();

    //int avformat_open_input(AVFormatContext **ps, const char *url,AVInputFormat *fmt, AVDictionary **options);
    if(avformat_open_input(&ps,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入视频文件失败");
        return -1;
    }
    //3.获取视频文件信息
    if(avformat_find_stream_info(ps,NULL) < 0){
        LOGE("%s","获取视频文件信息失败");
        return -1;
    }
    //4.查找解码器
    //4.1找到视频（AVSTREAM）
    int num_streams = ps->nb_streams;
    int  video_stream_index = -1;
    int  audio_stream_index = -1;
    for (int i = 0; i < num_streams; ++i) {
        if(ps->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
        } else if(ps->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
        }
    }
    if(video_stream_index == -1){
        LOGE("%s","没有视频文件");
        return -1;
    }
    if(audio_stream_index == -1){
        LOGE("%s","没有音频文件文件");
        return -1;
    }

    player->av_formate_context = ps;
    player->audio_stream_index=audio_stream_index;
    player->video_stream_index = video_stream_index;



    return 0 ;
}



int init_codec_context(Player *player,int  stream_index){

    AVFormatContext *ps=  player->av_formate_context;

    //4.2  拿到解码器
    AVCodecContext *  pCodecContext = ps->streams[stream_index]->codec;
    AVCodec * pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if(pCodec == NULL){
        LOGE("%s","找不到解码器");
        return -1;
    }

    //5.打开解码器
    if(avcodec_open2(pCodecContext,pCodec,NULL) < 0){
        LOGE("%s","解码器打开失败");
        return -1;
    }
    player->av_codec_context[stream_index]  = pCodecContext;

    return  0;
}




void  decode_video_prepare(Player *player,JNIEnv* env,jobject surface){

    AVCodecContext * pCodecContext = player->av_codec_context[player->video_stream_index];

    //像素数据
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height);



    //用于像素格式转换或者缩放
    struct SwsContext *swsContext = sws_getContext(pCodecContext->width,pCodecContext->height,pCodecContext->pix_fmt
            ,pCodecContext->width,pCodecContext->height, AV_PIX_FMT_YUV420P
            ,SWS_BILINEAR,NULL,NULL,NULL);

    //获取native window
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env,surface);

    AVFrame *argeFrame = av_frame_alloc();
    //argb缓冲区
    ANativeWindow_Buffer outBuffer;

    player->nativeWindow = nativeWindow;
    player->vFrame = pFrame;
    player->yuvFrame = yuvFrame;
    player->out_buffer=out_buffer;
    player->swsContext = swsContext;
    player->argeFrame = argeFrame;
    player->outBuffer = outBuffer;

}






void  decodeVideo(Player *player,AVPacket *pPacket){


    AVCodecContext *pCodecContext = player->av_codec_context[player->video_stream_index];
    AVFrame * pFrame = player->vFrame;

    AVFormatContext * avFormatContext =  player->av_formate_context;
    AVStream * avStream = avFormatContext->streams[player->video_stream_index];


    ANativeWindow* nativeWindow = player->nativeWindow;
    AVFrame *yuvFrame = player ->yuvFrame;
    AVFrame *argeFrame = player->argeFrame;
    uint8_t *out_buffer = player->out_buffer;
    struct SwsContext *swsContext = player->swsContext;
    ANativeWindow_Buffer outBuffer = player->outBuffer;


    int got_picture_ptr, len;
    int cur_frame = 1;


    //7.获取像素数据
    len = avcodec_decode_video2(pCodecContext, pFrame, &got_picture_ptr, pPacket);

    if (got_picture_ptr) {

        //1.锁定屏幕
        //设置缓冲区的格式大小
        ANativeWindow_setBuffersGeometry(nativeWindow,
                                         pCodecContext->width, pCodecContext->height,
                                         WINDOW_FORMAT_RGBA_8888);

        ANativeWindow_lock(nativeWindow, &outBuffer,
                           NULL);

        //初始化缓冲区
        avpicture_fill((AVPicture *) argeFrame, (const uint8_t *) outBuffer.bits,
                       AV_PIX_FMT_ARGB, pCodecContext->width, pCodecContext->height);

        //转为指定的YUV420P像素帧(缩放)
        sws_scale(swsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                  pFrame->height,
                  yuvFrame->data, yuvFrame->linesize);


        //Yuv转  RGBA
        I420ToARGB(yuvFrame->data[0], yuvFrame->linesize[0],
                   yuvFrame->data[2], yuvFrame->linesize[2],
                   yuvFrame->data[1], yuvFrame->linesize[1],
                   argeFrame->data[0], argeFrame->linesize[0],
                   pCodecContext->width, pCodecContext->height);


        //计算延迟
//        int64_t pts = av_frame_get_best_effort_timestamp(yuvFrame);
//        //转换（不同时间基时间转换）
//        int64_t time = av_rescale_q(pts,avStream->time_base,AV_TIME_BASE_Q);
//        LOGE("current time is %d",time);
//        player_wait_for_frame(player,time,player->video_stream_index);


//        int64_t time = av_gettime();
//        LOGE("current time is %d",time);

        //3.解锁
        ANativeWindow_unlockAndPost(nativeWindow);

//        usleep(1000 * 16);


        LOGE("视频第%d帧", cur_frame++);
    }
}



void decodeAudio(JNIEnv * env,Player *player,AVPacket *pPacket){

    int got_picture_ptr,len;
    int cur_frame = 1;
    AVCodecContext * pCodecContext = player->av_codec_context[player->audio_stream_index];
    AVFrame * pFrame = player->aFrame;
    AVFormatContext *av_formate_context = player->av_formate_context;
    AVStream *stream = av_formate_context->streams[player->audio_stream_index];

    struct SwrContext *swrContext = player->swrContext;
    uint8_t *out_buffer = player->a_out_buffer;
    int out_channel_nb = player->out_channel_nb;
    AVSampleFormat out_sample_fmt = player->out_sample_fmt;

//    7.解码
    len = avcodec_decode_audio4(pCodecContext, pFrame, &got_picture_ptr, pPacket);
    if (len < 0) {
        LOGI("%s", "解码完成");
    }
    if (got_picture_ptr) {


//        Convert audio.
        swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                    (const uint8_t **) pFrame->data, pFrame->nb_samples);

//        获取采样数据的实际大小 out_buffer size
//        Get the required buffer size for the given audio parameters.
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                         pFrame->nb_samples,
                                                         out_sample_fmt, 1);

//        int64_t pts = pPacket->pts;
//        if (pts != AV_NOPTS_VALUE) {
//            player->audio_clock = av_rescale_q(pts, stream->time_base, AV_TIME_BASE_Q);
//            //				av_q2d(stream->time_base) * pts;
//            LOGI("player_write_audio - read from pts");
//            player_wait_for_frame(player,
//                                  player->audio_clock + AUDIO_TIME_ADJUST_US, player->audio_stream_index);
//        }


//        写到AudioTrack中
//        buffer  -->  byte[]
        jbyteArray pcm_data_array = env->NewByteArray(out_buffer_size);
        jbyte *byte_array = env->GetByteArrayElements(pcm_data_array, NULL);
        memcpy(byte_array, out_buffer, out_buffer_size);

        env->ReleaseByteArrayElements(pcm_data_array, byte_array, 0);
        env->CallIntMethod(player->jAudioTrack_global, player->write_mid, pcm_data_array, 0, out_buffer_size);

        env->DeleteLocalRef(pcm_data_array);

        LOGE("音频第%d帧", cur_frame++);
    }

//    LOGE("start");

}







void decode_audio_prepare(jclass jclazz,Player *player,JNIEnv* env){


    AVCodecContext *  pCodecContext=player->av_codec_context[player->audio_stream_index];

    //解压缩数据
    AVFrame *pFrame = av_frame_alloc();


    //------------JNI--------START
    int nb_channels =  pCodecContext->channels;

    jmethodID  createAutioTrack_methodId = env->GetStaticMethodID(jclazz,"createAudioTrack","(I)Landroid/media/AudioTrack;");
    jobject  jAudioTrack = env->CallStaticObjectMethod(jclazz,createAutioTrack_methodId,nb_channels);


    jclass  jAudioTrack_class=  env->GetObjectClass(jAudioTrack);
    //play
    jmethodID  play_mid = env->GetMethodID(jAudioTrack_class,"play","()V");
    env->CallVoidMethod(jAudioTrack,play_mid,NULL);
    jmethodID  write_mid = env->GetMethodID(jAudioTrack_class, "write", "([BII)I");
    jobject jAudioTrack_global = env->NewGlobalRef(jAudioTrack);
    player->jAudioTrack_global = jAudioTrack_global;

    //write  public int write(@NonNull byte[] audioData, int offsetInBytes, int sizeInBytes)

    player->write_mid = write_mid;
//    env->CallIntMethod(jAudioTrack,write_mid,)


    //----------JNI--------END

    //重采样设置参数-------------start---------------------------------------------
    //16bit 44100 PCM 统一音频采样格式与采样率
    //Allocate SwrContext.
    struct SwrContext *swrContext = swr_alloc();
    //采样格式  (采样精度)
    AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;

    //采样率
    int in_sample_rate = pCodecContext->sample_rate;
    int out_sample_rate = 44100 ;

    //声道  声道布局  2个声道，默认立体声stereo
    uint64_t in_ch_layout = pCodecContext->channel_layout;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    //Allocate SwrContext if needed and set/reset common parameters
    swr_alloc_set_opts(swrContext,
                       out_ch_layout,out_sample_fmt,out_sample_rate,
                       in_ch_layout,in_sample_fmt,in_sample_rate,
                       0,NULL);
    //Initialize context after user parameters have been set
    swr_init(swrContext);

    //根据声道得到声道数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end------------------------------------------


    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);




    player->aFrame = pFrame;
    player->swrContext = swrContext;
    player->a_out_buffer = out_buffer;
    player->out_channel_nb = out_channel_nb;
    player->out_sample_fmt =  out_sample_fmt;
}

void decodeData(JNIEnv * env,Player *player,Queue * queue,int stream_index) {

    AVFormatContext *ps = player->av_formate_context;

//    //6.获取压缩数据
//    AVPacket *pPacket = (AVPacket *) av_frame_alloc();
//    av_init_packet(pPacket);



//    while (av_read_frame(ps, pPacket) >= 0) {
//        LOGE("start%d---%d",player->video_stream_index,player->audio_stream_index);
//        if (pPacket->stream_index == player->video_stream_index) {
//            decodeVideo(player,pPacket);
//        } else if (pPacket->stream_index == player->audio_stream_index) {
//             decodeAudio(env,player,pPacket);
//        }
//        av_free_packet(pPacket);
//    }

    for (;;) {
        pthread_mutex_lock(&player->pthread_mutex);
        AVPacket *pPacket = (AVPacket *) queue_pop(queue,&player->pthread_mutex,&player->pthread_cond);
        pthread_mutex_unlock(&player->pthread_mutex);

        if (stream_index == player->video_stream_index) {
            decodeVideo(player,pPacket);
        } else if (stream_index == player->audio_stream_index) {
             decodeAudio(env,player,pPacket);
        }

    }


}


/**
 * 释放队列中的资源
 */
void * queue_free_fun_item(void * arg){
    //如果是栈内存资源，不需要释放

    //如果是堆内存资源，需要手动释放
    AVPacket *packet = (AVPacket *) arg;
    av_free_packet(packet);

    return 0;
}


void player_free(Player * player){


    queue_free(player->audio_queue,queue_free_fun_item);

    queue_free(player->video_queue,queue_free_fun_item);


    pthread_mutex_destroy(&player->pthread_mutex);
    pthread_cond_destroy(&player->pthread_cond);



    //video
    ANativeWindow_release(player->nativeWindow);
    av_free(player->out_buffer);
    av_frame_free(&(player->vFrame));
    av_frame_free(&(player->yuvFrame));
    av_frame_free(&(player->argeFrame));


    //audio
    av_free(player->a_out_buffer);
    av_frame_free(&(player->aFrame));
    swr_free(&(player->swrContext));


    avcodec_close(player->av_codec_context[player->video_stream_index]);
    avcodec_close(player->av_codec_context[player->audio_stream_index]);
    avformat_free_context(player->av_formate_context);
}

void * decode_audio(void * arg){
    Player * player = (Player *)arg;
    JNIEnv *mEnv = NULL;
    javaVM->AttachCurrentThread(&mEnv,NULL);


    decodeData(mEnv,player,player->audio_queue,player->audio_stream_index);

    javaVM->DetachCurrentThread();

    return 0;
}

void * decode_video(void * arg){
    Player * player = (Player *)arg;
    JNIEnv *mEnv = NULL;
    javaVM->AttachCurrentThread(&mEnv,NULL);


    decodeData(mEnv,player,player->video_queue,player->video_stream_index);

    javaVM->DetachCurrentThread();

    return  0;
}


void* player_queue_fill_fun(void * arg) {
    AVPacket *packet = (AVPacket *) av_frame_alloc();
    av_init_packet(packet);
    return packet;
}

void player_alloc_queue(Player * player){

//    QUEUE_SIZE

    player->audio_queue = queue_init(QUEUE_SIZE,player_queue_fill_fun);

    player->video_queue = queue_init(QUEUE_SIZE,player_queue_fill_fun);


}


/**
 * 不断读取AVPacket,放入队列
 */
void * player_read_from_stream(void * arg){

    Player * player = (Player *)arg;



    JNIEnv *mEnv = NULL;
    javaVM->AttachCurrentThread(&mEnv,NULL);
    AVFormatContext *ps = player->av_formate_context;

    //6.获取压缩数据
    AVPacket *pPacket = (AVPacket *) av_frame_alloc();
    av_init_packet(pPacket);

    //使用栈内存，不要自己手动释放
//    AVPacket packet,*pPacket=&packet;


    while (av_read_frame(ps, pPacket) >= 0) {
        //LOGE("start%d---%d",player->video_stream_index,player->audio_stream_index);
        pthread_mutex_lock(&player->pthread_mutex);

        if (pPacket->stream_index == player->video_stream_index) {
            AVPacket *old = (AVPacket *) queue_push(player->video_queue,&player->pthread_mutex,&player->pthread_cond);
            av_copy_packet(old,pPacket);
        } else if (pPacket->stream_index == player->audio_stream_index) {
            AVPacket *old = (AVPacket *) queue_push(player->audio_queue,&player->pthread_mutex,&player->pthread_cond);
            av_copy_packet(old,pPacket);
        }
        av_free_packet(pPacket);

        pthread_mutex_unlock(&player->pthread_mutex);
    }
    javaVM->DetachCurrentThread();

    pthread_exit((void *) 1);

}


JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_play
        (JNIEnv * env,  jclass jclazz,jstring inputPath_jstr,jobject surface){
    const char* input_cstr =  env->GetStringUTFChars(inputPath_jstr, NULL);

    Player mPlayer;

    if(init_format_context(&mPlayer,input_cstr)<0){

        return;
    }

    int audio_stream_index = mPlayer.audio_stream_index;
    int video_stream_index = mPlayer.video_stream_index;

    if(init_codec_context(&mPlayer,video_stream_index)<0){
        return;
    }

    if(init_codec_context(&mPlayer,audio_stream_index)<0){
        return;
    }
    player_alloc_queue(&mPlayer);
    decode_video_prepare(&mPlayer,env,surface);
    decode_audio_prepare(jclazz,&mPlayer,env);

    //初始化互斥锁，条件变量
    pthread_mutex_init(&(mPlayer.pthread_mutex),NULL);
    pthread_cond_init(&(mPlayer.pthread_cond),NULL);

//    decodeData(env,&mPlayer);

    //生产者
    pthread_create(&(mPlayer.pthread_read_from_stream),NULL,player_read_from_stream,&mPlayer);
    usleep(1000);
    mPlayer.start_time = 0;
    //消费者
    if(mPlayer.audio_stream_index != -1){
        pthread_create(&(mPlayer.decode_threads[mPlayer.audio_stream_index]),NULL,decode_audio,&mPlayer);
    }
    if(mPlayer.video_stream_index != -1) {
        pthread_create(&(mPlayer.decode_threads[mPlayer.video_stream_index]), NULL, decode_video, &mPlayer);
    }


    pthread_join(mPlayer.pthread_read_from_stream,NULL);
    if(mPlayer.audio_stream_index != -1){
        pthread_join(mPlayer.decode_threads[mPlayer.audio_stream_index],NULL);
    }
    if(mPlayer.video_stream_index != -1) {
        pthread_join(mPlayer.decode_threads[mPlayer.video_stream_index],NULL);
    }


    env->DeleteGlobalRef(mPlayer.jAudioTrack_global);
    player_free(&mPlayer);


    env->ReleaseStringUTFChars(inputPath_jstr,input_cstr);

}



