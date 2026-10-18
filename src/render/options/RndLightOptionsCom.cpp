// render/RndLightOptionsCom.o (0x48ED90 to 0x49005B).
#include "render/options/RndLightOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/drawing/RndEditorDrawUtl.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x48FF90).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndLightOptionsCom::sId("LightOptions");
Symbol RndLightOptionsCom::sClassName("LightOptions");
PropRegistry RndLightOptionsCom::sPropRegistry;
ComMetaData RndLightOptionsCom::sMetaData;
RndLightOptionsCom* theRndLightOpts;

// Reconstructed from eboot.elf at 0x48ED90.
void RndLightOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndLightOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x48EDA0.
bool RndLightOptionsCom::_OnResourcesLoaded() {
    theRndLightOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x48EDB0.
RndLightOptionsCom::RndLightOptionsCom()
    : mShowLights(true),
      mShowLightsInEntities(false),
      mOnlyShowVolumetricLights(false),
      mShowSelectedLightFalloffs(true),
      mShowSelectedClipPlaneFalloffs(false),
      mShowAllLightFalloffs(false),
      mLightFalloffOpacity(0.2F),
      mShowShadowFrusta(1),
      mShowLightProbes(true),
      mShowSelectedProbeFalloffs(true),
      mLightProbeVisualization(-1),
      mLightProbeVisSmoothness(1.0F),
      mLightProbeVisOpacity(0.75F),
      mHideFlares(false),
      mShowSelectedFlareLightSources(false),
      mShowSelectedLightBoundingVolumes(false),
      mShowSelectedLightShadowCullingPlanes(false) {}

// Reconstructed from eboot.elf at 0x48EE20. The deleting destructor is at
// 0x48EE30.
RndLightOptionsCom::~RndLightOptionsCom() {}

// Reconstructed from eboot.elf at 0x48EE50. The "coder" struct groups the component's
// own members.
void RndLightOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options governing the appearance of lights";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndLightOptionsCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    BoolMetadata& showLights = TypeSpecificMetadata(
        registry.RegisterProp("show_lights", offsetof(RndLightOptionsCom, mShowLights), kPropertyBool, 1),
        prototype.mShowLights);
    showLights.mDefault = prototype.mShowLights;
    static_cast<void>(Symbol("prop"));
    showLights.mHelp = String("Should lights be drawn?");
    BoolMetadata& inEntities = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_lights_in_entities",
            offsetof(RndLightOptionsCom, mShowLightsInEntities),
            kPropertyBool,
            1),
        prototype.mShowLightsInEntities);
    inEntities.mDefault = prototype.mShowLightsInEntities;
    static_cast<void>(Symbol("prop"));
    inEntities.mHelp = String("Should lights nested in entities be drawn?");
    BoolMetadata& onlyVolumetric = TypeSpecificMetadata(
        registry.RegisterProp(
            "only_show_volumetric_lights",
            offsetof(RndLightOptionsCom, mOnlyShowVolumetricLights),
            kPropertyBool,
            1),
        prototype.mOnlyShowVolumetricLights);
    onlyVolumetric.mDefault = prototype.mOnlyShowVolumetricLights;
    static_cast<void>(Symbol("prop"));
    onlyVolumetric.mHelp = String("Should only volumetric lights be drawn?");
    BoolMetadata& selectedFalloffs = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_light_falloffs",
            offsetof(RndLightOptionsCom, mShowSelectedLightFalloffs),
            kPropertyBool,
            1),
        prototype.mShowSelectedLightFalloffs);
    selectedFalloffs.mDefault = prototype.mShowSelectedLightFalloffs;
    static_cast<void>(Symbol("prop"));
    selectedFalloffs.mHelp = String("Should the falloff volumes of selected lights be drawn?");
    BoolMetadata& clipFalloffs = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_clip_plane_falloffs",
            offsetof(RndLightOptionsCom, mShowSelectedClipPlaneFalloffs),
            kPropertyBool,
            1),
        prototype.mShowSelectedClipPlaneFalloffs);
    clipFalloffs.mDefault = prototype.mShowSelectedClipPlaneFalloffs;
    static_cast<void>(Symbol("prop"));
    clipFalloffs.mHelp = String(
        "Should the falloff volumes of lights be drawn when their clip planes are selected?");
    BoolMetadata& allFalloffs = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_all_light_falloffs",
            offsetof(RndLightOptionsCom, mShowAllLightFalloffs),
            kPropertyBool,
            1),
        prototype.mShowAllLightFalloffs);
    allFalloffs.mDefault = prototype.mShowAllLightFalloffs;
    static_cast<void>(Symbol("prop"));
    allFalloffs.mHelp = String("Should the falloff volumes of *all* lights be drawn?");
    FloatMetadata& falloffOpacity = TypeSpecificMetadata(
        registry.RegisterProp(
            "light_falloff_opacity",
            offsetof(RndLightOptionsCom, mLightFalloffOpacity),
            kPropertyFloat,
            4),
        prototype.mLightFalloffOpacity);
    falloffOpacity.mDefault = prototype.mLightFalloffOpacity;
    static_cast<void>(Symbol("prop"));
    falloffOpacity.mHelp = String("The opacity of light falloff volumes");
    falloffOpacity.mMin = 0.0F;
    falloffOpacity.mMax = 1.0F;
    falloffOpacity.mSlider = true;
    IntMetadata<int>& shadowFrusta = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "show_shadow_frusta",
        offsetof(RndLightOptionsCom, mShowShadowFrusta), kPropertyInt, 4).mMetadata.Get());
    shadowFrusta.mDefault = prototype.mShowShadowFrusta;
    static_cast<void>(Symbol("prop"));
    shadowFrusta.mHelp = String("Should we draw the frusta of cameras used to render shadowmaps?");
    shadowFrusta.mAllowedValues = RndEditorDrawUtl::GetDrawWhenAllowedValues();
    BoolMetadata& probes = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_light_probes",
            offsetof(RndLightOptionsCom, mShowLightProbes),
            kPropertyBool,
            1),
        prototype.mShowLightProbes);
    probes.mDefault = prototype.mShowLightProbes;
    static_cast<void>(Symbol("prop"));
    probes.mHelp = String("Should light probes be drawn?");
    BoolMetadata& probeFalloffs = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_probe_falloffs",
            offsetof(RndLightOptionsCom, mShowSelectedProbeFalloffs),
            kPropertyBool,
            1),
        prototype.mShowSelectedProbeFalloffs);
    probeFalloffs.mDefault = prototype.mShowSelectedProbeFalloffs;
    static_cast<void>(Symbol("prop"));
    probeFalloffs.mHelp = String("Should the falloff volumes of selected light probes be drawn?");
    IntMetadata<int>& probeVis = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "light_probe_visualization",
        offsetof(RndLightOptionsCom, mLightProbeVisualization), kPropertyInt, 4).mMetadata.Get());
    probeVis.mDefault = prototype.mLightProbeVisualization;
    static_cast<void>(Symbol("prop"));
    probeVis.mHelp = String(
        "Use to display the captured diffuse/specular textures for a selected light probe");
    probeVis.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{-1, String("None"), String("")},
        AllowedValue<int>{0, String("Diffuse"), String("")},
        AllowedValue<int>{1, String("Specular"), String("")}};
    FloatMetadata& visSmoothness = TypeSpecificMetadata(
        registry.RegisterProp(
            "light_probe_vis_smoothness",
            offsetof(RndLightOptionsCom, mLightProbeVisSmoothness),
            kPropertyFloat,
            4),
        prototype.mLightProbeVisSmoothness);
    visSmoothness.mDefault = prototype.mLightProbeVisSmoothness;
    static_cast<void>(Symbol("prop"));
    visSmoothness.mHelp = String("The smoothness level to display for specular light probe visualization");
    visSmoothness.mMin = 0.0F;
    visSmoothness.mMax = 1.0F;
    visSmoothness.mSlider = true;
    FloatMetadata& visOpacity = TypeSpecificMetadata(
        registry.RegisterProp(
            "light_probe_vis_opacity",
            offsetof(RndLightOptionsCom, mLightProbeVisOpacity),
            kPropertyFloat,
            4),
        prototype.mLightProbeVisOpacity);
    visOpacity.mDefault = prototype.mLightProbeVisOpacity;
    static_cast<void>(Symbol("prop"));
    visOpacity.mHelp = String("The opacity of light probe visualization");
    visOpacity.mMin = 0.0F;
    visOpacity.mMax = 1.0F;
    visOpacity.mSlider = true;
    BoolMetadata& hideFlares = TypeSpecificMetadata(
        registry.RegisterProp("hide_flares", offsetof(RndLightOptionsCom, mHideFlares), kPropertyBool, 1),
        prototype.mHideFlares);
    hideFlares.mDefault = prototype.mHideFlares;
    static_cast<void>(Symbol("prop"));
    hideFlares.mHelp = String("Should the rendering of light flares be suppressed?");
    BoolMetadata& flareSources = TypeSpecificMetadata(
        registry.RegisterProp(
            "show_selected_flare_light_sources",
            offsetof(RndLightOptionsCom, mShowSelectedFlareLightSources),
            kPropertyBool,
            1),
        prototype.mShowSelectedFlareLightSources);
    flareSources.mDefault = prototype.mShowSelectedFlareLightSources;
    static_cast<void>(Symbol("prop"));
    flareSources.mHelp = String(
        "Should the light sources (ie. occlusion-query spheres) for flares be drawn?");
    PropRegistry& coder = static_cast<GroupMetadata&>(
        *registry.RegisterProp("coder", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& boundingVolumes = TypeSpecificMetadata(
        coder.RegisterProp(
            "show_selected_light_bounding_volumes",
            offsetof(RndLightOptionsCom, mShowSelectedLightBoundingVolumes),
            kPropertyBool,
            1),
        prototype.mShowSelectedLightBoundingVolumes);
    boundingVolumes.mDefault = prototype.mShowSelectedLightBoundingVolumes;
    static_cast<void>(Symbol("prop"));
    boundingVolumes.mHelp = String(
        "Should the bounding volumes for lights be drawn? (eg. used for tiled light culling)");
    BoolMetadata& cullingPlanes = TypeSpecificMetadata(
        coder.RegisterProp(
            "show_selected_light_shadow_culling_planes",
            offsetof(RndLightOptionsCom, mShowSelectedLightShadowCullingPlanes),
            kPropertyBool,
            1),
        prototype.mShowSelectedLightShadowCullingPlanes);
    cullingPlanes.mDefault = prototype.mShowSelectedLightShadowCullingPlanes;
    static_cast<void>(Symbol("prop"));
    cullingPlanes.mHelp = String(
        "Should the extra culling planes for selected shadow-casting lights be drawn?");
}

// Reconstructed from eboot.elf at 0x48FDA0.
Symbol RndLightOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x48FDB0.
Symbol RndLightOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x48FDC0.
int RndLightOptionsCom::CurrentRev() const {
    return const_cast<RndLightOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x48FDE0.
bool RndLightOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x48FE10.
Component* RndLightOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x48FE20. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x48FF70.
PropRegistry& RndLightOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x48FF80.
ComMetaData& RndLightOptionsCom::_GetMetaData() {
    return sMetaData;
}
