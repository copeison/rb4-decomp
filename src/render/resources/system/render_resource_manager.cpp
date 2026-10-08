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

// Reconstructed from eboot.elf at 0x640D60.
void render_resource_manager_initialize_shader_constants(
    RenderResourceManager& manager) {
    auto& constants = manager.shader_constants;
    using Type = RenderShaderConstantType;

    constants.scene_block = render_shader_constant_block_create(
        "Scene", 0, 9, 5);
    auto& scene = *constants.scene_block;
    constants.time = render_shader_constant_block_add(
        scene, Type::vector4, "gTime");
    constants.smoothness_decay = render_shader_constant_block_add(
        scene, Type::scalar, "gSmoothnessDecay");
    constants.sgraph_trans_infos = render_shader_constant_block_add_array(
        scene, Type::matrix3x4, 4, "gSGraphTransInfos");
    constants.scene_global_floats = render_shader_constant_block_add_array(
        scene, Type::vector4, 1, "gSceneGlobalFloats");
    constants.scene_global_colors = render_shader_constant_block_add_array(
        scene, Type::vector4, 4, "gSceneGlobalColors");
    constants.tiled_lighting_params = render_shader_constant_block_add(
        scene, Type::vector3, "gTiledLightingParams");
    constants.fog_params = render_shader_constant_block_add(
        scene, Type::vector3, "gFogParams");
    constants.volumetric_params_0 = render_shader_constant_block_add(
        scene, Type::vector3, "gVolumetricParams0");
    constants.volumetric_params_1 = render_shader_constant_block_add(
        scene, Type::vector2, "gVolumetricParams1");

    constants.render_target_block = render_shader_constant_block_create(
        "RenderTarget", 1, 28, 80);
    constants.target_dimensions = render_shader_constant_block_add(
        *constants.render_target_block,
        Type::vector2,
        "gTargetDimensions");

    constants.camera_block = render_shader_constant_block_create(
        "Camera", 2, 29, 40);
    auto& camera = *constants.camera_block;
    constants.camera_near_far_params = render_shader_constant_block_add(
        camera, Type::vector4, "gCameraNearFarParams");
    constants.camera_misc_params = render_shader_constant_block_add(
        camera, Type::vector4, "gCameraMiscParams");
    constants.camera_view_extents = render_shader_constant_block_add_array(
        camera, Type::vector4, 4, "gCameraViewExtents");
    constants.camera_rt_sliced_data =
        render_shader_constant_block_add_sliced_array(
            camera, Type::vector4, 10, "gCameraRTSlicedData");

    constants.clip_planes_block = render_shader_constant_block_create(
        "ClipPlanes", 3, 1, 10);
    constants.clip_planes = render_shader_constant_block_add_array(
        *constants.clip_planes_block,
        Type::vector4,
        4,
        "gClipPlanes");

    constants.skeleton_block = render_shader_constant_block_create(
        "Skeleton", 4, 1, 1);
    constants.skeleton_bone_transforms =
        render_shader_constant_block_add_array(
            *constants.skeleton_block,
            Type::matrix3x4,
            256,
            "gSkeletonBoneXfms");

    constants.misc_draw_state_block = render_shader_constant_block_create(
        "MiscDrawState", 5, 8, 10);
    constants.environment_index = render_shader_constant_block_add(
        *constants.misc_draw_state_block,
        Type::scalar,
        "gEnvironIndex");
    constants.solid_color = render_shader_constant_block_add(
        *constants.misc_draw_state_block,
        Type::vector4,
        "gSolidColor");

    constants.occlusion_query_block = render_shader_constant_block_create(
        "OcclusionQuery", 6, 9, 10);
    constants.occlusion_query_coverage = render_shader_constant_block_add(
        *constants.occlusion_query_block,
        Type::vector2,
        "gOcclusionQueryCoverageParams");

    constants.debug_block = render_shader_constant_block_create(
        "Debug", 7, 24, 10);
    auto& debug = *constants.debug_block;
    constants.debug_modes = render_shader_constant_block_add(
        debug, Type::vector2, "gDebugModes");
    constants.debug_color = render_shader_constant_block_add(
        debug, Type::vector4, "gDebugColor");
    constants.batch_info = render_shader_constant_block_add(
        debug, Type::vector2, "gBatchInfo");
    constants.preview_node_index = render_shader_constant_block_add(
        debug, Type::scalar, "gPreviewNodeIndex");

    constexpr std::uint64_t kTransientCounts[] = {16, 32, 64};
    for (std::size_t index = 0; index < 3; ++index) {
        auto*& transient = constants.transient_blocks[index];
        transient = render_shader_constant_block_create(
            "Transient", 8, 29, 10);
        render_shader_constant_block_add_array(
            *transient,
            Type::vector4,
            kTransientCounts[index],
            "gTransientData");
    }

    constexpr std::uint32_t kFnv1aOffsetBasis = 0x811C9DC5U;
    std::uint32_t source_hash = kFnv1aOffsetBasis;
    render_shader_constant_block_accumulate_source_hash(
        *constants.scene_block, source_hash);
    render_shader_constant_registry_accumulate_source_hash(
        *constants.constant_registry, source_hash);

    RenderShaderConstantBlock* remaining_blocks[] = {
        constants.render_target_block,
        constants.camera_block,
        constants.clip_planes_block,
        constants.skeleton_block,
        constants.misc_draw_state_block,
        constants.debug_block,
    };
    for (const auto* block : remaining_blocks) {
        render_shader_constant_block_accumulate_source_hash(
            *block, source_hash);
    }
    manager.runtime.reserved_680 =
        (manager.runtime.reserved_680 & 0xFFFFFFFF00000000ULL) |
        source_hash;
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
    RenderShaderConstantBlock** constant_blocks[] = {
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
        render_shader_constant_block_release(*block);
    }
    for (auto*& storage : manager.shader_constants.transient_blocks) {
        render_shader_constant_block_release(storage);
    }

    auto*& names = manager.shader_constants.constant_registry;
    if (names != nullptr) {
        for (auto* item = names->begin; item != names->end; ++item) {
            render_resource_name_destruct(&item->comment);
        }
        if (names->begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(names->capacity) -
                reinterpret_cast<std::uint8_t*>(names->begin));
            engine_deallocate_sized(names->begin, byte_count);
        }
        render_release(names);
        names = nullptr;
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
