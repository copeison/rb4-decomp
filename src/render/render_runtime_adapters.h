#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RndLightCom;
struct RndLightDirectionalCom;
struct RndLightProbeCom;
struct RndLightSpotCom;
struct RndObject;
struct RndScene;
struct RndSceneResource;
struct RndSceneSettingsCom;

using RndObjectId = std::uint32_t;

RndSceneResource* resource_manager_load_scene(
    const char* path,
    bool block_until_loaded);
void rnd_scene_resource_release(RndSceneResource* resource);
RndScene* rnd_scene_resource_scene(RndSceneResource& resource);
RndSceneSettingsCom* rnd_scene_settings(RndScene& scene);

std::size_t rnd_scene_object_count(const RndScene& scene);
RndObject* rnd_scene_object_at(RndScene& scene, std::size_t index);
RndObject* rnd_scene_find_object(RndScene& scene, const char* name);
RndObject* rnd_scene_find_object(RndScene& scene, RndObjectId id);
RndObjectId rnd_object_id(const RndObject& object);

RndLightCom* rnd_object_light(RndObject& object);
RndLightDirectionalCom* rnd_object_directional_light(RndObject& object);
RndLightSpotCom* rnd_object_spot_light(RndObject& object);
RndLightProbeCom* rnd_object_light_probe(RndObject& object);

void rnd_light_set_enabled(RndLightCom& light, bool enabled);
void rnd_light_probe_set_enabled(RndLightProbeCom& probe, bool enabled);
void rnd_light_probe_set_falloff_start(
    RndLightProbeCom& probe,
    float distance);
void rnd_light_probe_set_falloff_end(
    RndLightProbeCom& probe,
    float distance);
void rnd_light_spot_set_falloff_start(RndLightSpotCom& light, float distance);
void rnd_light_spot_set_falloff_end(RndLightSpotCom& light, float distance);
void rnd_object_reset_transform_with_scaled_position(
    RndObject& object,
    float position_scale);

}  // namespace rb4
