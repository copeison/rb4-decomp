#include "render/resources/shaders/shader_backend_state.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/shaders/shader_source_hash.h"

namespace rb4 {

namespace {

void release_binding_array(RenderShaderBackendBindingArray& array) {
    if (array.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(array.capacity) -
            reinterpret_cast<std::uint8_t*>(array.begin));
        engine_deallocate_sized(array.begin, byte_count);
    }
    array = {};
}

RenderShaderBackendBinding& append_binding(
    RenderShaderBackendBindingArray& array) {
    const auto size = array.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(array.end - array.begin);
    const auto capacity = array.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(array.capacity - array.begin);
    if (size == capacity) {
        const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
        auto* replacement = static_cast<RenderShaderBackendBinding*>(
            engine_allocate_sized(
                new_capacity * sizeof(RenderShaderBackendBinding)));
        if (size != 0) {
            std::memmove(
                replacement,
                array.begin,
                size * sizeof(RenderShaderBackendBinding));
        }
        if (array.begin != nullptr) {
            engine_deallocate_sized(
                array.begin,
                capacity * sizeof(RenderShaderBackendBinding));
        }
        array.begin = replacement;
        array.end = replacement + size;
        array.capacity = replacement + new_capacity;
    }

    auto& binding = *array.end++;
    binding = {};
    return binding;
}

constexpr std::size_t kStageCount = 6;
constexpr std::size_t kSampledTextureGroup = 0;
constexpr std::size_t kUnsampledResourceGroup = 6;
constexpr std::size_t kInputGroup = 12;
constexpr std::size_t kOutputGroup = 18;
constexpr std::uint32_t kTextureBinding = 0;
constexpr std::uint32_t kBufferBinding = 1;

// Table at 0x192F260, read through 0x63E5D0.
constexpr const char* kProgramTypeNames[kStageCount] = {
    "HX_PROGRAM_TYPE_VERTEX",
    "HX_PROGRAM_TYPE_HULL",
    "HX_PROGRAM_TYPE_DOMAIN",
    "HX_PROGRAM_TYPE_GEOMETRY",
    "HX_PROGRAM_TYPE_PIXEL",
    "HX_PROGRAM_TYPE_COMPUTE",
};

const char* name_text(const void* symbol) {
    return static_cast<const char*>(symbol);
}

// Metal declarations keep the define and its declaration on one line.
const char* declaration_separator(bool metal) {
    return metal ? " " : "\n";
}

// Reconstructed from eboot.elf at 0x644AB0. Each distinct sampler symbol is
// declared once, numbered in first-use order.
void append_sampler_declarations(
    std::uint32_t& hash,
    const RenderShaderBackendBindingArray& textures,
    bool metal) {
    std::uint64_t sampler_index = 0;
    const auto count = static_cast<std::size_t>(textures.end - textures.begin);
    for (std::size_t index = 0; index < count; ++index) {
        const auto& binding = textures.begin[index];
        bool seen = false;
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (textures.begin[previous].sampler_name == binding.sampler_name) {
                seen = true;
                break;
            }
        }
        if (seen) {
            continue;
        }
        render_shader_source_hash_append(hash, "#  define HX_USE_SAMPLER_");
        render_shader_source_hash_append(hash, name_text(binding.sampler_name));
        render_shader_source_hash_append(hash, declaration_separator(metal));
        render_shader_source_hash_append(hash, "HX_SAMPLER_DEF(");
        render_shader_source_hash_append(hash, name_text(binding.sampler_name));
        render_shader_source_hash_append(hash, ", ");
        render_shader_source_hash_append_unsigned(hash, sampler_index++);
        render_shader_source_hash_append(hash, ", ");
        render_shader_source_hash_append_unsigned(
            hash, binding.stage_resource_count);
        render_shader_source_hash_append(hash, ")\n");
    }
}

