package github.com.ffmpegplayer_demo.jni;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/18
 * Version  1.0
 * Description:
 */

public class PushNative {

    static {
        System.loadLibrary("mylive");
    }
    public native void startPush(String url);

    public native void stopPush();

    public native void release();

    /**
     * set video params
     * @param width
     * @param height
     * @param bitrate
     * @param fps
     */
    public native void setVideoOptions(int width,int height,int bitrate,int fps);


    /**
     * set autdio params
     * @param sampleRateInHz
     * @param channel
     */
    public native void setAudioOptions(int sampleRateInHz,int channel);



    /**
     * send video data
     * @param buf
     */
    public native void fireVideo(byte[] buf);

    /**
     * send audio data
     * @param buf
     * @param len
     */
    public native void fireAudio(byte[] buf,int len);

}
