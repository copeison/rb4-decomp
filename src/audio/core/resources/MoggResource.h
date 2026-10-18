#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// A loaded Mogg file (audio/MoggResource.o). The class has not been
// reconstructed; only the members MoggGeneratorManager reads are declared.
class MoggResource : public Resource {
public:
    // Registers the resource type. Inline; emitted in audio/SoundManager.o
    // at 0x1680.
    static void Init();

    // Field names are not in the reference map. The loaders set both (for
    // example at 0x61A1C) and then call
    // MoggGeneratorManager::RegisterMoggResource.
    // The file's path, which the Mogg generator opens.
    Symbol mFilePath;
    // The file's name without its directory (FileGetName), the sound name
    // that plays the file.
    Symbol mSoundName;
};

// One track of a multi-channel Mogg as a sampler keyzone's "track_map"
// lists it (the map's MoggResource.o keeps it in an eastl::vector; this
// build's keyzones hold a PropArray of it). The members are declared, not
// reconstructed; field names are not in the reference map.
struct TrackChannelInfo {
    // One channel of the track: the "channels" element. Name not in the
    // reference map.
    struct Channel {
        // "stream_index": the channel's index in the Mogg, -1 when
        // unassigned.
        int mStreamIndex;
        // "volume", stored as a linear gain and shown in dB.
        float mVolume;
        float mPan;  // "pan"
    };

    TrackChannelInfo();                                // 0xC84F0
    TrackChannelInfo(const TrackChannelInfo& other);  // 0xC86A0
    // The keyzone array's element moves use it, then destroy the source.
    TrackChannelInfo(TrackChannelInfo&& other);       // 0xC8600
    ~TrackChannelInfo();                               // 0xC8810

    // -1 by default; no reader is identified, so the name is weak.
    int mIndex;
    Symbol mName;  // "name"
    // Empty by default; no reader is identified, so the name is weak.
    Symbol mAlias;
    PropArray<Channel> mChannels;  // "channels"
    // A map the constructor allocates (56 bytes); its contents are not
    // modelled.
    void* mChannelMap;
};

static_assert(sizeof(TrackChannelInfo::Channel) == 12);
static_assert(offsetof(TrackChannelInfo, mChannels) == 24);
static_assert(sizeof(TrackChannelInfo) == 72);

static_assert(offsetof(MoggResource, mFilePath) == 48);
static_assert(offsetof(MoggResource, mSoundName) == 56);
