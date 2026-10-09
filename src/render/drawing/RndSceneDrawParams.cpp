#include "render/drawing/RndSceneDrawParams.h"

// Reconstructed from eboot.elf at 0x434A00. Everything is drawn into a
// cleared target from the scene's own camera over the whole viewport.
RndSceneDrawParams::RndSceneDrawParams()
    : mContext(nullptr),
      mDrawScene(true),
      mClearColor(Hmx::Color::GetZero()),
      mClear(true),
      mOutputToBackBuffer(true),
      mCameraEntity(nullptr),
      mCamera(nullptr),
      mProjectionRect(0.0F, 0.0F, 1.0F, 1.0F),
      mShowHide{0, 0x21},
      mDepthOnly(false),
      mDrawSceneMask(false),
      mDrawToTextures(false),
      mTargetMode(0),
      mShadingMode(-1),
      mWireframeOnly(false),
      mWireframeColor(Hmx::Color::GetWhite()),
      mDrawShadows(true),
      mDrawAtmosphere(true),
      mDrawPostProc(true),
      mSkipDraw(false) {}
