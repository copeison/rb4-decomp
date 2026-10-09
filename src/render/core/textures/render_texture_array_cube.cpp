#include "render/core/textures/render_texture_array_cube.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/system/RndFactory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTextureArrayCubeDispatch {
    void (*destruct)(RenderTextureArrayCube& texture);
    void (*delete_texture)(RenderTextureArrayCube& texture);
    std::int32_t (*descriptor_type)(const RenderTextureArrayCube& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTextureArrayCube& texture);
    std::size_t (*source_size)(const RenderTextureArrayCube& texture);
    void (*apply_creation_state)(
        RenderTextureArrayCube& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTextureArrayCube& texture);
    RenderTextureArrayCube* (*identity)(
        RenderTextureArrayCube& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTextureArrayCube& texture,
        const RenderTextureArrayCube* reusable_texture);
    bool (*reserved_predicate)(const RenderTextureArrayCube& texture);
};

std::int32_t descriptor_type(const RenderTextureArrayCube& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTextureArrayCube& texture) {
    return render_texture_mip_chain_level_count(
        texture.cubes.begin->faces[0]);
}

std::size_t source_size(const RenderTextureArrayCube& texture) {
    return render_texture_cube_array_source_size(texture.cubes);
}

void apply_creation_state(
    RenderTextureArrayCube& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTextureArrayCube& texture) {
    render_texture_cube_array_release_source_data(texture.cubes);
}

RenderTextureArrayCube* identity(
    RenderTextureArrayCube& texture,
    std::int64_t&) {
    return &texture;
}

bool reserved_predicate(const RenderTextureArrayCube&) {
    return false;
}

RenderTextureArrayCubeDispatch kBaseTextureArrayCubeDispatch{
    render_texture_array_cube_destruct,
    render_texture_array_cube_delete,
    descriptor_type,
    {},
    mip_level_count,
    source_size,
    apply_creation_state,
    release_source_data,
    identity,
    nullptr,
    reserved_predicate,
};

static_assert(
    offsetof(RenderTextureArrayCubeDispatch, mip_level_count) == 80);
static_assert(
    offsetof(RenderTextureArrayCubeDispatch, release_source_data) == 104);
static_assert(
    offsetof(RenderTextureArrayCubeDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTextureArrayCubeDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTextureArrayCube& texture) {
    texture.implementation = &kBaseTextureArrayCubeDispatch;
}

std::size_t cube_count(const RenderTextureCubeArray& cubes) {
    return cubes.begin == nullptr
        ? 0
        : static_cast<std::size_t>(cubes.end - cubes.begin);
}

std::size_t cube_capacity(const RenderTextureCubeArray& cubes) {
    return cubes.begin == nullptr
        ? 0
        : static_cast<std::size_t>(cubes.capacity - cubes.begin);
}

void destroy_cubes(RenderTextureCubeState* begin, RenderTextureCubeState* end) {
    for (auto* cube = begin; cube != end; ++cube) {
        render_texture_cube_state_destruct(*cube);
    }
}

void release_cube_storage(RenderTextureCubeArray& cubes) {
    if (cubes.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            cubes.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(cubes.capacity) -
                reinterpret_cast<std::uint8_t*>(cubes.begin)));
    }
}

void reserve_cubes(RenderTextureCubeArray& cubes, std::size_t capacity) {
    if (capacity <= cube_capacity(cubes)) {
        return;
    }
    auto* replacement = static_cast<RenderTextureCubeState*>(
        HmxAllocator::gStlAllocator.allocate(capacity * sizeof(RenderTextureCubeState)));
    auto* output = replacement;
    for (auto* input = cubes.begin; input != cubes.end; ++input, ++output) {
        render_texture_cube_state_construct(
            *output,
            reinterpret_cast<const RenderTextureCubeDescriptorState&>(*input),
            true);
    }
    destroy_cubes(cubes.begin, cubes.end);
    release_cube_storage(cubes);
    cubes.begin = replacement;
    cubes.end = output;
    cubes.capacity = replacement + capacity;
}

void append_cube(
    RenderTextureCubeArray& cubes,
    const RenderTextureCubeDescriptorState& descriptor,
    bool has_source_data) {
    if (cubes.end == cubes.capacity) {
        const auto count = cube_count(cubes);
        reserve_cubes(cubes, count == 0 ? 1 : count * 2);
    }
    render_texture_cube_state_construct(
        *cubes.end, descriptor, has_source_data);
    ++cubes.end;
}

}  // namespace

