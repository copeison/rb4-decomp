#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

// 16-byte global shader define recorded in compiled-shader caches. Names are
// interned symbols, so equal names share one pointer.
struct RenderShaderCacheDefine {
    const char* name;
    std::int32_t value;
    std::uint8_t reserved_12[4];
};

static_assert(sizeof(RenderShaderCacheDefine) == 16);

struct RenderShaderCacheDefineArray {
    RenderShaderCacheDefine* begin;
    RenderShaderCacheDefine* end;
    RenderShaderCacheDefine* capacity;
    void* allocator;
};

static_assert(sizeof(RenderShaderCacheDefineArray) == 32);

std::uint32_t render_shader_hash_source_file(const char* path);
bool render_shader_cache_defines_match(
    const RenderShaderCacheDefineArray& live_defines,
    const RenderShaderCacheDefineArray& cached_defines);

}  // namespace rb4
