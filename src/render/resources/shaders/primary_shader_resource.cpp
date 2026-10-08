#include "render/resources/shaders/primary_shader_resource.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/names/render_resource_name_adapters.h"
#include "render/resources/shaders/primary_shader_resource_adapters.h"
#include "render/resources/shaders/shader_parameter_registry.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

namespace {

constexpr std::size_t kManagerLinkOffset = 272;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
};

struct RenderManagedObjectArray {
    RenderManagedObject** begin;
    RenderManagedObject** end;
    RenderManagedObject** capacity;
    void* allocator;
};

struct RenderShaderNameRecord {
    std::uint8_t reserved_0[16];
    std::uint8_t resource_name[16];
};

struct RenderShaderNameRecordArray {
    RenderShaderNameRecord* begin;
    RenderShaderNameRecord* end;
    RenderShaderNameRecord* capacity;
    void* allocator;
};

struct RenderShaderCompileState {
    const void* source_identifier;
    std::int32_t resource_type;
    std::int32_t shader_variant;
    void* owner;
    void* reserved_24;
    bool initialized;
    std::uint8_t reserved_33[7];
    void* entries_begin;
    void* entries_end;
    void* entries_capacity;
    void* entries_allocator;
};

struct RenderByteArray {
    void* begin;
    void* end;
    void* capacity;
    void* allocator;
};

struct RenderShaderBackendState {
    RenderByteArray arrays[24];
    std::uint8_t reserved_768[96];
};

static_assert(sizeof(RenderManagedObjectArray) == 32);
static_assert(sizeof(RenderShaderNameRecord) == 32);
static_assert(sizeof(RenderShaderNameRecordArray) == 32);
static_assert(sizeof(RenderShaderCompileState) == 72);
static_assert(sizeof(RenderByteArray) == 32);
static_assert(sizeof(RenderShaderBackendState) == 864);

template <typename Element>
void release_array_storage(
    Element*& begin,
    Element*& end,
    Element*& capacity) {
    if (begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(capacity) -
            reinterpret_cast<std::uint8_t*>(begin));
        engine_deallocate_sized(begin, byte_count);
    }
    begin = nullptr;
    end = nullptr;
    capacity = nullptr;
}

void clear_compiled_objects(RenderManagedObjectArray& objects) {
    for (auto** object = objects.begin; object != objects.end; ++object) {
        if (*object != nullptr) {
            (*object)->dispatch->release_dynamic(*object);
        }
    }
    objects.end = objects.begin;
}

void destruct_name_records(RenderShaderNameRecordArray& names) {
    for (auto* name = names.begin; name != names.end; ++name) {
        render_resource_name_destruct(name->resource_name);
    }
    release_array_storage(names.begin, names.end, names.capacity);
}

void destruct_parameter_registry(RenderShaderParameterRegistry& registry) {
    for (auto* parameter = registry.begin;
         parameter != registry.end;
         ++parameter) {
        render_resource_name_destruct(&parameter->name);
    }
    release_array_storage(
        registry.begin,
        registry.end,
        registry.capacity);
}

void destruct_backend_state(RenderShaderBackendState& state) {
    for (std::size_t index = 24; index != 0; --index) {
        auto& array = state.arrays[index - 1];
        if (array.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                static_cast<std::uint8_t*>(array.capacity) -
                static_cast<std::uint8_t*>(array.begin));
            engine_deallocate_sized(array.begin, byte_count);
        }
    }
}

}  // namespace

struct RenderPrimaryShaderResource {
    struct Dispatch {
        void* reserved_0[2];
        const void* (*source_identifier)(RenderPrimaryShaderResource* shader);
        void* (*backend_name)(RenderPrimaryShaderResource* shader);
        void (*initialize_support_objects)(
            RenderPrimaryShaderResource* shader,
            RenderShaderNameRecordArray* names,
            RenderShaderParameterRegistrySet* parameters,
            RenderShaderCompileState* compile_state,
            RenderShaderBackendState* backend_state);
        std::int32_t (*mode)(RenderPrimaryShaderResource* shader);
        std::int32_t (*variant)(RenderPrimaryShaderResource* shader);
    };

    Dispatch* dispatch;
    std::int32_t variant;
    bool compiled;
    std::uint8_t reserved_13[3];
    RenderManagedObjectArray compiled_objects[6];
    void* backend_name;
    RenderShaderNameRecordArray* names;
    RenderShaderParameterRegistrySet* parameters;
    RenderShaderCompileState* compile_state;
    RenderShaderBackendState* backend_state;
    RenderShaderParameterBinding render_target_slice_binding;
    std::uint8_t reserved_268[4];
    RenderResourceListNode manager_link;
};

