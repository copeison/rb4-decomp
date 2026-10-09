#pragma once

#include <cstddef>

#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/dsp/Ramper.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/VirtualInstrument.h"
#include "audio/core/modulation/ADSR.h"
#include "audio/core/modulation/LFO.h"
#include "audio/core/resources/Resource.h"
#include "utl/containers/Vector.h"

class AudioBusCallable;
class FusionPatchResource;
class FusionVoicePool;

// How a slave instrument follows its master; the map's AddSlave takes it.
// Values not modelled.
enum InstrumentSlaveType : int {};

// Sampler instrument (audio/FusionSampler.o) whose voices come from a
// FusionVoicePool. It is a VirtualInstrument and, at +312, an
// AudioGenerator; it is abstract, and FusionGenerator completes it. The class
// is only partly reconstructed: its vtable at 0x18E4D68 is declared so that
// FusionVoice and FusionVoicePool call the recovered slots, and the members
// those classes use are named. Slots 0-37 are VirtualInstrument's.
//
// Slots 51-54 also override AudioGenerator's SetSpeed, GetSpeed,
// SetPlayScale and GetPlayScale (secondary slots 8, 9, 25 and 26). The
// sampler's IsInstrument override (secondary slot 17, 0x43FC0) gets no
// primary slot, so it also overrides a primary-base function: AudioBus's
// slot 10, which AudioBus.h names IsVirtualInstrument, is likely
// IsInstrument. FusionSampler's slot 10 is 0x43C40 (true).
class FusionSampler : public VirtualInstrument, public AudioGenerator {
public:
    // The map's _SetPortamentoMode takes it. Values not modelled.
    enum PortamentoMode : int {};

    FusionSampler();            // 0x95C40
    ~FusionSampler() override;  // slots 0-1: 0x96E20, 0x97370