const char* texture_macro(
    const RenderShaderBackendBinding& binding,
    bool write) {
    const bool sliced =
        static_cast<std::uint8_t>(binding.render_target_sliced) != 0;
    switch (binding.resource_dimension) {
    case 0:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture1D"
                     : "HX_TEXTURE_DEF(HxTexture1D";
    case 1:
        if (sliced) {
            return "HX_TEXTURE_DEF(HxTexture2DRTSliced";
        }
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture2D"
                     : "HX_TEXTURE_DEF(HxTexture2D";
    case 2:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTexture3D"
                     : "HX_TEXTURE_DEF(HxTexture3D";
    case 3:
        return "HX_TEXTURE_DEF(HxTextureCube";
    case 4:
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTextureArray1D"
                     : "HX_TEXTURE_DEF(HxTextureArray1D";
    case 5:
        if (sliced) {
            return "HX_TEXTURE_DEF(HxTextureArray2DRTSliced";
        }
        return write ? "HX_TEXTURE_WRITE_DEF(HxRWTextureArray2D"
                     : "HX_TEXTURE_DEF(HxTextureArray2D";
    case 7:
        return "HX_TEXTURE_DEF(HxTextureArrayCube";
    default:
        return nullptr;
    }
}

// The original passes possibly-null text straight to the hashing stream,
// which would dereference it; no recovered binding reaches that case.
void append_optional(std::uint32_t& hash, const char* text) {
    if (text != nullptr) {
        render_shader_source_hash_append(hash, text);
    }
}

void append_resource_tail(
    std::uint32_t& hash,
    const RenderShaderBackendBinding& binding) {
    render_shader_source_hash_append(hash, name_text(binding.resource_name));
    render_shader_source_hash_append(hash, ", ");
    render_shader_source_hash_append_unsigned(hash, binding.resource_index);
    render_shader_source_hash_append(hash, ", ");
    render_shader_source_hash_append_unsigned(
        hash, binding.stage_resource_count);
    render_shader_source_hash_append(hash, ")\n");
}

// Reconstructed from eboot.elf at 0x644C80.
void append_texture_declaration(
    std::uint32_t& hash,
    const RenderShaderBackendBinding& binding,
    bool write,
    bool metal) {
    if (binding.resource_index == UINT64_MAX) {
        return;
    }
    const auto* macro = texture_macro(binding, write);
    const auto* element =
        render_shader_constant_type_name(binding.element_type);
    const auto* base = render_shader_constant_type_name(
        render_shader_constant_base_type(binding.element_type));
    render_shader_source_hash_append(hash, "#  define HX_USE_TEXTURE_");
    render_shader_source_hash_append(hash, name_text(binding.resource_name));
    render_shader_source_hash_append(hash, declaration_separator(metal));
    append_optional(hash, macro);
    render_shader_source_hash_append(hash, ", ");
    append_optional(hash, element);
    render_shader_source_hash_append(hash, ", ");
    append_optional(hash, base);
    render_shader_source_hash_append(hash, ", ");
    append_resource_tail(hash, binding);
}

// Reconstructed from eboot.elf at 0x644E40. Structured buffers name their
// structure in place of an element type.
void append_buffer_declaration(
    std::uint32_t& hash,
    const RenderShaderBackendBinding& binding,
    bool write,
    bool metal) {
    if (binding.resource_index == UINT64_MAX) {
        return;
    }
    const char* macro = nullptr;
    if (binding.buffer_kind == 0) {
        macro = write ? "HX_BUFFER_WRITE_DEF" : "HX_BUFFER_DEF";
    } else if (binding.buffer_kind == 1) {
        macro = "HX_BUFFER_APPEND_DEF";
    }
    const auto* element = binding.element_type != UINT32_MAX
        ? render_shader_constant_type_name(binding.element_type)
        : name_text(binding.structure_name);
    render_shader_source_hash_append(hash, "#  define HX_USE_BUFFER_");
    render_shader_source_hash_append(hash, name_text(binding.resource_name));
    render_shader_source_hash_append(hash, declaration_separator(metal));
    append_optional(hash, macro);
    render_shader_source_hash_append(hash, "(");
    append_optional(hash, element);
    render_shader_source_hash_append(hash, ", ");
    append_resource_tail(hash, binding);
}

