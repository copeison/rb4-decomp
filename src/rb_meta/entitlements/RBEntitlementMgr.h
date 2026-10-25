#pragma once

// The entitlement manager: an rb_meta object after DialogMgrCom.o that the
// map does not have. Its factory (0xAE25D0) sets the pointer and installs
// a content filter with SongMgr::SetContentFilter. Only what the song
// manager and the profiles call is declared; the class is not
// reconstructed. Names not in the reference map; the evidence for them is
// weak.
class RBEntitlementMgr {
public:
    // Whether the entitlement is owned. Takes the manager's lock.
    bool IsEntitled(int entitlement);  // 0xAE40C0
    // Whether the pad's user owns the song, asked through the dingo
    // service; false for the pad the manager skips (0x127AE24) and for a
    // song without an entitlement label (0xAE5D30).
    bool IsSongEntitled(int songId, int padNum);  // 0xAE5CD0
};

extern RBEntitlementMgr* theRBEntitlementMgr;  // 0x1AF4E38
