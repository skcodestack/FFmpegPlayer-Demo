package github.com.ffmpegplayer_demo;

import android.Manifest;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import github.com.ffmpegplayer_demo.pusher.LivePusher;
import github.com.permissionlib.StonePermission;
import github.com.permissionlib.annotation.PermissionFail;
import github.com.permissionlib.annotation.PermissionSuccess;

public class MainActivity extends AppCompatActivity {

    private LivePusher mLivePusher;
    private Button btn_switch;

    private String mUrl = "rtmp://60.205.185.255:1935/live/skcodestack";


    private static final int request_code = 0x00100;
    private SurfaceView surfaceView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = (SurfaceView) findViewById(R.id.sfv);
        btn_switch = (Button) findViewById(R.id.btn);




        StonePermission.with(this)
                .addRequestCode(request_code)
                .permissions(Manifest.permission.CAMERA,Manifest.permission.RECORD_AUDIO,Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .request();

    }


    @PermissionSuccess(requestCode = request_code)
    private void success(){
        //预览
        mLivePusher = new LivePusher(surfaceView.getHolder());
    }

    @PermissionFail(requestCode = request_code)
    private void failed(){
        Toast.makeText(this,"权限不够",Toast.LENGTH_SHORT).show();
    }



    /**
     * 开始直播
     * @param view
     */
    public void mStartLive(View view){

        if(btn_switch.getText().equals("开始直播")) {
            Log.e("dfdfd", "开始-----------------------------------");
            mLivePusher.startPush(mUrl);
            btn_switch.setText("停止直播");
        }else {
            Log.e("dfdfd","停止-----------------------------------");
            mLivePusher.stopPush();
            btn_switch.setText("开始直播");
        }
    }

    /**
     * 切换摄像头
     * @param view
     */
    public void btn_camera_switch(View view){
        mLivePusher.switchCamera();

    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        StonePermission.onRequestPermissionsResult(this,requestCode,permissions,grantResults);
    }
}
