#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "render/atmosphere/RndAtmosphereCom.h"
#include "render/textures/RndTexture1DResource.h"
#include "utl/text/Symbol.h"

class Rand;
class RndBufferCollection;
class RndCameraContext;
class RndContext;
class RndLightMgrCom;
class RndShaderCBuffer;
class RndTextureBase;
struct RndSceneDrawTarget;
enum class RndQualityLevel : std::uint32_t;

// Fog that scatters the scene's lights through a froxel volume
// (render/RndVolumetricScatteringCom.o, 0x453C00-0x45760F). Its class id is
// "VolumetricScattering": "Simulates volumetric light scattering throughout
// the scene volume". Like the fog it is the scene's atmosphere; its fog
// covers the atmosphere's range clamped to the camera, with a density that
// a waveform varies over a height range. Each frame BeginAsyncUpdate
// computes the density and in-scattering of every froxel and accumulates
// it along the view rays, and ApplyDeferred fogs the light accumulation
// with it; forward-shaded materials read the range from the scene
// constants. The map spells the class RndVolumtericScatteringCom; this
// build's own type string ("RndVolumetricScatteringCom::QualitySettings")
// spells it as here. The vtable at 0x1903048 has 41 slots. The object is
// 152 bytes.
class RndVolumetricScatteringCom : public RndAtmosphereCom {
public:
    // One quality level's settings, the map's
    // RndVolumtericScatteringCom::QualitySettings.
    struct QualitySettings {
        // The element constructor, emitted at 0x453E60.
        QualitySettings() : mEnabled(true), mResolution(0) {}

        bool mEnabled;  // "enabled"
        // Selects the volume textures: 128, 256 or 512 slices in depth.
        int mResolution;  // "resolution"
    };

    // The "height_density" property, the map's Waveform<float>
    // (entity/Waveform.o, not modelled): its resource, its own evaluation
    // state, the generator it draws from and an override. Modelled only as
    // far as this class touches it. Name and field names not in the
    // reference map; the evidence for the last three is weak.
    struct HeightDensity {
        ResourcePtr<Resource> mResource;
        // Random phases drawn when the waveform is constructed.
        float mEvalData[2];
        Rand* mRand;
        ResourcePtr<Resource> mOverride;
        int mOverrideData[2];
    };

    // The derived state, which imprints do not copy; the map's
    // RuntimeData, whose constructor is emitted at 0x453E70. Field names
    // are not in the reference map.
    struct RuntimeData {
        // mFogDensity scaled to the shaders' units.
        float mFogDensity;
        // Set when "height_density" changes; the edit poll rebakes it.
        bool mHeightDensityDirty;
        // The baked height density waveform.
        ResourcePtr<RndTexture1DResource> mHeightDensityTexture;
    };

    RndVolumetricScatteringCom();  // 0x453C00
    // _Imprint's copy. The waveform draws new phases, and the runtime data
    // is not copied.
    RndVolumetricScatteringCom(
        const RndVolumetricScatteringCom& other);  // 0x456770
    ~RndVolumetricScatteringCom() override;  // slots 0-1: 0x453D40, 0x453E40

    Symbol GetId() const override;         // slot 4: 0x456530
    Symbol GetClassName() const override;  // slot 5: 0x456540
    int CurrentRev() const override;       // slot 7: 0x456550
    bool IsA(Symbol type) const override;  // slot 8: 0x456570
    Component* AsComponent() override;     // slot 9: 0x4565A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4565B0
    // Slot 19: RndAtmosphereCom's, then one settings entry per quality
    // level.
    void _PostCreate() override;                // 0x4561F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x4566C0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x4566D0
    // Slot 29: scales the fog density; a height density left at the empty
    // waveform takes the entity's inlined resource for the property, and
    // the texture is baked. The map has _LoadResources(ObjPtr const&). Not
    // reconstructed: Waveform<float> and the inlined-resource path helper
    // (0x1CFEE0) are not modelled.
    bool _OnResourcesLoaded() override;  // 0x456220
    // Slot 38: rebakes a changed height density.
    void _EditPoll() override;  // 0x4564C0

