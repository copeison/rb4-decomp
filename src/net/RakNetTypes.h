#pragma once

#include <cstddef>
#include <cstdint>

// The parts of the RakNet middleware the game's online (SOMP) code uses. The
// binary links RakNet statically; its headers are not available, so the
// types are declared here with RakNet's own names (RakNetTypes.h) and the
// layout the binary uses.
namespace RakNet {

// A peer's unique id: a 64-bit value and the system's index (-1 when
// unassigned). 16 bytes.
struct RakNetGUID {
    RakNetGUID() : g(static_cast<uint64_t>(-1)), systemIndex(static_cast<unsigned short>(-1)) {}
    explicit RakNetGUID(uint64_t value) : g(value), systemIndex(static_cast<unsigned short>(-1)) {}

    // The value in decimal, or "UNASSIGNED_RAKNET_GUID", in one of eight
    // rotating static buffers.
    const char* ToString() const;  // 0x8EFF00
    bool operator==(const RakNetGUID& other) const;  // 0x8F0980
    bool operator!=(const RakNetGUID& other) const;  // 0x8EFE10

    uint64_t g;
    unsigned short systemIndex;
};

static_assert(offsetof(RakNetGUID, systemIndex) == 8);
static_assert(sizeof(RakNetGUID) == 16);

// RakNet's header defines the unassigned id as a static constant, so each
// object that includes it builds a copy in its static initializer (the band
// user manager's at 0x1ADFAE0, built at 0x9268F0).
static const RakNetGUID UNASSIGNED_RAKNET_GUID(static_cast<uint64_t>(-1));

}  // namespace RakNet
