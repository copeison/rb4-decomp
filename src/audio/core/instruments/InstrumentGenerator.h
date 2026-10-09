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
// instrument interface, which FusionSampler and MultiInstrumentGenerator
// implement and which Scheduler and MultiInstrumentGenerator::SetInstrument
// take. The map has no members of its own, so the class is taken to hold the
// virtuals both implementations share after VirtualInstrument's; their
// order is FusionSampler's vtable (0x18E4D68). The AudioGenerator base is at
// +312.
class InstrumentGenerator : public VirtualInstrument, public AudioGenerator {
public:
    explicit InstrumentGenerator(const char* name) : VirtualInstrument(name) {}

    // Slots 38-39: link and unlink a client run before each block. The map
    // has these names on FusionGenerator and MultiInstrumentGenerator.
    virtual void AddAudioThreadClient(AudioBusCallable* client) = 0;
    virtual void RemoveAudioThreadClient(AudioBusCallable* client) = 0;
    // Slot 40. Name not in the reference map; it rests on the slot's
    // position after the client members and is weak.
    virtual bool SupportsAudioThreadClients() const = 0;
    // Slot 41. FusionGenerator jumps to FusionSampler::LoadPatch. Name not
    // in the reference map.
    virtual void SetPatch(const ResourcePtr<FusionPatchResource>& patch) = 0;
    // Slots 42-43: register a slave generator by handle.
    virtual bool AddSlave(unsigned int handle, InstrumentSlaveType type) = 0;
    virtual bool RemoveSlave(unsigned int handle) = 0;
    // Slots 44-46: FusionGenerator clears, stores and tests an int with
    // them; FusionSampler::_DumpAllInstrumentSlaves calls slot 44 on each
    // slave. Names not in the reference map; behaviour only, the evidence is
    // weak.
    virtual void ClearGeneratorFlag() = 0;
    virtual void SetGeneratorFlag(int flag) = 0;
    virtual bool HasGeneratorFlag() const = 0;
    // Slots 47-48: a transpose in semitones added to each note-on. Names not
    // in the reference map.
    virtual void SetTranspose(int semitones) = 0;
    virtual int GetTranspose() const = 0;
    // Slots 49-50: a time-stretch mode for every voice. Names not in the
    // reference map.
    virtual void SetTimeStretchMode(int algorithm, int formantMode) = 0;
    virtual bool GetTimeStretchMode(int* algorithm, int* formantMode) const = 0;
    // Slots 51-54 also override AudioGenerator's slots 8, 9, 25 and 26.
    void SetSpeed(float speed, bool immediate) override = 0;
    float GetSpeed(bool* changing) override = 0;
    void SetPlayScale(float scale) override = 0;
    float GetPlayScale() override = 0;
};

static_assert(sizeof(VirtualInstrument) == 312);
