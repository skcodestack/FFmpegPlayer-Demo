package github.com.ffmpegplayer_demo.pusher;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.LinkedHashMap;
import java.util.List;

import github.com.ffmpegplayer_demo.jni.PushNative;
import github.com.ffmpegplayer_demo.params.VideoParams;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/17
 * Version  1.0
 * Description:
 */

public class VideoPusher extends Pusher implements SurfaceHolder.Callback, Camera.PreviewCallback {

    private static final String TAG = "VideoPusher";

    private final SurfaceHolder mSurfaceHolder;
    private Camera mCamera;
    private VideoParams mVideoParams;
    private PushNative mPushNative;
    private byte[] mBuf;
    //是否开始推送
    private  boolean isPushing = false;

    public VideoPusher(PushNative pushNative, SurfaceHolder surfaceHolder, VideoParams videoParams) {
        this.mSurfaceHolder = surfaceHolder;
        this.mPushNative = pushNative;
        this.mVideoParams = videoParams;
        mSurfaceHolder.addCallback(this);
    }

    /**
     * 开始推流
     */
    @Override
    public void startPush() {
        mPushNative.setVideoOptions(mVideoParams.getWidth(),mVideoParams.getHeight(),
                mVideoParams.getBitrate(),mVideoParams.getFps());
        isPushing = true;
    }

    /**
     * 停止推流
     */
    @Override
    public void stopPush() {
        isPushing = false;
        stopPreview();
    }

    @Override
    public void release() {
        stopPreview();
    }


    /**
     * surface初始化完成
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        startPreview(holder);

    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
//        // If your preview can change or rotate, take care of those events here.
//        // Make sure to stop the preview before resizing or reformatting it.
//        if (holder.getSurface() == null){
//            // preview surface does not exist
//            return;
//        }
//        // stop preview before making changes
//        try {
//            mCamera.stopPreview();
//        } catch (Exception e){
//            // ignore: tried to stop a non-existent preview
//        }
//        // set preview size and make any resize, rotate or
//        // reformatting changes here
//        // start preview with new settings
//        try {
//            mCamera.setPreviewDisplay(holder);
//            mCamera.startPreview();
//        } catch (Exception e){
//            //Log.d(TAG, "Error starting camera preview: " + e.getMessage());
//        }

        Log.e(TAG,"=============>"+format+"---"+width+"----"+height);

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPreview();
    }


    /**
     * 切换摄像头
     */
     public void switchCamera(){
         int cameraId = mVideoParams.getCameraId();
         if(cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT){
            cameraId = Camera.CameraInfo.CAMERA_FACING_BACK;
         }else {
             cameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;
         }
         mVideoParams.setCameraId(cameraId);

         stopPreview();

         startPreview(mSurfaceHolder);

     }

    /**
     * 开始预览
     * @param holder
     */
    private void startPreview(SurfaceHolder holder){
        try {
            mCamera = Camera.open(mVideoParams.getCameraId());
            mCamera.setPreviewDisplay(holder);
            mCamera.setDisplayOrientation(90);
            parameters(mCamera);
//            Camera.Parameters parameters = mCamera.getParameters();
//            Camera.Size previewSize = parameters.getPreviewSize();


            try {


                //设置预览界面大小
                Camera.Parameters parameters = mCamera.getParameters();
                Camera.Size previewSize = parameters.getPreviewSize();
                Log.i("pictureSize","=============>"+previewSize.width+" x "+previewSize.height);

                parameters.setPreviewSize(mVideoParams.getWidth(), mVideoParams.getHeight());
                parameters.setPreviewFormat(ImageFormat.NV21);
//                parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
                mCamera.setParameters(parameters);


            }catch (Exception ex){
                ex.printStackTrace();

            }



            //获取  every preview frame
            mBuf = new byte[mVideoParams.getHeight()*mVideoParams.getWidth()*4];
            mCamera.addCallbackBuffer(mBuf);
            mCamera.setPreviewCallbackWithBuffer(this);

            mCamera.startPreview();

        }catch (Exception ex){
            ex.printStackTrace();
        }
    }
    public void parameters(Camera camera) {
        List<Camera.Size> pictureSizes = camera.getParameters().getSupportedPictureSizes();
        List<Camera.Size> previewSizes = camera.getParameters().getSupportedPreviewSizes();
        Camera.Size psize;
        for (int i = 0; i < pictureSizes.size(); i++) {
            psize = pictureSizes.get(i);
            Log.i("pictureSize",psize.width+" x "+psize.height);
        }
        for (int i = 0; i < previewSizes.size(); i++) {
            psize = previewSizes.get(i);
            Log.i("previewSize",psize.width+" x "+psize.height);
        }
    }


    /**
     * 停止预览
     */
    private void stopPreview(){
        if(mCamera != null){
            mCamera.stopPreview();
            mCamera.release();
            mCamera = null;
        }
    }


    /**
     * 获取预览数据（每帧为单位）
     * @param data
     * @param camera
     */
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if(mCamera != null) {
            //获取图像数据，传给native层
            mCamera.addCallbackBuffer(mBuf);
            if(isPushing) {
                mPushNative.fireVideo(mBuf);
//                Log.i("pictureSize","=============>onPreviewFrame");
            }
        }
    }
}
