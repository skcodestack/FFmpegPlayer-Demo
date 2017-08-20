package github.com.ffmpegplayer;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.Surface;
import android.view.View;

import java.io.File;

import github.com.ffmpegplayer.util.VideoUtil;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/6/30
 * Version  1.0
 * Description:
 */

public class VedioSurface01 extends AppCompatActivity {

    private Surface surface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play);

        VedioView vedioView =  (VedioView) findViewById(R.id.vediosurface);
        surface = vedioView.getHolder().getSurface();
    }


    public void paly_btn(View view){

        String inputPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "input.mp4";

        VideoUtil.play(inputPath,surface);

//        VideoUtil.render(inputPath,surface);
    }
}
