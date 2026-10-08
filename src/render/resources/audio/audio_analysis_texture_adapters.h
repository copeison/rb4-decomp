#pragma once

#include <cstdint>

namespace rb4 {

struct AudioAnalysisTextureSet;
struct RenderContext;

struct AudioAnalysisTextureWidths {
    std::int32_t channels[2];
};

AudioAnalysisTextureWidths audio_analysis_texture_widths();
void audio_analysis_textures_rebuild(AudioAnalysisTextureSet& textures);
void audio_analysis_textures_update(
    AudioAnalysisTextureSet& textures,
    RenderContext& context);

}  // namespace rb4
