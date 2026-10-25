#pragma once

#include "rb_meta/state/RBMetaStateCom.h"
#include "utl/text/Symbol.h"

class BandUser;
class IGameplayOptions;
class RBSystemOptions;
class User;

// The client of a SOMP session (RBSompSessionMgr::mClient, 0x1B156C8): sends
// the local users' requests and settings to the session's server. Newer than
// the map; the class name and the method names are inferred from the
// overshell slots' callers, and the evidence for them is weak. Declared
// only.
class RBSompClient {
public:
    // Sends the user's gameplay options, the system options and the no fail
    // setting.
    void SendGameplayOptions(
        BandUser* user,
        IGameplayOptions* options,
        RBSystemOptions* systemOptions,
        bool noFail);  // 0x988E40
    // Requests that the user leave the session.
    void RequestDropOut(User* user);  // 0x988F50
    // Asks the server to drop the member (the kick confirmation's name and
    // id) from the session for the user.
    void RequestKickPlayer(BandUser* user, Symbol name, Symbol id);  // 0x989990
    // The session leader's requests to take the session back to the green
    // room and to the music library.
    void RequestReturnToGreenRoom(User* user);     // 0x989C40
    void RequestReturnToMusicLibrary(User* user);  // 0x989CC0
    // The user's request to leave the song.
    void RequestLeaveSong(BandUser* user);  // 0x989D40
    // The leader's request to skip to the next song.
    void RequestSkipSong(BandUser* user);  // 0x98A7F0
    // Tells the server the user moved on to choosing a difficulty.
    void SendChoosingDifficulty(BandUser* user);  // 0x98B040
};

// One of the session's emote requests (32 bytes, kept from 0x8EC8). Only the
// id and the emote's token are modelled; names not in the reference map.
struct RBSompEmoteRecord {
    int mOpaque0;  // Not modelled.
    int mId;
    unsigned char mOpaque8[8];  // Not modelled.
    Symbol mToken;
    unsigned char mOpaque24[8];  // Not modelled.
};

static_assert(sizeof(RBSompEmoteRecord) == 32);

// The manager of the online sessions of the SOMP mode, which the frame
// loop updates (0xD2C440) and whose job-complete messages it handles. It is
// newer than the map, which has no SOMP code; the class and global names
// are inferred from the mode's strings ("somp_session_*", the RBSomp*
// components). Only the members the meta state and the overshell use are
// declared; the class is not reconstructed. The names of the methods below
// IsSongLibraryFiltered are not in the reference map; the evidence for them
// is weak.
class RBSompSessionMgr {
public:
    // Whether a SOMP session is under way. Inlined by every reader, for
    // example RBMetaState::Goto at 0xBB5D6F.
    bool IsSompSession() const {
        return mIsSompSession;
    }
    // Whether the session filters the song library: false outside a
    // session. The evidence for the name is weak.
    bool IsSongLibraryFiltered() const;  // 0xD2A280
    // Whether a session is under way or one of the SOMP mode's menus
    // (kLayoutSompHub and layouts 101-103) shows; the overshell then prompts
    // users to switch rather than to join.
    bool IsInSompMode() const;  // 0xD2A0B0
    // The flag (+0x8991) the session sets when it loads a song (after
    // GotoLayout(kLayoutLoading), 0x98F632); setting it clears the local
    // user's part, difficulty and score type. The overshell stays disabled
    // in the game layout while it is set. Name not in the reference map;
    // the evidence is weak.
    bool IsSessionSongActive() const;  // 0xD2A210
    // The band user of the session's local platform user (+0x150), or
    // null without one. Name not in the reference map.
    BandUser* GetLocalUser() const;  // 0xD299E0
    // Makes the user the session's local user: records the platform user
    // id and the band slot (+0x158). Name not in the reference map.
    void SetLocalUser(BandUser* user);  // 0xD2BB50
    // Leaves the session with the error token, once. Not reconstructed.
    void LeaveSessionWithError(Symbol error);  // 0xD2BBD0
    // Goes to the layout unless it shows already or a UI transition runs;
    // the session's layouts wait for the session (the reason is kept with
    // the request).
    void GotoLayout(RBUILayout layout, Symbol reason);  // 0xD2BD90
    // Whether the session is ending: a pending leave flag or a pending
    // request id.
    bool IsSompSessionEnding() const;  // 0xD2BE60
    // The emote request with the id, or null.
    const RBSompEmoteRecord* FindEmoteRecord(int id) const;  // 0xD2F060
    // In a session, the first flag of the session's settings (+0xB2C,
    // SompSessionSettings), which the overshell toggles as no fail.
    bool IsSessionNoFail() const;  // 0xD30260
    // Mark the session's local members for an update after an overshell
    // slot showed the reconnect controller state or the options; both set
    // the same flag (+0x89BB) and ignore the user.
    void OnReconnectControllerShown(BandUser* user);  // 0xD30CD0
    void OnOptionsShown(BandUser* user);              // 0xD30CE0
    // Whether the session shows the remote users' track textures (the flag
    // at +0x8960; the overshell's "toggle_allow_remote_track_textures").
    bool AllowsRemoteTrackTextures() const;       // 0xD32420
    void SetAllowRemoteTrackTextures(bool allow);  // 0xD32430

    // The state before the client; not reconstructed.
    unsigned char mSessionState0[0x28];
    // The session's client, null outside a session (0x1B156C8). The name is
    // inferred.
    RBSompClient* mClient;
    // The state before the flags below; not reconstructed.
    unsigned char mSessionState[0xB5C - 0x30];
    // Read by IsSongLibraryFiltered. The evidence for the name is weak.
    bool mSongLibraryFiltered;
    // The state between the flags; not reconstructed.
    unsigned char mSessionState2[0x8990 - 0xB5D];
    bool mIsSompSession;
};

static_assert(__builtin_offsetof(RBSompSessionMgr, mClient) == 0x28);
static_assert(__builtin_offsetof(RBSompSessionMgr, mSongLibraryFiltered) == 0xB5C);
static_assert(__builtin_offsetof(RBSompSessionMgr, mIsSompSession) == 0x8990);

// The manager. At 0x1B156A0.
extern RBSompSessionMgr theRBSompSessionMgr;
