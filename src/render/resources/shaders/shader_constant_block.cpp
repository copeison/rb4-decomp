#include "render/resources/shaders/shader_constant_block.h"

#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/names/render_resource_name_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint64_t kTypeRegisterCounts[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 3, 3, 4, 4,
};

constexpr const char* kTypeNames[] = {
    "bool", "int", "int2", "int3", "int4",
    "uint", "uint2", "uint3", "uint4",
    "float", "float2", "float3", "float4",
    "float3x3", "float3x4", "float4x3", "float4x4",
};

void append_hash(std::uint32_t& hash, const char* text) {
    while (*text != '\0') {
        hash = (hash ^ static_cast<std::uint8_t>(*text)) * 0x01000193U;
        ++text;
    }
}

void append_unsigned_hash(std::uint32_t& hash, std::uint64_t value) {
    char digits[20];
    auto* end = digits + sizeof(digits);
    auto* cursor = end;
    do {
        *--cursor = static_cast<char>('0' + value % 10);
        value /= 10;
    } while (value != 0);
    while (cursor != end) {
        const char character[2] = {*cursor++, '\0'};
        append_hash(hash, character);
    }
}

void append_signed_hash(std::uint32_t& hash, std::int32_t value) {
    if (value < 0) {
        append_hash(hash, "-");
        append_unsigned_hash(
            hash,
            static_cast<std::uint64_t>(-
                static_cast<std::int64_t>(value)));
        return;
    }
    append_unsigned_hash(hash, static_cast<std::uint32_t>(value));
}

const char* type_name(RenderShaderConstantType type) {
    const auto index = static_cast<std::uint32_t>(type);
    if (index >= sizeof(kTypeNames) / sizeof(*kTypeNames)) {
        return "";
    }
    return kTypeNames[index];
}

void append_array_suffix(
    std::uint32_t& hash,
    const RenderShaderConstantMember& member) {
    if (member.element_count < 0 && !member.render_target_sliced) {
        return;
    }
    const auto count = member.element_count < 0
        ? std::uint64_t{1}
        : static_cast<std::uint64_t>(member.element_count);
    append_hash(hash, "[");
    append_unsigned_hash(
        hash, count * (member.render_target_sliced ? 6U : 1U));
    append_hash(hash, "]");
}

void append_metal_members(
    std::uint32_t& hash,
    const RenderShaderConstantBlock& block) {
    for (auto* member = block.members_begin;
         member != block.members_end;
         ++member) {
        const auto* local_name = member->name + 1;
        append_hash(hash, "   ");
        append_hash(hash, type_name(member->type));
        append_hash(hash, " ");
        append_hash(hash, local_name);
        append_array_suffix(hash, *member);
        append_hash(hash, ";\n");

        if (member->element_count >= 0 || member->render_target_sliced) {
            continue;
        }
        const auto type = static_cast<std::uint32_t>(member->type);
        const auto padding_count =
            type == 0 || type == 1 || type == 5 || type == 9
            ? 3U
            : type == 2 || type == 6 || type == 10 ? 2U : 0U;
        for (std::uint32_t index = 0; index < padding_count; ++index) {
            append_hash(hash, "   float _pad");
            append_unsigned_hash(hash, index);
            append_hash(hash, "_");
            append_hash(hash, local_name);
            append_hash(hash, ";\n");
        }
    }
}

void append_hlsl_members(
    std::uint32_t& hash,
    const RenderShaderConstantBlock& block) {
    for (auto* member = block.members_begin;
         member != block.members_end;
         ++member) {
        append_hash(hash, "   ");
        append_hash(hash, type_name(member->type));
        append_hash(hash, " ");
        append_hash(hash, member->name);
        append_array_suffix(hash, *member);
        append_hash(hash, " : packoffset(c");
        append_unsigned_hash(hash, member->offset);
        append_hash(hash, ");\n");
    }
}

std::uint64_t register_count(RenderShaderConstantType type) {
    const auto index = static_cast<std::uint32_t>(type);
    if (index >= sizeof(kTypeRegisterCounts) / sizeof(*kTypeRegisterCounts)) {
        return 0;
    }
    return kTypeRegisterCounts[index];
}

