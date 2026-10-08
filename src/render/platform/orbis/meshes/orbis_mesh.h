#pragma once

#include <cstdint>

#include "render/core/meshes/render_mesh.h"

namespace rb4 {

enum class RenderMeshFormat : std::uint32_t {
    kColor = 0,
    kColorTexture = 1,
    kUnskinned = 2,
    kSkinned = 3,
    kPositionOnly = 4,
    kParticle = 5,
    kUnskinnedCompressed = 6,
    kSkinnedCompressed = 7,
    kInvalid = 0xFFFFFFFF,
};

struct OrbisMesh : RenderMesh {};

static_assert(sizeof(OrbisMesh) == 128);

enum class MeshUpdateFlags : std::uint32_t {
    kVertices = 1,
};

OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name);
RenderMeshFormat render_mesh_format_from_name(const char* name);

}  // namespace rb4
