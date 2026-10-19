#pragma once

// The manager of the online sessions of the SOMP mode, which the frame
// loop updates (0xD2C440) and whose job-complete messages it handles. It is
// newer than the map, which has no SOMP code; the class and global names
// are inferred from the mode's strings ("somp_session_*", the RBSomp*
// components). Only the members the meta state reads are declared; the
// class is not reconstructed.
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

    // The state before the flags below; not reconstructed.
    unsigned char mSessionState[0xB5C];
    // Read by IsSongLibraryFiltered. The evidence for the name is weak.
    bool mSongLibraryFiltered;
    // The state between the flags; not reconstructed.
    unsigned char mSessionState2[0x8990 - 0xB5D];
    bool mIsSompSession;
};

static_assert(__builtin_offsetof(RBSompSessionMgr, mSongLibraryFiltered) == 0xB5C);
static_assert(__builtin_offsetof(RBSompSessionMgr, mIsSompSession) == 0x8990);

// The manager. At 0x1B156A0.
extern RBSompSessionMgr theRBSompSessionMgr;
