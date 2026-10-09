#pragma once

#include <cstddef>

#include "audio/core/dsp/AmpSimulator.h"
#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/dsp/BitCrusher.h"
#include "audio/core/dsp/Delay.h"
#include "audio/core/dsp/DistortionEffect.h"
#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/dsp/Ramper.h"
#include "audio/core/fusion/FusionPatchCom.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/modulation/ADSR.h"
#include "audio/core/modulation/LFO.h"
#include "audio/core/modulation/Modulator.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "audio/core/resources/FusionPatchResource.h"
#include "entity/resources/Resource.h"
#include "math/interp/Interp.h"
#include "utl/containers/Vector.h"

class AudioCpuTimer;
class FusionVoicePool;

// Sampler instrument (audio/FusionSampler.o, 0x95C40 to 0x9D90F) whose
// voices come from a FusionVoicePool. It renders the active patch preset's
// keyzones and runs the preset's distortion, bit crusher, amp model and
// delay over the block. It is abstract; FusionGenerator completes it. The
// vtable is at 0x18E4D68 and the AudioGenerator vtable at 0x18E4F30; the
// object is 0x5470 bytes.
class FusionSampler : public InstrumentGenerator {
public:
    // The map's _SetPortamentoMode takes it. With mode zero the glide ends
    // once no voice plays; FusionPatchCom names the modes "legato" and
    // "persistent". Value names not in the reference map; their order is
    // weak.
    enum PortamentoMode : int {
        kPortamentoLegato = 0,
        kPortamentoPersistent = 1,
    };
    // How a note picks among its matching keyzones: the "keyzone_select_mode"
    // values "layers", "random", "random_with_repetition" and "cycle". Names
    // not in the reference map.
    enum KeyzoneSelectMode : int {
        kSelectLayers = 0,
        kSelectRandom = 1,  // Never the zone the note played last.
        kSelectRandomWithRepetition = 2,
        kSelectCycle = 3,
    };

    // A note-on or note-off waiting for the next block. Name not in the
    // reference map.
    struct NoteAction {
        signed char mVelocity;  // -1 for none, zero for a note-off.
        float mStartOffsetMs;
    };

    FusionSampler();            // 0x95C40
    ~FusionSampler() override;  // slots 0-1: 0x96E20, 0x97370

