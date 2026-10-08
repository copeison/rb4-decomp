#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

constexpr std::size_t kMeshAttributeCount = 10;
constexpr std::size_t kMeshVertexStreamCount = 8;
constexpr std::size_t kInstanceVertexStreamCount = 9;

enum class MeshAttributeStorage : std::uint32_t {
    kFloat32 = 0,
    kFloat16 = 1,
    kUnorm8 = 2,
    kUnorm16 = 3,
    kSnorm8 = 4,
    kSnorm16 = 5,
    kUint8 = 6,
    kUint16 = 7,
};

enum class OrbisDataFormat : std::uint32_t {
    kInvalid = 0,
    kR32Uint = 0x00004404,
    kR8G8Uint = 0x0002C403,
    kR16G16Uint = 0x0002C405,
    kR8G8Unorm = 0x0022C003,
    kR16G16Unorm = 0x0022C005,
    kR8G8Snorm = 0x0022C103,
    kR16G16Snorm = 0x0022C105,
    kR16G16Float = 0x0022C705,
    kR32G32Float = 0x0022C70B,
    kR32G32B32Float = 0x003AC70D,
    kR8G8B8A8Uint = 0x00FAC40A,
    kR16G16B16A16Uint = 0x00FAC40C,
    kR8G8B8A8Unorm = 0x00FAC00A,
    kR16G16B16A16Unorm = 0x00FAC00C,
    kR8G8B8A8Snorm = 0x00FAC10A,
    kR16G16B16A16Snorm = 0x00FAC10C,
    kR16G16B16A16Float = 0x00FAC70C,
    kR32G32B32A32Float = 0x00FAC70E,
};

enum class OrbisResourceMemoryType : std::uint32_t {
    kReadOnly = 0x10,
};

struct OrbisBufferDescriptor {
    std::uint32_t registers[4];
};

struct MeshAttributeDescriptor {
    std::int64_t byte_offset;
    std::uint64_t component_count;
    MeshAttributeStorage storage;
    std::uint32_t semantic;
};

struct RenderMeshFormatDescriptor {
    std::uint64_t unknown;
    std::uint64_t vertex_stride;
    const MeshAttributeDescriptor* attributes;
};

struct MeshVertexStreamLayout {
    std::size_t byte_offset;
    std::size_t component_count;
    MeshAttributeStorage storage;
};

struct OrbisMeshInstanceData {
    float transform_rows[3][4];
    float normal_transform_rows[3][3];
    std::uint32_t packed_state;
    float parameters[2][4];
};

std::uint32_t render_mesh_attribute_stream_index(
    std::uint32_t attribute_index);
bool render_mesh_format_uses_stream(
    const RenderMeshFormatDescriptor& format,
    std::uint32_t stream_index);
MeshVertexStreamLayout render_mesh_format_stream_layout(
    const RenderMeshFormatDescriptor& format,
    std::uint32_t stream_index);
void orbis_build_mesh_vertex_descriptors(
    OrbisBufferDescriptor* descriptors,
    const void* vertex_data,
    std::uint32_t& descriptor_mask,
    std::uint32_t vertex_count,
    const RenderMeshFormatDescriptor& format);
void orbis_build_instance_vertex_descriptors(
    OrbisBufferDescriptor* descriptors,
    const OrbisMeshInstanceData* instance_data,
    std::uint32_t instance_count);

}  // namespace rb4
