package github.com.ffmpegplayer;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/6/29
 * Version  1.0
 * Description:
 */

public class VedioView extends SurfaceView {


    public VedioView(Context context) {
        this(context,null);
        init();
    }

    public VedioView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
        init();
    }

    public VedioView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();

    }

    public void init (){
        SurfaceHolder surfaceHolder = getHolder();
//        PixelFormat.YCbCr_420_SP   ImageFormat.YUV_420_888   ImageFormat.NV21
        surfaceHolder.setFormat(PixelFormat.RGBA_8888);


    }


}
