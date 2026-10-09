#pragma once

#include <cstddef>
#include <cstdint>

class RndTextureBase;

class RndContext;

namespace rb4 {

struct RenderSystem;

struct AudioAnalysisTextureSet {
    RndTextureBase** textures;
    std::size_t texture_count;
    std::size_t texture_capacity;
    RndTextureBase* inline_textures[2];
    float* samples_begin;
    float* samples_end;
    float* samples_capacity;
    void* samples_allocator;
};

static_assert(offsetof(AudioAnalysisTextureSet, textures) == 0);
static_assert(offsetof(AudioAnalysisTextureSet, texture_count) == 8);
static_assert(offsetof(AudioAnalysisTextureSet, inline_textures) == 24);
static_assert(offsetof(AudioAnalysisTextureSet, samples_begin) == 40);
static_assert(sizeof(AudioAnalysisTextureSet) == 72);

AudioAnalysisTextureSet& render_system_audio_analysis_textures(
    RenderSystem& system);
void audio_analysis_texture_set_construct(
    AudioAnalysisTextureSet& textures);
void audio_analysis_texture_set_destruct(
    AudioAnalysisTextureSet& textures);
void audio_analysis_textures_prepare_frame(
    AudioAnalysisTextureSet& textures,
    RndContext& context);

}  // namespace rb4