    // Slot 2 at 0x99BC0. The bus is prepared at a placeholder rate, which
    // SetSampleRate then replaces.
    void Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool allocate) override;
    // Slot 3 at 0x99C10: the control rate and every ramp and LFO.
    void SetSampleRate(float sampleRate) override;
    // Slot 4 at 0x9ACC0. The binary returns whatever the bus unlock leaves.
    bool Process(AudioBuffer<float>& buffer) override;
    bool IsInstrument() override {  // slot 10: 0x43C40
        return true;
    }
    // Slot 12 at 0x43C50: true while no voice is active.
    bool ProcessCallWillProduceSilence() const override {
        return mNumActiveVoices == 0;
    }
    void ResetInstrumentState() override;  // slot 13: 0x9A200
    void ResetMidiState() override;        // slot 14: 0x9A3F0
    // Slot 15 at 0x974A0: queued for the next block; a note keeps its
    // loudest velocity.
    void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) override;
    bool IsNotePlaying(signed char note) override;  // slot 16: 0x97530
    void NoteOff(signed char note, signed char channel) override;  // slot 17: 0x975D0
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
    int GetNumVoicesInUse() const override {
        return mNumActiveVoices;
    }
    void SetMidiChannelVolume(float volumeDb, float fadeSecs, signed char channel) override;  // slot 32: 0x9A600
    float GetMidiChannelVolume(signed char channel) const override;                         // slot 33: 0x9A740
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override;      // slot 34: 0x9A780
    float GetMidiChannelGain(signed char channel) const override;                           // slot 35: 0x9A8D0
    void SetMidiChannelMute(bool mute, signed char channel) override;                       // slot 36: 0x9A8E0
    // Slot 37 at 0x43D10.
    bool GetMidiChannelMute(signed char) const override {
        return mChannelMute;
    }
    // Slots 38-39 and 41 stay pure; FusionGenerator implements them.
    // Slot 40 at 0x43D20: true here and in FusionGenerator.
    bool SupportsAudioThreadClients() const override {
        return true;
    }
    bool AddSlave(unsigned int handle, InstrumentSlaveType type) override;  // slot 42: 0x9CA40
    bool RemoveSlave(unsigned int handle) override;                         // slot 43: 0x9CD90
    // Slots 44-46 at 0x51F40, 0x51F50 and 0x51F60: empty here.
    void ClearGeneratorFlag() override {}
    void SetGeneratorFlag(int) override {}
    bool HasGeneratorFlag() const override {
        return false;
    }
    void SetTranspose(int semitones) override {  // slot 47: 0x43D70
        mTranspose = semitones;
    }
    int GetTranspose() const override {  // slot 48: 0x43D80
        return mTranspose;
    }
    // Slots 49-50 at 0x43D90 and 0x43DA0: a time-stretch mode that replaces
    // the keyzones' for every voice the sampler plays; Get reports whether
    // one is set. FusionVoice hands it to SmbPitchShift::SetTimeStretchMode.
    void SetTimeStretchMode(int algorithm, int formantMode) override {
        mTimeStretchAlgorithm = algorithm;
        mTimeStretchFormantMode = formantMode;
    }
    bool GetTimeStretchMode(int* algorithm, int* formantMode) const override {
        *algorithm = mTimeStretchAlgorithm;
        *formantMode = mTimeStretchFormantMode;
        return (mTimeStretchAlgorithm | mTimeStretchFormantMode) != 0;
    }
    // Slot 51 at 0x975F0; AudioGenerator's slot 8 reaches it through 0x97860.
    // A non-positive speed becomes 0.0001. The flag is unused.
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
    // Releases every slave and empties the slave lists. At 0x97190.
    void _DumpAllInstrumentSlaves();
    // Takes the patch and loads its current preset. The map's version also
    // builds the preset state; this build moves that into _LoadPreset. At
    // 0x98720.
    void LoadPatch(const ResourcePtr<FusionPatchResource>& patch);
    // The patch's path, or the resolved empty path without one. No caller
    // in this build. At 0x9CFA0. Name not in the reference map.
    ResourcePath GetPatchPath() const;
    // Sets a track gain from a MIDI value, as the controllers 52-59 do.
    // Index 8 writes past the eight gains, into FusionGenerator's first
    // field. No caller in this build. At 0x9C9E0. Name not in the reference
    // map.
    void SetTrackGain(int track, signed char value);

    PortamentoMode _GetPortamentoMode() const;  // 0x9A440
    float _GetCurrentPortamentoPitch() const;   // 0x9A450
    float _GetPan() const;                      // 0x9A460
    float _GetFineTuneCents() const;            // 0x9A470
    float _GetMinPitchBendCents() const;        // 0x9A520
    float _GetMaxPitchBendCents() const;        // 0x9A5D0
    float _GetPitchBendFactor() const;          // 0x9A5E0
    float _GetTrimVolume() const;               // 0x9A9A0
    float _GetTrimGain() const;                 // 0x9A9B0
    float _GetStartPointMs() const;             // 0x9A9C0
    float _GetPortamentoTime() const;           // 0x97840, in seconds

    void _SetPortamentoTime(float seconds);  // 0x97760
    void _SetTrimVolume(float db);           // 0x99430
    void _SetMinPitchBendCents(float cents);  // 0x99510
    void _SetMaxPitchBendCents(float cents);  // 0x99630
    void _SetMaxNumVoices(int count);        // 0x99750
    void _SetStartPointMs(float ms);         // 0x997A0
    void _SetFineTuneCents(float cents);     // 0x99830
    void _SetPan(float pan);                 // 0x998C0
    void _SetPortamentoIsEnabled(bool enabled);  // 0x999B0
    void _SetPortamentoMode(PortamentoMode mode);  // 0x99A00
    // Prepares or frees the delay line. At 0x99A50.
    void _SetDelayEnabled(bool enabled);
    // Enabling clears the crusher's history. At 0x99B80.
    void _SetBitCrusherEnabled(bool enabled);
    // The expression (CC11) gain, ramped over at least kMinGainRampMs. At
    // 0x9A2C0.
    void _SetMidiExpressionGain(float gain, float fadeSecs, signed char channel);

    // The target of a modulator's "target" property. At 0x986F0.
    ModulatorTarget* _GetModulatorTargetByType(ModulatorTarget::Target target);
    // At 0x9A1A0.
    ModulatorTarget* _GetModulatorTargetByName(const char* name);
    // At 0x98690.
    void _ApplyModulatorSettings(const Modulator::Settings* settings, Modulator& modulator);

