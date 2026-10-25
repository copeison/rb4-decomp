#pragma once

#include <cstddef>

#include "mic/core/MicClientMapper.h"
#include "utl/containers/Vector.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"

class GameMic;

// "game_mics_changed", which the microphone manager exports when the
// connected microphones change. The map names the class; its constructor is
// inline here, the complete destructor (0x9BFC20) jumps to Message<0>'s
// (0x8DBA0), the deleting destructor is at 0x9BFEB0 and the vtable at
// 0x1962938.
class GameMicsChangedMsg : public Message<0> {
public:
    GameMicsChangedMsg() : Message<0>(Event()) {}
    ~GameMicsChangedMsg() override {}

    // The message type, built on first use (0x1AE6CA8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("game_mics_changed");
        }
        return t;
    }
};

// The game's microphones (rb_game/GameMicManager.o, 0x9BF490-0x9BFED0 with
// the static initializer at 0x9BF460): one GameMic for each of the four
// microphone ids the mapper reports connected, their playback and their
// energy, exported as "game_mics_changed" when they change. The vtable
// group at 0x19628A8 holds MicManagerInterface's five slots and MsgSource's
// (offset 8); the object is 112 bytes. The map's fake microphones, effects,
// proximity and pitch correction calls are not in this build's object. The
// static initializer only builds statics of included headers that nothing in
// the object reads; it is not reconstructed.
class GameMicManager : public MicManagerInterface, public MsgSource {
public:
    // Creates the manager, registers Terminate as an exit callback and
    // hands the manager to the microphone mapper.
    static void Init();  // 0x9BF490
    static void Terminate();  // 0x9BF600
    // Turns the high sensitivity on or off and returns the new setting.
    bool ToggleMicSensitivity();  // 0x9BF630

    GameMicManager();  // 0x9BF640
    // Slots 0-1: 0x9BF740, 0x9BF810; MsgSource thunks 0x9BF7A0,
    // 0x9BF880. The microphones are not deleted.
    ~GameMicManager() override;
    // Slot 2: creates the microphones of the connected ids, deletes the
    // others and exports "game_mics_changed".
    void HandleMicsChanged() override;  // 0x9BF8F0
    // Slot 3: applies the playback to every microphone when it changes or
    // a change is pending.
    void SetPlayback(bool playback) override;  // 0x9BFCE0
    // Slot 4: updates the client's microphone and returns its energy, or 0.
    float GetEnergyForMic(const MicClientID& client) override;  // 0x9BFC90

    void DeleteMic(int micID);  // 0x9BFB50
    // Creates the microphone unless it exists and applies the playback.
    void CreateMic(int micID);  // 0x9BFBA0
    // Whether a microphone is connected for the client. The map's
    // HasMic(MicClientID const&) const; this build asks the microphone
    // mapper for the client's microphone.
    bool HasMic(const MicClientID& client) const;  // 0x9BFC30
    // The client's microphone, or null.
    GameMic* GetMic(const MicClientID& client);  // 0x9BFC50
    // Turns the microphone's channel playback on or off. The second flag is
    // unused. Not reconstructed: it calls the microphone channel, whose
    // class is not modelled.
    void ApplyPlayback(bool playback, bool unused, GameMic* mic) const;  // 0x9BFDA0
    void SetPlaybackForMic(bool playback, bool unused, const MicClientID& client);  // 0x9BFDD0
    void SetRecordFileName(const char* name);  // 0x9BFE30
    int GetMicCount() const;  // 0x9BFEA0

    // Field names are not in the reference map.
    String mRecordFileName;
    // Four option flags (the fourth starts on) that the reconstructed code
    // does not read, and three effect values that start at 0.5, -1 and -1.
    // The names are weak.
    bool mFxFlags[4];
    bool mHighSensitivity;
    float mFxValues[3];
    int mMicCount;
    // One entry for each microphone id; null when not connected.
    eastl::vector<GameMic*> mMics;
    bool mPlayback;
    // Makes the next SetPlayback apply even an unchanged setting.
    bool mPlaybackPending;
};

static_assert(offsetof(GameMicManager, mRecordFileName) == 32);
static_assert(offsetof(GameMicManager, mFxFlags) == 48);
static_assert(offsetof(GameMicManager, mHighSensitivity) == 52);
static_assert(offsetof(GameMicManager, mFxValues) == 56);
static_assert(offsetof(GameMicManager, mMicCount) == 68);
static_assert(offsetof(GameMicManager, mMics) == 72);
static_assert(offsetof(GameMicManager, mPlayback) == 104);
static_assert(sizeof(GameMicManager) == 112);

// The manager. At 0x1AE6CA0.
extern GameMicManager* TheGameMicManager;
