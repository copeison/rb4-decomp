#include "render/platform/orbis/meshes/orbis_mesh_formats.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace rb4 {

namespace {

constexpr std::size_t kFormatCount = 8;
constexpr std::int64_t kUnusedAttribute = -1;
constexpr std::uint32_t kUnknownAttributeField = 0xFFFFFFFF;

constexpr MeshAttributeDescriptor attribute(
    std::int64_t byte_offset,
    std::uint64_t component_count,
    MeshAttributeStorage storage) {
    return {byte_offset, component_count, storage, kUnknownAttributeField};
}

constexpr MeshAttributeDescriptor unused() {
    return {
        kUnusedAttribute,
        0,
        static_cast<MeshAttributeStorage>(0xFFFFFFFF),
        kUnknownAttributeField,
    };
}

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kColorAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        attribute(12, 4, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kColorTextureAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        attribute(12, 4, MeshAttributeStorage::kFloat32),
        attribute(28, 2, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kUnskinnedAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        attribute(12, 3, MeshAttributeStorage::kFloat32),
        attribute(24, 3, MeshAttributeStorage::kFloat32),
        attribute(36, 3, MeshAttributeStorage::kFloat32),
        attribute(48, 4, MeshAttributeStorage::kFloat32),
        attribute(64, 2, MeshAttributeStorage::kFloat32),
        attribute(72, 2, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kSkinnedAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        attribute(12, 3, MeshAttributeStorage::kFloat32),
        attribute(24, 3, MeshAttributeStorage::kFloat32),
        attribute(36, 3, MeshAttributeStorage::kFloat32),
        attribute(48, 4, MeshAttributeStorage::kFloat32),
        attribute(64, 2, MeshAttributeStorage::kFloat32),
        attribute(72, 2, MeshAttributeStorage::kFloat32),
        attribute(80, 4, MeshAttributeStorage::kFloat32),
        attribute(96, 4, MeshAttributeStorage::kUint8),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kPositionAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kParticleAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        attribute(12, 4, MeshAttributeStorage::kFloat32),
        attribute(28, 2, MeshAttributeStorage::kFloat32),
        unused(),
        unused(),
        unused(),
        attribute(36, 4, MeshAttributeStorage::kFloat32),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kUnskinnedCompressedAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        attribute(12, 4, MeshAttributeStorage::kSnorm16),
        attribute(20, 4, MeshAttributeStorage::kSnorm16),
        attribute(28, 4, MeshAttributeStorage::kSnorm16),
        attribute(36, 4, MeshAttributeStorage::kFloat16),
        attribute(44, 2, MeshAttributeStorage::kFloat16),
        attribute(48, 2, MeshAttributeStorage::kFloat16),
        unused(),
        unused(),
        unused(),
    }};

constexpr std::array<MeshAttributeDescriptor, kMeshAttributeCount>
    kSkinnedCompressedAttributes = {{
        attribute(0, 3, MeshAttributeStorage::kFloat32),
        attribute(12, 4, MeshAttributeStorage::kSnorm16),
        attribute(20, 4, MeshAttributeStorage::kSnorm16),
        attribute(28, 4, MeshAttributeStorage::kSnorm16),
        attribute(36, 4, MeshAttributeStorage::kFloat16),
        attribute(44, 2, MeshAttributeStorage::kFloat16),
        attribute(48, 2, MeshAttributeStorage::kFloat16),
        attribute(52, 4, MeshAttributeStorage::kFloat16),
        attribute(60, 4, MeshAttributeStorage::kUint8),
        unused(),
    }};

const std::array<RenderMeshFormatDescriptor, kFormatCount> kFormats = {{
    {5, 28, kColorAttributes.data()},
    {5, 36, kColorTextureAttributes.data()},
    {5, 80, kUnskinnedAttributes.data()},
    {5, 100, kSkinnedAttributes.data()},
    {5, 12, kPositionAttributes.data()},
    {5, 52, kParticleAttributes.data()},
    {5, 52, kUnskinnedCompressedAttributes.data()},
    {5, 64, kSkinnedCompressedAttributes.data()},
}};

}  // namespace

// Reconstructed from eboot.elf at 0x4430C0 and 0x4435E0.
const RenderMeshFormatDescriptor* render_mesh_format_descriptor(
    RndVertexType format) {
    const auto index = static_cast<std::size_t>(format);
    return index < kFormats.size() ? &kFormats[index] : nullptr;
}

}  // namespace rb4
