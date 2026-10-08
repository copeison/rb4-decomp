#include "render/resources/shaders/shader_constant_block.h"

#include <cstring>

#include "core/memory/engine_memory.h"

namespace rb4 {

namespace {

constexpr std::uint64_t kTypeRegisterCounts[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 3, 3, 4, 4,
};

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

}  // namespace rb4
