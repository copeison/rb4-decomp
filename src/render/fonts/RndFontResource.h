#pragma once

#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

// A font loaded from a file. Only the class id, which the typed
// Resource::GetOrLoad reads, is modelled; the virtuals and the layout are
// not recovered.
class RndFontResource : public EntityResource {
public:
    // The class id, created on first use. Inlined into its users, for
    // example Resource::GetOrLoad<RndFontResource> at 0x65CD20, which
    // evaluates it twice.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndFontResource");
        }
        return id;
    }
};
