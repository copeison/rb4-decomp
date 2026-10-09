#pragma once

#include "audio/core/resources/Resource.h"
#include "utl/text/Symbol.h"

// A 2D texture loaded from a file. Only the class id, which the typed
// Resource::GetOrLoad reads, is modelled; the virtuals and the layout are
// not recovered.
class RndTexture2DResource : public Resource {
public:
    // The class id, created on first use. Inlined into its users, for
    // example Resource::GetOrLoad<RndTexture2DResource> at 0x44B1A0, which
    // evaluates it twice.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndTexture2DResource");
        }
        return id;
    }
};
