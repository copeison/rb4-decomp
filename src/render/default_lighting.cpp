#include "default_lighting.h"

namespace rb4 {

namespace {

constexpr const char* kDefaultLightingScene =
    "../../system/data/render/default_lighting.scene";
constexpr const char* kDefaultDirectional = "default_directional";
constexpr const char* kDefaultShadowedSpot = "default_spot_with_shadows";
constexpr const char* kDefaultProbe = "default_probe";
constexpr const char* kFallbackDirectional = "default_directional_light";

constexpr float kProbeFalloffStartScale = 2.0f;
constexpr float kProbeFalloffEndScale = 3.0f;
constexpr float kSpotFalloffStartScale = 1.0f;
constexpr float kSpotFalloffEndScale = 2.0f;

void replace_resource(
    RndSceneResource*& destination,
    RndSceneResource* replacement) {
    if (destination != nullptr) {
        rnd_scene_resource_release(destination);
    }
    destination = replacement;
}

void disable_authored_lighting(RndScene& scene) {
    const auto object_count = rnd_scene_object_count(scene);
    for (std::size_t index = 0; index < object_count; ++index) {
        auto* object = rnd_scene_object_at(scene, index);
        if (object == nullptr) {
            continue;
        }

        if (auto* light = rnd_object_light(*object)) {
            rnd_light_set_enabled(*light, false);
        }
        if (auto* probe = rnd_object_light_probe(*object)) {
            rnd_light_probe_set_enabled(*probe, false);
        }
    }
}

void collect_directional_light(
    DefaultLightingState& state,
    RndScene& scene) {
    auto* object = rnd_scene_find_object(scene, kDefaultDirectional);
    if (object != nullptr && rnd_object_directional_light(*object) != nullptr) {
        state.directional_lights.push_back(rnd_object_id(*object));
    }
}

void collect_shadowed_spot(
    DefaultLightingState& state,
    RndScene& scene) {
    auto* object = rnd_scene_find_object(scene, kDefaultShadowedSpot);
    if (object != nullptr && rnd_object_spot_light(*object) != nullptr) {
        render_configure_default_shadowed_spot(state, *object);
        state.shadowed_spot_lights.push_back(rnd_object_id(*object));
    }
}

void configure_probe(DefaultLightingState& state, RndScene& scene) {
    auto* object = rnd_scene_find_object(scene, kDefaultProbe);
    state.probe = object == nullptr ? nullptr : rnd_object_light_probe(*object);
    if (state.probe == nullptr) {
        return;
    }

    rnd_light_probe_set_enabled(*state.probe, true);
    rnd_light_probe_set_falloff_start(
        *state.probe, state.scale * kProbeFalloffStartScale);
    rnd_light_probe_set_falloff_end(
        *state.probe, state.scale * kProbeFalloffEndScale);
}

void set_light_list_enabled(
    RndScene& scene,
    const std::vector<RndObjectId>& object_ids,
    bool enabled) {
    for (const auto id : object_ids) {
        auto* object = rnd_scene_find_object(scene, id);
        if (object == nullptr) {
            continue;
        }

        auto* light = rnd_object_light(*object);
        if (light != nullptr) {
            rnd_light_set_enabled(*light, enabled);
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6C0160.
RndSceneResource* render_load_scene_resource(const char* path) {
    return resource_manager_load_scene(path, false);
}

// Reconstructed from eboot.elf at 0x6BEF40.
bool render_load_default_lighting(DefaultLightingState& state) {
    replace_resource(
        state.resource, render_load_scene_resource(kDefaultLightingScene));
    if (state.resource == nullptr) {
        return false;
    }

    auto* scene = rnd_scene_resource_scene(*state.resource);
    if (scene == nullptr) {
        replace_resource(state.resource, nullptr);
        return false;
    }

    state.scene_settings = rnd_scene_settings(*scene);
    if (state.scene_settings == nullptr) {
        replace_resource(state.resource, nullptr);
        return false;
    }

    state.directional_lights.clear();
    state.shadowed_spot_lights.clear();
    disable_authored_lighting(*scene);
    collect_directional_light(state, *scene);
    collect_shadowed_spot(state, *scene);
    configure_probe(state, *scene);
    render_apply_default_lighting_mode(state);
    return true;
}

// Reconstructed from eboot.elf at 0x6BFA60.
void render_apply_default_lighting_mode(DefaultLightingState& state) {
    if (state.resource == nullptr) {
        return;
    }

    auto* scene = rnd_scene_resource_scene(*state.resource);
    if (scene == nullptr) {
        return;
    }

    set_light_list_enabled(
        *scene,
        state.directional_lights,
        state.mode == DefaultLightingMode::kDirectional);
    set_light_list_enabled(
        *scene,
        state.shadowed_spot_lights,
        state.mode == DefaultLightingMode::kShadowedSpot);
}

// Reconstructed from eboot.elf at 0x6BFD40.
void render_configure_default_shadowed_spot(
    DefaultLightingState& state,
    RndObject& object) {
    auto* light = rnd_object_spot_light(object);
    if (light == nullptr) {
        return;
    }

    rnd_light_spot_set_falloff_start(
        *light, state.scale * kSpotFalloffStartScale);
    rnd_light_spot_set_falloff_end(
        *light, state.scale * kSpotFalloffEndScale);
    rnd_object_reset_transform_with_scaled_position(object, state.scale);
}

// Reconstructed from eboot.elf at 0x6BF4F0.
void render_create_fallback_default_lighting(
    DefaultLightingState& state,
    RndScene& scene) {
    state.scene_settings = rnd_scene_settings(scene);

    auto* object = rnd_scene_create_object(scene, kFallbackDirectional);
    if (object == nullptr) {
        return;
    }

    auto* light = rnd_object_add_directional_light(*object);
    if (light == nullptr) {
        return;
    }

    rnd_light_directional_set_intensity(*light, 2.0f);
    rnd_object_set_default_directional_light_transform(*object);
    state.directional_lights.push_back(rnd_object_id(*object));
    render_apply_default_lighting_mode(state);
}

// Reconstructed from eboot.elf at 0x6BFEA0.
float render_default_shadow_offset(const DefaultLightingState& state) {
    if (state.resource == nullptr || state.shadowed_spot_lights.empty()) {
        return 0.0f;
    }

    auto* scene = rnd_scene_resource_scene(*state.resource);
    if (scene == nullptr) {
        return 0.0f;
    }

    auto* object = rnd_scene_find_object(
        *scene, state.shadowed_spot_lights.front());
    if (object == nullptr) {
        return 0.0f;
    }

    auto* light = rnd_object_spot_light(*object);
    return light == nullptr ? 0.0f : rnd_light_spot_shadow_offset(*light);
}

}  // namespace rb4
