package github.com.ffmpegplayer;

import android.Manifest;
import android.content.Intent;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import java.io.File;
import java.util.HashMap;

import github.com.ffmpegplayer.util.VideoUtil;
import github.com.permissionlib.StonePermission;
import github.com.permissionlib.annotation.PermissionFail;
import github.com.permissionlib.annotation.PermissionSuccess;

public class MainActivity extends AppCompatActivity {



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        StonePermission.with(this)
                .permissions(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .addRequestCode(100)
                .request();
    }


    public void audio_decode_btn(View view){

        String inputPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "input.mp4";//"zghhr.flv";//"pfzl.mp3";
        String outputPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "pfzl.pcm";
        VideoUtil.audioDecode(inputPath,outputPath);

        Toast.makeText(this,"解压完成！",Toast.LENGTH_SHORT).show();

    }

    /**
     * 解码
     * @param view
     */
    public void decode_btn(View view){

        String inputPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "input.mp4";
        String outputPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "output.yuv";
        VideoUtil.decode(inputPath,outputPath);

        Toast.makeText(this,"解压完成！",Toast.LENGTH_SHORT).show();

    }


    @PermissionSuccess(requestCode =  100)
    private void decode(){

    }
    @PermissionFail(requestCode = 100)
    private void fail(){
        Toast.makeText(this,"获取权限失败！",Toast.LENGTH_SHORT).show();
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        StonePermission.onRequestPermissionsResult(this,requestCode,permissions,grantResults);
    }


    public void render_btn(View view){
        Intent intent = new Intent(this,VedioSurface01.class);
        startActivity(intent);
    }
}