static_assert(offsetof(RenderPrimaryShaderResource::Dispatch, mode) == 40);
static_assert(offsetof(RenderPrimaryShaderResource::Dispatch, variant) == 48);
static_assert(offsetof(RenderPrimaryShaderResource, compiled) == 12);
static_assert(offsetof(RenderPrimaryShaderResource, compiled_objects) == 16);
static_assert(offsetof(RenderPrimaryShaderResource, backend_name) == 208);
static_assert(offsetof(RenderPrimaryShaderResource, names) == 216);
static_assert(offsetof(RenderPrimaryShaderResource, parameters) == 224);
static_assert(offsetof(RenderPrimaryShaderResource, compile_state) == 232);
static_assert(offsetof(RenderPrimaryShaderResource, backend_state) == 240);
static_assert(
    offsetof(RenderPrimaryShaderResource, render_target_slice_binding) == 248);
static_assert(
    offsetof(RenderPrimaryShaderResource, manager_link) == kManagerLinkOffset);
static_assert(sizeof(RenderPrimaryShaderResource) == 288);

RenderPrimaryShaderResource& render_primary_shader_from_link(
    RenderResourceListNode& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderPrimaryShaderResource*>(
        bytes - kManagerLinkOffset);
}

// Reconstructed from eboot.elf at 0x638A20.
void render_primary_shader_register(RenderPrimaryShaderResource& shader) {
    auto& manager = render_system_resource_manager(*render_system_instance());
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
    render_primary_shader_set_base_dispatch(shader);
    shader.variant = 0;
    shader.compiled = false;
    for (auto& objects : shader.compiled_objects) {
        objects = {};
    }
    shader.backend_name = nullptr;
    shader.names = nullptr;
    shader.parameters = nullptr;
    shader.compile_state = nullptr;
    shader.backend_state = nullptr;
    shader.render_target_slice_binding = {};
    shader.manager_link.next = &shader.manager_link;
    shader.manager_link.previous = &shader.manager_link;
}

// Reconstructed from eboot.elf at 0x638110.
void render_primary_shader_destruct(RenderPrimaryShaderResource& shader) {
    render_primary_shader_set_base_dispatch(shader);

    if (shader.names != nullptr) {
        destruct_name_records(*shader.names);
        render_release(shader.names);
        shader.names = nullptr;
    }
    if (shader.parameters != nullptr) {
        for (std::size_t index = 6; index != 0; --index) {
            destruct_parameter_registry(
                shader.parameters->registries[index - 1]);
        }
        render_release(shader.parameters);
        shader.parameters = nullptr;
    }
    if (shader.compile_state != nullptr) {
        auto& state = *shader.compile_state;
        if (state.entries_begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                static_cast<std::uint8_t*>(state.entries_capacity) -
                static_cast<std::uint8_t*>(state.entries_begin));
            engine_deallocate_sized(state.entries_begin, byte_count);
        }
        render_release(shader.compile_state);
        shader.compile_state = nullptr;
    }
    if (shader.backend_state != nullptr) {
        destruct_backend_state(*shader.backend_state);
        render_release(shader.backend_state);
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
    if (shader.compile_state != nullptr) {
        return;
    }

    shader.variant = shader.dispatch->variant(&shader);

    auto* names = static_cast<RenderShaderNameRecordArray*>(
        render_allocate(sizeof(RenderShaderNameRecordArray)));
    *names = {};
    shader.names = names;

    auto* parameters = static_cast<RenderShaderParameterRegistrySet*>(
        render_allocate(sizeof(RenderShaderParameterRegistrySet)));
    render_shader_parameter_registry_set_construct(*parameters);
    shader.parameters = parameters;

    auto* compile_state = static_cast<RenderShaderCompileState*>(
        render_allocate(sizeof(RenderShaderCompileState)));
    *compile_state = {};
    compile_state->source_identifier =
        shader.dispatch->source_identifier(&shader);
    compile_state->resource_type = 8;
    compile_state->shader_variant = shader.variant;
    shader.compile_state = compile_state;

    auto* backend_state = static_cast<RenderShaderBackendState*>(
        render_allocate(sizeof(RenderShaderBackendState)));
    *backend_state = {};
    shader.backend_state = backend_state;

    const Symbol render_target_slices("HX_NUM_RT_SLICES");
    render_shader_parameter_registry_add(
        &shader.render_target_slice_binding,
        &parameters->registries[0],
        render_target_slices.value(),
        0,
        7);

    shader.dispatch->initialize_support_objects(
        &shader,
        names,
        parameters,
        compile_state,
        backend_state);
}

// Reconstructed from eboot.elf at 0x6383D0.
void render_primary_shader_finalize(RenderPrimaryShaderResource& shader) {
    render_primary_shader_prepare(shader);

    const auto mode = shader.dispatch->mode(&shader);
    const auto& options =
        render_system_core_state(*render_system_instance()).init_options;
    if (mode == 1) {
        if (!options.option2) {
            return;
        }
    } else if (mode == 0) {
        if (!options.initialize_rendering) {
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

}  // namespace rb4
