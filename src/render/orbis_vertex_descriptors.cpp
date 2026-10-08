#include "orbis_vertex_descriptors.h"

#include <cstddef>
#include <cstdint>

#include "orbis_mesh_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kUnusedStream = 0xFFFFFFFF;
constexpr std::uint32_t kAttributeStreams[kMeshAttributeCount] = {
    0, 1, 2, 3, 4, 5, 5, 6, 7, 6,
};

OrbisDataFormat two_component_format(MeshAttributeStorage storage) {
    switch (storage) {
    case MeshAttributeStorage::kFloat32:
        return OrbisDataFormat::kR32G32Float;
    case MeshAttributeStorage::kFloat16:
        return OrbisDataFormat::kR16G16Float;
    case MeshAttributeStorage::kUnorm8:
        return OrbisDataFormat::kR8G8Unorm;
    case MeshAttributeStorage::kUnorm16:
        return OrbisDataFormat::kR16G16Unorm;
    case MeshAttributeStorage::kSnorm8:
        return OrbisDataFormat::kR8G8Snorm;
    case MeshAttributeStorage::kSnorm16:
        return OrbisDataFormat::kR16G16Snorm;
    case MeshAttributeStorage::kUint8:
        return OrbisDataFormat::kR8G8Uint;
    case MeshAttributeStorage::kUint16:
        return OrbisDataFormat::kR16G16Uint;
    }
    return OrbisDataFormat::kInvalid;
}

OrbisDataFormat four_component_format(MeshAttributeStorage storage) {
    switch (storage) {
    case MeshAttributeStorage::kFloat32:
        return OrbisDataFormat::kR32G32B32A32Float;
    case MeshAttributeStorage::kFloat16:
        return OrbisDataFormat::kR16G16B16A16Float;
    case MeshAttributeStorage::kUnorm8:
        return OrbisDataFormat::kR8G8B8A8Unorm;
    case MeshAttributeStorage::kUnorm16:
        return OrbisDataFormat::kR16G16B16A16Unorm;
    case MeshAttributeStorage::kSnorm8:
        return OrbisDataFormat::kR8G8B8A8Snorm;
    case MeshAttributeStorage::kSnorm16:
        return OrbisDataFormat::kR16G16B16A16Snorm;
    case MeshAttributeStorage::kUint8:
        return OrbisDataFormat::kR8G8B8A8Uint;
    case MeshAttributeStorage::kUint16:
        return OrbisDataFormat::kR16G16B16A16Uint;
    }
    return OrbisDataFormat::kInvalid;
}

OrbisDataFormat data_format_for(const MeshVertexStreamLayout& stream) {
    switch (stream.component_count) {
    case 2:
        return two_component_format(stream.storage);
    case 3:
        if (stream.storage == MeshAttributeStorage::kFloat32) {
            return OrbisDataFormat::kR32G32B32Float;
        }
        return OrbisDataFormat::kInvalid;
    case 4:
        return four_component_format(stream.storage);
    default:
        return OrbisDataFormat::kInvalid;
    }
}

std::uint32_t component_byte_size(MeshAttributeStorage storage) {
    switch (storage) {
    case MeshAttributeStorage::kFloat32:
        return 4;
    case MeshAttributeStorage::kFloat16:
    case MeshAttributeStorage::kUnorm16:
    case MeshAttributeStorage::kSnorm16:
    case MeshAttributeStorage::kUint16:
        return 2;
    case MeshAttributeStorage::kUnorm8:
    case MeshAttributeStorage::kSnorm8:
    case MeshAttributeStorage::kUint8:
        return 1;
    }
    return 0;
}

void initialize_descriptor(
    OrbisBufferDescriptor& descriptor,
    const void* data,
    OrbisDataFormat format,
    std::uint32_t stride,
    std::uint32_t element_count) {
    gnm_buffer_init_as_vertex_buffer(
        descriptor, data, format, stride, element_count);
    gnm_buffer_set_resource_memory_type(
        descriptor, OrbisResourceMemoryType::kReadOnly);
}

}  // namespace

static_assert(sizeof(OrbisBufferDescriptor) == 16,
              "unexpected Gnm buffer descriptor size");
static_assert(sizeof(MeshAttributeDescriptor) == 24,
              "unexpected mesh attribute descriptor size");
static_assert(sizeof(OrbisMeshInstanceData) == 120,
              "unexpected instance vertex size");
static_assert(offsetof(OrbisMeshInstanceData, normal_transform_rows) == 48,
              "unexpected instance normal-transform offset");
static_assert(offsetof(OrbisMeshInstanceData, packed_state) == 84,
              "unexpected instance state offset");
