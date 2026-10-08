#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_data_format_adapters.h"
#include "render/core/textures/render_texture_array_1d.h"
#include "render/core/textures/render_texture_mip_chain_adapters.h"
#include "render/resources/system/render_resource_manager_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kResourceManagerOffset = 2544;
constexpr std::size_t kPrimaryShaderLinkOffset = 272;
constexpr std::ptrdiff_t kSecondaryShaderDirtyOffset = -395;

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

struct RenderSizedArrayOwner {
    std::uint8_t reserved_0[40];
    void* begin;
    void* end;
    void* capacity;
    void* allocator;
};

struct RenderResourceNameArrayOwner {
    std::uint8_t* begin;
    std::uint8_t* end;
    std::uint8_t* capacity;
    void* allocator;
};

struct RenderResourceNameRecord {
    std::uint8_t storage[40];
};

struct RenderResourceNameArray {
    RenderResourceNameRecord* begin;
    RenderResourceNameRecord* end;
    RenderResourceNameRecord* capacity;
    void* allocator;
    void* allocator_state;
};

struct RenderResourceSpecializedState {
    RenderResourceNameArray arrays[6];
};

static_assert(sizeof(RenderResourceNameRecord) == 40);
static_assert(sizeof(RenderResourceNameArray) == 40);
static_assert(sizeof(RenderResourceSpecializedState) == 240);

struct RenderPrimaryShaderResource {
    struct Dispatch {
        void* reserved_0[5];
        std::int32_t (*mode)(RenderPrimaryShaderResource* shader);
    };

    Dispatch* dispatch;
    std::uint8_t reserved_8[4];
    bool compiled;
    std::uint8_t reserved_13[3];
    RenderManagedObjectArray compiled_objects[6];
    std::uint8_t reserved_208[64];
    RenderResourceListNode manager_link;
};

static_assert(sizeof(RenderManagedObjectArray) == 32);
static_assert(offsetof(RenderPrimaryShaderResource::Dispatch, mode) == 40);
static_assert(offsetof(RenderPrimaryShaderResource, compiled) == 12);
static_assert(offsetof(RenderPrimaryShaderResource, compiled_objects) == 16);
static_assert(
    offsetof(RenderPrimaryShaderResource, manager_link) ==
    kPrimaryShaderLinkOffset);

void initialize_registry(std::uint8_t* registry) {
    std::fill_n(registry, 20, std::uint8_t{0});
}

RenderResourceListNode* create_list_sentinel() {
    auto* node = static_cast<RenderResourceListNode*>(
        render_allocate(sizeof(RenderResourceListNode)));
    node->next = node;
    node->previous = node;
    return node;
}

void release_list_sentinel(RenderResourceListNode*& node) {
    if (node == nullptr) {
        return;
    }

    node->next->previous = node->previous;
    node->previous->next = node->next;
    render_release(node);
    node = nullptr;
}

RenderPrimaryShaderResource& primary_shader_from_link(
    RenderResourceListNode& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderPrimaryShaderResource*>(
        bytes - kPrimaryShaderLinkOffset);
}

void clear_compiled_objects(RenderManagedObjectArray& objects) {
    for (auto** object = objects.begin; object != objects.end; ++object) {
        if (*object != nullptr) {
            (*object)->dispatch->release_dynamic(*object);
        }
    }
    objects.end = objects.begin;
}

void release_sized_array_owner(void*& storage) {
    auto* owner = static_cast<RenderSizedArrayOwner*>(storage);
    if (owner == nullptr) {
        return;
    }
    if (owner->begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            static_cast<std::uint8_t*>(owner->capacity) -
            static_cast<std::uint8_t*>(owner->begin));
        engine_deallocate_sized(owner->begin, byte_count);
    }
    render_release(owner);
    storage = nullptr;
}

void release_dynamic_resource(void*& storage) {
    auto* resource = static_cast<RenderManagedObject*>(storage);
    if (resource != nullptr) {
        resource->dispatch->release_dynamic(resource);
        storage = nullptr;
    }
}

