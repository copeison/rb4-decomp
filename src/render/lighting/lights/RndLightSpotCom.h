#pragma once

#include <cstddef>
#include <cstdint>

#include "render/lighting/lights/RndLightCom.h"
#include "utl/text/Symbol.h"

// A spot light. Only the falloff setters, which the renderer's defaults
// call, are reconstructed; the other members are declared as the setters
// and RndDefaults use them.
class RndLightSpotCom : public RndLightCom {
public:
    // Sets where the falloff starts, pushing the end out to it.
    void SetFalloffStart(float distance);  // 0x49F690
    // Sets where the falloff ends, pulling the start in to it.
    void SetFalloffEnd(float distance);  // 0x49F720

    // Rebuilds the light volume from the angles and falloff.
    void _SyncGeometry();  // 0x4A28A0

    // The class symbol, "LightSpot", constructed by the static initializer
    // at 0x4A8620.
    static Symbol sId;  // 0x1A892F8

    // Field names are not in the reference map; the members take the names
    // of the properties the spot light's registry (0x49F9A0) binds to their
    // offsets.
    // RndLightCom's members, which RndLightCom does not declare yet: its
    // registry (0x46D760) places "environments" at 24, "color" at 64,
    // "intensity" at 80, "illumination_type" at 84, "light_wrap" at 88,
    // "clip_plane" at 92, "volumetric" at 96, "environ_bits" at 128,
    // "active_environments" at 136, "isolate" at 212 and
    // "tiled_cookie_index" at 216; byte 23 is padding.
    unsigned char mLightComMembers[209];
    float mBulbRadius;
    float mFalloffStart;
    float mFalloffEnd;
    std::int32_t mFalloffFunction;
    float mStartAngle;
    float mEndAngle;
    std::int32_t mAngleFalloffFunction;
    float mTruncation;
    // The "shadows" group.
    bool mCastsShadows;
    // The "quality_settings" array object; its layout is not modelled.
    alignas(8) unsigned char mShadowQualitySettings[40];
    bool mOnlyFlaggedObjects;
    std::uint32_t mCastContext;
    // Returned by RndDefaults::GetLightingShadowOffset.
    float mShadowOffset;
    float mShadowSoftnessMin;
    float mShadowSoftnessMax;
    float mMaxShadowSoftnessDistance;
    // The "cookie" texture reference.
    void* mCookie;
    // "cookie_tiling": u, then v.
    float mCookieTiling[2];
    // Set when the light's GPU data must be rebuilt.
    bool mDirty;
};

static_assert(offsetof(RndLightSpotCom, mBulbRadius) == 232);
static_assert(offsetof(RndLightSpotCom, mFalloffStart) == 236);
static_assert(offsetof(RndLightSpotCom, mFalloffEnd) == 240);
static_assert(offsetof(RndLightSpotCom, mFalloffFunction) == 244);
static_assert(offsetof(RndLightSpotCom, mTruncation) == 260);
static_assert(offsetof(RndLightSpotCom, mCastsShadows) == 264);
static_assert(offsetof(RndLightSpotCom, mShadowQualitySettings) == 272);
static_assert(offsetof(RndLightSpotCom, mOnlyFlaggedObjects) == 312);
static_assert(offsetof(RndLightSpotCom, mCastContext) == 316);
static_assert(offsetof(RndLightSpotCom, mShadowOffset) == 320);
static_assert(offsetof(RndLightSpotCom, mCookie) == 336);
static_assert(offsetof(RndLightSpotCom, mCookieTiling) == 344);
static_assert(offsetof(RndLightSpotCom, mDirty) == 352);
