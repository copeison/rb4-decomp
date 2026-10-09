#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "math/geometry/Rect.h"
#include "render/drawing/RndCuller.h"
#include "utl/containers/FixedVector.h"

class Entity;
class GameObject;
class RndBufferCollection;
class RndContext;
class RndTextureBase;

// What a scene draw is asked to do (render/RndSceneDrawParams.o; the
// constructor at 0x434A00 is the object's only function besides its static
// initializer at 0x434B60). The caller fills one and passes it to
// RndSceneDrawer::Draw, which copies it into its RndSceneInternalContext
// and finalizes it there (_ExtractSingletonsAndFinalizeParams). Field names
// are not in the reference map; the names of the mode switches follow the
// draw paths they select (RndSceneDrawer::_DrawFullFramerate, 0x41D040).
class RndSceneDrawParams {
public:
    RndSceneDrawParams();  // 0x434A00

    // The buffer collections to draw into: one, or a stereo pair whose
    // target modes are the left and right eye.
    FixedVector<RndBufferCollection*, 2> mBuffers;
    // The context to draw with, or null for the device's immediate context.
    RndContext* mContext;
    // Set by the constructor and cleared by the texture renderer
    // (0x6916D7); the drawer never reads it. The name is a guess.
    bool mDrawScene;
    // The clear color used when the scene component does not choose one
    // (_ExtractClearColor, 0x41D960), and whether to clear at all.
    Hmx::Color mClearColor;
    bool mClear;
    // Converts the scene into the collection's back buffer (the output
    // conversion) and draws the debug views there: the shading-mode
    // display and the buffer inspection (RndBufferInspection::Draw,
    // 0x6B5560).
    bool mOutputToBackBuffer;
    // The entity the camera belongs to; a camera of another entity is
    // dropped.
    Entity* mCameraEntity;
    // The camera to draw from; when null the scene's first or second
    // camera, then the device's default camera, is used.
    GameObject* mCamera;
    Hmx::Rect mProjectionRect;
    RndShowHideContext mShowHide;
    // Draws only the shadow-casting depth (0x41E090).
    bool mDepthOnly;
    // Kept across partial frames with the shading mode
    // (RndScenePartialFramerateData::mDrawSceneMask). The name is weak.
    bool mDrawSceneMask;
    // Draws the impostor maps (shading mode 15) into the four textures of
    // mTargetTextures (0x41E420), as the texture renderer does.
    bool mDrawToTextures;
    FixedVector<RndTextureBase*, 4> mTargetTextures;
    // The texture renderer sets 2; the impostor path does not read it. The
    // name is weak.
    int mTargetMode;
    // The shading mode, or -1 for the first buffer collection's.
    int mShadingMode;
    // Draws only the wireframe overlay (0x41ECC0).
    bool mWireframeOnly;
    Hmx::Color mWireframeColor;
    // Cleared for shading modes and device settings that disable them
    // (RndConfig::mShadowsEnabled, mVolumetricScatteringEnabled and
    // mPostProcEnabled).
    bool mDrawShadows;
    bool mDrawAtmosphere;
    bool mDrawPostProc;
    // Skips the draw.
    bool mSkipDraw;
};

static_assert(offsetof(RndSceneDrawParams, mContext) == 40);
static_assert(offsetof(RndSceneDrawParams, mDrawScene) == 48);
static_assert(offsetof(RndSceneDrawParams, mClearColor) == 52);
static_assert(offsetof(RndSceneDrawParams, mClear) == 68);
static_assert(offsetof(RndSceneDrawParams, mOutputToBackBuffer) == 69);
static_assert(offsetof(RndSceneDrawParams, mCameraEntity) == 72);
static_assert(offsetof(RndSceneDrawParams, mCamera) == 80);
static_assert(offsetof(RndSceneDrawParams, mProjectionRect) == 88);
static_assert(offsetof(RndSceneDrawParams, mShowHide) == 104);
static_assert(offsetof(RndSceneDrawParams, mDepthOnly) == 112);
static_assert(offsetof(RndSceneDrawParams, mDrawSceneMask) == 113);
static_assert(offsetof(RndSceneDrawParams, mDrawToTextures) == 114);
static_assert(offsetof(RndSceneDrawParams, mTargetTextures) == 120);
static_assert(offsetof(RndSceneDrawParams, mTargetMode) == 176);
static_assert(offsetof(RndSceneDrawParams, mShadingMode) == 180);
static_assert(offsetof(RndSceneDrawParams, mWireframeOnly) == 184);
static_assert(offsetof(RndSceneDrawParams, mWireframeColor) == 188);
static_assert(offsetof(RndSceneDrawParams, mDrawShadows) == 204);
static_assert(offsetof(RndSceneDrawParams, mDrawAtmosphere) == 205);
static_assert(offsetof(RndSceneDrawParams, mDrawPostProc) == 206);
static_assert(offsetof(RndSceneDrawParams, mSkipDraw) == 207);
static_assert(sizeof(RndSceneDrawParams) == 208);

// What a scene draw produced for one of its buffer collections; Draw
// returns one per collection, and a caller may pass the previous results
// back in. The tiled-lighting passes receive it (0x485270) and copy its
// first 41 bytes. Name and field names are not in the reference map.
struct RndSceneDrawTarget {
    // Inlined into Draw (0x41AA10) and _DrawStereo (0x41BD40).
    RndSceneDrawTarget()
        : mBuffersIndex(0),
          mSrcLightAccum(0),
          mDstLightAccum(1),
          mResult(nullptr),
          mDepthTargetIndex(0),
          mScatteringResolution(-1),
          mResolved(false) {}

    // The index of the collection in RndSceneDrawParams::mBuffers.
    unsigned long mBuffersIndex;
    // A ping-pong pair of RndBufferCollection::mLightAccum indices for a
    // partial-framerate scene: the passes read the scene from the source
    // (0x41C760) and write the destination (the tiled lighting,
    // RndCShaderTiledLightsInterpolation::Dispatch); each post-processing
    // stage swaps them and clears mResult. Zero and one by default.
    unsigned long mSrcLightAccum;
    unsigned long mDstLightAccum;
    // The texture holding the drawn scene, when it is not the collection's
    // own (the partial-framerate copy); null otherwise.
    RndTextureBase* mResult;
    // Selects the frame interval's depth target the transparent passes
    // test (FrameIntervalBuffers::mDepthStencil and the two after it); Draw
    // resets it to 0 for each frame. The name is weak.
    int mDepthTargetIndex;
    // The volumetric scattering's resolution this frame
    // (RndVolumetricScatteringCom::EndAsyncUpdate); Draw resets it to -1.
    int mScatteringResolution;
    // Cleared by the constructor; the drawer never reads it. The name is
    // weak.
    bool mResolved;
};

static_assert(offsetof(RndSceneDrawTarget, mDstLightAccum) == 16);
static_assert(offsetof(RndSceneDrawTarget, mResult) == 24);
static_assert(offsetof(RndSceneDrawTarget, mDepthTargetIndex) == 32);
static_assert(offsetof(RndSceneDrawTarget, mResolved) == 40);
static_assert(sizeof(RndSceneDrawTarget) == 48);