RenderShaderConstantMember& append_member(RenderShaderConstantBlock& block) {
    if (block.members_end == block.members_capacity) {
        const auto old_count = block.members_begin == nullptr
            ? std::size_t{0}
            : static_cast<std::size_t>(
                block.members_end - block.members_begin);
        const auto new_count = old_count == 0 ? 1 : old_count * 2;
        auto* new_members = static_cast<RenderShaderConstantMember*>(
            engine_allocate_sized(
                new_count * sizeof(RenderShaderConstantMember)));
        if (old_count != 0) {
            std::memcpy(
                new_members,
                block.members_begin,
                old_count * sizeof(RenderShaderConstantMember));
            engine_deallocate_sized(
                block.members_begin,
                static_cast<std::size_t>(
                    reinterpret_cast<std::uint8_t*>(block.members_capacity) -
                    reinterpret_cast<std::uint8_t*>(block.members_begin)));
        }
        block.members_begin = new_members;
        block.members_end = new_members + old_count;
        block.members_capacity = new_members + new_count;
    }

    auto& member = *block.members_end;
    ++block.members_end;
    return member;
}

RenderShaderConstantDefinition& append_definition(
    RenderShaderConstantRegistry& registry) {
    if (registry.end == registry.capacity) {
        const auto old_count = registry.begin == nullptr
            ? std::size_t{0}
            : static_cast<std::size_t>(registry.end - registry.begin);
        const auto new_count = old_count == 0 ? 1 : old_count * 2;
        auto* new_definitions = static_cast<RenderShaderConstantDefinition*>(
            engine_allocate_sized(
                new_count * sizeof(RenderShaderConstantDefinition)));
        if (old_count != 0) {
            std::memcpy(
                new_definitions,
                registry.begin,
                old_count * sizeof(RenderShaderConstantDefinition));
            engine_deallocate_sized(
                registry.begin,
                static_cast<std::size_t>(
                    reinterpret_cast<std::uint8_t*>(registry.capacity) -
                    reinterpret_cast<std::uint8_t*>(registry.begin)));
        }
        registry.begin = new_definitions;
        registry.end = new_definitions + old_count;
        registry.capacity = new_definitions + new_count;
    }

    auto& definition = *registry.end;
    ++registry.end;
    return definition;
}

std::uint64_t add_member(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    std::int64_t element_count,
    bool render_target_sliced,
    const char* name) {
    const auto offset = block.next_offset;
    if (!block.finalized) {
        auto& member = append_member(block);
        member = {
            offset,
            type,
            0,
            element_count,
            render_target_sliced,
            {},
            name,
        };
    }

    const auto count = element_count < 0
        ? std::uint64_t{1}
        : static_cast<std::uint64_t>(element_count);
    const auto slice_count = render_target_sliced ? 6U : 1U;
    block.next_offset += register_count(type) * count * slice_count;
    return offset;
}

}  // namespace

// Reconstructed from eboot.elf at 0x63A0C0.
void render_shader_constant_block_construct(
    RenderShaderConstantBlock& block,
    const char* name,
    std::uint32_t buffer_index,
    std::uint32_t stage_mask,
    std::uint64_t instance_limit) {
    block.name = name;
    block.buffer_index = buffer_index;
    block.stage_mask = stage_mask;
    block.instance_limit = instance_limit;
    block.next_offset = 0;
    block.finalized = false;
    block.reserved_33[0] = 0;
    block.reserved_33[1] = 0;
    block.reserved_33[2] = 0;
    block.reserved_33[3] = 0;
    block.reserved_33[4] = 0;
    block.reserved_33[5] = 0;
    block.reserved_33[6] = 0;
    block.members_begin = nullptr;
    block.members_end = nullptr;
    block.members_capacity = nullptr;
    block.allocator = nullptr;
}

RenderShaderConstantBlock* render_shader_constant_block_create(
    const char* name,
    std::uint32_t buffer_index,
    std::uint32_t stage_mask,
    std::uint64_t instance_limit) {
    auto* block = static_cast<RenderShaderConstantBlock*>(
        render_allocate(sizeof(RenderShaderConstantBlock)));
    render_shader_constant_block_construct(
        *block, name, buffer_index, stage_mask, instance_limit);
    return block;
}

void render_shader_constant_block_release(RenderShaderConstantBlock*& block) {
    if (block == nullptr) {
        return;
    }
    if (block->members_begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(block->members_capacity) -
            reinterpret_cast<std::uint8_t*>(block->members_begin));
        engine_deallocate_sized(block->members_begin, byte_count);
    }
    render_release(block);
    block = nullptr;
}

// Reconstructed from eboot.elf at 0x63A1B0.
std::uint64_t render_shader_constant_block_add(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    const char* name) {
    return add_member(block, type, -1, false, name);
}

// Reconstructed from eboot.elf at 0x63A370.
std::uint64_t render_shader_constant_block_add_array(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    std::uint64_t element_count,
    const char* name) {
    return add_member(
        block,
        type,
        static_cast<std::int64_t>(element_count),
        false,
        name);
}

