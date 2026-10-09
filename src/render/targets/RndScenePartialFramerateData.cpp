#include "render/targets/RndScenePartialFramerateData.h"

// Reconstructed from eboot.elf at 0x6D18A0. Nothing is stored yet: the
// frame, interval, drawer and counts are -1 so no frame follows on, the
// show/hide flags take their default and the shading mode defers to the
// buffer collection. The padding after mDrawerIndex is left alone.
RndScenePartialFramerateData::RndScenePartialFramerateData()
    : mFrame(~0UL),
      mFrameInterval(~0UL),
      mDrawerIndex(-1),
      mNumDeRegistered(~0UL),
      mLightRemovalCount(~0UL),
      mResultsTag(0),
      mShowHide{0, 0x21},
      mShadingMode(-1),
      mDrawAtmosphere(false),
      mDrawPostProc(false),
      mDrawSceneMask(false),
      mHasInstances(false),
      mNeedsLightCulling(false),
      mSceneTexUsage(-1),
      mSceneDepthUsage(-1),
      mSceneTexCaptureUsage(-1),
      mMaterialsNeedLinearDepth(false),
      mNeedsLinearDepth(false),
      mLightMgrUpdateFlag(false) {}
