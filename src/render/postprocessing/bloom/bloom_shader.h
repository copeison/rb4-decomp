#pragma once

#include <cstddef>
#include <cstdint>

class RndTextureBase;

class RndContext;

namespace rb4 {

// 48-byte parameter block assembled by the bloom pass.
struct RenderBloomDrawParameters {
    RndTextureBase* source;
    RndTextureBase* half_size_bloom;
    RndTextureBase* quarter_size_bloom;
    float bloom[3];
    bool hue_preservation;
    std::uint8_t reserved_37[3];
    float overbright[2];
};

static_assert(offsetof(RenderBloomDrawParameters, bloom) == 0x18);
static_assert(offsetof(RenderBloomDrawParameters, hue_preservation) == 0x24);
static_assert(offsetof(RenderBloomDrawParameters, overbright) == 0x28);
static_assert(sizeof(RenderBloomDrawParameters) == 0x30);

void render_bloom_shader_draw(
    void* shader,
    RndContext& context,
    const RenderBloomDrawParameters& parameters);

}  // namespace rb4
