#pragma once

#include <cstddef>

// Phase-vocoder pitch shifter (the string "core/audio/dsp/SmbPitchShift" at
// 0x124E20A). FusionVoicePool owns a set of them for keyzones that shift
// pitch. The class is not in the reference map and has not been
// reconstructed; only what the pool uses is declared. The vtable is at
// 0x18E61E8; the object is 0x64070 bytes.
class SmbPitchShift {
public:
    // Inlined into FusionVoicePool's processor setup at 0xA12C0: the shared
    // window tables are built on first use.
    SmbPitchShift() {
        if (!sTablesInitialized) {
            InitTables();
        }
    }
    virtual ~SmbPitchShift();                   // slots 0-1: 0xE0350, 0xE0360
    // Slot 2 at 0xE0370, empty in this class: FusionVoicePool passes the
    // keyzone's shift, and FusionVoice the sampler's. Name not in the
    // reference map; inferred from those callers.
    virtual void SetShift(int coarse, int fine);
    // Slot 3 at 0xE0380, empty in this class. No caller was found; the name
    // follows the processor interface's setup-then-reset pattern and is a
    // guess. Name not in the reference map.
    virtual void Reset();
    // Slot 4 at 0xDF280: stores the rate per FFT bin and clears the state.
    virtual void SetSampleRate(float sampleRate);

    // Sizes the FFT frame and the oversampling factor. At 0xDEF30.
    void Setup(int fftFrameSize, int oversampling);

    // Builds the window tables for frame sizes 128 to 4096. At 0xDF040.
    static void InitTables();
    static bool sTablesInitialized;  // 0x19E26A0

    unsigned char mState[0x64068];  // FFT frames, phase history and window tables.
};

static_assert(sizeof(SmbPitchShift) == 0x64070);
