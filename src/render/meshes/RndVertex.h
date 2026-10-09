#pragma once

#include <cstdint>

// Vertex layouts, indexed by RndVertexType. Field names are not in the
// reference map. The uncompressed lit and colored layouts default to opaque
// white alpha; the others are zeroed.
enum RndVertexType : unsigned int {
    kVertexColor = 0,
    kVertexColorTex = 1,
    kVertexUnskinned = 2,
    kVertexSkinned = 3,
    kVertexPosOnly = 4,
    kVertexParticle = 5,
    kVertexUnskinnedCompressed = 6,
    kVertexSkinnedCompressed = 7,
    kVertexInvalid = 0xFFFFFFFF,
};

struct RndVertexColor {
    static constexpr RndVertexType kType = kVertexColor;
    float mPos[3] = {};
    float mColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
};

struct RndVertexColorTex {
    static constexpr RndVertexType kType = kVertexColorTex;
    float mPos[3] = {};
    float mColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
    float mTex[2] = {};
};

struct RndVertexUnskinned {
    static constexpr RndVertexType kType = kVertexUnskinned;
    float mPos[3] = {};
    float mNorm[3] = {};
    float mTangent[3] = {};
    float mBitangent[3] = {};
    float mColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
    float mTex[2] = {};
    float mTex2[2] = {};
};

struct RndVertexSkinned {
    static constexpr RndVertexType kType = kVertexSkinned;
    float mPos[3] = {};
    float mNorm[3] = {};
    float mTangent[3] = {};
    float mBitangent[3] = {};
    float mColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
    float mTex[2] = {};
    float mTex2[2] = {};
    float mWeights[4] = {};
    std::uint32_t mBones = 0;
};

struct RndVertexPosOnly {
    static constexpr RndVertexType kType = kVertexPosOnly;
    float mPos[3] = {};
};

struct RndVertexUnskinnedCompressed {
    static constexpr RndVertexType kType = kVertexUnskinnedCompressed;
    float mPos[3] = {};
    std::int16_t mNorm[4] = {};
    std::int16_t mTangent[4] = {};
    std::int16_t mBitangent[4] = {};
    std::uint16_t mColor[4] = {};
    std::uint16_t mTex[2] = {};
    std::uint16_t mTex2[2] = {};
};

struct RndVertexSkinnedCompressed {
    static constexpr RndVertexType kType = kVertexSkinnedCompressed;
    float mPos[3] = {};
    std::int16_t mNorm[4] = {};
    std::int16_t mTangent[4] = {};
    std::int16_t mBitangent[4] = {};
    std::uint16_t mColor[4] = {};
    std::uint16_t mTex[2] = {};
    std::uint16_t mTex2[2] = {};
    std::uint16_t mWeights[4] = {};
    std::uint32_t mBones = 0;
};

static_assert(sizeof(RndVertexColor) == 28);
static_assert(sizeof(RndVertexColorTex) == 36);
static_assert(sizeof(RndVertexUnskinned) == 80);
static_assert(sizeof(RndVertexSkinned) == 100);
static_assert(sizeof(RndVertexUnskinnedCompressed) == 52);
static_assert(sizeof(RndVertexSkinnedCompressed) == 64);
static_assert(sizeof(RndVertexPosOnly) == 12);

// Reconstructed from eboot.elf at 0x442930. Name not in the reference map.
RndVertexType VertexTypeFromName(const char* name);
