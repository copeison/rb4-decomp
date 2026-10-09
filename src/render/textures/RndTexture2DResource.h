#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndTexture2D;

// A 2D texture loaded from a file. Only the class id, which the typed
// Resource::GetOrLoad reads, and the texture are modelled; the virtuals and
// the rest of the layout are not recovered.
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

    // The loaded texture, which the light components read while the
    // resource has not failed (RndLightDirectionalCom::_GetCookieTextureImpl,
    // 0x4748C0). Name not in the reference map.
    RndTexture2D* mTexture;
};

static_assert(offsetof(RndTexture2DResource, mTexture) == 48);
