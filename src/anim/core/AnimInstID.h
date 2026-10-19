#pragma once

#include <cstddef>

class AnimDriver;

// A running animation: the driver that plays it and the playback's id in
// that driver (anim/AnimInstID.o, 0x32C6C0-0x32C6E8). AnimDriverCom and
// AnimCom take one in every call on a playback; an id without a driver is
// null. Field names are not in the reference map.
class AnimInstID {
public:
    AnimInstID();                                      // 0x32C6C0
    AnimInstID(AnimDriver* driver, unsigned long id);  // 0x32C6D0

    // The null id: no driver and id -1.
    static const AnimInstID& NullID();  // 0x32C6E0

    // The id NullID returns, statically initialized.
    static AnimInstID sNullID;  // 0x19BDA28

    AnimDriver* mDriver;
    // The playback's PlaybackInfo::mId.
    unsigned long mId;
};

static_assert(offsetof(AnimInstID, mId) == 8);
static_assert(sizeof(AnimInstID) == 16);
