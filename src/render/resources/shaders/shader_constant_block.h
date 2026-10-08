#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

enum class RenderShaderConstantType : std::uint32_t {
    boolean = 0,
    integer = 1,
    integer2 = 2,
    integer3 = 3,
    integer4 = 4,
    unsigned_integer = 5,
    unsigned_integer2 = 6,
    unsigned_integer3 = 7,
    unsigned_integer4 = 8,
    scalar = 9,
    vector2 = 10,
    vector3 = 11,
    vector4 = 12,
    matrix3x3 = 13,
    matrix3x4 = 14,
    matrix4x3 = 15,
    matrix4x4 = 16,
};

struct RenderShaderConstantMember {
    std::uint64_t offset;
    RenderShaderConstantType type;
    std::uint32_t reserved_12;
    std::int64_t element_count;
    bool render_target_sliced;
    std::uint8_t reserved_25[7];
    const char* name;
};

struct RenderShaderConstantBlock {
    const char* name;
    std::uint32_t buffer_index;
    std::uint32_t stage_mask;
    std::uint64_t instance_limit;
    std::uint64_t next_offset;
    bool finalized;
    std::uint8_t reserved_33[7];
    RenderShaderConstantMember* members_begin;
    RenderShaderConstantMember* members_end;
    RenderShaderConstantMember* members_capacity;
    void* allocator;
};

struct RenderShaderConstantResourceName {
    void* dispatch;
    const char* text;
};

struct RenderShaderConstantDefinition {
    const char* name;
    std::int32_t value;
    std::uint32_t reserved_12;
    RenderShaderConstantResourceName comment;
};

struct RenderShaderConstantRegistry {
    RenderShaderConstantDefinition* begin;
    RenderShaderConstantDefinition* end;
    RenderShaderConstantDefinition* capacity;
    void* allocator;
};

static_assert(sizeof(RenderShaderConstantMember) == 40);
static_assert(offsetof(RenderShaderConstantMember, name) == 32);
static_assert(sizeof(RenderShaderConstantBlock) == 72);
static_assert(offsetof(RenderShaderConstantBlock, next_offset) == 24);
static_assert(offsetof(RenderShaderConstantBlock, finalized) == 32);
static_assert(offsetof(RenderShaderConstantBlock, members_begin) == 40);
static_assert(sizeof(RenderShaderConstantResourceName) == 16);
static_assert(sizeof(RenderShaderConstantDefinition) == 32);
static_assert(offsetof(RenderShaderConstantDefinition, comment) == 16);
static_assert(sizeof(RenderShaderConstantRegistry) == 32);

void render_shader_constant_block_construct(
    RenderShaderConstantBlock& block,
    const char* name,
    std::uint32_t buffer_index,
    std::uint32_t stage_mask,
    std::uint64_t instance_limit);
RenderShaderConstantBlock* render_shader_constant_block_create(
    const char* name,
    std::uint32_t buffer_index,
    std::uint32_t stage_mask,
    std::uint64_t instance_limit);
void render_shader_constant_block_release(RenderShaderConstantBlock*& block);

std::uint64_t render_shader_constant_block_add(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    const char* name);
std::uint64_t render_shader_constant_block_add_array(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    std::uint64_t element_count,
    const char* name);
std::uint64_t render_shader_constant_block_add_sliced_array(
    RenderShaderConstantBlock& block,
    RenderShaderConstantType type,
    std::uint64_t element_count,
    const char* name);
void render_shader_constant_block_accumulate_source_hash(
    const RenderShaderConstantBlock& block,
    std::uint32_t& hash);
void render_shader_constant_registry_accumulate_source_hash(
    const RenderShaderConstantRegistry& registry,
    std::uint32_t& hash);

}  // namespace rb4