static_assert(offsetof(OrbisMeshInstanceData, parameters) == 88,
              "unexpected instance parameter offset");

// Reconstructed from eboot.elf at 0x442A80.
std::uint32_t render_mesh_attribute_stream_index(
    std::uint32_t attribute_index) {
    if (attribute_index >= kMeshAttributeCount) {
        return kUnusedStream;
    }
    return kAttributeStreams[attribute_index];
}

// Reconstructed from eboot.elf at 0x443720.
bool render_mesh_format_uses_stream(
    const RenderMeshFormatDescriptor& format,
    std::uint32_t stream_index) {
    for (std::uint32_t attribute = 0;
         attribute < kMeshAttributeCount;
         ++attribute) {
        if (format.attributes[attribute].byte_offset >= 0 &&
            render_mesh_attribute_stream_index(attribute) == stream_index) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x443880.
MeshVertexStreamLayout render_mesh_format_stream_layout(
    const RenderMeshFormatDescriptor& format,
    std::uint32_t stream_index) {
    MeshVertexStreamLayout result = {};
    bool found = false;
    for (std::uint32_t attribute = 0;
         attribute < kMeshAttributeCount;
         ++attribute) {
        const auto& candidate = format.attributes[attribute];
        if (candidate.byte_offset < 0 ||
            render_mesh_attribute_stream_index(attribute) != stream_index) {
            continue;
        }

        if (!found) {
            result.byte_offset = static_cast<std::size_t>(candidate.byte_offset);
            result.component_count = candidate.component_count;
            result.storage = candidate.storage;
            found = true;
        } else {
            result.component_count += candidate.component_count;
        }
    }
    return result;
}

// Reconstructed from eboot.elf at 0x8E1840.
void orbis_build_mesh_vertex_descriptors(
    OrbisBufferDescriptor* descriptors,
    const void* vertex_data,
    std::uint32_t& descriptor_mask,
    std::uint32_t vertex_count,
    const RenderMeshFormatDescriptor& format) {
    auto* bytes = static_cast<const std::uint8_t*>(vertex_data);
    for (std::uint32_t stream = 0;
         stream < kMeshVertexStreamCount;
         ++stream) {
        if (!render_mesh_format_uses_stream(format, stream)) {
            continue;
        }

        const auto layout = render_mesh_format_stream_layout(format, stream);
        const auto data_format = data_format_for(layout);
        const auto single_element_size = static_cast<std::uint32_t>(
            layout.component_count * component_byte_size(layout.storage));
        const auto stride = vertex_count == 1 ? 0 : format.vertex_stride;
        const auto element_count =
            vertex_count == 1 ? single_element_size : vertex_count;
        initialize_descriptor(
            descriptors[stream],
            bytes + layout.byte_offset,
            data_format,
            static_cast<std::uint32_t>(stride),
            element_count);
        descriptor_mask |= 1U << stream;
    }
}

// Reconstructed from eboot.elf at 0x8E1BD0.
void orbis_build_instance_vertex_descriptors(
    OrbisBufferDescriptor* descriptors,
    const OrbisMeshInstanceData* instance_data,
    std::uint32_t instance_count) {
    constexpr std::size_t offsets[kInstanceVertexStreamCount] = {
        0, 16, 32, 48, 60, 72, 84, 88, 104,
    };
    constexpr OrbisDataFormat formats[kInstanceVertexStreamCount] = {
        OrbisDataFormat::kR32G32B32A32Float,
        OrbisDataFormat::kR32G32B32A32Float,
        OrbisDataFormat::kR32G32B32A32Float,
        OrbisDataFormat::kR32G32B32Float,
        OrbisDataFormat::kR32G32B32Float,
        OrbisDataFormat::kR32G32B32Float,
        OrbisDataFormat::kR32Uint,
        OrbisDataFormat::kR32G32B32A32Float,
        OrbisDataFormat::kR32G32B32A32Float,
    };
    constexpr std::uint32_t sizes[kInstanceVertexStreamCount] = {
        16, 16, 16, 12, 12, 12, 4, 16, 16,
    };

    const auto* bytes = reinterpret_cast<const std::uint8_t*>(instance_data);
    for (std::size_t stream = 0;
         stream < kInstanceVertexStreamCount;
         ++stream) {
        const auto stride = instance_count == 1
            ? 0U
            : static_cast<std::uint32_t>(sizeof(OrbisMeshInstanceData));
        const auto element_count =
            instance_count == 1 ? sizes[stream] : instance_count;
        initialize_descriptor(
            descriptors[stream],
            bytes + offsets[stream],
            formats[stream],
            stride,
            element_count);
    }
}

}  // namespace rb4
