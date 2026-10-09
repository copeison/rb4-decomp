#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

// Numeric types of shader constants and resources. Enumerator names are not
// in the reference map.
enum RndShaderNumericType : unsigned int {
    kShaderNumericBool = 0,
    kShaderNumericInt = 1,
    kShaderNumericInt2 = 2,
    kShaderNumericInt3 = 3,
    kShaderNumericInt4 = 4,
    kShaderNumericUInt = 5,
    kShaderNumericUInt2 = 6,
    kShaderNumericUInt3 = 7,
    kShaderNumericUInt4 = 8,
    kShaderNumericFloat = 9,
    kShaderNumericFloat2 = 10,
    kShaderNumericFloat3 = 11,
    kShaderNumericFloat4 = 12,
    kShaderNumericFloat3x3 = 13,
    kShaderNumericFloat3x4 = 14,
    kShaderNumericFloat4x3 = 15,
    kShaderNumericFloat4x4 = 16,
};

// Layout of one shader constant buffer: its name, slot, the stages that read
// it, and the constants in register order. Shaders and the resource manager
// build these; RndShaderCBuffer allocates buffers from them.
class RndShaderCBufferConfig {
public:
    // Field names are not in the reference map.
    struct Constant {
        unsigned long mOffset;  // In 16-byte registers.
        RndShaderNumericType mType;
        long mCount;            // -1 for a scalar constant.
        bool mRTSliced;         // One copy per render-target slice.
        const char* mName;
    };

    // The map's last parameter is a parent RndShaderCBufferConfig; this build
    // passes the number of buffers to allocate instead.
    RndShaderCBufferConfig(
        const char* name,
        unsigned int slot,
        unsigned int stages,
        unsigned long numBuffers);  // 0x63A0C0

    unsigned long AddConstant(RndShaderNumericType type, const char* name);  // 0x63A1B0
    unsigned long AddConstantArray(
        RndShaderNumericType type,
        unsigned long count,
        const char* name);  // 0x63A370
    unsigned long AddRTSlicedConstantArray(
        RndShaderNumericType type,
        unsigned long count,
        const char* name);  // 0x63A700
    // Streams the generated declaration into the hashing text stream. The
    // map's parameter is TextStream&; the stream is modeled by its running
    // FNV-1a hash.
    void PrintCode(unsigned int& hash) const;  // 0x63A8C0

    // Name not in the reference map.
    unsigned long _AddConstant(
        RndShaderNumericType type,
        long count,
        bool rtSliced,
        const char* name);

    // Field names are not in the reference map.
    const char* mName;
    unsigned int mSlot;
    unsigned int mStages;
    unsigned long mNumBuffers;
    unsigned long mSize;  // In 16-byte registers.
    bool mLocked;         // Offsets are still assigned, but not recorded.
    eastl::vector<Constant> mConstants;
};

static_assert(sizeof(RndShaderCBufferConfig::Constant) == 40);
static_assert(offsetof(RndShaderCBufferConfig, mSize) == 24);
static_assert(offsetof(RndShaderCBufferConfig, mLocked) == 32);
static_assert(offsetof(RndShaderCBufferConfig, mConstants) == 40);
static_assert(sizeof(RndShaderCBufferConfig) == 72);
