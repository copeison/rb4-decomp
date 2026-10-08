#include "render/core/shaders/render_shader.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/core/context/render_context.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

struct RenderShaderDispatch {
    void (*destruct)(RenderShader& shader);
    void (*delete_shader)(RenderShader& shader);
    bool (*initialize)(
        RenderShader& shader,
        const RenderShaderBinary& binary);
    void (*bind)(const RenderShader& shader, RenderContext& context);
    void (*release)(RenderShader& shader);
    RenderShaderStage (*stage)();
};

static_assert(offsetof(RenderShaderDispatch, initialize) == 16);
static_assert(offsetof(RenderShaderDispatch, release) == 32);
static_assert(sizeof(RenderShaderDispatch) == 6 * sizeof(void*));

RenderShaderDispatch kBaseShaderDispatch{
    render_shader_destruct,
    render_shader_delete,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

void set_base_dispatch(RenderShader& shader) {
    shader.implementation = &kBaseShaderDispatch;
}

const RenderShaderDispatch& dispatch(const RenderShader& shader) {
    return *static_cast<const RenderShaderDispatch*>(shader.implementation);
}

}  // namespace

// Reconstructed from eboot.elf at 0x642250.
RenderShader* render_create_shader(RenderShaderStage stage) {
    auto& factory = *render_system_factory(*render_system_instance());
    return render_factory_create_shader(factory, stage);
}

// Reconstructed from eboot.elf at 0x642270.
void render_shader_construct(RenderShader& shader) {
    set_base_dispatch(shader);
    shader.owner = nullptr;
    shader.initialized = false;
    shader.variant_index = -1;
    shader.metadata = nullptr;
}

// Reconstructed from eboot.elf at 0x6422A0.
void render_shader_destruct(RenderShader&) {
}

// Reconstructed from eboot.elf at 0x6422B0.
void render_shader_delete(RenderShader& shader) {
    render_release(&shader);
}

// Reconstructed from eboot.elf at 0x6422C0.
bool render_shader_initialize(
    RenderShader& shader,
    void* owner,
    const RenderShaderBinary* binary,
    void* metadata) {
    shader.owner = owner;
    shader.metadata = metadata;
    render_shader_release(shader);

    if (binary == nullptr) {
        return true;
    }

    shader.initialized = dispatch(shader).initialize(shader, *binary);
    return shader.initialized;
}

// Reconstructed from eboot.elf at 0x642310.
void render_shader_release(RenderShader& shader) {
    if (!shader.initialized) {
        return;
    }

    dispatch(shader).release(shader);
    shader.initialized = false;
}

}  // namespace rb4
