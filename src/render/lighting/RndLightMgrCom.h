#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/Component.h"

class RndComputeBuffer;
class RndTextureArray2D;

// The scene's light manager: its tiled-light buffers and the spot shadow
// depth array. The functions follow RndLightGlobals in the binary, where the
// map places RndLightMgrCom.o, and the map's RndLightMgrCom::_InitBuffers()
// matches the buffer setup. Field
// names are not in the reference map; the property members take the names
// of the properties the registry (0x486830) binds to their offsets, and the
// constructors are at 0x480280 and 0x4803B0 (the part from offset 200).
class RndLightMgrCom : public Component {
public:
    // A spot shadow configuration: the number of shadow layers and the
    // index of their resolution. One element of the "quality_settings"
    // property, whose element registry also names the flag below. Name not
    // in the reference map.
    struct SpotShadowConfig {
        std::uint64_t mNumLayers;
        std::uint32_t mResolutionIndex;
        // "tonemapping_enabled": whether the quality setting tonemaps.
        bool mTonemappingEnabled;
    };

    // Creates the tiled-light buffers when tiled lighting is enabled, then
    // the spot shadow depth array.
    void _InitBuffers();  // 0x48A400
    // Recreates the spot shadow depth array for the active configuration.
    // Name not in the reference map.
    void _SyncSpotShadowDepthTexArray();  // 0x48AB30
    // The tiled-light portion of 0x480AD0. Name not in the reference map.
    void _TerminateBuffers();

    std::int32_t mDefaultEnvironment;  // An object id.
    std::int32_t mAmbientOcclusion;    // An object id.
    float mMasterIntensityMult;
    std::int32_t mTonemapping;
    std::int32_t mTonemappingOperator;
    float mTonemappingExposure;
    float mTonemappingPower;
    float mMaterialSmoothnessAdjustment;
    std::uint64_t mCookieTextureSize;
    // The "probe_lighting/states" array object; its layout is not modelled.
    unsigned char mProbeStates[40];
    // Symbols naming the two probe states that are blended, and the blend.
    std::uint64_t mCurStateA;
    std::uint64_t mCurStateB;
    float mCurStateBlend;
    // "capture/state", a symbol, and "capture/num_bounces".
    std::uint64_t mCaptureState;
    std::uint64_t mCaptureNumBounces;
    // The "quality_settings" array object: its vtable, the element data,
    // and its count, capacity and metadata.
    unsigned char mQualitySettingsHeader[8];
    SpotShadowConfig* mSpotShadowConfigs;
    unsigned char mQualitySettingsInfo[24];
    // The "lights" filters.
    std::int32_t mActiveFilter;
    std::int32_t mEntityFilter;
    std::int32_t mShadowFilter;
    std::int32_t mVolumetricFilter;
    // Two flags the constructor clears; the update (0x48A6E0) clears the
    // second. Not decoded further.
    unsigned char mUpdateFlags[4];
    std::int32_t mSpotShadowConfigIndex;
    // The registered objects and their bookkeeping, not modelled: the
    // "environments" (224), "lights" (360) and "probes" (432) arrays among
    // them, and GPU objects at 992-1040 that the destructor releases.
    unsigned char mObjectLists[840];
    bool mSpotShadowDepthActive;
    RndTextureArray2D* mSpotShadowDepthTexArray;
    // Two words the constructor zeroes; not decoded.
    unsigned char mIsolationState[16];
    // "isolated_light" and "isolated_probe": object ids, -1 when none, each
    // followed by a reference the constructor zeroes.
    std::int32_t mIsolatedLight;
    unsigned char mIsolatedLightRef[12];
    std::int32_t mIsolatedProbe;
    unsigned char mIsolatedProbeRef[12];
    std::uint64_t mCurProbeStateAIndex;
    std::uint64_t mCurProbeStateBIndex;
    // The pending probe-state renames (1128), then, from 1168, two vectors
    // per light type (64 bytes each, point, spot, directional) of the lights
    // the culling keeps: the second holds the lights whose
    // "illumination_type" is 3 (0x482EB0), and the light buffers list the
    // first vector before the second (0x484160).
    unsigned char mLightLists[560];
    // Each light buffer is followed by the number of lights written from the
    // first and from the second list.
    RndComputeBuffer* mPointLightBuffer;
    std::uint64_t mNumPointLights[2];
    RndComputeBuffer* mSpotLightBuffer;
    std::uint64_t mNumSpotLights[2];
    RndComputeBuffer* mDirectionalLightBuffer;
    std::uint64_t mNumDirectionalLights[2];
    RndComputeBuffer* mLightProbeBuffer;
    RndComputeBuffer* mSliceZeroLightIdBuffer;
};

static_assert(sizeof(RndLightMgrCom::SpotShadowConfig) == 16);
static_assert(offsetof(RndLightMgrCom::SpotShadowConfig, mTonemappingEnabled) == 12);
static_assert(offsetof(RndLightMgrCom, mDefaultEnvironment) == 24);
static_assert(offsetof(RndLightMgrCom, mCookieTextureSize) == 56);
static_assert(offsetof(RndLightMgrCom, mProbeStates) == 64);
static_assert(offsetof(RndLightMgrCom, mCurStateA) == 104);
static_assert(offsetof(RndLightMgrCom, mCurStateBlend) == 120);
static_assert(offsetof(RndLightMgrCom, mCaptureState) == 128);
static_assert(offsetof(RndLightMgrCom, mQualitySettingsHeader) == 144);
static_assert(offsetof(RndLightMgrCom, mSpotShadowConfigs) == 152);
static_assert(offsetof(RndLightMgrCom, mActiveFilter) == 184);
static_assert(offsetof(RndLightMgrCom, mVolumetricFilter) == 196);
static_assert(offsetof(RndLightMgrCom, mUpdateFlags) == 200);
static_assert(offsetof(RndLightMgrCom, mSpotShadowConfigIndex) == 204);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthActive) == 1048);
static_assert(offsetof(RndLightMgrCom, mSpotShadowDepthTexArray) == 1056);
static_assert(offsetof(RndLightMgrCom, mIsolatedLight) == 1080);
static_assert(offsetof(RndLightMgrCom, mIsolatedProbe) == 1096);
static_assert(offsetof(RndLightMgrCom, mCurProbeStateAIndex) == 1112);
static_assert(offsetof(RndLightMgrCom, mCurProbeStateBIndex) == 1120);
static_assert(offsetof(RndLightMgrCom, mLightLists) == 1128);
static_assert(offsetof(RndLightMgrCom, mNumPointLights) == 1696);
static_assert(offsetof(RndLightMgrCom, mPointLightBuffer) == 1688);
static_assert(offsetof(RndLightMgrCom, mSpotLightBuffer) == 1712);
static_assert(offsetof(RndLightMgrCom, mDirectionalLightBuffer) == 1736);
static_assert(offsetof(RndLightMgrCom, mLightProbeBuffer) == 1760);
static_assert(offsetof(RndLightMgrCom, mSliceZeroLightIdBuffer) == 1768);
