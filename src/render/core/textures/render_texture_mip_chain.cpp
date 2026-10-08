#include "render/core/textures/render_texture_mip_chain.h"

#include <cstddef>
#include <cstring>
#include <new>

#include "core/memory/engine_memory.h"

namespace rb4 {

namespace {

struct MipChainDispatch {
    void* reserved_destruct;
    void (*release_dynamic)(RenderTextureMipChainState* mip_chain);
};

std::size_t mip_chain_count(const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == nullptr) {
        return 0;
    }
    return static_cast<std::size_t>(mip_chains.end - mip_chains.begin);
}

std::size_t mip_chain_capacity(const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == nullptr) {
        return 0;
    }
    return static_cast<std::size_t>(mip_chains.capacity - mip_chains.begin);
}

std::size_t level_count(const RenderTextureMipChainState& mip_chain) {
    std::size_t count = 0;
    auto* level = &mip_chain;
    while (level != nullptr) {
        ++count;
        level = level->fields.next_mip;
    }
    return count;
}

void release_child(RenderTextureMipChainState*& child) {
    if (child == nullptr) {
        return;
    }
    if (child->fields.implementation != nullptr) {
        auto* dispatch = static_cast<MipChainDispatch*>(
            child->fields.implementation);
        dispatch->release_dynamic(child);
    } else {
        render_texture_mip_chain_destruct(*child);
        ::operator delete(child);
    }
    child = nullptr;
}

void destroy_mip_chain_fields(RenderTextureMipChainFields& fields) {
    delete[] static_cast<std::uint8_t*>(fields.source_data);
    fields.source_data = nullptr;
    fields.source_size = 0;

    release_child(fields.next_mip);
    if (fields.auxiliary_data != nullptr) {
        render_release(fields.auxiliary_data);
        fields.auxiliary_data = nullptr;
    }

    fields.width = 0;
    fields.height = 0;
    fields.depth = 0;
    fields.data_format = -1;
}

void copy_mip_chain_fields(
    RenderTextureMipChainFields& destination,
    const RenderTextureMipChainFields& source,
    bool has_source_data) {
    destination = {};
    destination.data_format = -1;
    destination.width = source.width;
    destination.height = source.height;
    destination.depth = source.depth;
    destination.data_format = source.data_format;
    destination.source_size = source.source_size;
    std::memcpy(destination.metadata, source.metadata, sizeof(source.metadata));

    if (source.source_data != nullptr) {
        auto* pixels = new std::uint8_t[source.source_size];
        std::memcpy(pixels, source.source_data, source.source_size);
        destination.source_data = pixels;
    }

    if (source.next_mip != nullptr) {
        destination.next_mip = new RenderTextureMipChainState;
        render_texture_mip_chain_construct(
            *destination.next_mip,
            reinterpret_cast<const RenderTextureMipChainDescriptor&>(
                *source.next_mip),
            has_source_data);
    }
}

void destroy_elements(
    RenderTextureMipChainState* begin,
    RenderTextureMipChainState* end) {
    for (auto* mip_chain = begin; mip_chain != end; ++mip_chain) {
        render_texture_mip_chain_destruct(*mip_chain);
    }
}

void release_storage(RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin != nullptr) {
        engine_deallocate_sized(
            mip_chains.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(mip_chains.capacity) -
                reinterpret_cast<std::uint8_t*>(mip_chains.begin)));
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x682930.
void render_texture_mip_chain_descriptor_construct(
    RenderTextureMipChainDescriptor& descriptor) {
    descriptor = {};
    descriptor.fields.data_format = -1;
}

// Reconstructed from eboot.elf at 0x682BC0.
void render_texture_mip_chain_descriptor_destruct(
    RenderTextureMipChainDescriptor& descriptor) {
    destroy_mip_chain_fields(descriptor.fields);
}

// Reconstructed from eboot.elf at 0x682960 and 0x6829A0.
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data) {
    copy_mip_chain_fields(
        mip_chain.fields, descriptor.fields, has_source_data);
}

void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain) {
    destroy_mip_chain_fields(mip_chain.fields);
}

void render_texture_mip_chain_array_construct(
    RenderTextureMipChainArray& mip_chains) {
    mip_chains = {};
}

// Reconstructed from eboot.elf at 0x697470.
void render_texture_mip_chain_array_reserve(
    RenderTextureMipChainArray& mip_chains,
    std::size_t capacity) {
    if (capacity <= mip_chain_capacity(mip_chains)) {
        return;
    }

    auto* replacement = static_cast<RenderTextureMipChainState*>(
        engine_allocate_sized(capacity * sizeof(RenderTextureMipChainState)));
    auto* output = replacement;
    for (auto* input = mip_chains.begin;
         input != mip_chains.end;
         ++input, ++output) {
        render_texture_mip_chain_construct(
            *output,
            reinterpret_cast<const RenderTextureMipChainDescriptor&>(*input),
            true);
    }

    destroy_elements(mip_chains.begin, mip_chains.end);
    release_storage(mip_chains);
    mip_chains.begin = replacement;
    mip_chains.end = output;
    mip_chains.capacity = replacement + capacity;
}

// Reconstructed from eboot.elf at 0x697890 and its inline caller.
void render_texture_mip_chain_array_append(
    RenderTextureMipChainArray& mip_chains,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data) {
    if (mip_chains.end == mip_chains.capacity) {
        const auto count = mip_chain_count(mip_chains);
        render_texture_mip_chain_array_reserve(
            mip_chains, count == 0 ? 1 : count * 2);
    }

    render_texture_mip_chain_construct(
        *mip_chains.end, descriptor, has_source_data);
    ++mip_chains.end;
}

// Reconstructed from the validation tail at 0x696D8E.
void render_texture_mip_chain_array_validate(
    const RenderTextureMipChainArray& mip_chains) {
    if (mip_chains.begin == mip_chains.end) {
        return;
    }

    const auto& first = *mip_chains.begin;
    const auto expected_levels = level_count(first);
    for (auto* mip_chain = mip_chains.begin + 1;
         mip_chain != mip_chains.end;
         ++mip_chain) {
        if (mip_chain->fields.width != first.fields.width ||
            mip_chain->fields.data_format != first.fields.data_format ||
            level_count(*mip_chain) != expected_levels) {
            return;
        }
    }
}

void render_texture_mip_chain_array_destruct(
    RenderTextureMipChainArray& mip_chains) {
    destroy_elements(mip_chains.begin, mip_chains.end);
    release_storage(mip_chains);
    mip_chains = {};
}

}  // namespace rb4