void append_group_declarations(
    std::uint32_t& hash,
    const RenderShaderBackendBindingArray& bindings,
    bool write,
    bool metal) {
    for (auto* binding = bindings.begin; binding != bindings.end; ++binding) {
        if (binding->kind == kBufferBinding) {
            append_buffer_declaration(hash, *binding, write, metal);
        } else if (binding->kind == kTextureBinding) {
            append_texture_declaration(hash, *binding, write, metal);
        }
    }
}

void append_stage_declarations(
    std::uint32_t& hash,
    const RenderShaderBackendState& state,
    std::size_t stage,
    bool metal) {
    const auto& sampled = state.binding_arrays[kSampledTextureGroup + stage];
    append_sampler_declarations(hash, sampled, metal);
    append_group_declarations(hash, sampled, false, metal);
    append_group_declarations(
        hash,
        state.binding_arrays[kUnsampledResourceGroup + stage],
        false,
        metal);
    append_group_declarations(
        hash, state.binding_arrays[kInputGroup + stage], false, metal);
    append_group_declarations(
        hash, state.binding_arrays[kOutputGroup + stage], true, metal);
}

bool stage_has_bindings(
    const RenderShaderBackendState& state,
    std::size_t stage) {
    constexpr std::size_t kGroups[] = {
        kSampledTextureGroup,
        kUnsampledResourceGroup,
        kInputGroup,
        kOutputGroup,
    };
    for (const auto group : kGroups) {
        const auto& array = state.binding_arrays[group + stage];
        if (array.begin != array.end) {
            return true;
        }
    }
    return false;
}

}  // namespace

void render_shader_backend_state_construct(RenderShaderBackendState& state) {
    state = {};
}

void render_shader_backend_state_destruct(RenderShaderBackendState& state) {
    for (std::size_t index = 24; index != 0; --index) {
        release_binding_array(state.binding_arrays[index - 1]);
    }
    for (auto& count : state.stage_resource_counts) {
        count = 0;
    }
}

