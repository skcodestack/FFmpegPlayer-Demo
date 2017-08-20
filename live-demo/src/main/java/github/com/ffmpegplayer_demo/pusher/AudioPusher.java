package github.com.ffmpegplayer_demo.pusher;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import github.com.ffmpegplayer_demo.jni.PushNative;
import github.com.ffmpegplayer_demo.params.AudioParams;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/17
 * Version  1.0
 * Description:
 */

public class AudioPusher extends Pusher {

    private static final String TAG = "AudioPusher";

    private final AudioParams mAudioParams;
    private AudioRecord mAudioRecord;
    private PushNative mPushNative;
    boolean isPusing = false;
    private final int minBufferSize;

    public AudioPusher(PushNative pushNative, AudioParams audioParams){
        this.mAudioParams = audioParams;
        this.mPushNative = pushNative;
        int channelConfig = mAudioParams.getChannel() == 1 ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO;
        minBufferSize = AudioRecord.getMinBufferSize(mAudioParams.getSampleRateInHZ(), channelConfig, AudioFormat.ENCODING_PCM_16BIT);
        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                mAudioParams.getSampleRateInHZ(),channelConfig, AudioFormat.ENCODING_PCM_16BIT, minBufferSize);

    }


    @Override
    public void startPush() {
        isPusing = true;
        mPushNative.setAudioOptions(mAudioParams.getSampleRateInHZ(),mAudioParams.getChannel());
        new Thread(new AudioTask()).start();
    }

    @Override
    public void stopPush() {
        Log.e(TAG,"音频停止-----------------------------------");
        isPusing = false;
        if(mAudioRecord != null) {
            mAudioRecord.stop();
        }
    }

    @Override
    public void release() {
        if(mAudioRecord != null){
            mAudioRecord.release();
            mAudioRecord = null;
        }


    }


    public class AudioTask implements Runnable {

        @Override
        public void run() {
            //开始录音
            mAudioRecord.startRecording();
            while (isPusing){
                byte[] buf = new byte[minBufferSize];
                int len = mAudioRecord.read(buf, 0, buf.length);
                if(len > 0){
                    //传给native层
//                    Log.e(TAG,"开始编码");
                    mPushNative.fireAudio(buf,len);
                }

            }
        }
    }
}
