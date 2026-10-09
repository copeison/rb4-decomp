#include "render/resources/audio/audio_analysis_textures.h"

#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/textures/RndTextureBase.h"
#include "render/resources/audio/audio_analysis_texture_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kAudioAnalysisTextureSetOffset = 3576;

bool texture_width_changed(
    const RndTextureBase* texture,
    std::int32_t requested_width) {
    return requested_width > 0 &&
        (texture == nullptr ||
         texture->mBaseDesc.mWidth != static_cast<std::uint32_t>(requested_width));
}

}  // namespace

AudioAnalysisTextureSet& render_system_audio_analysis_textures(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return **reinterpret_cast<AudioAnalysisTextureSet**>(
        bytes + kAudioAnalysisTextureSetOffset);
}

// Reconstructed from eboot.elf at 0x457620.
void audio_analysis_texture_set_construct(
    AudioAnalysisTextureSet& textures) {
    textures.textures = textures.inline_textures;
    textures.texture_count = 2;
    textures.texture_capacity = 2;
    textures.inline_textures[0] = nullptr;
    textures.inline_textures[1] = nullptr;
    textures.samples_begin = nullptr;
    textures.samples_end = nullptr;
    textures.samples_capacity = nullptr;
    textures.samples_allocator = nullptr;
}

// Reconstructed from eboot.elf at 0x4576A0.
void audio_analysis_texture_set_destruct(
    AudioAnalysisTextureSet& textures) {
    for (std::size_t index = 0;
         index < textures.texture_count;
         ++index) {
        if (textures.textures[index] != nullptr) {
            delete textures.textures[index];
            textures.textures[index] = nullptr;
        }
    }

    if (textures.samples_begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(textures.samples_capacity) -
            reinterpret_cast<std::uint8_t*>(textures.samples_begin));
        HmxAllocator::gStlAllocator.deallocate(textures.samples_begin, byte_count);
    }
    textures.samples_begin = nullptr;
    textures.samples_end = nullptr;
    textures.samples_capacity = nullptr;
}

// Reconstructed from eboot.elf at 0x457780.
void audio_analysis_textures_prepare_frame(
    AudioAnalysisTextureSet& textures,
    RenderContext& context) {
    const auto widths = audio_analysis_texture_widths();
    if (texture_width_changed(textures.textures[0], widths.channels[0]) ||
        texture_width_changed(textures.textures[1], widths.channels[1])) {
        audio_analysis_textures_rebuild(textures);
    }
    audio_analysis_textures_update(textures, context);
}

}  // namespace rb4