    // Whether the quality level scatters. Name not in the reference map.
    bool IsEnabled(RndQualityLevel level) const;  // 0x453E90
    // Computes the froxel volume of the quality level's resolution
    // ("Compute Density Inscattering", skipped for the right eye of a
    // shared stereo volume) and accumulates it ("Accumulate Scattering").
    // A stereo pair shares stereoBuffers' volume, computed with the stereo
    // camera. The map's signature is BeginAsyncUpdate(RndContext&,
    // RndCameraContext const&, RndBufferCollection&,
    // RndTiledLightsComputeBuffer*, RndTextureBase*) const.
    void BeginAsyncUpdate(
        RndContext& context,
        const RndCameraContext& camera,
        const RndCameraContext* stereoCamera,
        RndBufferCollection& buffers,
        RndBufferCollection* stereoBuffers,
        RndLightMgrCom* lightMgr,
        RndTextureBase* skyTexture,
        bool useSceneMask) const;  // 0x453EB0
    // Records the volume resolution in the target. The map's
    // signature is EndAsyncUpdate(RndContext&, RndCameraContext const&,
    // RndBufferCollection&) const.
    void EndAsyncUpdate(
        RndContext& context,
        const RndCameraContext& camera,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target) const;  // 0x454560
    // Fogs the target's source light accumulation into its destination and
    // swaps them. Nothing is drawn into cube targets. The map's signature
    // is ApplyDeferred(RndContext&, RndBufferCollection&, bool).
    void ApplyDeferred(
        RndContext& context,
        const RndCameraContext& camera,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        bool useSceneMask);  // 0x454590
    // What ApplyDeferred leaves in the target, without drawing. Name not
    // in the reference map; the evidence is weak.
    void SkipApplyDeferred(RndSceneDrawTarget& target) const;  // 0x454900
    // Writes the fog range of the camera into the scene buffer. The map's
    // signature is SetFwdShadingConstants(RndContext&, RndShaderCBuffer&)
    // const.
    void SetFwdShadingConstants(
        const RndCameraContext& camera,
        RndShaderCBuffer& cbuffer) const;  // 0x454940
    // Writes the constants of no scattering.
    static void SetNoAtmosphereFwdShadingConstants(
        RndShaderCBuffer& cbuffer);  // 0x454A10
    // Bakes the height density into a 128 by 8 texture unless the waveform
    // is constant. Not reconstructed: Waveform<float> is not modelled.
    void _CreateTexture();  // 0x456460

    // Describes the class and registers "quality_settings",
    // "use_camera_end_dist", "fog_density", "height_density",
    // "height_range_begin", "height_range_end" and
    // "volumetric_light_intensity" after
    // RndAtmosphereCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x454A70

    static Symbol sId;  // 0x1A75DB8, "VolumetricScattering"
    // Also "VolumetricScattering". Name not in the reference map.
    static Symbol sClassName;           // 0x1A75DC0
    static PropRegistry sPropRegistry;  // 0x1A75DD0
    static ComMetaData sMetaData;       // 0x1A75E70

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    // Indexed by RndQualityLevel.
    PropArray<QualitySettings> mQualitySettings;  // "quality_settings"
    float mFogDensity;        // "fog_density"
    bool mUseCameraEndDist;   // "use_camera_end_dist"
    // The heights the density waveform spans.
    float mHeightRangeBegin;  // "height_range_begin"
    float mHeightRangeEnd;    // "height_range_end"
    HeightDensity mHeightDensity;      // "height_density"
    float mVolumetricLightIntensity;  // "volumetric_light_intensity"
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndVolumetricScatteringCom::QualitySettings) == 8);
static_assert(sizeof(RndVolumetricScatteringCom::HeightDensity) == 40);
static_assert(offsetof(RndVolumetricScatteringCom, mQualitySettings) == 0x20);
static_assert(offsetof(RndVolumetricScatteringCom, mFogDensity) == 0x48);
static_assert(offsetof(RndVolumetricScatteringCom, mUseCameraEndDist) == 0x4C);
static_assert(offsetof(RndVolumetricScatteringCom, mHeightRangeBegin) == 0x50);
static_assert(offsetof(RndVolumetricScatteringCom, mHeightRangeEnd) == 0x54);
static_assert(offsetof(RndVolumetricScatteringCom, mHeightDensity) == 0x58);
static_assert(
    offsetof(RndVolumetricScatteringCom, mVolumetricLightIntensity) == 0x80);
static_assert(offsetof(RndVolumetricScatteringCom, mRuntimeData) == 0x88);
static_assert(sizeof(RndVolumetricScatteringCom) == 0x98);
