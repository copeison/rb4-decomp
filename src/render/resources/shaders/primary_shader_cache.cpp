#include "render/resources/shaders/primary_shader_cache.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <_pthread.h>

#include "utl/streams/BinStream.h"
#include "utl/streams/FileStream.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/text/Symbol.h"
#include "render/core/platform/render_platform_config.h"
#include "render/core/system/render_system_globals.h"
#include "utl/text/Str.h"
#include "render/resources/shaders/compiled_shader_objects.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_cache_validation.h"
#include "render/resources/shaders/shader_permutations.h"
#include "render/resources/shaders/shader_source_hash.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kGeometryStage = 3;
constexpr std::uint32_t kLayoutHashedStageCount = 5;
constexpr std::size_t kActivePlatformConfig = 7;
constexpr std::uint32_t kSixSliceFeature = 0x4;

void append_word(std::uint32_t& hash, std::uint32_t value) {
    for (std::uint32_t shift = 0; shift < 32; shift += 8) {
        render_shader_source_hash_append_byte(
            hash, static_cast<std::uint8_t>(value >> shift));
    }
}

void append_key(std::uint32_t& hash, std::uint64_t key) {
    for (std::uint32_t shift = 0; shift < 64; shift += 8) {
        render_shader_source_hash_append_byte(
            hash, static_cast<std::uint8_t>(key >> shift));
    }
}

struct PermutationHashContext {
    std::uint32_t stage;
    std::uint32_t* hash;
    RenderPrimaryShaderResource* shader;
};

// Reconstructed from eboot.elf at 0x639640, the call operator of the functor
// whose dispatch is at 0x192EFC8. Only one- and six-slice permutations are
// hashed. Six slices need shader and platform support; single-slice geometry
// programs must be declared by the shader. The shader's own validator has the
// final say.
void hash_permutation(
    void* context,
    const RenderShaderPermutationDefine*,
    std::size_t,
    std::uint64_t key) {
    auto& state = *static_cast<PermutationHashContext*>(context);
    auto& shader = *state.shader;
    const auto slices = render_shader_parameter_binding_value(
        shader.render_target_slice_binding, key);
    if (slices != 1 && slices != 6) {
        return;
    }
    if (slices >= 2) {
        if (!shader.dispatch->supports_render_target_slices(&shader)) {
            return;
        }
        auto& platform = render_system_platform_config_at(
            *render_system_instance(), kActivePlatformConfig);
        if ((platform.feature_flags & kSixSliceFeature) == 0) {
            return;
        }
    } else if (state.stage == kGeometryStage &&
               !shader.dispatch->uses_geometry_program(&shader)) {
        return;
    }
    if (!shader.dispatch->validate_permutation(&shader, state.stage, key)) {
        return;
    }
    append_key(*state.hash, key);
}

// Recursive critical section "hx crit sec" serializing backend
// initialization. The static initializer at 0x639810 clears the depth and
// creates the mutex with the recursive type.
struct ShaderCriticalSection {
    std::int32_t depth;
    ScePthreadMutex mutex;

    ShaderCriticalSection() : depth(0) {
        ScePthreadMutexattr attributes;
        scePthreadMutexattrInit(&attributes);
        scePthreadMutexattrSettype(
            &attributes, SCE_PTHREAD_MUTEX_RECURSIVE);
        scePthreadMutexInit(&mutex, &attributes, "hx crit sec");
        scePthreadMutexattrDestroy(&attributes);
    }

    // Reconstructed from eboot.elf at 0x12E70, registered with __cxa_atexit.
    // Releases every outstanding recursive hold before destroying the mutex.
    ~ShaderCriticalSection() {
        scePthreadMutexLock(&mutex);
        const auto held = depth;
        scePthreadMutexUnlock(&mutex);
        for (auto remaining = held; remaining > 0; --remaining) {
            --depth;
            scePthreadMutexUnlock(&mutex);
        }
        scePthreadMutexDestroy(&mutex);
    }
};

ShaderCriticalSection g_shader_critical_section;

std::uint32_t read_word(BinStream& stream) {
    std::uint32_t value = 0;
    stream.ReadEndian(&value, sizeof(value));
    return value;
}

// Reconstructed from eboot.elf at 0x50CA90 and the inline shrink path of
// 0x638A40. New entries have the empty name and value zero.
void resize_defines(RenderShaderCacheDefineArray& defines, std::size_t count) {
    const auto size = static_cast<std::size_t>(defines.end - defines.begin);
    if (count <= size) {
        defines.end = defines.begin + count;
        return;
    }
    const auto capacity =
        static_cast<std::size_t>(defines.capacity - defines.begin);
    if (count > capacity) {
        auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
        if (new_capacity < count) {
            new_capacity = count;
        }
        auto* storage = static_cast<RenderShaderCacheDefine*>(
            HmxAllocator::gStlAllocator.allocate(new_capacity * sizeof(RenderShaderCacheDefine)));
        if (size != 0) {
            std::memcpy(storage, defines.begin, size * sizeof(*storage));
        }
        if (defines.begin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                defines.begin, capacity * sizeof(RenderShaderCacheDefine));
        }
        defines.begin = storage;
        defines.end = storage + size;
        defines.capacity = storage + new_capacity;
    }
    const Symbol empty_name("");
    for (auto* define = defines.end; define != defines.begin + count; ++define) {
        define->name = empty_name.Str();
        define->value = 0;
    }
    defines.end = defines.begin + count;
}

