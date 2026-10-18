#pragma once

#include "render/scene/RndDrawableEntityResource.h"
#include "utl/text/Symbol.h"

// The resource of a renderer entity (render/RndEntityResource.o), registered
// under RndDrawableEntityResource by RndEntityResource::Init (0x3EAFB0). The
// class is not reconstructed; only its id is declared.
class RndEntityResource : public RndDrawableEntityResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as RndEntityCom::_Init (0x6CA5A0). The local static is at 0x1A71918.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndEntityResource");
        }
        return id;
    }
};
