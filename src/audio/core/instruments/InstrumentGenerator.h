#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/VirtualInstrument.h"

class AudioBusCallable;
class FusionPatchResource;
template <class T>
class ResourcePtr;

// How a slave instrument follows its master; the map's AddSlave takes it.
// FusionSampler mixes kSlaveBeforeEffects slaves into its own block before
// the patch's effects and every other type after them. Value names not in
// the reference map; only the one FusionSampler tests is named.
enum InstrumentSlaveType : int {
    kSlaveBeforeEffects = 1,
};

// A VirtualInstrument that plays as a pooled AudioGenerator: the map's
// instrument interface, which FusionSampler, MultiFusionGenerator and
// SynthRackGenerator implement and which Scheduler and
// MultiInstrumentGenerator::SetInstrument take. Its own vtable at 0x18E15F0
// has 51 slots: VirtualInstrument's 38 and the 13 below; the
// AudioGenerator vtable at 0x18E1798 has no overrides. The pool
// constructors install both vtables, and the implicit destructor (0x521F0,
// 0x52280) is emitted with the MultiFusion generator. The AudioGenerator
// base is at +312. The defaults below are emitted where they are first
// used; their addresses are noted.
class InstrumentGenerator : public VirtualInstrument, public AudioGenerator {
public:
    explicit InstrumentGenerator(const char* name) : VirtualInstrument(name) {}

    // Slots 38-39: link and unlink a client run before each block. The map
    // has these names on FusionGenerator and MultiInstrumentGenerator.
    virtual void AddAudioThreadClient(AudioBusCallable* client) = 0;
    virtual void RemoveAudioThreadClient(AudioBusCallable* client) = 0;
    // Slot 40 at 0x43D20: true here, and no class overrides it. Name not in
    // the reference map; it rests on the slot's position after the client
    // members and is weak.
    virtual bool SupportsAudioThreadClients() const {
        return true;
    }
    // Slot 41: loads the patch on the channel's instrument; FusionGenerator
    // jumps to FusionSampler::LoadPatch and ignores the channel. The map
    // has the name on FusionGenerator.
    virtual void SetPatch(const ResourcePtr<FusionPatchResource>& patch, int channel) = 0;
    // Slots 42-43 at 0x51F20 and 0x51F30: register a slave generator by
    // handle. False here; FusionSampler keeps slaves.
    virtual bool AddSlave(unsigned int handle, InstrumentSlaveType type) {
        static_cast<void>(handle);
        static_cast<void>(type);
        return false;
    }
    virtual bool RemoveSlave(unsigned int handle) {
        static_cast<void>(handle);
        return false;
    }
    // Slot 44 at 0x51F40: the master dropped this slave
    // (FusionSampler::_DumpAllInstrumentSlaves). Empty here; FusionGenerator
    // forgets its master's handle.
    virtual void DetachedFromMaster() {}
    // Slots 45-46 at 0x51F50 and 0x51F60: FusionGenerator stores and tests
    // its master's handle with them. Empty and false here. Names not in the
    // reference map; they mirror DetachedFromMaster.
    virtual void AttachedToMaster(unsigned int masterHandle) {
        static_cast<void>(masterHandle);
    }
    virtual bool IsAttachedToMaster() const {
        return false;
    }
    // Slots 47-48 at 0x52360 and 0x52370: a transpose in semitones added to
    // each note-on. Ignored and zero here. Names not in the reference map.
    virtual void SetTranspose(int semitones) {
        static_cast<void>(semitones);
    }
    virtual int GetTranspose() const {
        return 0;
    }
    // Slots 49-50 at 0x52380 and 0x52390: a time-stretch mode for every
    // voice; Get reports whether one is set. Ignored and false here. Names
    // not in the reference map.
    virtual void SetTimeStretchMode(int algorithm, int formantMode) {
        static_cast<void>(algorithm);
        static_cast<void>(formantMode);
    }
    virtual bool GetTimeStretchMode(int* algorithm, int* formantMode) const {
        static_cast<void>(algorithm);
        static_cast<void>(formantMode);
        return false;
    }
};

static_assert(sizeof(VirtualInstrument) == 312);
