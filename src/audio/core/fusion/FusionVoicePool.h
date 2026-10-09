#pragma once

#include <cstddef>
#include <vector>

#include "audio/core/fusion/FusionPatchCom.h"
#include "os/memory/PoolAlloc.h"
#include "os/threading/CritSec.h"
#include "utl/containers/List.h"

class FusionSampler;
class FusionVoice;
class SmbPitchShift;

// Voices shared by the Fusion samplers that play through one render target
// (audio/FusionVoicePool.o). Samplers register as clients; the voices and the
// pitch shifters exist while the pool has clients, or always once the
// creation is no longer deferred. The object is 128 bytes.
class FusionVoicePool {
public:
    POOL_OVERLOAD(FusionVoicePool)

    // A pitch shifter and whether a voice holds it. Name not in the
    // reference map.
    struct PitchShiftSlot {
        explicit PitchShiftSlot(SmbPitchShift* pitchShift) : mInUse(false), mPitchShift(pitchShift) {}
        PitchShiftSlot(PitchShiftSlot&& other) : mInUse(other.mInUse), mPitchShift(other.mPitchShift) {
            other.mPitchShift = nullptr;
            other.mInUse = false;
        }
        PitchShiftSlot(const PitchShiftSlot&) = delete;
        PitchShiftSlot& operator=(const PitchShiftSlot&) = delete;
        // Out of line at 0xA0400.
        ~PitchShiftSlot();

        bool mInUse;
        SmbPitchShift* mPitchShift;
    };

    FusionVoicePool();  // 0xA0430
    // The map's constructor takes no arguments; this overload, which
    // AudioRenderTarget uses, passes the sample rate. Name not in the
    // reference map.
    explicit FusionVoicePool(float sampleRate);  // 0xA07B0
    ~FusionVoicePool();                          // 0xA08A0

    // Rebuilds the voices at a new rate. At 0xA0520. Name not in the
    // reference map.
    void SetSampleRate(float sampleRate);
    void KillVoices();  // 0xA05F0
    // Frees the voices and the pitch shifters. At 0xA0660. Name not in the
    // reference map.
    void _DestroyVoices();
    // Creates the voices and the pitch shifters at the configured counts.
    // At 0xA0750. Name not in the reference map.
    void _CreateAllVoices();
    // Entry points for FusionVoice, which holds the pool. At 0xA0780 and
    // 0xA07A0. Names not in the reference map.
    void Lock();
    void Unlock();
    // Creates the voices when their creation is still deferred. At 0xA0B00.
    // Name not in the reference map.
    void CreatePendingVoices();
    // Defers the creation again, freeing the voices while the pool has no
    // clients. At 0xA0B40. Name not in the reference map.
    void DeferVoiceCreation();
    void AddClient(FusionSampler* sampler);     // 0xA0B90
    void RemoveClient(FusionSampler* sampler);  // 0xA0C50
    // Also records the peak. At 0xA0CF0.
    unsigned int GetNumVoicesInUse();
    // Clamped to 1 and the hard limit.
    void SetSoftVoiceLimit(unsigned int limit);  // 0xA0D70
    // Fast-releases the voices over the sampler's limit, or over the soft
    // limit for a null sampler, and returns the number considered. The
    // map gives no return type. At 0xA0DE0.
    unsigned int FastReleaseExcessVoices(FusionSampler* sampler);
    // Clamped to 1..256; lowers the soft limit to match.
    void SetHardVoiceLimit(unsigned int limit);  // 0xA1010
    void _CreateVoices(unsigned short numVoices);  // 0xA1090
    // Stores the flag every voice receives in FusionVoice::Init; a voice
    // given it creates no Mogg or XMA decoders of its own. At 0xA1250. Name
    // not in the reference map.
    void SetSkipVoiceDecoders(bool skip);
    // Stores the shifter count and rebuilds the shifters while the pool has
    // clients. At 0xA1260. Name not in the reference map.
    void SetNumPitchShifters(int count);
    // Kills the voices and replaces the shifters. At 0xA12C0. Name not in
    // the reference map.
    void _CreatePitchShifters(unsigned short count);
    // Takes and returns a free shifter. At 0xA14C0 and 0xA1540. Names not in
    // the reference map.
    SmbPitchShift* AcquirePitchShift();
    void ReleasePitchShift(SmbPitchShift* pitchShift);
    // Picks a voice for a keyzone, stealing the least important one when
    // none is free, and releases the voices already playing the same id.
    // At 0xA15C0.
    FusionVoice* GetFreeVoice(
        const FusionSampler* sampler,
        unsigned int id,
        const FusionPatchCom::KeyzoneSettings* keyzone);
    // Whether a voice in use plays the sampler's id. At 0xA1990. Name not in
    // the reference map.
    bool IsPlaying(const FusionSampler* sampler, unsigned char id);
    void KillVoices(const FusionSampler* sampler);                     // 0xA1A10
    void KillVoices(const FusionPatchCom::KeyzoneSettings* keyzone);  // 0xA1AA0

    // Field names are not in the reference map.
    FusionVoice* mVoices;
    std::vector<PitchShiftSlot> mPitchShifts;
    unsigned int mNumVoices;
    unsigned int mHardVoiceLimit;
    unsigned int mNumPitchShifts;
    unsigned int mNumFreePitchShifts;
    int mPitchShiftCount;  // Requested through SetNumPitchShifters.
    unsigned int mSoftVoiceLimit;
    unsigned int mPeakVoicesInUse;
    bool mSkipVoiceDecoders;
    eastl::list<FusionSampler*> mClients;
    CritSec mCritSec;
    float mSampleRate;
    bool mVoicesPending;
};

static_assert(sizeof(std::vector<FusionVoicePool::PitchShiftSlot>) == 32);
static_assert(offsetof(FusionVoicePool, mPitchShifts) == 8);
static_assert(offsetof(FusionVoicePool, mNumVoices) == 40);
static_assert(offsetof(FusionVoicePool, mNumPitchShifts) == 48);
static_assert(offsetof(FusionVoicePool, mPitchShiftCount) == 56);
static_assert(offsetof(FusionVoicePool, mPeakVoicesInUse) == 64);
static_assert(offsetof(FusionVoicePool, mSkipVoiceDecoders) == 68);
static_assert(offsetof(FusionVoicePool, mClients) == 72);
static_assert(offsetof(FusionVoicePool, mCritSec) == 104);
static_assert(offsetof(FusionVoicePool, mSampleRate) == 120);
static_assert(offsetof(FusionVoicePool, mVoicesPending) == 124);
static_assert(sizeof(FusionVoicePool) == 128);
