//
// Created by darrenyuan on 2022/10/1.
//

#include "com_example_oboesample_MainActivity.h"

// opensl
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define TAG "MainActivityJNI"

// 定义日志打印方法
#include <android/log.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO, TAG, FORMAT, ##__VA_ARGS__);

// 引擎接口
SLObjectItf engineObject = nullptr;
SLEngineItf engineEngine = nullptr;

// 混音器
SLObjectItf outputMixObject = nullptr;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;
SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// uri播放器
SLObjectItf uriPlayer = nullptr;
SLPlayItf uriPlayerPlay = nullptr;
SLVolumeItf uriPlayerVolume = nullptr;

// assets播放器
SLObjectItf fdPlayer = nullptr;
SLPlayItf fdPlayerPlay = nullptr;
SLVolumeItf fdPlayerVolume = nullptr;

// 录制器
#include "RecordBuffer.h"
#include <cstdio>
SLObjectItf recorderObj = nullptr;
SLRecordItf recorder = nullptr;
SLAndroidSimpleBufferQueueItf recorderBufferQueue;
//录制大小设为4096
#define RECORDER_FRAMES (2048)
static unsigned recorderSize = RECORDER_FRAMES * 2;

//PCM文件
FILE *pcmFile;
//录音buffer
RecordBuffer *recordBuffer;

bool finished = false;


void release();

void createEngine() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
}

JNIEXPORT void JNICALL Java_com_example_oboesample_MainActivity_native_1playUriAudio
(JNIEnv *env, jobject, jstring uri) {
    release();

    SLresult result;

    char *source = const_cast<char *>(env->GetStringUTFChars(uri, nullptr));
    LOGI("playUriAudio %s", source)

    // 第一步，创建引擎
    createEngine();

    // 第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }

    // 第三步，设置播放器参数和创建播放器
    // configure audio source
    // (requires the INTERNET permission depending on the uri parameter)
    SLDataLocator_URI loc_uri = {SL_DATALOCATOR_URI, (SLchar *) source};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_uri, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &uriPlayer, &audioSrc, &audioSnk, 3, ids, req);

    (void)result;

    // release the Java string and UTF-8
    env->ReleaseStringUTFChars(uri, source);

    // realize the player
    result = (*uriPlayer)->Realize(uriPlayer, SL_BOOLEAN_FALSE);
    // this will always succeed on Android, but we check result for portability to other platforms
    if (SL_RESULT_SUCCESS != result) {
        (*uriPlayer)->Destroy(uriPlayer);
        uriPlayer = NULL;
        return;
    }

    // get the play interface
    result = (*uriPlayer)->GetInterface(uriPlayer, SL_IID_PLAY, &uriPlayerPlay);
    (void)result;


    // get the volume interface
    result = (*uriPlayer)->GetInterface(uriPlayer, SL_IID_VOLUME, &uriPlayerVolume);
    (void)result;

    if (nullptr != uriPlayerPlay) {

        // set the player's state
        result = (*uriPlayerPlay)->SetPlayState(uriPlayerPlay, SL_PLAYSTATE_PLAYING);
        (void)result;
    }

}

JNIEXPORT void JNICALL
Java_com_example_oboesample_MainActivity_native_1playAssetAudio(JNIEnv *env, jobject thiz,
                                                                jobject asset_manager,
                                                                jstring file_name) {
    release();
    const char* source = const_cast<char *>(env->GetStringUTFChars(file_name, nullptr));
    LOGI("playAssetAudio %s", source)
    AAssetManager* manager = AAssetManager_fromJava(env, asset_manager);
    AAsset* asset = AAssetManager_open(manager, source, AASSET_MODE_UNKNOWN);

    // why need to release here
    env->ReleaseStringChars(file_name, reinterpret_cast<const jchar *>(source));

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);

    SLresult result;


    //第一步，创建引擎
    createEngine();

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    //第三步，设置播放器参数和创建播放器
    // 1、 配置 audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, nullptr, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // 2、 配置 audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    // 创建播放器
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayer, &audioSrc, &audioSnk, 3, ids, req);
    // 创建录制器
//    (*engineEngine)->CreateAudioRecorder(engineEngine, &recorder, SLDataSource, pAudioSnk, )

    (void)result;

    // 实现播放器
    result = (*fdPlayer)->Realize(fdPlayer, SL_BOOLEAN_FALSE);
    (void)result;

    // 得到播放器接口
    result = (*fdPlayer)->GetInterface(fdPlayer, SL_IID_PLAY, &fdPlayerPlay);
    (void)result;

    // 得到声音控制接口
    result = (*fdPlayer)->GetInterface(fdPlayer, SL_IID_VOLUME, &fdPlayerVolume);
    (void)result;

    // 设置播放状态
    if (nullptr != fdPlayerPlay) {

        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_PLAYING);
        (void)result;
    }

    //设置播放音量 （100 * -50：静音 ）
    (*fdPlayerVolume)->SetVolumeLevel(fdPlayerVolume, 20 * -50);
}

void release() {
    if (uriPlayer != nullptr) {
        (*uriPlayer)->Destroy(uriPlayer);
        uriPlayer = nullptr;
        uriPlayerPlay = nullptr;
        uriPlayerVolume = nullptr;
    }

    if (nullptr != fdPlayer) {
        (*fdPlayer)->Destroy(fdPlayer);
        fdPlayer = nullptr;
        fdPlayerPlay = nullptr;
        fdPlayerVolume = nullptr;
    }
}

// 录制回调
void bqRecorderCallBack(SLAndroidSimpleBufferQueueItf bq, void *context) {
    fwrite(recordBuffer->getNowBuffer(), 1, recorderSize, pcmFile);
    if(finished)
    {
        (*recorder)->SetRecordState(recorder, SL_RECORDSTATE_STOPPED);
        fclose(pcmFile);
        LOGI("停止录音");
    } else{
        (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffer->getRecordBuffer(), recorderSize);
    }
}

JNIEXPORT void JNICALL
Java_com_example_oboesample_MainActivity_native_1recordPcm(JNIEnv *env, jobject thiz,
                                                           jstring save_path) {
    const char* savePath = env->GetStringUTFChars(save_path, nullptr);
    LOGI("recordPcm %s", save_path);
    // pcm file
    pcmFile = fopen(savePath, "w");
    // pcm buffer queue
    recordBuffer = new RecordBuffer(RECORDER_FRAMES * 2);
    SLresult result;
    /**
     * 创建引擎对象
     */
     result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
     result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
     result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    /**
    * 设置IO设备（麦克风）
    */
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};
    /**
     * 设置buffer队列
     */
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    /**
     * 设置录制规格：PCM、2声道、44100HZ、16bit
     */
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    /**
     * 创建录制器
     */
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObj, &audioSrc,
                                                  &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*recorderObj)->Realize(recorderObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*recorderObj)->GetInterface(recorderObj, SL_IID_RECORD, &recorderObj);
    result = (*recorderObj)->GetInterface(recorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &recorderBufferQueue);
    finished = false;
    result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffer->getRecordBuffer(),
                                             recorderSize);
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallBack, NULL);
    LOGI("开始录音");
    /**
     * 开始录音
     */
    (*recorder)->SetRecordState(recorder, SL_RECORDSTATE_RECORDING);
    env->ReleaseStringUTFChars(save_path, savePath);
}

// 录制部分：https://github.com/wanliyang1990/OpenSL-ES-Record
// 播放部分：https://github.com/wanliyang1990/Android_OpenSl_Audio
