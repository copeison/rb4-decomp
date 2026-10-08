#include "orbis_mesh.h"

#include <cstddef>

#include "orbis_mesh_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;

bool is_supported_mesh_format(RenderMeshFormat format) {
    switch (format) {
    case RenderMeshFormat::kColor:
    case RenderMeshFormat::kColorTexture:
    case RenderMeshFormat::kUnskinned:
    case RenderMeshFormat::kSkinned:
    case RenderMeshFormat::kPositionOnly:
    case RenderMeshFormat::kUnskinnedCompressed:
    case RenderMeshFormat::kSkinnedCompressed:
        return true;
    case RenderMeshFormat::kParticle:
        return false;
    }
    return false;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D85F0.
OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name) {
    if (!is_supported_mesh_format(format)) {
        return nullptr;
    }

    auto* storage = render_allocate(kOrbisMeshSize);
    auto* mesh = reinterpret_cast<OrbisMesh*>(storage);
    mesh_construct(*mesh, name);
    orbis_mesh_set_format_backend_defaults(*mesh, format);
    return mesh;
}

}  // namespace rb4
