#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "math/scalar/Half.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "math/vector/Vector4.h"
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

    // Attribute indices, in the order above. Names not in the reference
    // map.
    static constexpr unsigned int kPositionAttribute = 0;
    static constexpr unsigned int kNormalAttribute = 1;
    static constexpr unsigned int kTangentAttribute = 2;
    static constexpr unsigned int kBitangentAttribute = 3;
    static constexpr unsigned int kColorAttribute = 4;
    static constexpr unsigned int kFirstUVAttribute = 5;
    static constexpr unsigned int kWeightsAttribute = 7;
    static constexpr unsigned int kBonesAttribute = 8;

    // Writes a value in an attribute's storage type. The map's helper also
    // reads values back; only the writers the mesh builders call are
    // reconstructed. The binary keeps the out-of-line copies as COMDAT
    // functions at the addresses on the specializations below.
    template <typename T>
    class _AttributeValueHelper {
    public:
        static void Set(const AttributeInfo& info, const T& value, void* data);
        static T Get(const AttributeInfo& info, const void* data);
    };

    // Reconstructed from eboot.elf at 0x447880, the out-of-line Vector3
    // copy. Reads an attribute of a vertex. Name not in the reference map.
    template <typename T>
    T GetAttribute(int attribute, const void* vertex) const {
        const AttributeInfo& info = mAttributes[attribute];
        return _AttributeValueHelper<T>::Get(info, static_cast<const char*>(vertex) + info.mOffset);
    }

    // Returns null for an unknown type.
    static const RndVertexInterpreter* GetInstance(RndVertexType type);  // 0x4430C0, 0x4435E0

    // The two UV sets share a stream, as do the weights and the particle
    // data. Names not in the reference map.
    static unsigned int GetAttributeStream(unsigned int attribute);  // 0x442A80
    bool UsesStream(unsigned int stream) const;                      // 0x443720
    StreamLayout GetStreamLayout(unsigned int stream) const;         // 0x443880

    // The number of UV sets. The binary's interpreter caches it at +0x118
    // when it is initialized (the map's _CountUVs); this build's interpreter
    // record derives it from the attribute table. Name not in the reference
    // map.
    unsigned long GetNumUVs() const {
        unsigned long count = mAttributes[kFirstUVAttribute].mOffset != -1 ? 1 : 0;
        if (mAttributes[kFirstUVAttribute + 1].mOffset != -1) {
            ++count;
        }
        return count;
    }

    // Rounds half away from zero, saturating at the int range, then clamps
    // into the 16-bit normalized ranges. Names not in the reference map.
    static int _RoundToInt(float value) {
        if (value > 0.0F) {
            value += 0.5F;
            return value < 2147483648.0F ? static_cast<int>(value)
                                         : std::numeric_limits<int>::max();
        }
        value -= 0.5F;
        return value > -2147483648.0F ? static_cast<int>(value)
                                      : std::numeric_limits<int>::min();
    }
    static std::int16_t _ToSNorm16(float value) {
        const int rounded = _RoundToInt(value * 32767.0F);
        const int clamped = rounded > 32767 ? 32767 : (rounded > -32767 ? rounded : -32767);
        return static_cast<std::int16_t>(clamped);
    }
    static std::uint16_t _ToUNorm16(float value) {
        const int rounded = _RoundToInt(value * 65535.0F);
        const int clamped = rounded > 0xFFFF ? 0xFFFF : (rounded > 0 ? rounded : 0);
        return static_cast<std::uint16_t>(clamped);
    }
    // The most negative value reads as -1, like the one above it. Name not
    // in the reference map.
    static float _FromSNorm16(std::int16_t value) {
        return value == -32768 ? -1.0F : static_cast<float>(value) * (1.0F / 32767.0F);
    }

    // Field names are not in the reference map.
    unsigned long mUnknown0;
    unsigned long mStride;
    const AttributeInfo* mAttributes;
};

