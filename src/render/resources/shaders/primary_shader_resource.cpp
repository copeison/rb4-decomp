#include "render/resources/shaders/primary_shader_resource.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/text/Symbol.h"
#include "render/context/RndContext.h"
#include "render/system/RndDevice.h"
#include "render/resources/shaders/builtin_shader_resources.h"
#include "render/resources/shaders/compiled_shader_objects.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/shaders/RndShaderProgram.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

namespace {

constexpr std::size_t kManagerLinkOffset = 272;

template <typename Element>
void release_array_storage(
    Element*& begin,
    Element*& end,
    Element*& capacity) {
    if (begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(capacity) -
            reinterpret_cast<std::uint8_t*>(begin));
        HmxAllocator::gStlAllocator.deallocate(begin, byte_count);
    }
    begin = nullptr;
    end = nullptr;
    capacity = nullptr;
}

void clear_compiled_objects(RenderManagedObjectArray& objects) {
    for (auto** object = objects.begin; object != objects.end; ++object) {
        delete *object;
    }
    objects.end = objects.begin;
}

void destruct_parameter_registry(RenderShaderParameterRegistry& registry) {
    for (auto* parameter = registry.begin;
         parameter != registry.end;
         ++parameter) {
        (parameter->name).~String();
    }
    release_array_storage(
        registry.begin,
        registry.end,
        registry.capacity);
}

}  // namespace

