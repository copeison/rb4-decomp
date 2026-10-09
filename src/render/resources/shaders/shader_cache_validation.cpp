#include "render/resources/shaders/shader_cache_validation.h"

#include <cstring>

#include "core/io/file_stream.h"
#include "render/resources/shaders/shader_source_hash.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x6456C0. Hashes a shader source file
// byte by byte, ignoring carriage returns so line-ending conversions do not
// invalidate compiled caches.
std::uint32_t render_shader_hash_source_file(const char* path) {
    FileStream stream;
    file_stream_construct(stream, path, 0, 0);
    auto remaining = file_stream_size(&stream);
    auto hash = kShaderSourceHashBasis;
    for (; remaining != 0; --remaining) {
        char character = 0;
        bin_stream_read(stream, &character, 1);
        if (character != '\r') {
            render_shader_source_hash_append_byte(
                hash, static_cast<std::uint8_t>(character));
        }
    }
    file_stream_destruct(stream);
    return hash;
}

// Reconstructed from eboot.elf at 0x63EB70. The live defines are sorted by
// case-insensitive name. Every cached define must be found by binary search
// with the same interned name pointer and value.
bool render_shader_cache_defines_match(
    const RenderShaderCacheDefineArray& live_defines,
    const RenderShaderCacheDefineArray& cached_defines) {
    for (auto* cached = cached_defines.begin;
         cached != cached_defines.end;
         ++cached) {
        auto* first = live_defines.begin;
        auto count = live_defines.end - live_defines.begin;
        while (count > 0) {
            const auto half = count / 2;
            auto* middle = first + half;
            if (middle->name != cached->name &&
                strcasecmp(middle->name, cached->name) < 0) {
                first = middle + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        if (first == live_defines.end || first->name != cached->name ||
            first->value != cached->value) {
            return false;
        }
    }
    return true;
}

}  // namespace rb4