void destruct_name_array(RenderResourceNameArray& names) {
    for (auto* name = names.begin; name != names.end; ++name) {
        render_resource_name_destruct(name);
    }
    if (names.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(names.capacity) -
            reinterpret_cast<std::uint8_t*>(names.begin));
        engine_deallocate_sized(names.begin, byte_count);
    }
}

// Reconstructed from eboot.elf at 0x6383D0.
void finalize_primary_shader(RenderPrimaryShaderResource& shader) {
    render_primary_shader_prepare(&shader);

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

    render_primary_shader_initialize_backend(&shader);
}

// Reconstructed from eboot.elf at 0x645F20.
float sample_function_table(
    std::uint32_t function_index,
    float input) {
    float output = 0.0F;
    switch (function_index) {
    case 0:
        output = 1.0F - input;
        break;
    case 1: {
        const auto denominator = 1.25F * input + 0.25F;
        output = 0.0642857179F / (denominator * denominator) -
            0.0285714306F;
        break;
    }
    case 2: {
        constexpr float kMinimum = 0.006737947F;
        const auto exponential = 1.0F / std::exp(5.0F * input);
        output = (exponential - kMinimum) / (1.0F - kMinimum);
        break;
    }
    case 3: {
        constexpr float kMinimum = 0.006692851F;
        constexpr float kMaximum = 0.993307173F;
        const auto sigmoid =
            1.0F / (std::exp((input - 0.5F) * 10.0F) + 1.0F);
        output = (sigmoid - kMinimum) / (kMaximum - kMinimum);
        break;
    }
    default:
        break;
    }
    return std::max(0.0F, std::min(1.0F, output));
}

}  // namespace

RenderResourceManager& render_system_resource_manager(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderResourceManager*>(
        bytes + kResourceManagerOffset);
}

// Reconstructed from eboot.elf at 0x63F180.
void render_resource_manager_construct(RenderResourceManager& manager) {
    for (std::size_t index = 0; index < 4; ++index) {
        initialize_registry(manager.registries + index * 20);
    }

    std::fill_n(
        manager.handle_state,
        34,
        std::int64_t{-1});
    for (const auto index : {0U, 1U, 11U, 13U, 20U, 22U, 24U, 27U, 29U}) {
        manager.handle_state[index] = 0;
    }
    manager.runtime = {};

    manager.pointer_array = static_cast<RenderResourcePointerArray*>(
        render_allocate(sizeof(RenderResourcePointerArray)));
    *manager.pointer_array = {};
    manager.primary_list = create_list_sentinel();
    manager.secondary_list = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x63F350.
void render_resource_manager_destruct(RenderResourceManager& manager) {
    release_list_sentinel(manager.primary_list);
    release_list_sentinel(manager.secondary_list);

    if (manager.pointer_array != nullptr) {
        auto& array = *manager.pointer_array;
        if (array.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(array.capacity) -
                reinterpret_cast<std::uint8_t*>(array.begin));
            engine_deallocate_sized(array.begin, byte_count);
        }
        render_release(manager.pointer_array);
        manager.pointer_array = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x641370.
void render_resource_manager_finalize(RenderResourceManager& manager) {
    reinterpret_cast<std::uint8_t*>(manager.handle_state)[1] = 1;
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        finalize_primary_shader(primary_shader_from_link(*node));
    }

    constexpr std::uint32_t kFunctionCount = 4;
    constexpr std::uint32_t kSampleCount = 128;
    constexpr float kSampleStep = 1.0F / 127.0F;
    constexpr RenderDataFormatDescriptor kFunctionTableFormat{
        32,
        10,
        2,
        1,
        -1,
    };

    RenderTextureArray1DDescriptor descriptor;
    render_texture_array_1d_descriptor_construct(descriptor);
    descriptor.texture_state.name = "function_table";
    descriptor.texture_state.address_mode = static_cast<std::uint32_t>(
        render_texture_default_address_mode(6));
    descriptor.texture_state.filter_mode = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(6));

    std::array<RenderTextureMipChainDescriptor, kFunctionCount> mip_chains;
    descriptor.mip_chains = {
        mip_chains.data(),
        mip_chains.data() + mip_chains.size(),
        mip_chains.data() + mip_chains.size(),
    };

    const auto data_format =
        render_data_format_resolve(kFunctionTableFormat, 7);
    const RenderTextureExtent3D extent{kSampleCount, 1, 1};
    std::array<RenderFloatPixel, kSampleCount> pixels;
    for (std::uint32_t function_index = 0;
         function_index < kFunctionCount;
         ++function_index) {
        for (std::uint32_t sample_index = 0;
             sample_index < kSampleCount;
             ++sample_index) {
            const auto sample = sample_function_table(
                function_index,
                static_cast<float>(sample_index) * kSampleStep);
            pixels[sample_index] = {sample, sample, sample, sample};
        }

        auto& mip = mip_chains[function_index];
        render_texture_mip_chain_descriptor_construct(mip);
        render_texture_mip_chain_descriptor_allocate_source(
            mip, extent, data_format, nullptr);
        const RenderFloatImageView image{
            nullptr,
            kSampleCount,
            1,
            1,
            0,
            pixels.data(),
            nullptr,
        };
        render_texture_mip_chain_descriptor_copy_float_image(mip, image);
    }

    manager.runtime.function_table_texture =
        render_create_texture_array_1d(descriptor, nullptr);

    for (auto& mip : mip_chains) {
        render_texture_mip_chain_descriptor_destruct(mip);
    }
}

