#include "render/resources/shaders/compiled_shader_objects.h"

#include <cstring>

#include "utl/streams/BinStream.h"
#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/core/platform/render_platform.h"
#include "render/core/shaders/render_shader.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kMinimumCacheVersion = 3;
constexpr std::uint32_t kMinimumStageCount = 4;

std::uint32_t read_u32(BinStream& stream) {
    std::uint32_t value = 0;
    stream.ReadEndian(&value, sizeof(value));
    return value;
}

// Reconstructed from eboot.elf at 0x63C2B0. Each compiled-object record
// starts with a discarded 32-bit field followed by its 64-bit permutation key.
std::uint64_t read_permutation_key(BinStream& stream) {
    std::uint32_t discarded = 0;
    stream.ReadEndian(&discarded, sizeof(discarded));
    std::uint64_t key = 0;
    stream.ReadEndian(&key, sizeof(key));
    return key;
}

void release_objects(RenderManagedObjectArray& objects) {
    const auto count = static_cast<std::size_t>(objects.end - objects.begin);
    for (std::size_t index = 0; index < count; ++index) {
        auto* object = objects.begin[index];
        if (object != nullptr) {
            object->dispatch->release_dynamic(object);
        }
    }
    objects.end = objects.begin;
}

}  // namespace

// Reconstructed from eboot.elf at 0x63BB50 and the inline shrink path of
// 0x63B2B0. Growth zero-fills new entries and reallocates to the larger of
// the requested size and double the current size (one when empty).
void render_compiled_shader_objects_resize(
    RenderManagedObjectArray& objects,
    std::size_t count) {
    const auto size = static_cast<std::size_t>(objects.end - objects.begin);
    if (count <= size) {
        objects.end = objects.begin + count;
        return;
    }

    const auto added = count - size;
    const auto capacity =
        static_cast<std::size_t>(objects.capacity - objects.end);
    if (capacity >= added) {
        std::memset(objects.end, 0, added * sizeof(*objects.end));
        objects.end += added;
        return;
    }

    auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
    if (new_capacity < count) {
        new_capacity = count;
    }
    constexpr auto kEntrySize = sizeof(RenderManagedObject*);
    auto** storage = static_cast<RenderManagedObject**>(
        HmxAllocator::gStlAllocator.allocate(new_capacity * kEntrySize));
    if (size != 0) {
        std::memmove(storage, objects.begin, size * kEntrySize);
    }
    std::memset(storage + size, 0, added * kEntrySize);
    if (objects.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            objects.begin,
            static_cast<std::size_t>(objects.capacity - objects.begin) *
                kEntrySize);
    }
    objects.begin = storage;
    objects.end = storage + count;
    objects.capacity = storage + new_capacity;
}

// Reconstructed from eboot.elf at 0x63B2B0. The cache stores a version and a
// stage count, then six stage-indexed groups of compiled-object records. Each
// record carries a permutation key and a binary size; non-empty binaries are read
// directly from the stream by the platform shader initializer.
bool render_compiled_shader_objects_load(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    void* metadata,
    BinStream& stream) {
    for (auto& stage_objects : objects) {
        release_objects(stage_objects);
    }

    if (read_u32(stream) < kMinimumCacheVersion) {
        return false;
    }
    if (read_u32(stream) < kMinimumStageCount) {
        return false;
    }

    for (std::size_t stage = 0; stage < kRenderShaderStageCount; ++stage) {
        auto& stage_objects = objects[stage];
        const auto count = read_u32(stream);
        render_compiled_shader_objects_resize(stage_objects, count);

        for (std::size_t index = 0; index < count; ++index) {
            const auto key = read_permutation_key(stream);
            const auto binary_size = read_u32(stream);

            // The original compares the active render API with itself; the
            // cache never records a foreign platform on this build.
            if (orbis_render_api() != orbis_render_api()) {
                stream.Seek(binary_size, kSeekCur);
                continue;
            }

            const auto* binary = binary_size == 0
                ? nullptr
                : reinterpret_cast<const RenderShaderBinary*>(&stream);
            auto* shader =
                render_create_shader(static_cast<RenderShaderStage>(stage));
            stream.Tell();
            auto* object = reinterpret_cast<RenderManagedObject*>(shader);
            if (!render_shader_initialize(*shader, key, binary, metadata)) {
                if (object != nullptr) {
                    object->dispatch->release_dynamic(object);
                }
                return false;
            }
            stream.Tell();
            stage_objects.begin[index] = object;
        }
    }
    return true;
}

}  // namespace rb4
