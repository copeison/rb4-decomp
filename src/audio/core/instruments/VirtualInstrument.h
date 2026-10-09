#pragma once

#include <cstddef>

#include "audio/core/output/AudioBus.h"
#include "os/threading/CritSec.h"
#include "utl/text/Str.h"

// An AudioBus played through MIDI (audio/VirtualInstrument.o). The class has
// not been reconstructed; its vtable at 0x18E2C28 is declared so that
// FusionSampler's slots line up. Slots 0-10 are AudioBus's. Inline defaults
// the base supplies are noted with their addresses. The object is 312 bytes.
class VirtualInstrument : public AudioBus {
public:
    // The controller numbers of SetController. Values not modelled.
    enum ControllerID : int {};

    explicit VirtualInstrument(const char* name);  // 0x66DF0
    ~VirtualInstrument() override;                  // slots 0-1: 0x66F40, 0x66FE0

    // Slot 11, empty here (0x51D70). FusionGenerator's override hands the
    // message to its listeners and drops those that return false. Name not
    // in the reference map; it rests on that forwarding and is weak.
    virtual void ForwardMidiMessage(
        signed char status, signed char data1, signed char data2, float detune, bool immediate);
    // Slot 12 at 0x51D80. Name not in the reference map.
    virtual bool ProcessCallWillProduceSilence() const;
    // Slot 13. Matched to the map's FusionSampler member by its patch
    // reset; a guess.
    virtual void ResetPatchRelatedState() = 0;
    virtual void ResetInstrumentState() = 0;  // slot 14
    virtual void NoteOn(signed char note, signed char velocity, signed char channel, float detune) = 0;  // slot 15
    virtual void NoteOff(signed char note, signed char channel) = 0;  // slot 16
    // Slot 17: a note-on of zero velocity in FusionSampler. Name not in the
    // reference map.
    virtual void ReleaseNote(signed char note) = 0;
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
    virtual void HandleMidiMessage(signed char status, signed char data1, signed char data2, float detune);
    virtual int GetMaxNumVoices() const;  // slot 30 at 0x51EF0
    // Slot 31. Name not in the reference map.
    virtual int GetNumActiveVoices() const = 0;
    virtual void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) = 0;  // slot 32
    virtual float GetMidiChannelVolume(signed char channel) const = 0;                       // slot 33
    virtual void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) = 0;    // slot 34
    virtual float GetMidiChannelGain(signed char channel) const = 0;                         // slot 35
    virtual void SetMidiChannelMute(bool mute, signed char channel) = 0;                     // slot 36
    // Slot 37. Name not in the reference map.
    virtual bool GetMidiChannelMute(signed char channel) const = 0;

    // Field names are not in the reference map.
    String mName;
    unsigned char mOpaque208[88];  // Not modelled.
    CritSec mInstrumentLock;
};

static_assert(offsetof(VirtualInstrument, mName) == 192);
static_assert(offsetof(VirtualInstrument, mInstrumentLock) == 296);
static_assert(sizeof(VirtualInstrument) == 312);
