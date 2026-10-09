#pragma once

#include <cstddef>

#include "render/meshes/RndVertex.h"

// Storage type of one vertex attribute. Enumerator names are not in the
// reference map.
enum RndVertexDataType : unsigned int {
    kVertexDataFloat32 = 0,
    kVertexDataFloat16 = 1,
    kVertexDataUNorm8 = 2,
    kVertexDataUNorm16 = 3,
    kVertexDataSNorm8 = 4,
    kVertexDataSNorm16 = 5,
    kVertexDataUInt8 = 6,
    kVertexDataUInt16 = 7,
    kVertexDataInvalid = 0xFFFFFFFF,
};

// Describes where each attribute of a vertex layout lives. The map's class
// also keeps sorted offsets and supports loading and saving; this build's
// interpreter is the 24-byte record below, one per RndVertexType, with ten
// attributes: position, normal, tangent, bitangent, color, two UV sets,
// weights, bone indices and the particle data.
class RndVertexInterpreter {
public:
    // Field names are not in the reference map.
    struct AttributeInfo {
        long mOffset;  // Negative when the layout lacks the attribute.
        unsigned long mNumComponents;
        RndVertexDataType mType;
        unsigned int mUnknown20;
    };

    // The attributes that share one vertex stream, merged. Name not in the
    // reference map.
    struct StreamLayout {
        unsigned long mOffset;
        unsigned long mNumComponents;
        RndVertexDataType mType;
    };

    // Names not in the reference map.
    static constexpr unsigned int kNumAttributes = 10;
    static constexpr unsigned int kNumStreams = 8;
    static constexpr unsigned int kNoStream = 0xFFFFFFFF;

    // Returns null for an unknown type.
    static const RndVertexInterpreter* GetInstance(RndVertexType type);  // 0x4430C0, 0x4435E0

    // The two UV sets share a stream, as do the weights and the particle
    // data. Names not in the reference map.
    static unsigned int GetAttributeStream(unsigned int attribute);  // 0x442A80
    bool UsesStream(unsigned int stream) const;                      // 0x443720
    StreamLayout GetStreamLayout(unsigned int stream) const;         // 0x443880

    // Field names are not in the reference map.
    unsigned long mUnknown0;
    unsigned long mStride;
    const AttributeInfo* mAttributes;
};

static_assert(sizeof(RndVertexInterpreter::AttributeInfo) == 24);
static_assert(offsetof(RndVertexInterpreter, mStride) == 8);
static_assert(offsetof(RndVertexInterpreter, mAttributes) == 16);
static_assert(sizeof(RndVertexInterpreter) == 24);
