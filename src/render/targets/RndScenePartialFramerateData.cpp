#include "render/targets/RndScenePartialFramerateData.h"

// Reconstructed from eboot.elf at 0x6D18A0. The cull results start empty,
// the show/hide flags take their default and the shading mode defers to the
// buffer collection; the padding at +20 is left alone.
RndScenePartialFramerateData::RndScenePartialFramerateData()
    : mLightCullResults{-1, -1, -1, -1, -1},
      mLightProbeCullResults{-1, -1, -1, -1},
      mPad40(0),
      mShowHideContext{0, 33},
      mShadingMode(-1),
      mDrawSky(false),
      mDrawPostProc(false),
      mDrawSceneMask(false),
      mPad59(false),
      mCullResultsStored(false),
      mShadowCullResults{-1, -1, -1},
      mCullResultCount(0),
      mCullResultsRestored(false) {}
