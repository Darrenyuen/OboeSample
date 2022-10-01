package com.example.oboesample;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    static {
        System.loadLibrary("Oboe");
    }

    private Button mPlayUriBtn;
    private Button playAssetBtn;
    private Button recordPcmBtn;

    private AssetManager assetManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mPlayUriBtn = findViewById(R.id.play_uri_btn);
        playAssetBtn = findViewById(R.id.play_asset_btn);
        recordPcmBtn = findViewById(R.id.record_pcm_btn);
        mPlayUriBtn.setOnClickListener(this);
        playAssetBtn.setOnClickListener(this);
        recordPcmBtn.setOnClickListener(this);
        assetManager = getAssets();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.play_uri_btn :
                playUriAudio("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                break;
            case R.id.play_asset_btn:
                playAssetAudio(assetManager, "1.mp3");
                break;
            case R.id.record_pcm_btn:
                recordPcm("mic.pcm");
                break;
        }
    }

    private void playUriAudio(String uri) {
        native_playUriAudio(uri);
    }

    private void playAssetAudio(AssetManager assetManager, String fileName) {
        native_playAssetAudio(assetManager, fileName);
    }

    private void recordPcm(String savePath) {
        native_recordPcm(savePath);
    }

    private native void native_playUriAudio(String uri);

    private native void native_playAssetAudio(AssetManager assetManager, String fileName);

    private native void native_recordPcm(String savePath);
}