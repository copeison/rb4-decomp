#pragma once

#include <cstddef>

#include "audio/core/analysis/Meter.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "os/threading/CritSec.h"
#include "utl/text/Str.h"

// An AudioBus played through MIDI (audio/VirtualInstrument.o). The class has
// not been reconstructed; its vtable at 0x18E2C28 is declared so that
// FusionSampler's slots line up. Slots 0-10 are AudioBus's. Inline defaults
// the base supplies are noted with their addresses. The object is 312 bytes.
class VirtualInstrument : public AudioBus {
public:
    // The controller numbers of SetController: MIDI control changes. Only
    // those FusionSampler handles are named; names not in the reference map.
    enum ControllerID : int {
        kBankSelect = 0,
        kPortamentoTime = 5,
        kVolume = 7,
        kPan = 10,
        kExpression = 11,
        kPortamento = 65,
        kResonance = 71,
        kReleaseTime = 72,
        kAttackTime = 73,
        kBrightness = 74,
    };

    explicit VirtualInstrument(const char* name);  // 0x66DF0
    ~VirtualInstrument() override;                  // slots 0-1: 0x66F40, 0x66FE0

    // Slot 11, empty here (0x51D70). FusionGenerator's override, the map's
    // FusionGenerator::CallPreProcessCallbacks, runs _PrepareToMakeSamples
    // on its audio-thread clients and drops those that return false.
    // FusionSampler::Process calls it before each block.
    virtual void CallPreProcessCallbacks(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock);
    // Slot 12 at 0x51D80. Name not in the reference map.
    virtual bool ProcessCallWillProduceSilence() const;
    // Slot 13. FusionSampler's (0x9A200) sits where the map places
    // ResetInstrumentState among its members.
    virtual void ResetInstrumentState() = 0;
    // Slot 14: all notes off and the pitch bend reset in FusionSampler. Name
    // not in the reference map; it rests on that behaviour.
    virtual void ResetMidiState() = 0;
    // Slot 15. The float is a start offset in milliseconds, which
    // FusionSampler adds to the patch's start point.
    virtual void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) = 0;
    // Slot 16: whether a note-on is pending or a voice plays the note. Name
    // not in the reference map.
    virtual bool IsNotePlaying(signed char note) = 0;
    // Slot 17. HandleMidiMessage sends note-off messages here.
    virtual void NoteOff(signed char note, signed char channel) = 0;
    virtual void SetExtraPitchBend(float bend, signed char channel);  // slot 18: empty here (0x52340)
    virtual void SetPitchBend(float bend, signed char channel) = 0;   // slot 19
    virtual float GetPitchBend(signed char channel) const = 0;        // slot 20
    // Slots 21-24: the controller accessors. The float forms (0x43C60 and
    // 0x43C90) convert to and from the 14-bit forms.
    virtual void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) = 0;
    virtual void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const = 0;
    virtual void SetController(ControllerID controller, float value, signed char channel);
    virtual float GetController(ControllerID controller, signed char channel);
    virtual void KillAllVoices() = 0;  // slot 25. Matched by elimination; a guess.
    virtual void AllNotesOff() = 0;    // slot 26
    virtual void SetBeat(float beat);    // slot 27: empty here (0x51EE0)
    virtual void SetTempo(float tempo);  // slot 28: empty here (0x52350)
    // Slot 29 at 0x67080: dispatches on the status nibble.
    virtual void HandleMidiMessage(signed char status, signed char data1, signed char data2, float startOffsetMs);
    virtual int GetMaxNumVoices() const;  // slot 30 at 0x51EF0
    virtual int GetNumVoicesInUse() const = 0;  // slot 31
    virtual void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) = 0;  // slot 32
    virtual float GetMidiChannelVolume(signed char channel) const = 0;                       // slot 33
    virtual void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) = 0;    // slot 34
    virtual float GetMidiChannelGain(signed char channel) const = 0;                         // slot 35
    virtual void SetMidiChannelMute(bool mute, signed char channel) = 0;                     // slot 36
    // Slot 37. Name not in the reference map.
    virtual bool GetMidiChannelMute(signed char channel) const = 0;

    // Field names are not in the reference map.
    String mName;
    // Runs while FusionSampler::Process renders its voices.
    GeneratorTimer mProcessTimer;
    // No updater is identified; by analogy with mVoiceMeter it is taken to
    // track the processing time, which is weak.
    Meter mProcessMeter;
    // The peak of the voices in use, raised by FusionSampler's note-ons.
    Meter mVoiceMeter;
    CritSec mInstrumentLock;
};

static_assert(offsetof(VirtualInstrument, mName) == 192);
static_assert(offsetof(VirtualInstrument, mProcessTimer) == 208);
static_assert(offsetof(VirtualInstrument, mProcessMeter) == 232);
static_assert(offsetof(VirtualInstrument, mVoiceMeter) == 264);
static_assert(offsetof(VirtualInstrument, mInstrumentLock) == 296);
static_assert(sizeof(VirtualInstrument) == 312);