// Reconstructed from eboot.elf at 0x643260.
std::uint64_t render_shader_backend_add_texture_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* sampler_name,
    std::uint32_t resource_dimension,
    std::uint32_t stage,
    std::uint32_t element_type) {
    constexpr std::size_t kStageCount = 6;
    if (stage >= kStageCount) {
        return 0;
    }

    const Symbol resource_symbol(resource_name);
    const Symbol sampler_symbol(sampler_name);
    static const Symbol empty_symbol("");

    std::uint64_t resource_index = 0;
    constexpr std::uint32_t kBindingArrayGroups[]{0, 6, 12};
    for (const auto array_offset : kBindingArrayGroups) {
        const auto& array = state.binding_arrays[stage + array_offset];
        resource_index += static_cast<std::uint64_t>(
            array.begin == nullptr ? 0 : array.end - array.begin);
    }

    const auto array_offset = sampler_symbol.value() == empty_symbol.value()
        ? 6U
        : 0U;
    auto& array = state.binding_arrays[stage + array_offset];
    auto& binding = append_binding(array);
    binding.resource_dimension = resource_dimension;
    binding.element_type = element_type;
    binding.buffer_kind = -1;
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = sampler_symbol.value();
    binding.structure_name = empty_symbol.value();
    binding.resource_index = resource_index;
    binding.stage_resource_count = state.stage_resource_counts[stage]++;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x6438D0.
std::uint64_t render_shader_backend_add_graphics_texture_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* sampler_name,
    std::uint32_t resource_dimension,
    std::uint32_t element_type) {
    constexpr std::uint32_t kPixelStage = 4;
    const Symbol resource_symbol(resource_name);
    const Symbol sampler_symbol(sampler_name);
    static const Symbol empty_symbol("");

    std::uint64_t resource_index = 0;
    constexpr std::uint32_t kBindingArrayGroups[]{0, 6, 12};
    for (const auto array_offset : kBindingArrayGroups) {
        const auto& array = state.binding_arrays[kPixelStage + array_offset];
        resource_index += static_cast<std::uint64_t>(
            array.begin == nullptr ? 0 : array.end - array.begin);
    }

    const auto array_offset = sampler_symbol.value() == empty_symbol.value()
        ? 6U
        : 0U;
    auto& binding = append_binding(
        state.binding_arrays[kPixelStage + array_offset]);
    binding.resource_dimension = resource_dimension;
    binding.element_type = element_type;
    binding.buffer_kind = -1;
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = sampler_symbol.value();
    binding.structure_name = empty_symbol.value();
    binding.render_target_sliced = 1;
    binding.resource_index = resource_index;
    binding.stage_resource_count =
        state.stage_resource_counts[kPixelStage]++;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x643670.
std::uint64_t render_shader_backend_add_output_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t resource_dimension,
    std::uint32_t stage,
    std::uint32_t element_type) {
    constexpr std::size_t kStageCount = 6;
    constexpr std::size_t kOutputArrayOffset = 18;
    if (stage >= kStageCount) {
        return 0;
    }

    auto& array = state.binding_arrays[kOutputArrayOffset + stage];
    const auto size = array.begin == nullptr
        ? std::uint64_t{0}
        : static_cast<std::uint64_t>(array.end - array.begin);
    const auto resource_index = stage == 4 ? 7 - size : size;

    const Symbol resource_symbol(resource_name);
    static const Symbol empty_symbol("");
    auto& binding = append_binding(array);
    binding.resource_dimension = resource_dimension;
    binding.element_type = element_type;
    binding.buffer_kind = -1;
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = empty_symbol.value();
    binding.structure_name = empty_symbol.value();
    binding.resource_index = resource_index;
    binding.stage_resource_count = state.stage_resource_counts[stage]++;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x643C70.
std::uint64_t render_shader_backend_add_buffer_input(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t buffer_kind,
    std::uint32_t element_type,
    std::uint32_t stage) {
    constexpr std::size_t kStageCount = 6;
    constexpr std::size_t kBufferInputArrayOffset = 12;
    constexpr std::uint64_t kBufferRegisterOffset = 12;
    if (stage >= kStageCount) {
        return 0;
    }

    std::uint64_t resource_index = 0;
    constexpr std::uint32_t kBindingArrayGroups[]{0, 6, 12};
    for (const auto array_offset : kBindingArrayGroups) {
        const auto& array = state.binding_arrays[stage + array_offset];
        resource_index += static_cast<std::uint64_t>(
            array.begin == nullptr ? 0 : array.end - array.begin);
    }

    const Symbol resource_symbol(resource_name);
    static const Symbol empty_symbol("");
    auto& binding = append_binding(
        state.binding_arrays[kBufferInputArrayOffset + stage]);
    binding.kind = 1;
    binding.resource_dimension = UINT32_MAX;
    binding.element_type = element_type;
    binding.buffer_kind = static_cast<std::int32_t>(buffer_kind);
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = empty_symbol.value();
    binding.structure_name = empty_symbol.value();
    binding.resource_index = resource_index;
    auto& buffer_count = state.stage_resource_counts[kStageCount + stage];
    binding.stage_resource_count = buffer_count++ + kBufferRegisterOffset;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x643EF0.
std::uint64_t render_shader_backend_add_buffer_output(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t buffer_kind,
    std::uint32_t element_type,
    std::uint32_t stage) {
    constexpr std::size_t kStageCount = 6;
    constexpr std::size_t kOutputArrayOffset = 18;
    constexpr std::uint64_t kBufferRegisterOffset = 12;
    if (stage >= kStageCount) {
        return 0;
    }

    auto& array = state.binding_arrays[kOutputArrayOffset + stage];
    const auto size = array.begin == nullptr
        ? std::uint64_t{0}
        : static_cast<std::uint64_t>(array.end - array.begin);
    const auto resource_index = stage == 4 ? 7 - size : size;

    const Symbol resource_symbol(resource_name);
    static const Symbol empty_symbol("");
    auto& binding = append_binding(array);
    binding.kind = 1;
    binding.resource_dimension = UINT32_MAX;
    binding.element_type = element_type;
    binding.buffer_kind = static_cast<std::int32_t>(buffer_kind);
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = empty_symbol.value();
    binding.structure_name = empty_symbol.value();
    binding.resource_index = resource_index;
    auto& buffer_count = state.stage_resource_counts[kStageCount + stage];
    binding.stage_resource_count = buffer_count++ + kBufferRegisterOffset;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x644400.
std::uint64_t render_shader_backend_add_structured_buffer_output(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* structure_name,
    std::uint32_t buffer_kind,
    std::uint32_t stage) {
    constexpr std::size_t kStageCount = 6;
    constexpr std::size_t kOutputArrayOffset = 18;
    constexpr std::uint64_t kBufferRegisterOffset = 12;
    if (stage >= kStageCount) {
        return 0;
    }

    auto& array = state.binding_arrays[kOutputArrayOffset + stage];
    const auto size = array.begin == nullptr
        ? std::uint64_t{0}
        : static_cast<std::uint64_t>(array.end - array.begin);
    const auto resource_index = stage == 4 ? 7 - size : size;

    const Symbol resource_symbol(resource_name);
    const Symbol structure_symbol(structure_name);
    static const Symbol empty_symbol("");
    auto& binding = append_binding(array);
    binding.kind = 1;
    binding.resource_dimension = UINT32_MAX;
    binding.element_type = UINT32_MAX;
    binding.buffer_kind = static_cast<std::int32_t>(buffer_kind);
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = empty_symbol.value();
    binding.structure_name = structure_symbol.value();
    binding.resource_index = resource_index;
    auto& buffer_count = state.stage_resource_counts[kStageCount + stage];
    binding.stage_resource_count = buffer_count++ + kBufferRegisterOffset;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x644150.
std::uint64_t render_shader_backend_add_structured_buffer_input(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* structure_name,
    std::uint32_t buffer_kind,
    std::uint32_t stage) {
    constexpr std::size_t kStageCount = 6;
    constexpr std::size_t kBufferInputArrayOffset = 12;
    constexpr std::uint64_t kBufferRegisterOffset = 12;
    if (stage >= kStageCount) {
        return 0;
    }

    std::uint64_t resource_index = 0;
    constexpr std::uint32_t kBindingArrayGroups[]{0, 6, 12};
    for (const auto array_offset : kBindingArrayGroups) {
        const auto& array = state.binding_arrays[stage + array_offset];
        resource_index += static_cast<std::uint64_t>(
            array.begin == nullptr ? 0 : array.end - array.begin);
    }

    const Symbol resource_symbol(resource_name);
    const Symbol structure_symbol(structure_name);
    static const Symbol empty_symbol("");
    auto& binding = append_binding(
        state.binding_arrays[kBufferInputArrayOffset + stage]);
    binding.kind = 1;
    binding.resource_dimension = UINT32_MAX;
    binding.element_type = UINT32_MAX;
    binding.buffer_kind = static_cast<std::int32_t>(buffer_kind);
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = empty_symbol.value();
    binding.structure_name = structure_symbol.value();
    binding.resource_index = resource_index;
    auto& buffer_count = state.stage_resource_counts[kStageCount + stage];
    binding.stage_resource_count = buffer_count++ + kBufferRegisterOffset;
    return resource_index;
}

// Reconstructed from eboot.elf at 0x6446A0. Each stage with any binding
// emits a Metal and a non-Metal declaration block guarded by its program type.
void render_shader_backend_state_accumulate_source_hash(
    const RenderShaderBackendState& state,
    std::uint32_t& hash) {
    for (std::size_t stage = 0; stage < kStageCount; ++stage) {
        if (!stage_has_bindings(state, stage)) {
            continue;
        }
        render_shader_source_hash_append(hash, "#if (HX_PROGRAM_TYPE == ");
        render_shader_source_hash_append(hash, kProgramTypeNames[stage]);
        render_shader_source_hash_append(hash, ")\n");
        render_shader_source_hash_append(hash, "# if (HX_METAL == 1)\n");
        append_stage_declarations(hash, state, stage, true);
        render_shader_source_hash_append(hash, "# else\n");
        append_stage_declarations(hash, state, stage, false);
        render_shader_source_hash_append(hash, "# endif // ...HX_METAL\n");
        render_shader_source_hash_append(
            hash, "#endif // ...HX_PROGRAM_TYPE\n");
    }
}

}  // namespace rb4
