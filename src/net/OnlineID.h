#pragma once

#include <cstddef>

#include "utl/text/Str.h"

// A player's network identity (net/OnlineID_PS4.o). This build keeps the
// PSN account id, whether it is set, and a name. Only the layout and the
// members the reconstructed code calls are declared; the object is not
// reconstructed. Field names are not in the reference map.
class OnlineID {
public:
    // Leaves the account id unset; the binary does not clear it.
    OnlineID() : mValid(false) {}

    // The account id in decimal, or "" when it is unset. The map has
    // OnlineID::ToString() const.
    const char* ToString() const;  // 0x11AE630
    // Sets the account id. Name not in the reference map; the map's
    // SetOnlineID takes an SceNpOnlineId.
    void SetAccountId(const unsigned long& accountId);  // 0x11AE6B0
    // Sets the account id from its decimal text. Name not in the reference
    // map.
    void SetAccountIdFromString(const char* text);  // 0x11AE6D0

    unsigned long mAccountId;
    bool mValid;
    // Not read by the reconstructed code.
    String mName;
};

static_assert(offsetof(OnlineID, mValid) == 8);
static_assert(offsetof(OnlineID, mName) == 16);
static_assert(sizeof(OnlineID) == 32);
