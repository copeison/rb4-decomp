#include "render/shaders/shader_source_hash.h"

#include <cstddef>

namespace rb4 {

namespace {

constexpr std::uint32_t kFnvPrime = 0x01000193U;

constexpr const char* kTypeNames[] = {
    "bool", "int", "int2", "int3", "int4",
    "uint", "uint2", "uint3", "uint4",
    "float", "float2", "float3", "float4",
    "float3x3", "float3x4", "float4x3", "float4x4",
};

// Table at 0x12AC200: the scalar type underlying each constant type.
constexpr std::uint32_t kBaseTypes[] = {
    0, 1, 1, 1, 1, 5, 5, 5, 5, 9, 9, 9, 9, 9, 9, 9, 9,
};

constexpr std::size_t kTypeCount = sizeof(kTypeNames) / sizeof(*kTypeNames);
static_assert(sizeof(kBaseTypes) / sizeof(*kBaseTypes) == kTypeCount);

}  // namespace

// Reconstructed from eboot.elf at 0x50CA20. Characters are sign-extended
// before being mixed into the hash.
void render_shader_source_hash_append(std::uint32_t& hash, const char* text) {
    while (*text != '\0') {
        render_shader_source_hash_append_byte(
            hash, static_cast<std::uint8_t>(*text));
        ++text;
    }
}

void render_shader_source_hash_append_byte(
    std::uint32_t& hash,
    std::uint8_t value) {
    const auto extended = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(static_cast<std::int8_t>(value)));
    hash = (hash ^ extended) * kFnvPrime;
}

// Reconstructed from eboot.elf at 0x2589D0, which formats with "%lu".
void render_shader_source_hash_append_unsigned(
    std::uint32_t& hash,
    std::uint64_t value) {
    char digits[21];
    auto* end = digits + sizeof(digits) - 1;
    auto* cursor = end;
    *end = '\0';
    do {
        *--cursor = static_cast<char>('0' + value % 10);
        value /= 10;
    } while (value != 0);
    render_shader_source_hash_append(hash, cursor);
}

void render_shader_source_hash_append_signed(
    std::uint32_t& hash,
    std::int32_t value) {
    if (value < 0) {
        render_shader_source_hash_append(hash, "-");
        render_shader_source_hash_append_unsigned(
            hash,
            static_cast<std::uint64_t>(-static_cast<std::int64_t>(value)));
        return;
    }
    render_shader_source_hash_append_unsigned(
        hash, static_cast<std::uint32_t>(value));
}

// Reconstructed from eboot.elf at 0x645680.
const char* render_shader_constant_type_name(std::uint32_t type) {
    return type < kTypeCount ? kTypeNames[type] : nullptr;
}

// Reconstructed from eboot.elf at 0x645630.
std::uint32_t render_shader_constant_base_type(std::uint32_t type) {
    return type < kTypeCount ? kBaseTypes[type] : 0xFFFFFFFFU;
}

}  // namespace rb4