// Reconstructed from eboot.elf at 0x641740.
void render_resource_manager_shutdown(RenderResourceManager& manager) {
    constexpr std::size_t kSizedHandleIndices[] = {
        1, 11, 13, 20, 22, 24, 27, 29,
    };
    for (const auto index : kSizedHandleIndices) {
        auto*& storage = reinterpret_cast<void*&>(manager.handle_state[index]);
        release_sized_array_owner(storage);
    }
    for (auto*& storage : manager.runtime.sized_array_owners) {
        release_sized_array_owner(storage);
    }

    auto*& names_storage = manager.runtime.name_array_owner;
    auto* names = static_cast<RenderResourceNameArrayOwner*>(names_storage);
    if (names != nullptr) {
        for (auto* item = names->begin; item != names->end; item += 32) {
            render_resource_name_destruct(item + 16);
        }
        if (names->begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                names->capacity - names->begin);
            engine_deallocate_sized(names->begin, byte_count);
        }
        render_release(names);
        names_storage = nullptr;
    }

    if (manager.runtime.specialized_state != nullptr) {
        auto* state = static_cast<RenderResourceSpecializedState*>(
            manager.runtime.specialized_state);
        for (std::size_t index = 6; index != 0; --index) {
            destruct_name_array(state->arrays[index - 1]);
        }
        render_release(state);
        manager.runtime.specialized_state = nullptr;
    }

    release_dynamic_resource(manager.runtime.function_table_texture);
    for (auto*& resource : manager.runtime.resources) {
        release_dynamic_resource(resource);
    }
}

// Reconstructed from eboot.elf at 0x641F30, with the primary-resource clear
// helper at 0x6388C0 and compiled-array clear at 0x63B210.
void render_resource_manager_reload_shaders(RenderResourceManager& manager) {
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        auto& shader = primary_shader_from_link(*node);
        for (auto& objects : shader.compiled_objects) {
            clear_compiled_objects(objects);
        }
        shader.compiled = false;
    }

    const auto& settings = *render_system_settings(*render_system_instance());
    if (settings.max_partial_framerate_scenes != 0) {
        return;
    }

    for (auto* node = manager.secondary_list->next;
         node != manager.secondary_list;
         node = node->next) {
        auto* bytes = reinterpret_cast<std::uint8_t*>(node);
        bytes[kSecondaryShaderDirtyOffset] = true;
    }
}

}  // namespace rb4
