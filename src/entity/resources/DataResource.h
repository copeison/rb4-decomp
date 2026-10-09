#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"

// A script data file loaded as a resource (entity/DataResource.o in the
// map). This build keeps no code of the class; Resource::FileChangedOnDisk
// still tests for it by id and reads its included files, so only those are
// declared.
class DataResource : public Resource {
public:
    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19E4600.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("DataResource");
        }
        return id;
    }

    // Field names are not in the reference map.
    // The members between the Resource base and the file list are not
    // identified.
    unsigned char mData[16];
    // The files the data includes.
    eastl::vector<ResourcePath> mFiles;
};

static_assert(offsetof(DataResource, mFiles) == 64);
