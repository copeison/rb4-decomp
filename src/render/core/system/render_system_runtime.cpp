#include "render/core/system/render_system_runtime.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/buffers/render_constant_buffer.h"
#include "render/core/context/render_context.h"
#include "render/core/debug/render_gpu_stat_block.h"
#include "render/core/synchronization/render_deferred_release.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/audio/audio_analysis_textures.h"
#include "render/resources/lighting/render_lighting_resources.h"
#include "render/resources/lighting/render_lighting_resources_adapters.h"
#include "render/resources/meshes/primitive_mesh_set.h"
#include "render/resources/system/default_render_resources.h"
#include "render/resources/system/render_resource_manager.h"
#include "render/resources/shaders/fog_deferred_shader.h"

namespace rb4 {

namespace {

constexpr std::size_t kBuiltinBufferDescriptorOffset = 2712;
constexpr std::size_t kBuiltinBufferStorageOffset = 3712;
constexpr std::size_t kFloatsPerConstantBufferElement = 4;
constexpr std::size_t kPrimitiveMeshSetOffset = 3568;
constexpr std::size_t kAudioAnalysisTextureSetOffset = 3576;

struct RenderBuiltinBufferDescriptors {
    const RenderConstantBufferDescriptor* zero_pair;
    std::size_t zero_pair_index;
    std::uint8_t reserved_16[56];
    const RenderConstantBufferDescriptor* zero_vectors;
    std::size_t zero_vectors_index;
    std::uint8_t reserved_88[16];
    const RenderConstantBufferDescriptor* sentinel;
    std::size_t negative_sentinel_index;
    std::size_t zero_sentinel_index;
    const RenderConstantBufferDescriptor* default_values;
    std::size_t default_values_index;
};

struct RenderBuiltinBuffers {
    std::array<RenderConstantBuffer*, 4> entries;
};

static_assert(offsetof(RenderBuiltinBufferDescriptors, zero_pair) == 0);
static_assert(
    offsetof(RenderBuiltinBufferDescriptors, zero_vectors) == 72);
static_assert(offsetof(RenderBuiltinBufferDescriptors, sentinel) == 104);
static_assert(
    offsetof(RenderBuiltinBufferDescriptors, default_values) == 128);
static_assert(sizeof(RenderBuiltinBufferDescriptors) == 144);
static_assert(sizeof(RenderBuiltinBuffers) == 32);

RenderBuiltinBufferDescriptors& builtin_buffer_descriptors(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderBuiltinBufferDescriptors*>(
        bytes + kBuiltinBufferDescriptorOffset);
}

RenderBuiltinBuffers& builtin_buffers(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderBuiltinBuffers*>(
        bytes + kBuiltinBufferStorageOffset);
}

void* runtime_state_at(RenderSystem& system, std::size_t offset) {
    return reinterpret_cast<std::uint8_t*>(&system) + offset;
}

void*& runtime_pointer_at(RenderSystem& system, std::size_t offset) {
    return *reinterpret_cast<void**>(runtime_state_at(system, offset));
}

float* constant_buffer_element(
    RenderConstantBuffer& buffer,
    std::size_t index) {
    return static_cast<float*>(buffer.data) +
        kFloatsPerConstantBufferElement * index;
}

RenderConstantBuffer* create_deferred_builtin_buffer(
    const RenderConstantBufferDescriptor& descriptor) {
    return render_create_constant_buffer(descriptor, 1);
}

void finish_builtin_buffer_upload(RenderConstantBuffer& buffer) {
    if (buffer.upload_pending) {
        render_constant_buffer_initialize_backend(buffer);
        buffer.upload_pending = false;
    }
}

void initialize_runtime_resources(RenderSystem& system) {
    render_lighting_resources_initialize(
        render_system_lighting_resources(system));
    fog_deferred_shader_create(
        render_system_fog_deferred_shader(system));

    auto*& primitive_meshes =
        runtime_pointer_at(system, kPrimitiveMeshSetOffset);
    primitive_meshes = render_allocate(sizeof(RenderPrimitiveMeshSet));
    render_primitive_mesh_set_construct(
        *static_cast<RenderPrimitiveMeshSet*>(primitive_meshes));

    auto*& audio_textures =
        runtime_pointer_at(system, kAudioAnalysisTextureSetOffset);
    audio_textures = render_allocate(sizeof(AudioAnalysisTextureSet));
    audio_analysis_texture_set_construct(
        *static_cast<AudioAnalysisTextureSet*>(audio_textures));

    render_gpu_stat_block_initialize(
        render_system_gpu_stat_block(system));
}

void shutdown_runtime_resources(RenderSystem& system) {
    fog_deferred_shader_release(
        render_system_fog_deferred_shader(system));
    render_lighting_resources_shutdown(
        render_system_lighting_resources(system));
    render_resource_manager_shutdown(
        render_system_resource_manager(system));

    auto*& primitive_meshes =
        runtime_pointer_at(system, kPrimitiveMeshSetOffset);
    if (primitive_meshes != nullptr) {
        render_primitive_mesh_set_destruct(
            *static_cast<RenderPrimitiveMeshSet*>(primitive_meshes));
        render_release(primitive_meshes);
        primitive_meshes = nullptr;
    }

    auto*& audio_textures =
        runtime_pointer_at(system, kAudioAnalysisTextureSetOffset);
    if (audio_textures != nullptr) {
        audio_analysis_texture_set_destruct(
            *static_cast<AudioAnalysisTextureSet*>(audio_textures));
        render_release(audio_textures);
        audio_textures = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DDAE0.
void render_system_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options) {
    auto& runtime = render_system_core_state(system);
    runtime.initialized = true;
    runtime.init_options = options;
    auto& resource_manager = render_system_resource_manager(system);
    render_resource_manager_initialize(resource_manager);
    render_system_platform_initialize(system, options);
    render_resource_manager_finalize(resource_manager);

    initialize_runtime_resources(system);
    render_system_initialize_builtin_buffers(system);

    render_context_initialize(render_system_primary_render_context(system));
    const auto context_count = render_system_render_context_count(system);
    for (std::size_t index = 0; index < context_count; ++index) {
        render_context_initialize(
            render_system_render_context_at(system, index));
    }

    render_system_platform_finish_initialization(system);
    render_system_begin_runtime_epoch(system);
}

// Reconstructed from eboot.elf at 0x3DDC20.
void render_system_initialize_builtin_buffers(RenderSystem& system) {
    const auto& descriptors = builtin_buffer_descriptors(system);
    auto& buffers = builtin_buffers(system).entries;

    buffers[0] = create_deferred_builtin_buffer(*descriptors.zero_pair);
    auto* zero_pair = constant_buffer_element(
        *buffers[0], descriptors.zero_pair_index);
    zero_pair[0] = 0.0F;
    zero_pair[1] = 0.0F;
    finish_builtin_buffer_upload(*buffers[0]);

    buffers[1] = create_deferred_builtin_buffer(*descriptors.zero_vectors);
    auto* zero_vectors = constant_buffer_element(
        *buffers[1], descriptors.zero_vectors_index);
    std::fill_n(zero_vectors, 16, 0.0F);
    finish_builtin_buffer_upload(*buffers[1]);

    buffers[2] = create_deferred_builtin_buffer(*descriptors.sentinel);
    auto* negative_sentinel = constant_buffer_element(
        *buffers[2], descriptors.negative_sentinel_index);
    negative_sentinel[0] = -1.0F;
    auto* zero_sentinel = constant_buffer_element(
        *buffers[2], descriptors.zero_sentinel_index);
    std::fill_n(zero_sentinel, kFloatsPerConstantBufferElement, 0.0F);
    finish_builtin_buffer_upload(*buffers[2]);

    buffers[3] = create_deferred_builtin_buffer(*descriptors.default_values);
    auto* default_values = constant_buffer_element(
        *buffers[3], descriptors.default_values_index);
    default_values[0] = 0.0F;
    default_values[1] = 1.0F;
    finish_builtin_buffer_upload(*buffers[3]);
}

// Reconstructed from eboot.elf at 0x3DDE60.
void render_system_release_builtin_buffers(RenderSystem& system) {
    for (auto*& buffer : builtin_buffers(system).entries) {
        if (buffer != nullptr) {
            render_constant_buffer_release_dynamic(*buffer);
            buffer = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0x3DDE60.
void render_system_shutdown(RenderSystem& system) {
    render_system_core_state(system).shutting_down = true;
    render_system_flush_deferred_releases(system);
    render_release_default_resources(
        render_system_default_resources(system));
    shutdown_runtime_resources(system);
    render_system_release_builtin_buffers(system);

    render_context_shutdown(render_system_primary_render_context(system));
    const auto context_count = render_system_render_context_count(system);
    for (std::size_t index = 0; index < context_count; ++index) {
        render_context_shutdown(
            render_system_render_context_at(system, index));
    }

    render_system_platform_shutdown(system);
}

}  // namespace rb4
