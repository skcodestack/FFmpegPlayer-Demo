package github.com.ffmpegplayer_demo;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/7
 * Version  1.0
 * Description:
 */

public class PosixThread {
    static {

        System.loadLibrary("pthread");

    }

    public native void pthread();
}
