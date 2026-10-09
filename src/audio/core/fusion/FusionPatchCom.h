#pragma once

#include <cstddef>

#include "audio/core/resources/AudioSampleResource.h"
#include "audio/core/resources/Resource.h"

// Sampler patch component (audio/FusionPatchCom.o). The component has not
// been reconstructed; only the keyzone fields FusionVoicePool reads are
// declared.
class FusionPatchCom {
public:
    // One key and velocity range of a patch. Field names are not in the
    // reference map.
    struct KeyzoneSettings {
        unsigned char mOpaque0[6];  // Bytes the pool does not use.
        // Voice-stealing rank: a voice whose rank is zero, or lower than the
        // new note's, is never stolen for it.
        unsigned char mPriority;
        unsigned char mOpaque7[33];
        bool mUsePitchShift;  // The voice needs one of the pool's shifters.
        // The shift handed to SmbPitchShift::SetShift, coarse first. The
        // split into coarse and fine is inferred from the call order.
        int mShiftFine;
        int mShiftCoarse;
        unsigned char mOpaque52[52];
        ResourcePtr<AudioSampleResource> mSample;
    };
};

static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mPriority) == 6);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mUsePitchShift) == 40);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mShiftFine) == 44);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSample) == 104);
