#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/transition_aliases.h"
#include "render/shaders/RndShaderEnums.h"

class BinStream;

// Compiled program for one shader stage and permutation. The key orders
// compiled programs for binary-search selection. The base vtable is at
// 0x192F360.
class RndShaderProgram {
public:
    // Reconstructed from eboot.elf at 0x642250.
    static RndShaderProgram* New(RndShaderProgramType type);

    RndShaderProgram();                // 0x642270
    virtual ~RndShaderProgram() {}     // slots 0-1: 0x6422A0, 0x6422B0

    // Slot 2. Loads the compiled program from the stream.
    virtual bool _CreateImpl(BinStream& stream) = 0;
    // Slot 3.
    virtual void _SelectImpl(RndContext& context) = 0;
    // Slot 4.
    virtual void _FreeImpl() = 0;
    // Slot 5.
    virtual RndShaderProgramType _GetTypeImpl() const = 0;

    // Reconstructed from eboot.elf at 0x6422C0. A null stream creates an
    // empty program. The map types the key as RndShaderKey.
    bool Create(unsigned long key, BinStream* stream, const char* name);
    // Reconstructed from eboot.elf at 0x642310.
    void Free();

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    unsigned long mKey;
    bool mCreated;
    // Render-system frame epoch of the most recent bind.
    long mLastSelectFrame;
    const char* mName;
};

static_assert(offsetof(RndShaderProgram, mKey) == 8);
static_assert(offsetof(RndShaderProgram, mCreated) == 16);
static_assert(offsetof(RndShaderProgram, mLastSelectFrame) == 24);
static_assert(sizeof(RndShaderProgram) == 40);
