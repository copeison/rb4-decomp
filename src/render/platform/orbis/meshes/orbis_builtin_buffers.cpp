#include "render/platform/orbis/meshes/orbis_builtin_buffers.h"

#include <cstring>

#include "render/platform/orbis/meshes/orbis_mesh_adapters.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "render/platform/orbis/meshes/orbis_skinned_mesh.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

static_assert(sizeof(SkinnedMeshVertex) == 100);
static_assert(sizeof(OrbisMeshInstanceData) == 120);

namespace {

constexpr const char* kDefaultVertexBufferName = "DefaultVBuffer";
constexpr const char* kIdentityInstanceBufferName =
    "IdentityInstanceVBuffer";

OrbisMeshInstanceData identity_instance_data() {
    OrbisMeshInstanceData instance{};
    instance.transform_rows[0][0] = 1.0F;
    instance.transform_rows[1][1] = 1.0F;
    instance.transform_rows[2][2] = 1.0F;
    instance.normal_transform_rows[0][0] = 1.0F;
    instance.normal_transform_rows[1][1] = 1.0F;
    instance.normal_transform_rows[2][2] = 1.0F;
    return instance;
}

SkinnedMeshVertex default_skinned_vertex() {
    SkinnedMeshVertex vertex{};
    vertex.normal[2] = 1.0F;
    vertex.tangent[0] = 1.0F;
    vertex.bitangent[1] = 1.0F;
    vertex.color[0] = 1.0F;
    vertex.color[1] = 1.0F;
    vertex.color[2] = 1.0F;
    vertex.color[3] = 1.0F;
    vertex.bone_weights[0] = 1.0F;
    return vertex;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D7DB0.
void orbis_create_default_vertex_buffer(OrbisRenderSystem& system) {
    const auto* format =
        render_mesh_format_descriptor(RenderMeshFormat::kSkinned);
    auto* buffer = render_allocate_named(
        format->vertex_stride, kDefaultVertexBufferName, 4);
    orbis_set_default_vertex_buffer(system, buffer);

    const auto vertex = default_skinned_vertex();
    std::memcpy(buffer, &vertex, sizeof(vertex));
    std::uint32_t descriptor_mask = 0;
    orbis_build_mesh_vertex_descriptors(
        orbis_default_vertex_descriptors(system),
        buffer,
        descriptor_mask,
        1,
        *format);
}

// Reconstructed from eboot.elf at 0x8D7EB0.
void orbis_create_identity_instance_buffer(OrbisRenderSystem& system) {
    auto* buffer = render_allocate_named(
        sizeof(OrbisMeshInstanceData),
        kIdentityInstanceBufferName,
        4);
    orbis_set_identity_instance_buffer(system, buffer);
    orbis_build_instance_vertex_descriptors(
        orbis_identity_instance_descriptors(system),
        static_cast<const OrbisMeshInstanceData*>(buffer),
        1);

    const auto instance = identity_instance_data();
    std::memcpy(buffer, &instance, sizeof(instance));
}

}  // namespace rb4
