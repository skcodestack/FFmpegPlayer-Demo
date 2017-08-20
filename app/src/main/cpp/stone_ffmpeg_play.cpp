#include <jni.h>
#include <android/log.h>
#include "github_com_ffmpegplayer_util_VideoUtil.h"
#include <unistd.h>

#define MAX_AUDIO_FRME_SIZE 48000 * 4

extern  "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"

#include <android/native_window_jni.h>
#include <android/native_window.h>
};
#include "libyuv.h"

using namespace libyuv;

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGW(FORMAT,...) __android_log_print(ANDROID_LOG_WARN,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"ffmpeg",FORMAT,##__VA_ARGS__);


JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_decode
        (JNIEnv * env, jclass jclazz, jstring inPath_jstr, jstring outPath_jstr){


    const char* input_cstr =  env->GetStringUTFChars(inPath_jstr, NULL);
    const char* output_cstr =  env->GetStringUTFChars(outPath_jstr, NULL);


    //1.注册组件
    av_register_all();
    //2.打开文件

    //2.1  封装格式上下文
    AVFormatContext *ps = avformat_alloc_context();

    //int avformat_open_input(AVFormatContext **ps, const char *url,AVInputFormat *fmt, AVDictionary **options);
    if(avformat_open_input(&ps,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频文件信息
    if(avformat_find_stream_info(ps,NULL) < 0){
        LOGE("%s","获取视频文件信息失败");
        return;
    }
    //4.查找解码器
    //4.1找到视频（AVSTREAM）
    int num_streams = ps->nb_streams;
    int  video_stream_index = -1;
    for (int i = 0; i < num_streams; ++i) {
        if(ps->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }
    if(video_stream_index == -1){
        LOGE("%s","没有视频文件");
        return;
    }

    //4.2  拿到解码器
    AVCodecContext *  pCodecContext = ps->streams[video_stream_index]->codec;
    AVCodec * pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if(pCodec == NULL){
        LOGE("%s","找不到解码器");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodecContext,pCodec,NULL) < 0){
        LOGE("%s","解码器打开失败");
        return;
    }
    //6.获取压缩数据
    AVPacket *pPacket = (AVPacket *) av_frame_alloc();
    av_init_packet(pPacket);
    //像素数据
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height);

    //输出文件
    FILE* fp_yuv = fopen(output_cstr,"wb");

    //用于像素格式转换或者缩放
    struct SwsContext *swsContext = sws_getContext(pCodecContext->width,pCodecContext->height,pCodecContext->pix_fmt
            ,pCodecContext->width,pCodecContext->height, AV_PIX_FMT_YUV420P
            ,SWS_BILINEAR,NULL,NULL,NULL);

    int got_picture_ptr,len;
    int cur_frame = 1;

    while (av_read_frame(ps,pPacket) >= 0){
        //7.获取像素数据
        len = avcodec_decode_video2(pCodecContext,pFrame,&got_picture_ptr,pPacket);

        if (got_picture_ptr) {
            //转为指定的YUV420P像素帧
            sws_scale(swsContext, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0, pFrame->height,
                      yuvFrame->data, yuvFrame->linesize);

            //向YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size = pCodecContext->width * pCodecContext->height;

            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);

            LOGE("第%d帧",cur_frame++);
        }
        av_free_packet(pPacket);
    }
    fclose(fp_yuv);

    av_frame_free(&pFrame);
    avcodec_close(pCodecContext);
    avformat_free_context(ps);


    env->ReleaseStringUTFChars(inPath_jstr,input_cstr);
    env->ReleaseStringUTFChars(outPath_jstr,output_cstr);



}

JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_render
        (JNIEnv * env,  jclass jclazz,jstring inputPath_jstr,jobject surface){

    const char* input_cstr =  env->GetStringUTFChars(inputPath_jstr, NULL);



    //1.注册组件
    av_register_all();
    //2.打开文件

    //2.1  封装格式上下文
    AVFormatContext *ps = avformat_alloc_context();

    //int avformat_open_input(AVFormatContext **ps, const char *url,AVInputFormat *fmt, AVDictionary **options);
    if(avformat_open_input(&ps,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频文件信息
    if(avformat_find_stream_info(ps,NULL) < 0){
        LOGE("%s","获取视频文件信息失败");
        return;
    }
    //4.查找解码器
    //4.1找到视频（AVSTREAM）
    int num_streams = ps->nb_streams;
    int  video_stream_index = -1;
    for (int i = 0; i < num_streams; ++i) {
        if(ps->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }
    if(video_stream_index == -1){
        LOGE("%s","没有视频文件");
        return;
    }

    //4.2  拿到解码器
    AVCodecContext *  pCodecContext = ps->streams[video_stream_index]->codec;
    AVCodec * pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if(pCodec == NULL){
        LOGE("%s","找不到解码器");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodecContext,pCodec,NULL) < 0){
        LOGE("%s","解码器打开失败");
        return;
    }
    //6.获取压缩数据
    AVPacket *pPacket = (AVPacket *) av_frame_alloc();
    av_init_packet(pPacket);
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

    int got_picture_ptr,len;
    int cur_frame = 1;
    //获取native window
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env,surface);
    AVFrame *argeFrame = av_frame_alloc();

    //argb缓冲区
    ANativeWindow_Buffer outBuffer;


    while (av_read_frame(ps,pPacket) >= 0){
        if(pPacket->stream_index == video_stream_index) {
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


                //3.解锁
                ANativeWindow_unlockAndPost(nativeWindow);

                usleep(1000 * 16);


                LOGE("第%d帧", cur_frame++);
            }
        }
        av_free_packet(pPacket);
    }
    ANativeWindow_release(nativeWindow);
    av_free(out_buffer);
    av_frame_free(&pFrame);
    av_frame_free(&argeFrame);
    av_frame_free(&yuvFrame);
    avcodec_close(pCodecContext);
    avformat_free_context(ps);


    env->ReleaseStringUTFChars(inputPath_jstr,input_cstr);

    
}


JNIEXPORT void JNICALL Java_github_com_ffmpegplayer_util_VideoUtil_audioDecode
        (JNIEnv * env, jclass jclazz, jstring inPath_jstr, jstring outPath_jstr){


    const char* input_cstr =  env->GetStringUTFChars(inPath_jstr, NULL);
//    const char* output_cstr =  env->GetStringUTFChars(outPath_jstr, NULL);


    //1.注册组件
    av_register_all();
    //2.打开文件

    //2.1  封装格式上下文
    AVFormatContext *ps = avformat_alloc_context();

    //int avformat_open_input(AVFormatContext **ps, const char *url,AVInputFormat *fmt, AVDictionary **options);
    if(avformat_open_input(&ps,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入音频文件失败");
        return;
    }
    //3.获取视频文件信息
    if(avformat_find_stream_info(ps,NULL) < 0){
        LOGE("%s","获取音频文件信息失败");
        return;
    }
    //4.查找解码器
    //4.1找到视频（AVSTREAM）
    int num_streams = ps->nb_streams;
    int  audio_stream_index = -1;
    for (int i = 0; i < num_streams; ++i) {
        if(ps->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
            break;
        }
    }
    if(audio_stream_index == -1){
        LOGE("%s","没有音频文件");
        return;
    }

    //4.2  拿到解码器
    AVCodecContext *  pCodecContext = ps->streams[audio_stream_index]->codec;
    AVCodec * pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if(pCodec == NULL){
        LOGE("%s","找不到解码器");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodecContext,pCodec,NULL) < 0){
        LOGE("%s","解码器打开失败");
        return;
    }
    //6.获取压缩数据
    AVPacket *pPacket = (AVPacket *) av_frame_alloc();
    av_init_packet(pPacket);
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

    //write  public int write(@NonNull byte[] audioData, int offsetInBytes, int sizeInBytes)
    jmethodID  write_mid = env->GetMethodID(jAudioTrack_class,"write","([BII)I");
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

    //输出文件
//    FILE* fp_pcm = fopen(output_cstr,"wb");


    int got_picture_ptr,len;
    int cur_frame = 1;

    while (av_read_frame(ps,pPacket) >= 0){
        if(pPacket->stream_index == audio_stream_index) {
            //7.解码
            len = avcodec_decode_audio4(pCodecContext, pFrame, &got_picture_ptr, pPacket);
            if (len < 0) {
                LOGI("%s", "解码完成");
            }
            if (got_picture_ptr) {


                //Convert audio.
                swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) pFrame->data, pFrame->nb_samples);

                //获取采样数据的实际大小 out_buffer size
                //Get the required buffer size for the given audio parameters.
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 pFrame->nb_samples,
                                                                 out_sample_fmt, 1);

                //写到AudioTrack中
                //buffer  -->  byte[]
                jbyteArray pcm_data_array = env->NewByteArray(out_buffer_size);
                jbyte *byte_array = env->GetByteArrayElements(pcm_data_array, NULL);
                memcpy(byte_array, out_buffer, out_buffer_size);

                env->ReleaseByteArrayElements(pcm_data_array, byte_array, 0);

                env->CallIntMethod(jAudioTrack, write_mid, pcm_data_array, 0, out_buffer_size);
//            fwrite(out_buffer,1,out_buffer_size,fp_pcm);

                env->DeleteLocalRef(pcm_data_array);

                LOGE("第%d帧", cur_frame++);
            }
        }
        av_free_packet(pPacket);
    }


//    fclose(fp_pcm);


    av_free(out_buffer);
    av_frame_free(&pFrame);
    avcodec_close(pCodecContext);
    avformat_free_context(ps);
    swr_free(&swrContext);

    env->ReleaseStringUTFChars(inPath_jstr,input_cstr);
//    env->ReleaseStringUTFChars(outPath_jstr,output_cstr);

}

