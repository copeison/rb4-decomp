#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's drawing of lights, light probes and flares
// (render/RndLightOptionsCom.o, 0x48ED90-0x49005B). Its class id is
// "LightOptions". The vtable at 0x1906A38 has 41 slots. The object is 64
// bytes.
class RndLightOptionsCom : public RndOptionsCom {
public:
    RndLightOptionsCom();  // 0x48EDB0
    ~RndLightOptionsCom() override;  // slots 0-1: 0x48EE20, 0x48EE30

    Symbol GetId() const override;         // slot 4: 0x48FDA0
    Symbol GetClassName() const override;  // slot 5: 0x48FDB0
    int CurrentRev() const override;       // slot 7: 0x48FDC0
    bool IsA(Symbol type) const override;  // slot 8: 0x48FDE0
    Component* AsComponent() override;     // slot 9: 0x48FE10
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x48FE20
    // Slot 20: stops being theRndLightOpts.
    void _PreDestroy(DestroyType type) override;  // 0x48ED90
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x48FF70
    ComMetaData& _GetMetaData() override;       // slot 23: 0x48FF80
    // Slot 29: becomes theRndLightOpts.
    bool _OnResourcesLoaded() override;  // 0x48EDA0

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x48EE50

    static Symbol sId;                  // 0x1A88628, "LightOptions"
    // Also "LightOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1A88630
    static PropRegistry sPropRegistry;  // 0x1A88640
    static ComMetaData sMetaData;       // 0x1A886E0

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowLights;  // "show_lights"
    bool mShowLightsInEntities;  // "show_lights_in_entities"
    bool mOnlyShowVolumetricLights;  // "only_show_volumetric_lights"
    bool mShowSelectedLightFalloffs;  // "show_selected_light_falloffs"
    bool mShowSelectedClipPlaneFalloffs;  // "show_selected_clip_plane_falloffs"
    bool mShowAllLightFalloffs;  // "show_all_light_falloffs"
    float mLightFalloffOpacity;  // "light_falloff_opacity"
    int mShowShadowFrusta;  // "show_shadow_frusta"
    bool mShowLightProbes;  // "show_light_probes"
    bool mShowSelectedProbeFalloffs;  // "show_selected_probe_falloffs"
    int mLightProbeVisualization;  // "light_probe_visualization"
    float mLightProbeVisSmoothness;  // "light_probe_vis_smoothness"
    float mLightProbeVisOpacity;  // "light_probe_vis_opacity"
    bool mHideFlares;  // "hide_flares"
    bool mShowSelectedFlareLightSources;  // "show_selected_flare_light_sources"
    bool mShowSelectedLightBoundingVolumes;  // "show_selected_light_bounding_volumes"
    bool mShowSelectedLightShadowCullingPlanes;  // "show_selected_light_shadow_culling_planes"
};

static_assert(offsetof(RndLightOptionsCom, mShowLights) == 0x17);
static_assert(offsetof(RndLightOptionsCom, mShowLightsInEntities) == 0x18);
static_assert(offsetof(RndLightOptionsCom, mOnlyShowVolumetricLights) == 0x19);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedLightFalloffs) == 0x1A);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedClipPlaneFalloffs) == 0x1B);
static_assert(offsetof(RndLightOptionsCom, mShowAllLightFalloffs) == 0x1C);
static_assert(offsetof(RndLightOptionsCom, mLightFalloffOpacity) == 0x20);
static_assert(offsetof(RndLightOptionsCom, mShowShadowFrusta) == 0x24);
static_assert(offsetof(RndLightOptionsCom, mShowLightProbes) == 0x28);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedProbeFalloffs) == 0x29);
static_assert(offsetof(RndLightOptionsCom, mLightProbeVisualization) == 0x2C);
static_assert(offsetof(RndLightOptionsCom, mLightProbeVisSmoothness) == 0x30);
static_assert(offsetof(RndLightOptionsCom, mLightProbeVisOpacity) == 0x34);
static_assert(offsetof(RndLightOptionsCom, mHideFlares) == 0x38);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedFlareLightSources) == 0x39);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedLightBoundingVolumes) == 0x3A);
static_assert(offsetof(RndLightOptionsCom, mShowSelectedLightShadowCullingPlanes) == 0x3B);
static_assert(sizeof(RndLightOptionsCom) == 0x40);

// The live component, or null while none has loaded its resources.
extern RndLightOptionsCom* theRndLightOpts;  // 0x1A88888