namespace {

void delete_primary_shader(RenderPrimaryShaderResource* shader) {
    render_primary_shader_destruct(*shader);
    MemFree(shader);
}

std::int32_t primary_shader_mode(RenderPrimaryShaderResource*) {
    return 0;
}

std::int32_t primary_shader_variant(RenderPrimaryShaderResource*) {
    return 13;
}

// The base dispatch at 0x192EF60 leaves the shader-specific slots pure.
RenderPrimaryShaderResource::Dispatch kBasePrimaryShaderDispatch{
    [](RenderPrimaryShaderResource* shader) {
        render_primary_shader_destruct(*shader);
    },
    delete_primary_shader,
    nullptr,
    nullptr,
    nullptr,
    primary_shader_mode,
    primary_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

void set_base_dispatch(RenderPrimaryShaderResource& shader) {
    shader.dispatch = &kBasePrimaryShaderDispatch;
}

}  // namespace

RenderPrimaryShaderResource& render_primary_shader_from_link(
    RenderResourceListNode& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderPrimaryShaderResource*>(
        bytes - kManagerLinkOffset);
}

// Reconstructed from eboot.elf at 0x638A20.
void render_primary_shader_register(RenderPrimaryShaderResource& shader) {
    auto& manager = TheRndDevice()->mResourceMgr;
    auto* sentinel = manager.primary_list;
    auto* previous = sentinel->previous;
    shader.manager_link.next = sentinel;
    shader.manager_link.previous = previous;
    previous->next = &shader.manager_link;
    sentinel->previous = &shader.manager_link;

    if (manager.shader_constants.initialization_phases[1] != 0) {
        render_primary_shader_finalize(shader);
    }
}

// Reconstructed from eboot.elf at 0x6380B0.
void render_primary_shader_construct(RenderPrimaryShaderResource& shader) {
    set_base_dispatch(shader);
    shader.variant = 0;
    shader.compiled = false;
    for (auto& objects : shader.compiled_objects) {
        objects = {};
    }
    shader.backend_name = nullptr;
    shader.constants = nullptr;
    shader.parameters = nullptr;
    shader.constant_block = nullptr;
    shader.backend_state = nullptr;
    shader.render_target_slice_binding = {};
    shader.manager_link.next = &shader.manager_link;
    shader.manager_link.previous = &shader.manager_link;
}

// Reconstructed from eboot.elf at 0x638110.
void render_primary_shader_destruct(RenderPrimaryShaderResource& shader) {
    set_base_dispatch(shader);

    render_shader_constant_registry_release(shader.constants);
    if (shader.parameters != nullptr) {
        for (std::size_t index = 6; index != 0; --index) {
            destruct_parameter_registry(
                shader.parameters->registries[index - 1]);
        }
        MemFree(shader.parameters);
        shader.parameters = nullptr;
    }
    if (shader.constant_block != nullptr) {
        render_shader_constant_block_release(shader.constant_block);
    }
    if (shader.backend_state != nullptr) {
        render_shader_backend_state_destruct(*shader.backend_state);
        MemFree(shader.backend_state);
        shader.backend_state = nullptr;
    }

    shader.manager_link.next->previous = shader.manager_link.previous;
    shader.manager_link.previous->next = shader.manager_link.next;

    for (auto& objects : shader.compiled_objects) {
        clear_compiled_objects(objects);
    }
    for (std::size_t index = 6; index != 0; --index) {
        auto& objects = shader.compiled_objects[index - 1];
        release_array_storage(objects.begin, objects.end, objects.capacity);
    }
}

// Reconstructed from eboot.elf at 0x638270.
void render_primary_shader_prepare(RenderPrimaryShaderResource& shader) {
    if (shader.constant_block != nullptr) {
        return;
    }

    shader.variant = shader.dispatch->variant(&shader);

    auto* constants = static_cast<RenderShaderConstantRegistry*>(
        operator new(sizeof(RenderShaderConstantRegistry)));
    render_shader_constant_registry_construct(*constants);
    shader.constants = constants;

    auto* parameters = static_cast<RenderShaderParameterRegistrySet*>(
        operator new(sizeof(RenderShaderParameterRegistrySet)));
    render_shader_parameter_registry_set_construct(*parameters);
    shader.parameters = parameters;

    auto* constant_block = static_cast<RenderShaderConstantBlock*>(
        operator new(sizeof(RenderShaderConstantBlock)));
    render_shader_constant_block_construct(
        *constant_block,
        static_cast<const char*>(
            shader.dispatch->source_identifier(&shader)),
        8,
        static_cast<std::uint32_t>(shader.variant),
        0);
    shader.constant_block = constant_block;

    auto* backend_state = static_cast<RenderShaderBackendState*>(
        operator new(sizeof(RenderShaderBackendState)));
    render_shader_backend_state_construct(*backend_state);
    shader.backend_state = backend_state;

    const Symbol render_target_slices("HX_NUM_RT_SLICES");
    render_shader_parameter_registry_add(
        &shader.render_target_slice_binding,
        &parameters->registries[0],
        render_target_slices.Str(),
        0,
        7);

    shader.dispatch->initialize_support_objects(
        &shader,
        constants,
        parameters,
        constant_block,
        backend_state);
}

// Reconstructed from eboot.elf at 0x6383D0.
void render_primary_shader_finalize(RenderPrimaryShaderResource& shader) {
    render_primary_shader_prepare(shader);

    const auto mode = shader.dispatch->mode(&shader);
    const auto& options =
        TheRndDevice()->mInitParams;
    if (mode == 1) {
        if (!options.mUnknown2) {
            return;
        }
    } else if (mode == 0) {
        if (!options.mInitRendering) {
            return;
        }
    } else {
        return;
    }

    render_primary_shader_initialize_backend(shader);
}

// Reconstructed from eboot.elf at 0x6388C0 and 0x63B210.
void render_primary_shader_clear_compiled_objects(
    RenderPrimaryShaderResource& shader) {
    for (auto& objects : shader.compiled_objects) {
        clear_compiled_objects(objects);
    }
    shader.compiled = false;
}

// Reconstructed from eboot.elf at 0x6388F0.
bool render_primary_shader_validate_permutation(
    void*,
    std::uint32_t,
    std::uint64_t) {
    return true;
}

// Reconstructed from eboot.elf at 0x638900. Shaders without a fallback of
// their own bind the error shader with geometry type zero.
void render_primary_shader_bind_fallback(void*, void* context) {
    auto& resources =
        TheRndDevice()->mResourceMgr
            .runtime.resources;
    render_error_shader_bind(resources.error_shader, context, 0);
}

// Reconstructed from eboot.elf at 0x450870.
bool render_primary_shader_supports_render_target_slices(void*) {
    return false;
}

// Reconstructed from eboot.elf at 0x450880.
bool render_primary_shader_uses_geometry_program(void*) {
    return false;
}

// Matches the one-instruction true overrides at 0x639EF0, 0x6364B0,
// 0x63E810, and 0x6F3520.
bool render_primary_shader_returns_true(void*) {
    return true;
}

// Reconstructed from eboot.elf at 0x63E820.
void render_primary_shader_bind_nothing(void*, void*) {}

// Reconstructed from eboot.elf at 0x638920. Selects the HX_NUM_RT_SLICES
// value for the context's slice mode, writes it into the global half of every
// program key, and binds the matching compiled objects. Single-slice draws
// drop the geometry program unless the shader declares one. A failed bind
// falls back through dispatch slot 8.
bool render_primary_shader_bind(
    RenderPrimaryShaderResource& shader,
    RndContext& context,
    std::uint64_t (&keys)[kRenderShaderProgramKeyCount]) {
    // Table at 0x12A99A0, indexed by slice mode.
    constexpr std::uint32_t kSliceCounts[] = {1, 2, 6, 1, 1, 1, 1, 1, 1, 1, 1};
    constexpr std::int32_t kGeometryProgramBit = 0x4;

    if (!shader.compiled) {
        render_primary_shader_initialize_backend(shader);
    }

    const auto mode = (context).mSliceMode;
    std::uint32_t slices = 0;
    if (mode == -1) {
        slices = 1;
    } else if (static_cast<std::uint32_t>(mode) <
               sizeof(kSliceCounts) / sizeof(*kSliceCounts)) {
        slices = kSliceCounts[mode];
    }

    const auto& binding = shader.render_target_slice_binding;
    const auto field = static_cast<std::uint64_t>(
        (slices - binding.first_value) << binding.bit_offset) << 32;
    const auto mask = static_cast<std::uint64_t>(binding.shifted_mask) << 32;
    for (auto& key : keys) {
        key = (key & ~mask) | field;
    }

    auto variant = shader.variant;
    if (slices == 1 && !shader.dispatch->uses_geometry_program(&shader)) {
        variant &= ~kGeometryProgramBit;
    }
    if (render_compiled_shader_objects_bind(
            shader.compiled_objects, context, variant, keys)) {
        return true;
    }
    shader.dispatch->bind_fallback(&shader, &context);
    return false;
}

}  // namespace rb4
