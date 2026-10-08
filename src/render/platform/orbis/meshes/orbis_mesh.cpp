#include "render/platform/orbis/meshes/orbis_mesh.h"

#include <cstddef>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/meshes/orbis_mesh_adapters.h"

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
    case RenderMeshFormat::kInvalid:
        return false;
    }
    return false;
}

}  // namespace

// Reconstructed from eboot.elf at 0x442930.
RenderMeshFormat render_mesh_format_from_name(const char* name) {
    if (std::strcmp(name, "Color") == 0) {
        return RenderMeshFormat::kColor;
    }
    if (std::strcmp(name, "ColorTex") == 0) {
        return RenderMeshFormat::kColorTexture;
    }
    if (std::strcmp(name, "Unskinned") == 0) {
        return RenderMeshFormat::kUnskinned;
    }
    if (std::strcmp(name, "Skinned") == 0) {
        return RenderMeshFormat::kSkinned;
    }
    if (std::strcmp(name, "PosOnly") == 0) {
        return RenderMeshFormat::kPositionOnly;
    }
    if (std::strcmp(name, "Particle") == 0) {
        return RenderMeshFormat::kParticle;
    }
    if (std::strcmp(name, "UnskinnedCompressed") == 0) {
        return RenderMeshFormat::kUnskinnedCompressed;
    }
    if (std::strcmp(name, "SkinnedCompressed") == 0) {
        return RenderMeshFormat::kSkinnedCompressed;
    }
    return RenderMeshFormat::kInvalid;
}

// Reconstructed from eboot.elf at 0x8D85F0.
OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name) {
    if (!is_supported_mesh_format(format)) {
        return nullptr;
    }

    auto* storage = render_allocate(kOrbisMeshSize);
    auto* mesh = reinterpret_cast<OrbisMesh*>(storage);
    render_mesh_construct(*mesh, name);
    orbis_mesh_set_format_backend_defaults(*mesh, format);
    return mesh;
}

}  // namespace rb4
