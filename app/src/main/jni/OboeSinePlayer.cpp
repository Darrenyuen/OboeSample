//
// Created by darrenyuan on 2022/9/12.
//
// The following class is a complete implementation of an audio player that renders a sine wave.
#include "oboe/include/oboe/Oboe.h"
#include <cmath>
using namespace oboe;
using namespace std;

class OboeSinePlayer: public AudioStreamDataCallback {
public:
    ~OboeSinePlayer() override = default;

    // Call this from Activity onResume()
    int32_t startAudio();

    // Call this from Activity onPause()
    void stopAudio();

    DataCallbackResult onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

private:
    mutex mLock;
    shared_ptr<AudioStream> mStream;

    // Stream params
    static int constexpr kChannelCount = 2;
    static int constexpr kSampleRate = 48000;
    // Wave params, these could be instance variables in order to modify at runtime
    static float constexpr kAmplitude = 0.5f;
    static float constexpr kFrequency = 440;
    static constexpr float kPI = M_PI;
    static constexpr float kTwoPi = kPI * 2;
    static constexpr double mPhaseIncrement = kFrequency * kTwoPi / (double) kSampleRate;
    // Keeps track of where the wave is
    float mPhase = 0.0;
};

int32_t OboeSinePlayer::startAudio() {
    lock_guard<mutex> lock(mLock);
    AudioStreamBuilder builder;
    // The builder set methods can be chained for convenience.
    Result result = builder.setSharingMode(SharingMode::Exclusive)
            ->setPerformanceMode(PerformanceMode::LowLatency)
            ->setChannelCount(kChannelCount)
            ->setSampleRate(kSampleRate)
            ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
            ->setFormat(AudioFormat::Float)
            ->setDataCallback(this)
            ->openStream(mStream);
    if (result != oboe::Result::OK) return (int32_t) result;

    //Typically, start the stream after querying some stream information, as well as some input from the user
    result = mStream->requestStart();
    return (int32_t) result;
}

void OboeSinePlayer::stopAudio() {
    // Stop, close and delete in case not already closed.
    lock_guard<mutex> lock(mLock);
    if (mStream) {
        mStream->stop();
        mStream->close();
        mStream.reset();
    }
}

DataCallbackResult
OboeSinePlayer::onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    auto *floatData = (float *) audioData;
    for (int i = 0; i < numFrames; ++i) {
        float sampleValue = kAmplitude * sinf(mPhase);
        for (int j = 0; j < kChannelCount; ++j) {
            floatData[i * kChannelCount + j] = sampleValue;
        }
        mPhase += mPhaseIncrement;
        if (mPhase >= kTwoPi) mPhase -= kTwoPi;
    }
    return DataCallbackResult::Continue;
}

