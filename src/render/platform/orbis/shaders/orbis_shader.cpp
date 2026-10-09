#include "render/platform/orbis/shaders/orbis_shader.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/shaders/orbis_shader_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisVertexShaderSize = 104;
constexpr std::size_t kOrbisGeometryShaderSize = 72;
constexpr std::size_t kOrbisPixelShaderSize = 64;
constexpr std::size_t kOrbisComputeShaderSize = 64;

OrbisShader* allocate_shader(std::size_t size) {
    return reinterpret_cast<OrbisShader*>(operator new(size));
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D8B30.
OrbisShader* orbis_create_shader(RenderShaderStage stage) {
    switch (stage) {
    case RenderShaderStage::kVertex: {
        auto* shader = allocate_shader(kOrbisVertexShaderSize);
        orbis_vertex_shader_construct(*shader);
        return shader;
    }
    case RenderShaderStage::kGeometry: {
        auto* shader = allocate_shader(kOrbisGeometryShaderSize);
        orbis_geometry_shader_construct(*shader);
        return shader;
    }
    case RenderShaderStage::kPixel: {
        auto* shader = allocate_shader(kOrbisPixelShaderSize);
        orbis_pixel_shader_construct(*shader);
        return shader;
    }
    case RenderShaderStage::kCompute: {
        auto* shader = allocate_shader(kOrbisComputeShaderSize);
        orbis_compute_shader_construct(*shader);
        return shader;
    }
    case RenderShaderStage::kHull:
    case RenderShaderStage::kDomain:
        return nullptr;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x8E46E0.
void orbis_vertex_shader_construct(OrbisShader& shader) {
    render_shader_construct(shader);
    orbis_vertex_shader_set_backend_defaults(shader);
}

// Reconstructed from eboot.elf at 0x8E40A0.
void orbis_geometry_shader_construct(OrbisShader& shader) {
    render_shader_construct(shader);
    orbis_geometry_shader_set_backend_defaults(shader);
}

// Reconstructed from eboot.elf at 0x8E43D0.
void orbis_pixel_shader_construct(OrbisShader& shader) {
    render_shader_construct(shader);
    orbis_pixel_shader_set_backend_defaults(shader);
}

// Reconstructed from eboot.elf at 0x8E3D20.
void orbis_compute_shader_construct(OrbisShader& shader) {
    render_shader_construct(shader);
    orbis_compute_shader_set_backend_defaults(shader);
}

}  // namespace rb4
