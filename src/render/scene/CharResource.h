#pragma once

#include "render/scene/RndEntityResource.h"
#include "utl/text/Symbol.h"

// A character's entity resource (render/CharResource.o), registered under
// RndEntityResource by CharResource::Init (0x3EBF80). The class is not
// reconstructed; only its id is declared.
class CharResource : public RndEntityResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as CharBoneMaskCom::_Init (0x2E1D90) and CharIkCom::_Init
    // (0x2F1360). The local static is at 0x19F8A08.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharResource");
        }
        return id;
    }
};
