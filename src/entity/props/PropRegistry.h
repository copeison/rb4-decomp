#pragma once

#include "entity/props/PropInfo.h"

// The properties of a component type (entity/PropRegistry.o). Only the
// registration the reconstructed code uses is declared.
class PropRegistry {
public:
    // Adds a property and returns its info, whose metadata is created for
    // the type. The map does not give the return type.
    PropInfo& RegisterProp(
        const char* name,
        int offset,
        PropertyType type,
        unsigned long count);  // 0x1807F0
};