    // Slot 2 at 0x99BC0.
    void Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool allocate) override;
    void SetSampleRate(float sampleRate) override;    // slot 3: 0x99C10
    bool Process(AudioBuffer<float>& buffer) override;  // slot 4: 0x9ACC0
    bool IsVirtualInstrument() override {  // slot 10: 0x43C40
        return true;
    }
    // Slot 12 at 0x43C50: true while no voice is active.
    bool ProcessCallWillProduceSilence() const override {
        return mNumActiveVoices == 0;
    }
    void ResetPatchRelatedState() override;  // slot 13: 0x9A200
    // Slot 14 at 0x9A3F0: all notes off, then the pitch bend reset.
    void ResetInstrumentState() override;
    void NoteOn(signed char note, signed char velocity, signed char channel, float detune) override;  // slot 15: 0x974A0
    void NoteOff(signed char note, signed char channel) override;  // slot 16: 0x97530
    void ReleaseNote(signed char note) override;                    // slot 17: 0x975D0
    void SetExtraPitchBend(float bend, signed char channel) override;  // slot 18: 0x9A5F0
    void SetPitchBend(float bend, signed char channel) override;       // slot 19: 0x9A490
    float GetPitchBend(signed char channel) const override;           // slot 20: 0x9A480
    void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) override;  // slot 21: 0x9C060
    void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const override;  // slot 22: 0x9BE10
    // Slots 23-24 stay VirtualInstrument's.
    using VirtualInstrument::GetController;
    using VirtualInstrument::SetController;
    void KillAllVoices() override;  // slot 25: 0x98570
    void AllNotesOff() override;    // slot 26: 0x98630
    void SetBeat(float beat) override;    // slot 27: 0x979D0
    void SetTempo(float tempo) override;  // slot 28: 0x97870
    // Slot 30 at 0x9A420: one while portamento is on, otherwise the voice
    // limit.
    int GetMaxNumVoices() const override;
    // Slot 31 at 0x43D00.
    int GetNumActiveVoices() const override {
        return mNumActiveVoices;
    }
    void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) override;  // slot 32: 0x9A600
    float GetMidiChannelVolume(signed char channel) const override;                       // slot 33: 0x9A740
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override;    // slot 34: 0x9A780
    float GetMidiChannelGain(signed char channel) const override;                         // slot 35: 0x9A8D0
    void SetMidiChannelMute(bool mute, signed char channel) override;                     // slot 36: 0x9A8E0
    // Slot 37 at 0x43D10.
    bool GetMidiChannelMute(signed char) const override {
        return mChannelMute;
    }
    // Slots 38-39, implemented by FusionGenerator (0x42840, 0x428B0): link
    // and unlink a client in its list. The map has these names on
    // FusionGenerator.
    virtual void AddAudioThreadClient(AudioBusCallable* client) = 0;
    virtual void RemoveAudioThreadClient(AudioBusCallable* client) = 0;
    // Slot 40 at 0x43D20: true here and in FusionGenerator. Name not in the
    // reference map; it rests on the slot's position after the client
    // members and is weak.
    virtual bool SupportsAudioThreadClients() const {
        return true;
    }
    // Slot 41, implemented by FusionGenerator as a jump to LoadPatch. Name
    // not in the reference map.
    virtual void SetPatch(const ResourcePtr<FusionPatchResource>& patch) = 0;
    virtual void AddSlave(unsigned int handle, InstrumentSlaveType type);  // slot 42: 0x9CA40
    virtual void RemoveSlave(unsigned int handle);                         // slot 43: 0x9CD90
    // Slots 44-46 (0x51F40, 0x51F50, 0x51F60) are empty here;
    // FusionGenerator clears, stores and tests an int with them. Names not
    // in the reference map; behaviour only, the evidence is weak.
    virtual void ClearGeneratorFlag() {}
    virtual void SetGeneratorFlag(int) {}
    virtual bool HasGeneratorFlag() const {
        return false;
    }
    // Slots 47-48 at 0x43D70 and 0x43D80. Names not in the reference map;
    // behaviour only, the evidence is weak.
    virtual void SetInstrumentTag(int tag) {
        mInstrumentTag = tag;
    }
    virtual int GetInstrumentTag() const {
        return mInstrumentTag;
    }
    // Slots 49-50 at 0x43D90 and 0x43DA0: a time-stretch mode that replaces
    // the keyzones' for every voice the sampler plays; Get reports whether
    // one is set. FusionVoice hands it to SmbPitchShift::SetTimeStretchMode. Names not
    // in the reference map.
    virtual void SetTimeStretchMode(int algorithm, int formantMode) {
        mTimeStretchAlgorithm = algorithm;
        mTimeStretchFormantMode = formantMode;
    }
    virtual bool GetTimeStretchMode(int* algorithm, int* formantMode) const {
        *algorithm = mTimeStretchAlgorithm;
        *formantMode = mTimeStretchFormantMode;
        return (mTimeStretchAlgorithm | mTimeStretchFormantMode) != 0;
    }
    // Slot 51 at 0x975F0; AudioGenerator's slot 8 reaches it through 0x97860.
    // The flag is unused.
    void SetSpeed(float speed, bool immediate) override;
    // Slot 52 at 0x43DC0. The speed never ramps.
    float GetSpeed(bool* changing) override {
        if (changing != nullptr) {
            *changing = false;
        }
        return mSpeed;
    }
    void SetPlayScale(float scale) override {  // slot 53: 0x43DE0
        mPlayScale = scale;
    }
    float GetPlayScale() override {  // slot 54: 0x43DF0
        return mPlayScale;
    }

    // Binds the sampler to a voice pool, first registering with the new one
    // and then killing its voices in the old one. At 0x972C0.
    void SetVoicePool(FusionVoicePool* pool);
    // Drops the sampler's reference to a pool being destroyed. At 0x973C0.
    void VoicePoolWillDestruct(const FusionVoicePool* pool);
    void _DumpAllInstrumentSlaves();  // 0x97190
    // The map's version also builds the patch state; this build splits that
    // into helpers. At 0x98720.
    void LoadPatch(const ResourcePtr<FusionPatchResource>& patch);

    PortamentoMode _GetPortamentoMode() const;  // 0x9A440
    float _GetCurrentPortamentoPitch() const;   // 0x9A450
    float _GetPan() const;                      // 0x9A460
    float _GetFineTuneCents() const;            // 0x9A470
    float _GetPitchBendFactor() const;          // 0x9A5E0
    float _GetTrimVolume() const;               // 0x9A9A0
    float _GetTrimGain() const;                 // 0x9A9B0
    float _GetStartPointMs() const;             // 0x9A9C0

    // Field names are not in the reference map. mOpaque arrays are bytes the
    // reconstructed code does not use.
    unsigned char mOpaque392[92];
    float mSpeed;  // GetSpeed's value; SetSpeed keeps it positive.
    // The song position in beats, from SetBeat. FusionVoice keeps tempo-synced
    // keyzones on it.
    float mBeat;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    unsigned char mOpaque500[36];
    SPL::Ramper mChannelGain;  // GetMidiChannelGain's value.
    bool mChannelMute;
    SPL::Ramper mMuteGain;  // Ramps to zero while the channel is muted.
    unsigned char mOpaque640[12];
    float mTrimVolume;
    unsigned char mOpaque656[16];
    float mTrimGain;
    unsigned char mOpaque676[36];
    SPL::Ramper mExpressionGain;
    unsigned char mOpaque760[12];
    float mPan;  // The set pan; mPanRamp follows it.
    unsigned char mOpaque776[16];
    SPL::Ramper mPanRamp;
    // Portamento makes the sampler monophonic; voices then follow
    // mPortamentoPitch.
    bool mPortamentoEnabled;
    PortamentoMode mPortamentoMode;
    unsigned char mOpaque848[40];
    SPL::Ramper mPortamentoPitch;  // Semitones.
    SPL::Ramper mPitchBend;        // GetPitchBend returns its target.
    float mExtraPitchBend;
    float mPitchBendFactor;  // Playback-rate factor of the pitch bends.
    unsigned char mOpaque992[12];
    float mFineTuneCents;
    unsigned char mOpaque1008[28];
    float mStartPointMs;
    unsigned char mOpaque1040[16];
    BiquadFilter::Settings mFilterSettings;
    // The patch's envelopes and LFOs: two each, created by the constructor.
    // Element 0 of the envelopes is the voice volume envelope.
    eastl::vector<ADSR::Settings> mADSRSettings;
    eastl::vector<LFO::Settings> mLFOSettings;
    // Free-running LFOs whose phase non-retriggered voice LFOs take.
    eastl::vector<LFO> mLFOs;
    unsigned char mOpaque1168[64];
    int mMaxNumVoices;
    int mNumActiveVoices;  // Raised by FusionVoice::AssignIDs.
    unsigned char mOpaque1240[64];
    FusionVoicePool* mVoicePool;
    unsigned char mOpaque1312[20232];
    float mTempo;      // BPM, from SetTempo.
    float mPlayScale;  // GetPlayScale's value.
    int mInstrumentTag;
};