// Reconstructed from eboot.elf at 0x69AF50.
void render_texture_cube_array_construct(
    RenderTextureDescriptorState& descriptor_state,
    RenderTextureCubeArray& cubes,
    const RenderTextureArrayCubeDescriptor& descriptor,
    bool has_source_data) {
    descriptor_state = descriptor.texture_state;
    cubes = {};
    const auto count = static_cast<std::size_t>(
        descriptor.cubes.end - descriptor.cubes.begin);
    reserve_cubes(cubes, count);
    for (auto* cube = descriptor.cubes.begin;
         cube != descriptor.cubes.end;
         ++cube) {
        append_cube(cubes, *cube, has_source_data);
    }
}

// Reconstructed from eboot.elf at 0x69ABB0.
bool render_texture_cube_array_validate(
    const RenderTextureCubeArray& cubes) {
    const auto count = cube_count(cubes);
    if (count == 0 || count > 341 ||
        !render_texture_cube_prepare_descriptor(
            reinterpret_cast<const RenderTextureCubeDescriptorState&>(
                *cubes.begin))) {
        return false;
    }

    const auto& first = cubes.begin->faces[0];
    const auto expected_levels = render_texture_mip_chain_level_count(first);
    for (auto* cube = cubes.begin + 1; cube != cubes.end; ++cube) {
        const auto& face = cube->faces[0];
        if (face.fields.width != first.fields.width ||
            face.fields.height != first.fields.height ||
            face.fields.depth != first.fields.depth ||
            face.fields.data_format != first.fields.data_format ||
            render_texture_mip_chain_level_count(face) != expected_levels) {
            return false;
        }
    }
    return true;
}

void render_texture_cube_array_destruct(RenderTextureCubeArray& cubes) {
    destroy_cubes(cubes.begin, cubes.end);
    release_cube_storage(cubes);
    cubes = {};
}

std::size_t render_texture_cube_array_source_size(
    const RenderTextureCubeArray& cubes) {
    if (cubes.begin == cubes.end) {
        return 0;
    }
    return cube_count(cubes) *
        render_texture_cube_state_source_size(*cubes.begin);
}

void render_texture_cube_array_release_source_data(
    RenderTextureCubeArray& cubes) {
    for (auto* cube = cubes.begin; cube != cubes.end; ++cube) {
        render_texture_cube_state_release_source_data(*cube);
    }
}

// Reconstructed from eboot.elf at 0x69AF00.
void render_texture_array_cube_descriptor_construct(
    RenderTextureArrayCubeDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    descriptor.cubes = {};
    descriptor.texture_state.descriptor_type = 7;
}

// Reconstructed from eboot.elf at 0x69AAC0.
void render_texture_array_cube_construct(
    RenderTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor) {
    render_texture_construct(texture);
    set_base_dispatch(texture);
    render_texture_cube_array_construct(
        texture.descriptor_state,
        texture.cubes,
        descriptor,
        render_texture_descriptor_has_source_data(descriptor.texture_state));
    render_texture_cube_array_validate(texture.cubes);

    texture.resource_index = 2;
    const auto& first_face = texture.cubes.begin->faces[0].fields;
    texture.descriptor_state.data_format = first_face.data_format;
    texture.descriptor_state.width = first_face.width;
    texture.descriptor_state.height = first_face.height;
    texture.descriptor_state.depth = first_face.depth;
    texture.descriptor_state.array_size =
        static_cast<std::size_t>(texture.cubes.end - texture.cubes.begin);
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x69AA60.
RenderTextureArrayCube* render_create_texture_array_cube(
    RenderTextureArrayCubeDescriptor& descriptor,
    RenderTextureArrayCube* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state, 7, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = factory.CreateTextureArrayCube(descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x69ACD0.
void render_texture_array_cube_destruct(RenderTextureArrayCube& texture) {
    set_base_dispatch(texture);
    render_texture_cube_array_destruct(texture.cubes);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x69AD10.
void render_texture_array_cube_delete(RenderTextureArrayCube& texture) {
    render_texture_array_cube_destruct(texture);
    MemFree(&texture);
}

}  // namespace rb4