static_assert(sizeof(RndVertexInterpreter::AttributeInfo) == 24);
static_assert(offsetof(RndVertexInterpreter, mStride) == 8);
static_assert(offsetof(RndVertexInterpreter, mAttributes) == 16);
static_assert(sizeof(RndVertexInterpreter) == 24);

// Two-component values are stored as floats or halves; other storage types
// are left untouched.
template <>
inline void RndVertexInterpreter::_AttributeValueHelper<Vector2>::Set(
    const AttributeInfo& info,
    const Vector2& value,
    void* data) {
    if (info.mType == kVertexDataFloat16) {
        auto* halves = static_cast<Half*>(data);
        halves[0].Set(value.x);
        halves[1].Set(value.y);
    } else if (info.mType == kVertexDataFloat32) {
        auto* floats = static_cast<float*>(data);
        floats[0] = value.x;
        floats[1] = value.y;
    }
}

// Reconstructed from eboot.elf at 0x4479C0.
template <>
inline void RndVertexInterpreter::_AttributeValueHelper<Vector3>::Set(
    const AttributeInfo& info,
    const Vector3& value,
    void* data) {
    switch (info.mType) {
    case kVertexDataSNorm16: {
        auto* shorts = static_cast<std::int16_t*>(data);
        shorts[0] = _ToSNorm16(value.x);
        shorts[1] = _ToSNorm16(value.y);
        shorts[2] = _ToSNorm16(value.z);
        break;
    }
    case kVertexDataFloat16: {
        auto* halves = static_cast<Half*>(data);
        halves[0].Set(value.x);
        halves[1].Set(value.y);
        halves[2].Set(value.z);
        break;
    }
    case kVertexDataFloat32: {
        auto* floats = static_cast<float*>(data);
        floats[0] = value.x;
        floats[1] = value.y;
        floats[2] = value.z;
        break;
    }
    default:
        break;
    }
}

// Inlined into GetAttribute<Vector3> at 0x447880. Other storage types read
// as zero.
template <>
inline Vector3 RndVertexInterpreter::_AttributeValueHelper<Vector3>::Get(
    const AttributeInfo& info,
    const void* data) {
    switch (info.mType) {
    case kVertexDataSNorm16: {
        const auto* shorts = static_cast<const std::int16_t*>(data);
        return {_FromSNorm16(shorts[0]), _FromSNorm16(shorts[1]), _FromSNorm16(shorts[2])};
    }
    case kVertexDataFloat16: {
        const auto* halves = static_cast<const Half*>(data);
        return {halves[0].ToFloat(), halves[1].ToFloat(), halves[2].ToFloat()};
    }
    case kVertexDataFloat32: {
        const auto* floats = static_cast<const float*>(data);
        return {floats[0], floats[1], floats[2]};
    }
    default:
        return {0.0F, 0.0F, 0.0F};
    }
}

// Reconstructed from eboot.elf at 0x447B80.
template <>
inline void RndVertexInterpreter::_AttributeValueHelper<Vector4>::Set(
    const AttributeInfo& info,
    const Vector4& value,
    void* data) {
    switch (info.mType) {
    case kVertexDataFloat32:
        *static_cast<Vector4*>(data) = value;
        break;
    case kVertexDataFloat16: {
        auto* halves = static_cast<Half*>(data);
        halves[0].Set(value.x);
        halves[1].Set(value.y);
        halves[2].Set(value.z);
        halves[3].Set(value.w);
        break;
    }
    case kVertexDataUNorm16: {
        auto* shorts = static_cast<std::uint16_t*>(data);
        shorts[0] = _ToUNorm16(value.x);
        shorts[1] = _ToUNorm16(value.y);
        shorts[2] = _ToUNorm16(value.z);
        shorts[3] = _ToUNorm16(value.w);
        break;
    }
    case kVertexDataSNorm16: {
        auto* shorts = static_cast<std::int16_t*>(data);
        shorts[0] = _ToSNorm16(value.x);
        shorts[1] = _ToSNorm16(value.y);
        shorts[2] = _ToSNorm16(value.z);
        shorts[3] = _ToSNorm16(value.w);
        break;
    }
    default:
        break;
    }
}
