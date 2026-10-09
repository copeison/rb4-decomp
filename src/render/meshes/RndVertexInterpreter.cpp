#include "render/meshes/RndVertexInterpreter.h"

namespace {

using AttributeInfo = RndVertexInterpreter::AttributeInfo;
using Layout = AttributeInfo[RndVertexInterpreter::kNumAttributes];

constexpr unsigned int kNumVertexTypes = 8;
constexpr unsigned int kUnknownField = 0xFFFFFFFF;

constexpr AttributeInfo Attribute(
    long offset,
    unsigned long numComponents,
    RndVertexDataType type) {
    return {offset, numComponents, type, kUnknownField};
}

constexpr AttributeInfo Unused() {
    return {-1, 0, kVertexDataInvalid, kUnknownField};
}

constexpr unsigned int kAttributeStreams[RndVertexInterpreter::kNumAttributes] = {
    0, 1, 2, 3, 4, 5, 5, 6, 7, 6,
};

constexpr Layout kColorAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Attribute(12, 4, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
};

constexpr Layout kColorTexAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Attribute(12, 4, kVertexDataFloat32),
    Attribute(28, 2, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
};

constexpr Layout kUnskinnedAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Attribute(12, 3, kVertexDataFloat32),
    Attribute(24, 3, kVertexDataFloat32),
    Attribute(36, 3, kVertexDataFloat32),
    Attribute(48, 4, kVertexDataFloat32),
    Attribute(64, 2, kVertexDataFloat32),
    Attribute(72, 2, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
};

constexpr Layout kSkinnedAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Attribute(12, 3, kVertexDataFloat32),
    Attribute(24, 3, kVertexDataFloat32),
    Attribute(36, 3, kVertexDataFloat32),
    Attribute(48, 4, kVertexDataFloat32),
    Attribute(64, 2, kVertexDataFloat32),
    Attribute(72, 2, kVertexDataFloat32),
    Attribute(80, 4, kVertexDataFloat32),
    Attribute(96, 4, kVertexDataUInt8),
    Unused(),
};

constexpr Layout kPosOnlyAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
    Unused(),
};

constexpr Layout kParticleAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Attribute(12, 4, kVertexDataFloat32),
    Attribute(28, 2, kVertexDataFloat32),
    Unused(),
    Unused(),
    Unused(),
    Attribute(36, 4, kVertexDataFloat32),
};

constexpr Layout kUnskinnedCompressedAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Attribute(12, 4, kVertexDataSNorm16),
    Attribute(20, 4, kVertexDataSNorm16),
    Attribute(28, 4, kVertexDataSNorm16),
    Attribute(36, 4, kVertexDataFloat16),
    Attribute(44, 2, kVertexDataFloat16),
    Attribute(48, 2, kVertexDataFloat16),
    Unused(),
    Unused(),
    Unused(),
};

constexpr Layout kSkinnedCompressedAttributes = {
    Attribute(0, 3, kVertexDataFloat32),
    Attribute(12, 4, kVertexDataSNorm16),
    Attribute(20, 4, kVertexDataSNorm16),
    Attribute(28, 4, kVertexDataSNorm16),
    Attribute(36, 4, kVertexDataFloat16),
    Attribute(44, 2, kVertexDataFloat16),
    Attribute(48, 2, kVertexDataFloat16),
    Attribute(52, 4, kVertexDataFloat16),
    Attribute(60, 4, kVertexDataUInt8),
    Unused(),
};

// One interpreter per RndVertexType.
const RndVertexInterpreter kInterpreters[kNumVertexTypes] = {
    {5, 28, kColorAttributes},
    {5, 36, kColorTexAttributes},
    {5, 80, kUnskinnedAttributes},
    {5, 100, kSkinnedAttributes},
    {5, 12, kPosOnlyAttributes},
    {5, 52, kParticleAttributes},
    {5, 52, kUnskinnedCompressedAttributes},
    {5, 64, kSkinnedCompressedAttributes},
};

}  // namespace

// Reconstructed from eboot.elf at 0x4430C0 and 0x4435E0.
const RndVertexInterpreter* RndVertexInterpreter::GetInstance(RndVertexType type) {
    const auto index = static_cast<unsigned int>(type);
    return index < kNumVertexTypes ? &kInterpreters[index] : nullptr;
}

// Reconstructed from eboot.elf at 0x442A80, which lies with the RndVertex
// functions.
unsigned int RndVertexInterpreter::GetAttributeStream(unsigned int attribute) {
    if (attribute >= kNumAttributes) {
        return kNoStream;
    }
    return kAttributeStreams[attribute];
}

// Reconstructed from eboot.elf at 0x443720.
bool RndVertexInterpreter::UsesStream(unsigned int stream) const {
    for (unsigned int attribute = 0; attribute < kNumAttributes; ++attribute) {
        if (mAttributes[attribute].mOffset >= 0 &&
            GetAttributeStream(attribute) == stream) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x443880.
RndVertexInterpreter::StreamLayout RndVertexInterpreter::GetStreamLayout(
    unsigned int stream) const {
    StreamLayout result = {};
    bool found = false;
    for (unsigned int attribute = 0; attribute < kNumAttributes; ++attribute) {
        const auto& info = mAttributes[attribute];
        if (info.mOffset < 0 || GetAttributeStream(attribute) != stream) {
            continue;
        }

        if (!found) {
            result.mOffset = static_cast<unsigned long>(info.mOffset);
            result.mNumComponents = info.mNumComponents;
            result.mType = info.mType;
            found = true;
        } else {
            result.mNumComponents += info.mNumComponents;
        }
    }
    return result;
}
