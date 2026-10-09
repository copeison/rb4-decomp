#pragma once

#include <cstddef>

#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// Permutation key of one shader program. Global defines use the high 32
// bits; a program's own defines use the low 32 bits.
typedef unsigned long RndShaderKey;

// Fixed preprocessor defines and comments emitted at the top of a shader.
class RndShaderFixedDefines {
public:
    // Field names are not in the reference map.
    struct Define {
        const char* mName;  // An interned symbol; empty for a comment.
        int mValue;
        String mComment;
    };

    void Add(Symbol name, int value);  // 0x63D520
    void AddComment(const char* comment);  // 0x63D5D0
    // The map's parameter is TextStream&; see RndShaderCBufferConfig.
    void PrintCode(unsigned int& hash) const;  // 0x63D6E0

    eastl::vector<Define> mDefines;  // Name not in the reference map.
};

static_assert(sizeof(RndShaderFixedDefines::Define) == 32);
static_assert(sizeof(RndShaderFixedDefines) == 32);

// Where one permutation define sits in a key. Field names are not in the
// reference map.
struct RndShaderDefInfo {
    int mFirst;              // Value of the define's zero field.
    int mEnd;                // One past the last value.
    unsigned int mMask;      // Field mask, already shifted.
    unsigned int mShift;
    bool mGlobal;            // The field is in the key's high 32 bits.

    // Name not in the reference map.
    unsigned int GetValue(RndShaderKey key) const;  // 0x63C380
    // Writes the value's field, sign-extending the shifted field as every
    // recovered draw does. Name not in the reference map.
    RndShaderKey SetValue(RndShaderKey key, unsigned int value) const;
};

static_assert(sizeof(RndShaderDefInfo) == 20);

// One preprocessor define chosen for a permutation.
struct RndShaderMacro {
    const char* mName;
    const char* mDefinition;
};

static_assert(sizeof(RndShaderMacro) == 16);

// The permutation defines of one program type, packed into consecutive bits.
class RndShaderDefines {
public:
    // Field names are not in the reference map.
    struct Entry {
        String mName;
        int mFirst;
        int mEnd;
        unsigned int mMask;
        unsigned int mShift;
        bool mGlobal;
    };

    explicit RndShaderDefines(bool global = false)
        : mNumBits(0), mGlobal(global) {}

    // Adds a define with values first..end-1.
    RndShaderDefInfo Add(Symbol name, int first, int end);  // 0x63C3F0
    RndShaderDefInfo AddBool(Symbol name);  // 0x63C550

    // Field names are not in the reference map.
    eastl::vector<Entry> mEntries;
    unsigned int mNumBits;
    bool mGlobal;
};

static_assert(sizeof(RndShaderDefines::Entry) == 40);
static_assert(sizeof(RndShaderDefines) == 40);

// Visitor for every permutation of a program type: the chosen defines in
// order and the packed key. The map's visitor is a std::function taking an
// eastl::vector of macros.
typedef void (*RndShaderPermutationVisitor)(
    void* context,
    const RndShaderMacro* macros,
    unsigned long numMacros,
    RndShaderKey key);

// The global defines (index 0) and the defines of each program type: vertex,
// tessellation (hull and domain), geometry, pixel, and compute.
class RndShaderDefinesGroup {
public:
    RndShaderDefinesGroup() {
        mDefines[0].mGlobal = true;
    }

    RndShaderDefines& GetDefines(RndShaderProgramType type) {
        return mDefines[kProgramDefines[type]];
    }
    RndShaderDefines& GetGlobalDefines() {
        return mDefines[0];
    }

    // Enumerates every combination of the global and the program type's
    // defines.
    void Visit(
        RndShaderProgramType type,
        RndShaderPermutationVisitor visitor,
        void* context) const;  // 0x63D100

    // Index of each program type's defines. Name not in the reference map.
    static constexpr unsigned long kProgramDefines[kNumShaderProgramTypes] = {
        1, 2, 2, 3, 4, 5,
    };

    RndShaderDefines mDefines[6];  // Name not in the reference map.
};

static_assert(sizeof(RndShaderDefinesGroup) == 240);

// One key per program kind a shader can select: vertex, tessellation,
// geometry, pixel, and compute.
struct RndShaderKeyGroup {
    RndShaderKey mKeys[5];  // Name not in the reference map.
};

// Whether a shader's stage mask includes a program type. Bits 0-4 select the
// vertex, tessellation, geometry, pixel, and compute programs. Name not in the
// reference map.
bool ShaderStagesInclude(int stages, RndShaderProgramType type);  // 0x63E5E0
