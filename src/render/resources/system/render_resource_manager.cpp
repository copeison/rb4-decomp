#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_data_format_adapters.h"
#include "render/core/textures/render_texture_array_1d.h"
#include "render/core/textures/render_texture_mip_chain_adapters.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/primary_shader_resource_adapters.h"
#include "render/resources/system/render_resource_manager_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kResourceManagerOffset = 2544;
constexpr std::ptrdiff_t kSecondaryShaderDirtyOffset = -395;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
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

void destruct_parameter_registry(RenderShaderParameterRegistry& parameters) {
    for (auto* parameter = parameters.begin;
         parameter != parameters.end;
         ++parameter) {
        render_resource_name_destruct(parameter->resource_name);
    }
    if (parameters.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(parameters.capacity) -
            reinterpret_cast<std::uint8_t*>(parameters.begin));
        engine_deallocate_sized(parameters.begin, byte_count);
    }
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
    for (auto& binding : manager.shader_parameter_bindings) {
        binding = {};
    }

    auto* constant_words = reinterpret_cast<std::int64_t*>(
        &manager.shader_constants);
    std::fill_n(constant_words, 34, std::int64_t{-1});
    for (const auto index : {0U, 1U, 11U, 13U, 20U, 22U, 24U, 27U, 29U}) {
        constant_words[index] = 0;
    }
    std::fill_n(constant_words + 34, 4, std::int64_t{0});
    manager.runtime = {};

    manager.pointer_array = static_cast<RenderResourcePointerArray*>(
        render_allocate(sizeof(RenderResourcePointerArray)));
    *manager.pointer_array = {};
    manager.primary_list = create_list_sentinel();
    manager.secondary_list = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x640BF0.
void render_resource_manager_initialize_shader_parameters(
    RenderResourceManager& manager) {
    auto* parameters = static_cast<RenderShaderParameterRegistrySet*>(
        render_allocate(sizeof(RenderShaderParameterRegistrySet)));
    render_shader_parameter_registry_set_construct(*parameters);
    manager.runtime.shader_parameters = parameters;

    struct BindingDefinition {
        const char* name;
        std::uint32_t first_value;
        std::uint32_t last_value;
        std::size_t registry_index;
    };
    constexpr BindingDefinition kBindings[] = {
        {"HX_BT709_TO_BT2020", 0, 2, 0},
        {"HX_NUM_RT_SLICES", 0, 7, 0},
        {"HX_SHADING_MODE", 0, 19, 0},
        {"HX_GEO_TYPE", 0, 2, 1},
    };

    for (std::size_t index = 0; index < 4; ++index) {
        const auto& definition = kBindings[index];
        const Symbol name(definition.name);
        render_shader_parameter_registry_add(
            &manager.shader_parameter_bindings[index],
            &parameters->registries[definition.registry_index],
            name.value(),
            definition.first_value,
            definition.last_value);
    }
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
    manager.shader_constants.initialization_phases[1] = 1;
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        render_primary_shader_finalize(
            render_primary_shader_from_link(*node));
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
    void** constant_blocks[] = {
        &manager.shader_constants.scene_block,
        &manager.shader_constants.render_target_block,
        &manager.shader_constants.camera_block,
        &manager.shader_constants.clip_planes_block,
        &manager.shader_constants.skeleton_block,
        &manager.shader_constants.misc_draw_state_block,
        &manager.shader_constants.occlusion_query_block,
        &manager.shader_constants.debug_block,
    };
    for (auto** block : constant_blocks) {
        release_sized_array_owner(*block);
    }
    for (auto*& storage : manager.shader_constants.transient_blocks) {
        release_sized_array_owner(storage);
    }

    auto*& names_storage = manager.shader_constants.constant_registry;
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

    if (manager.runtime.shader_parameters != nullptr) {
        auto* parameters = manager.runtime.shader_parameters;
        for (std::size_t index = 6; index != 0; --index) {
            destruct_parameter_registry(
                parameters->registries[index - 1]);
        }
        render_release(parameters);
        manager.runtime.shader_parameters = nullptr;
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
        render_primary_shader_clear_compiled_objects(
            render_primary_shader_from_link(*node));
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
