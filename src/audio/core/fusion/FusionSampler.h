#pragma once

#include "audio/core/output/AudioBus.h"

class FusionVoicePool;

// Sampler instrument (audio/FusionSampler.o) whose voices come from a
// FusionVoicePool. It renders as an AudioBus and takes MIDI through the
// VirtualInstrument interface of the map. The class has not been
// reconstructed; its vtable at 0x18E4D68 is declared so the call
// FusionVoicePool makes uses the recovered offset. Slots 0-10 are AudioBus's.
// The slot names come from the map's VirtualInstrument and FusionSampler
// members matched by behaviour; where noted the match is a guess.
class FusionSampler : public AudioBus {
public:
    // Slot 11, empty: VirtualInstrument's default. The map has SetTempo and
    // SetBeat there; which one is a guess.
    virtual void SetTempo(float tempo);
    // Slot 12 at 0x43C50: true while no voice is active.
    virtual bool ProcessCallWillProduceSilence() const;
    // Slot 13 at 0x9A200. Matched by its patch reset; a guess.
    virtual void ResetPatchRelatedState();
    // Slot 14 at 0x9A3F0: all notes off, then the pitch bend reset.
    virtual void ResetInstrumentState();
    virtual void NoteOn(signed char note, signed char velocity, signed char channel, float detune);  // slot 15: 0x974A0
    virtual void NoteOff(signed char note, signed char channel);  // slot 16: 0x97530
    // Slot 17 at 0x975D0: a note-on of zero velocity. Name not in the
    // reference map.
    virtual void ReleaseNote(signed char note);
    virtual void SetExtraPitchBend(float bend, signed char channel);  // slot 18: 0x9A5F0
    virtual void SetPitchBend(float bend, signed char channel);       // slot 19: 0x9A490
    virtual float GetPitchBend(signed char channel) const;           // slot 20: 0x9A480
    // Slots 21-24: the controller accessors; the float forms (slots 23-24 at
    // 0x43C60 and 0x43C90) convert to and from the 14-bit forms.
    virtual void SetController(int controller, signed char msb, signed char lsb, signed char channel);
    virtual void GetController(int controller, signed char& msb, signed char& lsb, signed char channel) const;
    virtual void SetController(int controller, float value, signed char channel);
    virtual float GetController(int controller, signed char channel);
    virtual void KillAllVoices();  // slot 25: 0x98570. Matched by elimination; a guess.
    virtual void AllNotesOff();    // slot 26: 0x98630
    // Slots 27-28 at 0x979D0 and 0x97870 take one float each. The map has
    // SetSpeed and SetBeat; the order is a guess.
    virtual void SetSpeed(float speed);
    virtual void SetBeat(float beat);
    // Slot 29 at 0x67080: dispatches on the status nibble.
    virtual void HandleMidiMessage(signed char status, signed char data1, signed char data2, float detune);
    // Slot 30 at 0x9A420: one while the flag at +0x348 is set, otherwise the
    // voice limit at +0x4D0.
    virtual int GetMaxNumVoices() const;

    // Drops the sampler's reference to a pool being destroyed. At 0x973C0.
    void VoicePoolWillDestruct(const FusionVoicePool* pool);
};
