package github.com.ffmpegplayer_demo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void btn_thread_01(View view){

        PosixThread posixThread = new PosixThread();
        posixThread.pthread();

    }
}
