package github.com.ffmpegplayer_demo.pusher;

import android.hardware.Camera;
import android.view.SurfaceHolder;

import github.com.ffmpegplayer_demo.jni.PushNative;
import github.com.ffmpegplayer_demo.params.AudioParams;
import github.com.ffmpegplayer_demo.params.VideoParams;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/17
 * Version  1.0
 * Description:
 */

public class LivePusher implements SurfaceHolder.Callback {

    SurfaceHolder mSurfaceHolder;
    private VideoPusher mVideoPusher;
    private AudioPusher mAudioPusher;
    private PushNative mPushNative;

    public LivePusher(SurfaceHolder surfaceHolder){
        mSurfaceHolder = surfaceHolder;
        mSurfaceHolder.addCallback(this);
        prepare();
    }

    /**
     * 准备推流
     */
    private void prepare() {
        mPushNative = new PushNative();
        //640 x 480  320 x 240
        VideoParams videoParams = new VideoParams(320,240, Camera.CameraInfo.CAMERA_FACING_BACK);
        mVideoPusher = new VideoPusher(mPushNative,mSurfaceHolder,videoParams);

        AudioParams audioParams = new AudioParams();
        mAudioPusher = new AudioPusher(mPushNative,audioParams);

    }

    /**
     * 切换摄像头
     */
    public void  switchCamera(){
        mVideoPusher.switchCamera();
    }

    /**
     * 开始推流
     */
    public void startPush(String url) {

        mVideoPusher.startPush();
        mAudioPusher.startPush();

        mPushNative.startPush(url);
    }

    /**
     * 停止推流
     */
    public void stopPush(){
        mVideoPusher.stopPush();
        mAudioPusher.stopPush();

        mPushNative.stopPush();
    }

    public void release(){
        mVideoPusher.release();
        mAudioPusher.release();

        mPushNative.release();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPush();
        release();
    }
}
