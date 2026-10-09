#pragma once

#include "entity/props/PropInfo.h"

// The properties of a component type (entity/PropRegistry.o). Only the
// registration and destruction the reconstructed code uses are declared.
class PropRegistry {
public:
    // Destroys the property entries, the visitor callback and the
    // secondary list. Not reconstructed.
    ~PropRegistry();  // 0x180540

    // Adds a property and returns its info, whose metadata is created for
    // the type. The map does not give the return type.
    PropInfo& RegisterProp(
        const char* name,
        int offset,
        PropertyType type,
        unsigned long count);  // 0x1807F0

    // The members are not reconstructed: a vector of 40-byte entries (a
    // name and its PropInfo) at 0, a std::function at 64 and a vector at
    // 112, which the destructor releases. Name not in the reference map.
    unsigned char mMembers[176];
};

static_assert(sizeof(PropRegistry) == 176);
