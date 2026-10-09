#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/music/SongPos.h"

// The music options of a request, which MusicGenerator's setup (0x52AF0)
// copies whole. The defaults are those the music request builders store, as
// the emitter's PlayMusic (0x34960) does. Field names are not in the
// reference map; the map's PlayMusic overloads name the first, third and
// seventh by their parameter types.
struct MusicPlayOptions {
    MusicSyncOptions mSync;
    // The music to follow; zero follows the emitter's master music.
    unsigned int mMasterHandle;
    MusicTimelineMapping mMapping;
    // Zero in every builder; no reader was found.
    int mReserved[3];
    MusicUnmutePoint mUnmutePoint;
    // 64 by default; the MIDI generator passes it with each note to its
    // cursor, so it is taken to be a MIDI velocity. Weak evidence.
    int mVelocity;
};

static_assert(sizeof(MusicPlayOptions) == 32);

// A music request, marked by PlayArgs::mFormat 1. Its builders inline the
// constructor. Names not in the reference map.
struct PlayMusicArgs : public PlayArgs {
    static constexpr int kMusicFormat = 1;

    PlayMusicArgs()
        : mOptions{static_cast<MusicSyncOptions>(0),
                   0,
                   static_cast<MusicTimelineMapping>(1),
                   {0, 0, 0},
                   static_cast<MusicUnmutePoint>(0),
                   64},
          mSpeed(1.0F),
          mReservedFlag(false),
          mPlayScale(1.0F),
          mReservedTail{0, 0, 0} {
        mFormat = kMusicFormat;
    }

    MusicPlayOptions mOptions;
    // Both 1 by default; the names rest on the music generators' speed and
    // play scale and are weak. No reader was found for the flag or the tail.
    float mSpeed;
    bool mReservedFlag;
    float mPlayScale;
    int mReservedTail[3];
};

static_assert(offsetof(PlayMusicArgs, mOptions) == 0x68);
static_assert(offsetof(PlayMusicArgs, mSpeed) == 0x88);
static_assert(offsetof(PlayMusicArgs, mPlayScale) == 0x90);
static_assert(sizeof(PlayMusicArgs) == 0xA0);

// The base of the music generators, which follow a song's timeline
// (audio/MusicGenerator.o). MidiMusicGenerator, MoggMusicGenerator and
// MusicTimelineGenerator derive from it. The class has not been
// reconstructed; only its construction, the AudioGenerator slots it
// implements and its first own slots are declared. Its vtable at 0x18E18A8
// has 130 slots; the slots from 44 on (the map's SetSyncMaster,
// MasterAdvanced, section and track queries) are not declared. The object
// is 344 bytes.
class MusicGenerator : public AudioGenerator {
public:
    // At 0x52780. The map emits it in audio/MidiMusicGenerator.o.
    MusicGenerator();

    void Pause() override;                 // slot 0: 0x53800
    void Continue() override;              // slot 1: 0x53890
    void Stop() override;                  // slot 2: 0x55D10
    // Slot 6 at 0x46A70, emitted with the MidiMusic generator.
    float GetLengthMs() const override;
    // Slots 8-9 at 0x46AA0 and 0x46AF0, emitted with the MidiMusic
    // generator.
    void SetSpeed(float speed, bool immediate) override;
    float GetSpeed(bool* changing) override;
    bool IsMusic() override;               // slot 16: 0x46BD0
    ~MusicGenerator() override;            // slots 20-21: 0x52C00, 0x52DA0
    void Kill() override;                  // slot 29: 0x55E00

    // Slot 32, pure here: whether the music can start; the Mogg music
    // generator asks its Mogg voice. Name not in the reference map.
    virtual bool IsReady() = 0;
    virtual int GetCurrentContentTick() const;  // slot 33: 0x46CF0
    virtual int GetCurrentSongTick() const;     // slot 34: 0x46D00
    // Slots 35-36 at 0x52E10 and 0x52E60: the positions of the content and
    // song ticks (slots 33 and 34).
    virtual SongPos GetCurrentContentPos();
    virtual SongPos GetCurrentSongPos();
    virtual int GetSmoothedCurrentContentTick() const;   // slot 37: 0x52EB0
    virtual float GetSmoothedCurrentContentMs() const;   // slot 38: 0x52EC0
    virtual SongPos GetSmoothedCurrentContentPos() const;  // slot 39: 0x52ED0
    virtual void DoSmoothCurrentContentTickAndMs();      // slot 40: 0x52F20
    virtual Symbol GetCurrentSectionName();     // slot 41: 0x46D10
    virtual int GetCurrentSectionIndex();       // slot 42: 0x46D40
    // Slot 43 at 0x55EF0: the tempo, scaled by the speed. The map has it
    // inline.
    virtual float GetCurrentBPM();

    // "kMaster" and "kSlave", or kNoSync for any other name. At 0x559D0.
    static MusicSyncOptions SymbolToMusicSyncOptions(Symbol name);

    // The state after the AudioGenerator base; not reconstructed.
    unsigned char mMusicState[0x158 - 0x50];
};

static_assert(sizeof(MusicGenerator) == 0x158);
