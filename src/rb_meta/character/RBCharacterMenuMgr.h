#pragma once

#include "rb_meta/state/RBMetaStateCom.h"

class RBProfile;
class User;

// The character menus' session (rb_meta/RBCharacterMenuMgr.o): the player
// customizing a character in the rock shop and the layout the session
// started from. Its members are static. Only what the meta state calls is
// declared; the class is not reconstructed.
class RBCharacterMenuMgr {
public:
    // What a player session customizes. The map names the type
    // (RBCharacterMenuMgr::CharSessionType); the enumerator names are
    // inferred from the overshell's callers (creating, editing, and the
    // shops), and the evidence for them is weak.
    enum CharSessionType : int {
        kCharSessionCreate = 0,
        kCharSessionEdit = 1,
        kCharSessionShop = 2,
    };

    // Whether a character session is under way.
    static bool SessionIsActive();  // 0xE5B160
    // The layout the session started from.
    static RBUILayout GetPreviousLayout();  // 0xE5BAC0
    // Starts the user's session on the profile from the layout; false when
    // a session cannot start. Not reconstructed.
    static bool TryStartPlayerSession(
        User* user,
        int padNum,
        RBProfile* profile,
        CharSessionType type,
        RBUILayout layout);  // 0xE5BAE0
    // Not reconstructed.
    static void EndPlayerSession();  // 0xE5BEB0
    // Marks the shop session as the track texture shop rather than the
    // generic one. Name not in the reference map; the evidence is weak. Not
    // reconstructed.
    static void SetTrackTexShopSession(bool trackTexShop);  // 0xE5BCF0
};
