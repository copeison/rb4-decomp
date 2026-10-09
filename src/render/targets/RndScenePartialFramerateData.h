#pragma once

#include <cstddef>

#include "render/drawing/RndCuller.h"

// What a partial-framerate scene's first frame of an interval leaves for
// the following frames (render/RndScenePartialFramerateData.o).
// RndBufferCollection allocates one for each frame interval. The scene
// drawer fills and reads it: the parameter finalization (RndSceneDrawer::
// _ExtractSingletonsAndFinalizeParams, 0x41B060) saves the draw parameters
// below on the interval's first frame and restores them on the others,
// _StoreCullResults (0x41FFC0) and _RestoreCullResults (0x420240) keep the
// camera's cull analysis here, and _DrawPartialFramerate (0x41C760) records
// which frame and drawer the stored results belong to. Field names are not
// in the reference map.
class RndScenePartialFramerateData {
public:
    RndScenePartialFramerateData();  // 0x6D18A0

    // The device frame (RndDevice::mFrameCount) and the scene's frame
    // interval the results were drawn in, and the drawer's index; the next
    // frame reuses them only when they follow on.
    unsigned long mFrame;
    unsigned long mFrameInterval;
    int mDrawerIndex;
    // The drawer's deregistration count and the light manager's light
    // removal count when the results were stored; a change invalidates
    // them.
    unsigned long mNumDeRegistered;
    unsigned long mLightRemovalCount;
    // Set to the FNV-1 offset basis (0x811C9DC5) when results are stored;
    // a reuse of other results is reported (the report is compiled out).
    // The name is weak.
    unsigned int mResultsTag;
    // Copies of RndSceneDrawParams::mShowHide, mShadingMode (-1 takes the
    // buffer collection's), mDrawAtmosphere, mDrawPostProc and
    // mDrawSceneMask.
    RndShowHideContext mShowHide;
    int mShadingMode;
    bool mDrawAtmosphere;
    bool mDrawPostProc;
    bool mDrawSceneMask;
    // The camera's cull analysis (RndSceneInternalContext::CameraData).
    bool mHasInstances;
    bool mNeedsLightCulling;
    int mSceneTexUsage;
    int mSceneDepthUsage;
    int mSceneTexCaptureUsage;
    bool mMaterialsNeedLinearDepth;
    bool mNeedsLinearDepth;
    // The light manager's second update flag (+0xC9), saved and restored
    // with the culled lights (RndLightMgrCom::StoreCullResults, 0x484D80).
    bool mLightMgrUpdateFlag;
};

static_assert(offsetof(RndScenePartialFramerateData, mFrameInterval) == 8);
static_assert(offsetof(RndScenePartialFramerateData, mDrawerIndex) == 16);
static_assert(offsetof(RndScenePartialFramerateData, mNumDeRegistered) == 24);
static_assert(offsetof(RndScenePartialFramerateData, mLightRemovalCount) == 32);
static_assert(offsetof(RndScenePartialFramerateData, mResultsTag) == 40);
static_assert(offsetof(RndScenePartialFramerateData, mShowHide) == 44);
static_assert(offsetof(RndScenePartialFramerateData, mShadingMode) == 52);
static_assert(offsetof(RndScenePartialFramerateData, mDrawAtmosphere) == 56);
static_assert(offsetof(RndScenePartialFramerateData, mDrawSceneMask) == 58);
static_assert(offsetof(RndScenePartialFramerateData, mHasInstances) == 59);
static_assert(offsetof(RndScenePartialFramerateData, mNeedsLightCulling) == 60);
static_assert(offsetof(RndScenePartialFramerateData, mSceneTexUsage) == 64);
static_assert(offsetof(RndScenePartialFramerateData, mSceneTexCaptureUsage) == 72);
static_assert(offsetof(RndScenePartialFramerateData, mMaterialsNeedLinearDepth) == 76);
static_assert(offsetof(RndScenePartialFramerateData, mLightMgrUpdateFlag) == 78);
static_assert(sizeof(RndScenePartialFramerateData) == 80);