private:
    // Restores the defaults that a patch preset overrides. The constructor
    // and LoadPatch call it. At 0x96610.
    void ResetPatchRelatedState();
    // Applies the patch's current preset, clamping the preset index. Split
    // out of the map's LoadPatch. At 0x987E0. Name not in the reference map.
    void _LoadPreset();
    // Re-binds the LFO settings of every voice the sampler plays. At
    // 0x97920.
    void _UpdateVoiceLFOS();
    void _ResetNoteActions();    // 0x97A20
    void _ProcessNoteActions();  // 0x97A90
    // Releases the voices that play the note. At 0x97CB0.
    void _ProcessNoteOff(unsigned char note);
    // Starts the keyzones the note selects. The map has
    // _ProcessNoteOn(unsigned char, unsigned char, float); this build adds
    // whether the note-on or the note-off keyzones play. At 0x97D50.
    void _ProcessNoteOn(unsigned char note, unsigned char velocity, bool noteOn, float startOffsetMs);
    // Collects the indices of the keyzones that would play. No caller in
    // this build: _ProcessNoteOn inlines it. At 0x981F0. Name not in the
    // reference map.
    int _FindKeyzones(
        unsigned char key,
        unsigned char velocity,
        bool noteOn,
        const FusionKeyzoneArray& keyzones,
        unsigned short* indices);
    // Starts one keyzone on a free voice. The map has
    // _TryKeyOnZone(unsigned char, unsigned char, float, KeyzoneSettings
    // const*); this build passes both the note and the transposed key. At
    // 0x982D0.
    bool _TryKeyOnZone(
        unsigned char note,
        unsigned char key,
        unsigned char velocity,
        float startOffsetMs,
        const FusionPatchCom::KeyzoneSettings* keyzone);
    // Adds a slave to the list its type selects, under the bus lock. At
    // 0x9CB90. Name not in the reference map.
    void _AddSlaveGenerator(InstrumentGenerator* slave, InstrumentSlaveType type);
    // Removes a slave from both lists; true when it was in one. At 0x9CE90.
    // Name not in the reference map.
    bool _RemoveSlaveGenerator(InstrumentGenerator* slave);
    // Advances the control ramps and the LFOs by a block. At 0x9A9D0.
    void _PrepareToProcess(unsigned int numFrames);
    // Sets the portamento time and the pitch ramp's length, which the speed
    // shortens. Inlined into _SetPortamentoTime, SetSpeed, SetSampleRate,
    // ResetPatchRelatedState, _LoadPreset and SetController. Name not in the
    // reference map.
    void _SetPortamentoTimeMs(float ms) {
        ScopedCritSecPtr lock(GetBusLock());
        mPortamentoTimeMs.Set(ms);
        mPortamentoPitch.SetRateMs(mControlRate, mPortamentoTimeMs.mValue / mSpeed);
    }
    // The pitch bend's playback-rate factor; the bend setters and
    // _PrepareToProcess inline it. At 0x9A530.
    void _UpdatePitchBendFactor();

