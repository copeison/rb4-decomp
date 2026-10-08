#include "default_materials.h"

namespace rb4 {

namespace {

constexpr const char* kDefaultUnlitShader =
    "../../system/data/shared/shadergraph/default_unlit.sgraph";
constexpr const char* kDefaultLitShader =
    "../../system/data/shared/shadergraph/default.sgraph";
constexpr const char* kDefaultTextShader =
    "../../system/data/shared/shadergraph/default_text_unlit.sgraph";
constexpr const char* kDefaultParticleShader =
    "../../system/data/shared/shadergraph/default_particle.sgraph";
constexpr const char* kDefaultDecalShader =
    "../../system/data/shared/shadergraph/default_decal_lit.sgraph";

RndMaterial* create_material_object(
    RndScene& scene,
    const char* object_name) {
    auto* object = rnd_scene_create_object(scene, object_name);
    if (object == nullptr) {
        return nullptr;
    }

    auto* material = rnd_object_add_material(*object);
    if (material == nullptr) {
        return nullptr;
    }

    rnd_material_set_sharing_type(
        *material, *object, RndMaterialSharingType::kUnique);
    return material;
}

RndMaterial* create_default_material(
    RndScene& scene,
    const char* object_name,
    const char* shader_graph) {
    auto* material = create_material_object(scene, object_name);
    if (material != nullptr) {
        rnd_material_set_shader_graph(*material, shader_graph);
    }
    return material;
}

RndMaterial* create_default_material(
    RndScene& scene,
    const char* object_name,
    const char* shader_graph,
    RndMaterialBlendMode blend_mode) {
    auto* material = create_material_object(scene, object_name);
    if (material != nullptr) {
        rnd_material_set_blend_mode(*material, blend_mode);
        rnd_material_set_shader_graph(*material, shader_graph);
    }
    return material;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BEC50.
void render_create_default_materials(
    DefaultMaterialSet& materials,
    RndScene& scene) {
    materials.unlit = create_default_material(
        scene, "default_mat_unlit", kDefaultUnlitShader);
    materials.additive = create_default_material(
        scene,
        "default_mat_add",
        kDefaultUnlitShader,
        RndMaterialBlendMode::kAdd);
    materials.lit = create_default_material(
        scene, "default_mat_lit", kDefaultLitShader);
    materials.text = create_default_material(
        scene, "default_text_mat", kDefaultTextShader);
    materials.particle = create_default_material(
        scene,
        "default_particle_mat",
        kDefaultParticleShader,
        RndMaterialBlendMode::kAdd);
    materials.decal = create_default_material(
        scene,
        "default_decal_mat",
        kDefaultDecalShader,
        RndMaterialBlendMode::kSource);
}

}  // namespace rb4
