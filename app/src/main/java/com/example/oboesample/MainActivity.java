package com.example.oboesample;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("Oboe");
    }

    private TextView mHelloTV;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mHelloTV = findViewById(R.id.hello_tv);
//        mHelloTV.setText(getHelloText());
    }

//    private native String getHelloText();
}