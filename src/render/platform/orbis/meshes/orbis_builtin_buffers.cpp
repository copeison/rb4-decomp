
#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_gnm_mesh_api.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "render/meshes/RndVertex.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"
#include "renderps4/system/PS4Device.h"

namespace rb4 {

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

// The default stream data: a unit normal, tangent, and bitangent, opaque
// white, and full weight on the first bone. Name not in the reference map.
RndVertexSkinned DefaultStreamVertex() {
    RndVertexSkinned vertex;
    vertex.mNorm[2] = 1.0F;
    vertex.mTangent[0] = 1.0F;
    vertex.mBitangent[1] = 1.0F;
    vertex.mColor[0] = 1.0F;
    vertex.mColor[1] = 1.0F;
    vertex.mColor[2] = 1.0F;
    vertex.mColor[3] = 1.0F;
    vertex.mWeights[0] = 1.0F;
    return vertex;
}

}  // namespace

}  // namespace rb4

using namespace rb4;

// Reconstructed from eboot.elf at 0x8D7DB0.
void PS4Device::_InitDefaultVertexBuffers() {
    auto& system = *this;
    const auto* format =
        render_mesh_format_descriptor(kVertexSkinned);
    auto* buffer = MemAlloc(
        format->vertex_stride, kDefaultVertexBufferName, 4);
    system.mDefaultVertexBuffer = buffer;

    const auto vertex = DefaultStreamVertex();
    std::memcpy(buffer, &vertex, sizeof(vertex));
    std::uint32_t descriptor_mask = 0;
    orbis_build_mesh_vertex_descriptors(
        system.mDefaultVertexDescs,
        buffer,
        descriptor_mask,
        1,
        *format);
}

// Reconstructed from eboot.elf at 0x8D7EB0.
void PS4Device::_InitIdentityInstanceBuffers() {
    auto& system = *this;
    auto* buffer = MemAlloc(
        sizeof(OrbisMeshInstanceData),
        kIdentityInstanceBufferName,
        4);
    system.mIdentityInstanceBuffer = buffer;
    orbis_build_instance_vertex_descriptors(
        system.mIdentityInstanceDescs,
        static_cast<const OrbisMeshInstanceData*>(buffer),
        1);

    const auto instance = identity_instance_data();
    std::memcpy(buffer, &instance, sizeof(instance));
}
