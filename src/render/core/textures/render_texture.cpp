#include "render/core/textures/render_texture.h"

#include <cstddef>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/resources/resource_mode.h"

namespace rb4 {

namespace {

struct RenderTextureResolvedFields {
    RenderTextureUsage usage_type;
    std::uint32_t values[7];
    std::uint32_t address_mode;
    std::uint32_t filter_mode;
    std::uint32_t flags;
};

struct RenderTextureDispatch {
    void (*destruct)(RenderTexture& texture);
    void (*release_dynamic)(RenderTexture& texture);
    std::int32_t (*descriptor_type)(const RenderTexture& texture);
    void (*reserved_methods[10])();
    void (*update_gpu_data)(RenderTexture& texture);
    RenderTexture* (*identity)(
        RenderTexture& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTexture& texture,
        const RenderTexture* reusable_texture);
    bool (*reserved_predicate)(const RenderTexture& texture);
};

std::int32_t base_descriptor_type(const RenderTexture& texture) {
    return texture.descriptor_type;
}

RenderTexture* base_identity(RenderTexture& texture, std::int64_t&) {
    return &texture;
}

bool base_predicate(const RenderTexture&) {
    return false;
}

RenderTextureDispatch kBaseTextureDispatch{
    render_texture_destruct,
    render_texture_delete,
    base_descriptor_type,
    {},
    nullptr,
    base_identity,
    nullptr,
    base_predicate,
};

void set_base_dispatch(RenderTexture& texture) {
    texture.implementation = &kBaseTextureDispatch;
}

const RenderTextureDispatch& dispatch(const RenderTexture& texture) {
    return *static_cast<const RenderTextureDispatch*>(
        texture.implementation);
}

static_assert(offsetof(RenderTextureDispatch, update_gpu_data) == 104);
static_assert(offsetof(RenderTextureDispatch, descriptor_type) == 16);
static_assert(offsetof(RenderTextureDispatch, release_dynamic) == 8);
static_assert(offsetof(RenderTextureDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTextureDispatch) == 17 * sizeof(void*));
static_assert(sizeof(RenderTextureResolvedFields) == 44);

}  // namespace

// Reconstructed from eboot.elf at 0x6A4770.
void render_texture_resolve_descriptor_fields(
    RenderTextureDescriptorState& descriptor,
    std::int32_t descriptor_type,
    std::int64_t fallback_mode) {
    auto& fields = reinterpret_cast<RenderTextureResolvedFields&>(
        descriptor.usage_type);
    const auto& defaults = descriptor.creation_state.values;
    fields.usage_type = static_cast<RenderTextureUsage>(defaults[0]);
    for (std::size_t index = 0; index < 7; ++index) {
        if (fields.values[index] == 0) {
            fields.values[index] = defaults[index + 1];
        }
    }
    if (fields.address_mode == 0) {
        fields.address_mode = defaults[8];
    }
    if (fields.filter_mode == 0) {
        fields.filter_mode = defaults[9];
    }
    fields.flags |= defaults[10];

    const auto usage = static_cast<std::int32_t>(fields.usage_type);
    const auto default_like_usage = usage == 0 || usage == 8 || usage == 9;
    const auto standard_usage = usage == 0 || usage == 8;
    if (fields.values[3] == 0) {
        fields.values[3] = default_like_usage ? 2 : 1;
    }
    if (usage == 9) {
        fields.values[3] = 2;
    }
    if (fields.values[5] == 0) {
        fields.values[5] = usage == 8 ? 1 : 2;
    }
    if (fields.address_mode == 0) {
        const auto is_cube = (descriptor_type | 4) == 7;
        fields.address_mode = is_cube ? 1 : (usage == 8 ? 1 : 2);
    }
    if (fields.filter_mode == 0) {
        fields.filter_mode = fields.values[5] == 1 ? 2 : 3;
    }

    const auto resolved_fallback = fallback_mode == -1 ? 4 : fallback_mode;
    if (fields.values[0] == 0 && standard_usage) {
        fields.values[0] = resolved_fallback == 4 ? 3 : 1;
    }
    if (fields.values[1] == 0) {
        fields.values[1] = resolved_fallback == 4
            ? (standard_usage ? 3 : 1)
            : 1;
    }
    if (fields.values[4] == 0) {
        fields.values[4] = (fields.flags & 3U) == 0 ? 2 : 1;
    }

    if (usage == 9) {
        fields.values[0] = 1;
        if ((fields.flags & 2U) != 0) {
            fields.values[4] = 1;
            fields.values[5] = 1;
            fields.filter_mode = 2;
        } else {
            fields.flags |= 4U;
            fields.values[4] = 2;
            fields.values[5] = 2;
            fields.filter_mode = 3;
        }
        fields.address_mode = (descriptor_type | 4) == 7 ? 1 : 2;
    }
}

// Reconstructed from eboot.elf at 0x69B930.
void render_texture_descriptor_construct(
    RenderTextureDescriptorState& descriptor) {
    descriptor = {};
    descriptor.descriptor_type = -1;
    descriptor.data_format = -1;
    descriptor.attachment_index = -1;
}

// Reconstructed from eboot.elf at 0x69B990.
bool render_texture_descriptor_has_source_data(
    const RenderTextureDescriptorState& descriptor) {
    return descriptor.backend_initialized || (descriptor.flags & 5U) != 0;
}

void render_texture_apply_descriptor_state(
    RenderTexture& texture,
    const RenderTextureDescriptorState& descriptor) {
    std::memcpy(
        &texture.descriptor_type,
        &descriptor,
        sizeof(descriptor));
}

// Reconstructed from eboot.elf at 0x69B6E0.
void render_texture_construct(RenderTexture& texture) {
    set_base_dispatch(texture);
    texture.frame_stamp = -1;
    texture.descriptor_type = -1;
    texture.creation_state = {};
    texture.usage_type = RenderTextureUsage::kDefault;
    texture.resolved_state = {};
    texture.address_mode = 0;
    texture.filter_mode = 0;
    texture.flags = 0;
    texture.data_format = -1;
    texture.width = 0;
    texture.height = 0;
    texture.depth = 0;
    texture.source_data = nullptr;
    texture.source_size = 0;
    texture.backend_initialized = false;
    texture.bindless_index = -1;
    texture.resource_flags = 0;
    texture.name = nullptr;
    texture.resource_index = -1;
}

// Reconstructed from eboot.elf at 0x69B770.
void render_texture_destruct(RenderTexture&) {
}

// Reconstructed from eboot.elf at 0x69B780.
void render_texture_delete(RenderTexture& texture) {
    render_texture_destruct(texture);
    render_release(&texture);
}

void render_texture_release_dynamic(RenderTexture& texture) {
    dispatch(texture).release_dynamic(texture);
}

// Reconstructed from eboot.elf at 0x69B7A0.
void render_texture_initialize_backend(
    RenderTexture& texture,
    const RenderTexture* reusable_texture) {
    if (g_resource_precache_mode) {
        return;
    }

    const auto& methods = dispatch(texture);
    methods.initialize_backend(texture, reusable_texture);
    if (texture.source_data == nullptr && (texture.flags & 5U) == 0) {
        methods.update_gpu_data(texture);
    }
}

std::int32_t render_texture_runtime_descriptor_type(
    const RenderTexture& texture) {
    return dispatch(texture).descriptor_type(texture);
}

// Reconstructed from eboot.elf at 0x50CE00.
std::int32_t render_texture_default_address_mode(
    std::uint32_t resource_kind) {
    constexpr std::uint64_t kAddressModeOneKinds = 0x1F80400E0ULL;
    if (resource_kind > 32) {
        return -1;
    }
    return (kAddressModeOneKinds & (1ULL << resource_kind)) != 0
        ? 1
        : -1;
}

// Reconstructed from eboot.elf at 0x50CE30.
std::int32_t render_texture_default_filter_mode(
    std::uint32_t resource_kind) {
    constexpr std::uint64_t kFilterModeTwoKinds = 0x178040060ULL;
    constexpr std::uint64_t kFilterModeOneKinds = 0x80000080ULL;
    if (resource_kind > 32) {
        return -1;
    }
    const auto kind_bit = 1ULL << resource_kind;
    if ((kFilterModeTwoKinds & kind_bit) != 0) {
        return 2;
    }
    return (kFilterModeOneKinds & kind_bit) != 0 ? 1 : -1;
}

}  // namespace rb4
