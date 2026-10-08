#pragma once

#include <cstdint>
#include <vector>

#include "render/core/render_runtime_adapters.h"

namespace rb4 {

enum class DefaultLightingMode : std::int32_t {
    kDirectional = 0,
    kShadowedSpot = 1,
};

struct DefaultLightingState {
    RndSceneResource* resource = nullptr;
    RndSceneSettingsCom* scene_settings = nullptr;
    RndLightProbeCom* probe = nullptr;
    std::vector<RndObjectId> directional_lights;
    std::vector<RndObjectId> shadowed_spot_lights;
    DefaultLightingMode mode = DefaultLightingMode::kDirectional;
    float scale = 100.0f;
};

RndSceneResource* render_load_scene_resource(const char* path);
bool render_load_default_lighting(DefaultLightingState& state);
void render_apply_default_lighting_mode(DefaultLightingState& state);
void render_configure_default_shadowed_spot(
    DefaultLightingState& state,
    RndObject& object);
void render_create_fallback_default_lighting(
    DefaultLightingState& state,
    RndScene& scene);
float render_default_shadow_offset(const DefaultLightingState& state);

}  // namespace rb4
