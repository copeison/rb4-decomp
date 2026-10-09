#pragma once

#include <cstddef>

#include "audio/core/fusion/FusionPatchCom.h"
#include "os/memory/MemMgr.h"

class FusionSampler;
class FusionVoicePool;
class SmbPitchShift;

// One sampler voice (audio/FusionVoice.o). FusionVoicePool allocates them in
// an array. The class has not been reconstructed; only the members the pool
// uses are declared. The object is 784 bytes.
class FusionVoice {
public:
    // Voice arrays come from the tracked heap under the label "FusionVoice",
    // as FusionVoicePool's voice setup at 0xA1090 shows.
    static void* operator new[](unsigned long size) {
        return MemAlloc(size, "FusionVoice", 0);
    }
    static void operator delete[](void* allocation) {
        MemFree(allocation);
    }

    FusionVoice();   // 0x9D920
    ~FusionVoice();  // 0x9DD30

    // The map has Init(char); this build also binds the pool and, unless
    // skipDecoders is set, creates the voice's Mogg and XMA decoders through
    // AudioDecoder::NewDecoderForFormat. At 0x9E520.
    void Init(FusionVoicePool* pool, char id, bool skipDecoders);
    // The map has AssignIDs(FusionSampler const*, KeyzoneSettings const*,
    // unsigned int); this build also passes the voice's pitch shifter and
    // returns whether the voice took the note. At 0x9DE50.
    bool AssignIDs(
        const FusionSampler* sampler,
        const FusionPatchCom::KeyzoneSettings* keyzone,
        unsigned int id,
        SmbPitchShift* pitchShift);
    // A null keyzone matches any. At 0xA0320.
    bool MatchesIDs(
        const FusionSampler* sampler,
        unsigned int id,
        const FusionPatchCom::KeyzoneSettings* keyzone);
    void Release();      // 0x9E2E0
    void FastRelease();  // 0x9E380
    // Silences the voice and returns its pitch shifter to the pool. At
    // 0x9E440.
    void Kill();
    bool IsInUse() const;             // 0x9F120
    bool IsWaitingForAttack() const;  // 0x9F110

    // Envelope stage that marks a released voice. Name not in the reference
    // map.
    static constexpr int kStateReleasing = 3;

    // Field names are not in the reference map. mOpaque arrays are bytes the
    // pool does not use.
    bool mWaitingForAttack;  // Returned by IsWaitingForAttack.
    char mId;                // Set by Init: 'A' plus the pool index.
    unsigned char mOpaque2[10];
    int mNoteId;             // Compared by MatchesIDs; -1 once killed.
    const FusionPatchCom::KeyzoneSettings* mKeyzone;
    const FusionSampler* mSampler;
    unsigned char mOpaque32[32];
    SmbPitchShift* mPitchShift;
    unsigned char mOpaque72[8];
    int mState;
    unsigned int mAge;  // The larger value is stolen first.
    unsigned char mOpaque88[588];
    float mLevels[2];  // The two envelope levels compared when stealing.
    unsigned char mOpaque684[68];
    FusionVoicePool* mPool;  // Set by Init.
    unsigned char mOpaque760[24];
};

static_assert(offsetof(FusionVoice, mNoteId) == 12);
static_assert(offsetof(FusionVoice, mKeyzone) == 16);
static_assert(offsetof(FusionVoice, mSampler) == 24);
static_assert(offsetof(FusionVoice, mPitchShift) == 64);
static_assert(offsetof(FusionVoice, mState) == 80);
static_assert(offsetof(FusionVoice, mAge) == 84);
static_assert(offsetof(FusionVoice, mLevels) == 676);
static_assert(offsetof(FusionVoice, mPool) == 752);
static_assert(sizeof(FusionVoice) == 784);