public:
    // Guards mNoteActions, which the game thread fills and the audio thread
    // drains. 0x19C8EB0.
    static CritSec sNoteActionCritSec;

    // Field names are not in the reference map.
    // The sums the patch's modulators add to for each note: a start offset
    // in milliseconds and a pitch offset in cents.
    float mStartPointModulation;
    float mPitchModulation;
    ModulatorTarget mStartPointTarget;
    ModulatorTarget mPitchTarget;
    float mControlRate;  // Blocks per second.
    float mSpeed;  // GetSpeed's value; SetSpeed keeps it positive.
    // The song position in beats, from SetBeat. FusionVoice keeps tempo-synced
    // keyzones on it.
    float mBeat;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    SPL::Parameter mChannelVolume;  // dB.
    SPL::Ramper mChannelGain;  // GetMidiChannelGain's value.
    bool mChannelMute;
    SPL::Ramper mMuteGain;  // Ramps to zero while the channel is muted.
    SPL::Parameter mTrimVolume;  // dB, the preset's "volume".
    float mTrimGain;
    SPL::Parameter mExpression;
    SPL::Ramper mExpressionGain;
    SPL::Parameter mPan;  // The set pan; mPanRamp follows it.
    SPL::Ramper mPanRamp;
    // Portamento makes the sampler monophonic; voices then follow
    // mPortamentoPitch.
    bool mPortamentoEnabled;
    PortamentoMode mPortamentoMode;
    SPL::Parameter mPortamentoTimeMs;
    // Set by the first note of a glide; until then a note jumps to its
    // pitch.
    bool mPortamentoActive;
    SPL::Ramper mPortamentoPitch;  // Semitones.
    SPL::Ramper mPitchBend;        // GetPitchBend returns its target.
    float mExtraPitchBend;  // Semitones.
    float mPitchBendFactor;  // Playback-rate factor of the pitch bends.
    SPL::Parameter mFineTuneCents;
    SPL::Parameter mStartPointMs;
    BiquadFilter::Settings mFilterSettings;
    // The preset's envelopes and LFOs: two each, created by the constructor.
    // Element 0 of the envelopes is the voice volume envelope.
    eastl::vector<ADSR::Settings> mADSRSettings;
    eastl::vector<LFO::Settings> mLFOSettings;
    // Free-running LFOs whose phase non-retriggered voice LFOs take.
    eastl::vector<LFO> mLFOs;
    // The preset's modulators: the first two take a random value for each
    // note ("randomizers"), the other two its velocity ("velocity_mods").
    eastl::vector<Modulator> mRandomModulators;
    eastl::vector<Modulator> mVelocityModulators;
    int mMaxNumVoices;
    int mNumActiveVoices;  // Raised by FusionVoice::AssignIDs.
    SPL::Parameter mMinPitchBendCents;
    SPL::Parameter mMaxPitchBendCents;
    FusionVoicePool* mVoicePool;
    Delay mDelay;
    bool mDelayEnabled;
    BitCrusher mBitCrusher;
    bool mBitCrusherEnabled;
    DistortionEffect mDistortion;
    bool mDistortionEnabled;
    AmpSimulator mAmpSimulator;
    bool mAmpEnabled;
    // The amp's EQ: three biquads run on the mono amp output.
    BiquadFilter::Coefs mAmpEqCoefs[3];
    double mAmpEqHistory[3][2];
    bool mAmpEqEnabled[3];
    NoteAction mNoteActions[128];
    bool mNoteActionsPending;
    ResourcePtr<FusionPatchResource> mPatch;
    FusionPatchCom* mPatchCom;
    // The candidate index each note last played, for the random and cycle
    // selections, for note-on and note-off keyzones.
    signed char mLastNoteOnZones[128];
    signed char mLastNoteOffZones[128];
    // The velocity of each held note; its release plays the note-off
    // keyzones at it.
    signed char mHeldVelocities[128];
    // The voices' output for one block, and the channel pointers to it.
    float mScratch[2][2048];
    float* mScratchChannels[2];
    // The slave generators: their handles, and the slaves mixed before and
    // after the effects.
    eastl::vector<unsigned int> mSlaveHandles;
    eastl::vector<InstrumentGenerator*> mPreEffectSlaves;
    eastl::vector<InstrumentGenerator*> mPostEffectSlaves;
    float mTempo;      // BPM, from SetTempo.
    float mPlayScale;  // GetPlayScale's value.
    int mTranspose;    // Semitones added to each note-on.
    unsigned long mPresetIndex;  // Chosen by bank select.
    KeyzoneSelectMode mKeyzoneSelectMode;
    // The default render target's "fusion" timer, run around each block.
    AudioCpuTimer* mCpuTimer;
    // Gains of the sample tracks, set by the controllers 52-59 and read by
    // the voices' decoders. The name is weak.
    float mTrackGains[8];
};

