#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

class Symbol;

// The checksum of every shader include file, recorded in compiled-shader
// caches and compared against the live set when a cache is loaded. The
// shader manager owns the live set.
class RndShaderIncludeChecksums {
public:
    // eastl::pair<Symbol, int>. Symbols are interned, so equal names share one
    // pointer. Field names are not in the reference map.
    struct Checksum {
        const char* mName;
        int mChecksum;
    };

    // Storage of an eastl::vector<eastl::pair<Symbol, int>>, sorted by
    // case-insensitive name. Name not in the reference map.
    struct ChecksumArray {
        Checksum* mBegin;
        Checksum* mEnd;
        Checksum* mCapacity;
        void* mAllocator;
    };

    void Gather(
        const eastl::vector<Symbol>& files,
        ChecksumArray& checksums) const;
    // Every cached checksum must be found by binary search with the same
    // interned name pointer and value.
    bool Check(const ChecksumArray& checksums) const;  // 0x63EB70
    void Add(Symbol file, int checksum);

    // Name not in the reference map.
    ChecksumArray mChecksums;
};

static_assert(sizeof(RndShaderIncludeChecksums::Checksum) == 16);
static_assert(sizeof(RndShaderIncludeChecksums::ChecksumArray) == 32);
static_assert(offsetof(RndShaderIncludeChecksums, mChecksums) == 0);
static_assert(sizeof(RndShaderIncludeChecksums) == 32);
