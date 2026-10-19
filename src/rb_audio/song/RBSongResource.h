#pragma once

#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

// A song: an entity resource whose root holds the song's information
// (rb_audio/RBSongResource.o). Only the class id the property animations
// test for is declared; the class is not reconstructed.
class RBSongResource : public EntityResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as PropAnimResource::GetTimeFromFrame (0x2FDD70). The local static is
    // at 0x19F99B8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RBSongResource");
        }
        return id;
    }
};
