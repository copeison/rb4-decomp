#pragma once

#include "os/users/HxGuid.h"

class String;

// The id that identifies a User (os/UserGuid.o). Users are matched and
// ordered by it; the band user manager keeps one per band slot.
class UserGuid : public HxGuid {
public:
    // Parses the hexadecimal text through HxGuid's assignment.
    UserGuid& operator=(String str);  // 0x11B15D0
};

static_assert(sizeof(UserGuid) == 16);