void release_defines(RenderShaderCacheDefineArray& defines) {
    if (defines.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            defines.begin,
            static_cast<std::size_t>(defines.capacity - defines.begin) *
                sizeof(RenderShaderCacheDefine));
    }
    defines = {};
}

RenderResourceManager& resource_manager() {
    return render_system_resource_manager(*render_system_instance());
}

// Reconstructed from eboot.elf at 0x638A40. A compiled-shader cache begins
// with a non-zero marker, the shader variant, and four validation hashes,
// followed by the global defines it was built with and the compiled objects.
// Unvalidated loads skip the hash and define checks.
bool load_cache(
    RenderPrimaryShaderResource& shader,
    const char* path,
    bool validate) {
    FileStream stream(path, kRead, false);
    bool loaded = false;
    if (!stream.Fail()) {
        const auto marker = read_word(stream);
        if (marker != 0 &&
            static_cast<std::int32_t>(read_word(stream)) ==
                shader.dispatch->variant(&shader)) {
            const auto constant_hash = read_word(stream);
            const auto layout_hash = read_word(stream);
            const auto declaration_hash = read_word(stream);
            const auto source_hash = read_word(stream);

            RenderShaderCacheDefineArray defines{};
            std::uint32_t heap_mode = 0;
            MemPushTemp(heap_mode, true, true);
            resize_defines(defines, read_word(stream));
            for (auto* define = defines.begin; define != defines.end; ++define) {
                Symbol name(define->name);
                stream >> name;
                define->name = name.Str();
                stream.ReadEndian(&define->value, sizeof(define->value));
            }
            MemPopTemp(heap_mode);

            bool valid = true;
            if (validate) {
                auto& manager = resource_manager();
                valid = constant_hash ==
                        static_cast<std::uint32_t>(
                            manager.runtime.constant_source_hash) &&
                    layout_hash == render_primary_shader_layout_hash(shader);
                if (valid) {
                    auto hash = kShaderSourceHashBasis;
                    render_shader_constant_block_accumulate_source_hash(
                        *shader.constant_block, hash);
                    render_shader_backend_state_accumulate_source_hash(
                        *shader.backend_state, hash);
                    valid = declaration_hash == hash &&
                        source_hash ==
                            render_shader_hash_source_file(static_cast<const char*>(
                                shader.dispatch->backend_name(&shader))) &&
                        render_shader_cache_defines_match(
                            *manager.shader_cache_defines, defines);
                }
            }
            if (valid) {
                loaded = render_compiled_shader_objects_load(
                    shader.compiled_objects,
                    static_cast<const char*>(shader.backend_name),
                    stream);
            }
            release_defines(defines);
        }
    }
    return loaded;
}

}  // namespace

// Reconstructed from eboot.elf at 0x638D70. Hashes everything that shapes the
// compiled permutation set: the generated constant definitions, every
// parameter registry's names and ranges, and the keys of each valid
// permutation for the variant's graphics stages. Compute permutations are not
// part of this hash.
std::uint32_t render_primary_shader_layout_hash(
    RenderPrimaryShaderResource& shader) {
    auto hash = kShaderSourceHashBasis;
    render_shader_constant_registry_accumulate_source_hash(
        *shader.constants, hash);

    for (const auto& registry : shader.parameters->registries) {
        append_word(
            hash, static_cast<std::uint32_t>(registry.end - registry.begin));
        for (auto* record = registry.begin; record != registry.end; ++record) {
            render_shader_source_hash_append(hash, record->name.c_str());
            append_word(hash, record->first_value);
            append_word(hash, record->last_value_exclusive);
        }
    }

    const auto variant = shader.dispatch->variant(&shader);
    for (std::uint32_t stage = 0; stage < kLayoutHashedStageCount; ++stage) {
        if (!render_shader_stage_enabled(variant, stage)) {
            continue;
        }
        PermutationHashContext context{stage, &hash, &shader};
        render_shader_enumerate_stage_permutations(
            *shader.parameters, stage, hash_permutation, &context);
    }
    return hash;
}

// Reconstructed from eboot.elf at 0x638430. Retail builds cannot compile
// shaders, so a missing, stale, or mismatched cache falls back to loading the
// generated cache without validation. When archive mode is set, caches are
// trusted and validation is skipped. The original's stripped compile path
// still snapshots the global defines and queries the source identifier and
// platform 7 name; those side-effect-free calls are omitted here.
void render_primary_shader_initialize_backend(
    RenderPrimaryShaderResource& shader) {
    auto& section = g_shader_critical_section;
    scePthreadMutexLock(&section.mutex);
    ++section.depth;
    if (!shader.compiled) {
        auto* path = shader.dispatch->backend_name(&shader);
        shader.backend_name = path;

        bool rebuild_needed = false;
        String generated_path;
        new (&generated_path) String("");
        FileStat timestamp{};
        Symbol source("");
        FileResolvePath(source, static_cast<const char*>(path));
        if (FileFindGenerated(
                source, "", generated_path, rebuild_needed, timestamp)) {
            const bool archive_mode = gFileArchiveMode != 0;
            bool failed = true;
            if (!rebuild_needed || archive_mode) {
                failed = !load_cache(
                    shader, generated_path.c_str(), !archive_mode);
            }
            if (failed && !archive_mode) {
                load_cache(shader, generated_path.c_str(), false);
            }
        }
        shader.compiled = true;
        (generated_path).~String();
    }
    --section.depth;
    scePthreadMutexUnlock(&section.mutex);
}

}  // namespace rb4
