package github.com.ffmpegplayer.util;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.util.Log;
import android.view.Surface;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/6/28
 * Version  1.0
 * Description:
 */

public class VideoUtil {

    private static String TAG ="VideoUtil";

    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");

        System.loadLibrary("myffmpeg");
    }

    public  static  native void  decode(String inFilePath,String outFilePath);

    public static native void render(String inputPath, Surface surface);

    public static native void audioDecode(String inputPath,String outputPath);

    public  static AudioTrack createAudioTrack(int nb_channels){
        Log.e(TAG," createAudioTrack ===>"+nb_channels);
        int sampleRateInHz = 44100;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        int channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        if(nb_channels == 1) {
            //声道布局
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        }
        int bufferSizeInBytes = AudioRecord.getMinBufferSize(sampleRateInHz,channelConfig,audioFormat);


        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz,channelConfig,
                audioFormat,bufferSizeInBytes,
                AudioTrack.MODE_STREAM);


        //audioTrack.play();




        return audioTrack;
    }


    public static native void play(String inputPath, Surface surface);
}
