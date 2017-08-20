package github.com.ffmpegplayer_demo.params;

/**
 * Email  1562363326@qq.com
 * Github https://github.com/skcodestack
 * Created by sk on 2017/7/17
 * Version  1.0
 * Description:
 */

public class AudioParams {

    //采样率
    private int sampleRateInHZ = 44100;
    //声道个数
    private int channel = 1;

    public AudioParams(){

    }

    public AudioParams(int sampleRateInHZ, int channel) {
        this.sampleRateInHZ = sampleRateInHZ;
        this.channel = channel;
    }



    public int getSampleRateInHZ() {
        return sampleRateInHZ;
    }

    public void setSampleRateInHZ(int sampleRateInHZ) {
        this.sampleRateInHZ = sampleRateInHZ;
    }

    public int getChannel() {
        return channel;
    }

    public void setChannel(int channel) {
        this.channel = channel;
    }
}
