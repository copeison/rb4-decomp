#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RndLightCom;
struct RndLightDirectionalCom;
struct RndLightProbeCom;
struct RndLightSpotCom;
struct RndMaterial;
struct RndCameraCom;
struct RndComputeBuffer;
struct RndObject;
struct RndScene;
struct RndSceneResource;
struct RndSceneSettingsCom;

using RndObjectId = std::uint32_t;

enum class RndMaterialSharingType : std::int32_t {
    kAutomatic = 0,
    kShared = 1,
    kUnique = 2,
};

// Values confirmed from the material blend-mode metadata table.
enum class RndMaterialBlendMode : std::int32_t {
    kSourceAlpha = 0,
    kSourceAlphaAdd = 1,
    kPremultipliedAlpha = 2,
    kScreen = 3,
    kDestination = 4,
    kSource = 5,
    kAdd = 6,
};

RndSceneResource* resource_manager_load_scene(
    const char* path,
    bool block_until_loaded);
void rnd_scene_resource_release(RndSceneResource* resource);
RndSceneResource* rnd_scene_resource_create();
RndScene* rnd_scene_resource_scene(RndSceneResource& resource);
RndSceneSettingsCom* rnd_scene_settings(RndScene& scene);
void rnd_scene_resource_finalize_contents(RndSceneResource& resource);
void rnd_scene_resource_finalize(RndSceneResource& resource);
bool render_force_default_resources();
RndComputeBuffer* render_create_default_compute_buffer(
    std::uint32_t index,
    const char* name);

std::size_t rnd_scene_object_count(const RndScene& scene);
RndObject* rnd_scene_object_at(RndScene& scene, std::size_t index);
RndObject* rnd_scene_find_object(RndScene& scene, const char* name);
RndObject* rnd_scene_find_object(RndScene& scene, RndObjectId id);
RndObject* rnd_scene_create_object(RndScene& scene, const char* name);
RndObjectId rnd_object_id(const RndObject& object);

RndLightCom* rnd_object_light(RndObject& object);
RndLightDirectionalCom* rnd_object_directional_light(RndObject& object);
RndLightDirectionalCom* rnd_object_add_directional_light(RndObject& object);
RndLightSpotCom* rnd_object_spot_light(RndObject& object);
RndLightProbeCom* rnd_object_light_probe(RndObject& object);
RndMaterial* rnd_object_add_material(RndObject& object);
RndCameraCom* rnd_object_add_camera(RndObject& object);

void rnd_light_set_enabled(RndLightCom& light, bool enabled);
void rnd_light_directional_set_intensity(
    RndLightDirectionalCom& light,
    float intensity);
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
void rnd_object_set_default_directional_light_transform(RndObject& object);
void rnd_material_set_sharing_type(
    RndMaterial& material,
    RndObject& owner,
    RndMaterialSharingType sharing_type);
void rnd_material_set_blend_mode(
    RndMaterial& material,
    RndMaterialBlendMode blend_mode);
void rnd_material_set_shader_graph(RndMaterial& material, const char* path);

}  // namespace rb4
