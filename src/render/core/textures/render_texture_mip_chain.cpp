#include "render/core/textures/render_texture_mip_chain.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/core/textures/render_texture_mip_chain_adapters.h"

namespace rb4 {

namespace {

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
        level = reinterpret_cast<const RenderTextureMipChainState*>(
            level->fields.source_state);
    }
    return count;
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