// Reconstructed from eboot.elf at 0x63A700.
std::uint64_t render_shader_constant_block_add_sliced_array(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    std::uint64_t element_count,
    const char* name) {
    return add_member(
        block,
        type,
        static_cast<std::int64_t>(element_count),
        true,
        name);
}

// Reconstructed from eboot.elf at 0x63A8C0, with declaration emitters at
// 0x63AB40 and 0x63AE10.
void render_shader_constant_block_accumulate_source_hash(
    const RenderShaderConstantBlock& block,
    std::uint32_t& hash) {
    append_hash(hash, "// ");
    append_hash(hash, block.name);
    append_hash(hash, " constants\n");
    append_hash(hash, "#if (HX_METAL == 1)\n");
    append_hash(hash, "HxCBuffer ");
    append_hash(hash, block.name);
    append_hash(hash, "\n{\n");
    append_metal_members(hash, block);
    append_hash(hash, "};\n");
    append_hash(hash, "# define HX_USE_CBUFFER_");
    append_hash(hash, block.name);
    append_hash(hash, " , constant ");
    append_hash(hash, block.name);
    append_hash(hash, "& _g");
    append_hash(hash, block.name);
    append_hash(hash, " [[buffer(");
    append_unsigned_hash(hash, block.buffer_index + 3U);
    append_hash(hash, ")]]\n");
    for (auto* member = block.members_begin;
         member != block.members_end;
         ++member) {
        append_hash(hash, "# define ");
        append_hash(hash, member->name);
        append_hash(hash, " _g");
        append_hash(hash, block.name);
        append_hash(hash, ".");
        append_hash(hash, member->name + 1);
        append_hash(hash, "\n");
    }
    append_hash(hash, "#else\n");
    append_hash(hash, "HxCBuffer ");
    append_hash(hash, block.name);
    append_hash(hash, " : register(b");
    append_unsigned_hash(hash, block.buffer_index);
    append_hash(hash, ")\n{\n");
    append_hlsl_members(hash, block);
    append_hash(hash, "};\n");
    append_hash(hash, "# define HX_USE_CBUFFER_");
    append_hash(hash, block.name);
    append_hash(hash, "\n");
    append_hash(hash, "#endif // ...if/else HX_METAL\n");
}

// Reconstructed from eboot.elf at 0x63D6E0.
void render_shader_constant_registry_accumulate_source_hash(
    const RenderShaderConstantRegistry& registry,
    std::uint32_t& hash) {
    for (auto* definition = registry.begin;
         definition != registry.end;
         ++definition) {
        if (definition->comment.text[0] != '\0') {
            append_hash(hash, "\n// ");
            append_hash(hash, definition->comment.text);
            append_hash(hash, "\n");
        }
        if (definition->name[0] != '\0') {
            append_hash(hash, "#define ");
            append_hash(hash, definition->name);
            append_hash(hash, " ");
            append_signed_hash(hash, definition->value);
            append_hash(hash, "\n");
        }
    }
}

void render_shader_constant_registry_construct(
    RenderShaderConstantRegistry& registry) {
    registry = {};
}

// Reconstructed from eboot.elf at 0x63D5D0.
void render_shader_constant_registry_add_comment(
    RenderShaderConstantRegistry& registry,
    const char* comment) {
    auto& definition = append_definition(registry);
    const Symbol empty_name("");
    definition.name = static_cast<const char*>(empty_name.value());
    definition.value = 0;
    definition.reserved_12 = 0;
    render_resource_name_construct(definition.comment, comment);
}

// Reconstructed from eboot.elf at 0x63D520.
void render_shader_constant_registry_add_definition(
    RenderShaderConstantRegistry& registry,
    const char* name,
    std::int32_t value) {
    auto& definition = append_definition(registry);
    const Symbol symbol(name);
    definition.name = static_cast<const char*>(symbol.value());
    definition.value = value;
    definition.reserved_12 = 0;
    render_resource_name_construct(definition.comment, "");
}

void render_shader_constant_registry_release(
    RenderShaderConstantRegistry*& registry) {
    if (registry == nullptr) {
        return;
    }
    for (auto* definition = registry->begin;
         definition != registry->end;
         ++definition) {
        render_resource_name_destruct(&definition->comment);
    }
    if (registry->begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(registry->capacity) -
            reinterpret_cast<std::uint8_t*>(registry->begin));
        engine_deallocate_sized(registry->begin, byte_count);
    }
    render_release(registry);
    registry = nullptr;
}

}  // namespace rb4
