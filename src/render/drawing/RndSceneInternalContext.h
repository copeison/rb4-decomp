#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "render/context/RndCameraContext.h"
#include "render/drawing/PodVector.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/VectorAdapter.h"

class Entity;
class RndCMAACom;
class RndFogCom;
class RndLightMgrCom;
class RndPostProcCom;
class RndSceneCom;
class RndSkyCom;
class RndVolumetricScatteringCom;

// The scene drawer's working state for one draw (28384 bytes): the
// finalized draw parameters, a camera per buffer collection with the
// buckets it culls into, the camera the shared work culls with, and the
// scene's singleton components. The map names the type; it has no object
// of its own. Draw builds one on its stack; the multithreaded draw keeps
// one in the drawer (RndSceneDrawer::mThreadedContext). The constructor is
// implicit (inlined at 0x419594, 0x41AA60 and 0x41B0A0); the copy
// assignment at 0x41D710 is the implicit one. Field names are not in the
// reference map.
struct RndSceneInternalContext {
    // One buffer collection's camera and cull results (9368 bytes). Name
    // not in the reference map.
    // Its constructor is inlined where the cameras are added (0x41B0F0).
    struct CameraData {
        RndCameraContext mCamera;
        // The cull analysis (_AnalyzeCullResults, 0x420400), kept across
        // partial frames by RndScenePartialFramerateData.
        // Some bucket has instances.
        bool mHasInstances = false;
        // Never written by the drawer.
        bool mReserved9313 = false;
        // Some lit bucket has instances: the lights must be culled.
        bool mNeedsLightCulling = false;
        // The largest UsesSceneTex and NeedsSceneTexCapture and the
        // smallest UsesSceneDepth of the materials drawn, -1 for none.
        int mSceneTexUsage = -1;
        int mSceneDepthUsage = -1;
        int mSceneTexCaptureUsage = -1;
        // Some material of the lit buckets uses the linear depth.
        bool mMaterialsNeedLinearDepth = false;
        // The linear depth must be generated ("Analyze Depth"): for the
        // light culling, those materials, buckets 10-12, fog or volumetric
        // scattering.
        bool mNeedsLinearDepth = false;
        // The drawer's bucket sets this camera culls into and sorts.
        VectorAdapter<PodVector<RndDrawInstance>> mDrawInstances{nullptr, 0};
        VectorAdapter<PodVector<RndDrawInstance*>> mSortableInstances{nullptr, 0};
    };

    Entity* mEntity = nullptr;
    RndSceneDrawParams mParams;
    // The scene draws at a partial framerate this frame.
    bool mPartialFramerate = false;
    // RndConfig::mAsyncCopyEnabled.
    bool mAsyncCopy = false;
    // mParams.mBuffers is a left-eye and right-eye pair.
    bool mStereoPair = false;
    // The pair is drawn with the stereo optimizations: shared culling and
    // lighting (RndConfig::mStereoOptimizationsEnabled).
    bool mStereoOptimized = false;
    FixedVector<CameraData, 2> mCameras;
    // The camera the shared work culls with: the first collection's, or
    // the combined stereo camera.
    RndCameraContext mCullCamera;
    // Whether the targets are cleared, and to which color
    // (_ExtractClearColor). The flag has no initializer.
    bool mHasClearColor;
    Hmx::Color mClearColor = Hmx::Color(0.0F, 0.0F, 0.0F, 1.0F);
    // The singleton components of the scene. mLightMgr falls back to the
    // device's default light manager.
    RndSceneCom* mSceneCom = nullptr;
    RndLightMgrCom* mLightMgr = nullptr;
    // The scene's sky, and its atmosphere when it is a fog component or a
    // volumetric scattering component the quality level and the device
    // support; only with mParams.mDrawAtmosphere.
    RndSkyCom* mSky = nullptr;
    RndFogCom* mFog = nullptr;
    RndVolumetricScatteringCom* mVolumetricScattering = nullptr;
    // The scene's post-processing components, for the first and second
    // camera.
    RndPostProcCom* mPostProc[2] = {nullptr, nullptr};
    // The scene's antialiasing component when the quality level supports
    // it.
    RndCMAACom* mAntialiasing = nullptr;
};

static_assert(offsetof(RndSceneInternalContext::CameraData, mHasInstances) == 9312);
static_assert(offsetof(RndSceneInternalContext::CameraData, mNeedsLightCulling) == 9314);
static_assert(offsetof(RndSceneInternalContext::CameraData, mSceneTexUsage) == 9316);
static_assert(offsetof(RndSceneInternalContext::CameraData, mMaterialsNeedLinearDepth) == 9328);
static_assert(offsetof(RndSceneInternalContext::CameraData, mNeedsLinearDepth) == 9329);
static_assert(offsetof(RndSceneInternalContext::CameraData, mDrawInstances) == 9336);
static_assert(offsetof(RndSceneInternalContext::CameraData, mSortableInstances) == 9352);
static_assert(sizeof(RndSceneInternalContext::CameraData) == 9368);
static_assert(offsetof(RndSceneInternalContext, mParams) == 8);
static_assert(offsetof(RndSceneInternalContext, mPartialFramerate) == 216);
static_assert(offsetof(RndSceneInternalContext, mStereoOptimized) == 219);
static_assert(offsetof(RndSceneInternalContext, mCameras) == 224);
static_assert(offsetof(RndSceneInternalContext, mCullCamera) == 18984);
static_assert(offsetof(RndSceneInternalContext, mHasClearColor) == 28296);
static_assert(offsetof(RndSceneInternalContext, mClearColor) == 28300);
static_assert(offsetof(RndSceneInternalContext, mSceneCom) == 28320);
static_assert(offsetof(RndSceneInternalContext, mLightMgr) == 28328);
static_assert(offsetof(RndSceneInternalContext, mSky) == 28336);
static_assert(offsetof(RndSceneInternalContext, mPostProc) == 28360);
static_assert(offsetof(RndSceneInternalContext, mAntialiasing) == 28376);
static_assert(sizeof(RndSceneInternalContext) == 28384);
