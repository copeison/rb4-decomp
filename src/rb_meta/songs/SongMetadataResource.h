#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class SongMetadata;

// The resource of a song's .songdta file (rb_meta/SongMetadataResource.o,
// 0xF29C90-0xF29E3F). Only the members the song manager uses are declared;
// the class is not reconstructed.
class SongMetadataResource : public Resource {
public:
    // The class id, built on first use. The static lives in this object
    // (0x1B43470); the function is inlined into its users, for example
    // Resource::GetOrLoad<SongMetadataResource> in rb_meta/SongMgr.o.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("SongMetadataResource");
        }
        return id;
    }

    // The loaded metadata. Name not in the reference map.
    SongMetadata* mMetadata;
};

static_assert(offsetof(SongMetadataResource, mMetadata) == 48);
