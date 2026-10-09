#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/dsp/Ramper.h"
#include "audio/core/dsp/TempoListener.h"

// Feedback delay line (audio/Delay.o), optionally synced to the tempo. The
// out-of-line members have not been reconstructed; they are declared with
// the inline ones FusionSampler uses. The vtable is at 0x18E6180; the object
// is 512 bytes.
class Delay : public TempoListener {
public:
    Delay();  // 0xDA9F0
    // Slots 0-1; the out-of-line copies are at 0xDCB80 and 0xDCC60.
    // FusionSampler's destructor (0x96E20) inlines it.
    ~Delay() override {
        Unregister();
    }
    // Slot 2 at 0xDCA30: stores both and recomputes a synced delay.
    void OnTempoChanged(float tempo, float speed) override;

    // Allocates the line for the longest delay. The map's signature is
    // Prepare(float, unsigned int, float). At 0xDADD0.
    void Prepare(float sampleRate, unsigned int numChannels, float maxDelayMs);
    // A time in beats while synced. At 0xDB220.
    void SetDelaySeconds(float seconds);
    void Process(AudioBuffer<float>& buffer);  // 0xDB510
    void SetTempo(float tempo);                // 0xDCA80
    // Name not in the reference map. At 0xDCAD0.
    void SetSpeed(float speed);
    void SetBeatSync(bool beatSync);  // 0xDCB20

    // Inline in the map's build, which emits them in
    // audio/FusionSampler.o; FusionSampler::LoadPatch inlines them here.
    void SetDryGain(float gain) {
        mDryGain.Set(gain);
    }
    void SetWetGain(float gain) {
        mWetGain.Set(gain);
    }
    void SetFeedbackGain(float gain) {
        mFeedbackGain.Set(gain);
    }
    // Ends the ramps the gain callbacks start, so that a new patch's gains
    // apply at once. Inlined into FusionSampler::LoadPatch at 0x987E0. Name
    // not in the reference map.
    void FinishGainRamps() {
        mGainRamps[3].Finish();
        mGainRamps[2].Finish();
        mGainRamps[0].Finish();
        mGainRamps[1].Finish();
    }

    // Field names are not in the reference map. mOpaque arrays are bytes the
    // reconstructed code does not use.
    unsigned char mOpaque40[16];
    AudioBuffer<float> mBuffer;  // The delay line.
    unsigned char mOpaque184[8];
    SPL::Parameter mFeedbackGain;
    SPL::Parameter mWetGain;
    SPL::Parameter mDryGain;
    float mDelayTime;  // Seconds, or beats while synced.
    bool mBeatSync;
    SPL::Ramper mGainRamps[4];
    unsigned char mOpaque488[24];
};

static_assert(offsetof(Delay, mBuffer) == 0x38);
static_assert(offsetof(Delay, mFeedbackGain) == 0xC0);
static_assert(offsetof(Delay, mWetGain) == 0xE0);
static_assert(offsetof(Delay, mDryGain) == 0x100);
static_assert(offsetof(Delay, mDelayTime) == 0x120);
static_assert(offsetof(Delay, mBeatSync) == 0x124);
static_assert(offsetof(Delay, mGainRamps) == 0x128);
static_assert(sizeof(Delay) == 0x200);
