#pragma once

class RndContext;

// Texture selection for each shader stage. The map has
// SelectTextureFor*(RndContext&, unsigned long, sce::Gnm::Texture*,
// RndTexWrapMode, RndTexFilterMode); this build adds a flags argument to
// every stage but the vertex stage reads none. Slots below 16 also get a
// sampler unless the flags suppress it.
namespace PS4RenderUtl {

void SelectTextureForVS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter);                 // 0x8E1EE0
void SelectTextureForHS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E1F90
void SelectTextureForDS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E2040
void SelectTextureForGS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E20F0
// Writable textures bind without a sampler.
void SelectTextureForPS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E21A0
// Uses the compute queue's own tables when the context records compute work.
void SelectTextureForCS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E2290

// Flags understood by the selects. Names not in the reference map.
constexpr unsigned int kSelectWritable = 1U << 0;
constexpr unsigned int kSelectNoSampler = 1U << 4;

}  // namespace PS4RenderUtl
