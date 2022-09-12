//
// Created by darrenyuan on 2022/9/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <jni.h>

extern "C"
jstring Java_com_example_oboesample_MainActivity_getHelloText(JNIEnv *env, jobject jobj) {
    char* text = "from c++ text";
    return env->NewStringUTF(text);
}