static_assert(offsetof(FusionSampler, mOpaque392) == 392);
static_assert(offsetof(FusionSampler, mSpeed) == 0x1E4);
static_assert(offsetof(FusionSampler, mBeat) == 0x1E8);
static_assert(offsetof(FusionSampler, mTimeStretchAlgorithm) == 0x1EC);
static_assert(offsetof(FusionSampler, mChannelGain) == 0x218);
static_assert(offsetof(FusionSampler, mChannelMute) == 0x248);
static_assert(offsetof(FusionSampler, mMuteGain) == 0x250);
static_assert(offsetof(FusionSampler, mTrimVolume) == 0x28C);
static_assert(offsetof(FusionSampler, mTrimGain) == 0x2A0);
static_assert(offsetof(FusionSampler, mExpressionGain) == 0x2C8);
static_assert(offsetof(FusionSampler, mPan) == 0x304);
static_assert(offsetof(FusionSampler, mPanRamp) == 0x318);
static_assert(offsetof(FusionSampler, mPortamentoEnabled) == 0x348);
static_assert(offsetof(FusionSampler, mPortamentoMode) == 0x34C);
static_assert(offsetof(FusionSampler, mPortamentoPitch) == 0x378);
static_assert(offsetof(FusionSampler, mPitchBend) == 0x3A8);
static_assert(offsetof(FusionSampler, mExtraPitchBend) == 0x3D8);
static_assert(offsetof(FusionSampler, mFineTuneCents) == 0x3EC);
static_assert(offsetof(FusionSampler, mStartPointMs) == 0x40C);
static_assert(offsetof(FusionSampler, mFilterSettings) == 0x420);
static_assert(offsetof(FusionSampler, mADSRSettings) == 0x430);
static_assert(offsetof(FusionSampler, mLFOSettings) == 0x450);
static_assert(offsetof(FusionSampler, mLFOs) == 0x470);
static_assert(offsetof(FusionSampler, mMaxNumVoices) == 0x4D0);
static_assert(offsetof(FusionSampler, mNumActiveVoices) == 0x4D4);
static_assert(offsetof(FusionSampler, mVoicePool) == 0x518);
static_assert(offsetof(FusionSampler, mTempo) == 0x5428);
static_assert(offsetof(FusionSampler, mPlayScale) == 0x542C);
static_assert(offsetof(FusionSampler, mInstrumentTag) == 0x5430);