static_assert(offsetof(FusionSampler, mStartPointModulation) == 0x188);
static_assert(offsetof(FusionSampler, mStartPointTarget) == 0x190);
static_assert(offsetof(FusionSampler, mPitchTarget) == 0x1B8);
static_assert(offsetof(FusionSampler, mControlRate) == 0x1E0);
static_assert(offsetof(FusionSampler, mSpeed) == 0x1E4);
static_assert(offsetof(FusionSampler, mBeat) == 0x1E8);
static_assert(offsetof(FusionSampler, mTimeStretchAlgorithm) == 0x1EC);
static_assert(offsetof(FusionSampler, mChannelVolume) == 0x1F8);
static_assert(offsetof(FusionSampler, mChannelGain) == 0x218);
static_assert(offsetof(FusionSampler, mChannelMute) == 0x248);
static_assert(offsetof(FusionSampler, mMuteGain) == 0x250);
static_assert(offsetof(FusionSampler, mTrimVolume) == 0x280);
static_assert(offsetof(FusionSampler, mTrimGain) == 0x2A0);
static_assert(offsetof(FusionSampler, mExpression) == 0x2A8);
static_assert(offsetof(FusionSampler, mExpressionGain) == 0x2C8);
static_assert(offsetof(FusionSampler, mPan) == 0x2F8);
static_assert(offsetof(FusionSampler, mPanRamp) == 0x318);
static_assert(offsetof(FusionSampler, mPortamentoEnabled) == 0x348);
static_assert(offsetof(FusionSampler, mPortamentoMode) == 0x34C);
static_assert(offsetof(FusionSampler, mPortamentoTimeMs) == 0x350);
static_assert(offsetof(FusionSampler, mPortamentoActive) == 0x370);
static_assert(offsetof(FusionSampler, mPortamentoPitch) == 0x378);
static_assert(offsetof(FusionSampler, mPitchBend) == 0x3A8);
static_assert(offsetof(FusionSampler, mExtraPitchBend) == 0x3D8);
static_assert(offsetof(FusionSampler, mFineTuneCents) == 0x3E0);
static_assert(offsetof(FusionSampler, mStartPointMs) == 0x400);
static_assert(offsetof(FusionSampler, mFilterSettings) == 0x420);
static_assert(offsetof(FusionSampler, mADSRSettings) == 0x430);
static_assert(offsetof(FusionSampler, mLFOSettings) == 0x450);
static_assert(offsetof(FusionSampler, mLFOs) == 0x470);
static_assert(offsetof(FusionSampler, mRandomModulators) == 0x490);
static_assert(offsetof(FusionSampler, mVelocityModulators) == 0x4B0);
static_assert(offsetof(FusionSampler, mMaxNumVoices) == 0x4D0);
static_assert(offsetof(FusionSampler, mNumActiveVoices) == 0x4D4);
static_assert(offsetof(FusionSampler, mMinPitchBendCents) == 0x4D8);
static_assert(offsetof(FusionSampler, mMaxPitchBendCents) == 0x4F8);
static_assert(offsetof(FusionSampler, mVoicePool) == 0x518);
static_assert(offsetof(FusionSampler, mDelay) == 0x520);
static_assert(offsetof(FusionSampler, mDelayEnabled) == 0x720);
static_assert(offsetof(FusionSampler, mBitCrusher) == 0x728);
static_assert(offsetof(FusionSampler, mBitCrusherEnabled) == 0x7D8);
static_assert(offsetof(FusionSampler, mDistortion) == 0x7E0);
static_assert(offsetof(FusionSampler, mDistortionEnabled) == 0xD18);
static_assert(offsetof(FusionSampler, mAmpSimulator) == 0xD20);
static_assert(offsetof(FusionSampler, mAmpEnabled) == 0xD58);
static_assert(offsetof(FusionSampler, mAmpEqCoefs) == 0xD60);
static_assert(offsetof(FusionSampler, mAmpEqHistory) == 0xDF0);
static_assert(offsetof(FusionSampler, mAmpEqEnabled) == 0xE20);
static_assert(offsetof(FusionSampler, mNoteActions) == 0xE24);
static_assert(offsetof(FusionSampler, mNoteActionsPending) == 0x1224);
static_assert(offsetof(FusionSampler, mPatch) == 0x1228);
static_assert(offsetof(FusionSampler, mPatchCom) == 0x1230);
static_assert(offsetof(FusionSampler, mLastNoteOnZones) == 0x1238);
static_assert(offsetof(FusionSampler, mLastNoteOffZones) == 0x12B8);
static_assert(offsetof(FusionSampler, mHeldVelocities) == 0x1338);
static_assert(offsetof(FusionSampler, mScratch) == 0x13B8);
static_assert(offsetof(FusionSampler, mScratchChannels) == 0x53B8);
static_assert(offsetof(FusionSampler, mSlaveHandles) == 0x53C8);
static_assert(offsetof(FusionSampler, mPreEffectSlaves) == 0x53E8);
static_assert(offsetof(FusionSampler, mPostEffectSlaves) == 0x5408);
static_assert(offsetof(FusionSampler, mTempo) == 0x5428);
static_assert(offsetof(FusionSampler, mPlayScale) == 0x542C);
static_assert(offsetof(FusionSampler, mTranspose) == 0x5430);
static_assert(offsetof(FusionSampler, mPresetIndex) == 0x5438);
static_assert(offsetof(FusionSampler, mKeyzoneSelectMode) == 0x5440);
static_assert(offsetof(FusionSampler, mCpuTimer) == 0x5448);
static_assert(offsetof(FusionSampler, mTrackGains) == 0x5450);
static_assert(sizeof(FusionSampler) == 0x5470);
