#pragma once

#include <cstddef>

// Per-frame state of a partial-framerate scene (render/
// RndScenePartialFramerateData.o). RndBufferCollection allocates one for each
// partial frame interval. Field names are not in the reference map.
//
// Only mShowHideContext through mDrawSceneMask are used in this build: the
// scene drawer's parameter finalization (0x41B060) saves those draw
// parameters on a scene's first partial frame and restores them on the
// following ones. The other fields are only initialized; their names follow
// the map's RndSceneDrawer::_StoreCullResults and _RestoreCullResults, which
// used this data in the older build, and are uncertain.
class RndScenePartialFramerateData {
public:
    RndScenePartialFramerateData();  // 0x6D18A0

    int mLightCullResults[5];       // -1 when empty. Never read.
    // Neither initialized nor read: alignment padding.
    unsigned int mPad20;
    int mLightProbeCullResults[4];  // -1 when empty. Never read.
    // Only cleared by the constructor's vector store; never read.
    unsigned int mPad40;
    // Copy of the draw parameters' RndShowHideContext: a value and the
    // show/hide flags (0x21 by default; the drawer ORs in 2 or 4 for the
    // main or aux camera).
    unsigned int mShowHideContext[2];
    // The draw parameters' shading mode; -1 takes the buffer collection's
    // mShadingMode.
    int mShadingMode;
    // The draw parameters' sky, post-processing and scene-mask switches.
    bool mDrawSky;
    bool mDrawPostProc;
    bool mDrawSceneMask;
    // Cleared with the switches by one dword store; never read.
    bool mPad59;
    bool mCullResultsStored;          // Never read.
    int mShadowCullResults[3];        // -1 when empty. Never read.
    unsigned short mCullResultCount;  // Never read.
    bool mCullResultsRestored;        // Never read.
};

static_assert(sizeof(RndScenePartialFramerateData) == 80);
static_assert(offsetof(RndScenePartialFramerateData, mLightProbeCullResults) == 24);
static_assert(offsetof(RndScenePartialFramerateData, mShowHideContext) == 44);
static_assert(offsetof(RndScenePartialFramerateData, mShadingMode) == 52);
static_assert(offsetof(RndScenePartialFramerateData, mDrawSky) == 56);
static_assert(offsetof(RndScenePartialFramerateData, mDrawSceneMask) == 58);
static_assert(offsetof(RndScenePartialFramerateData, mCullResultsStored) == 60);
static_assert(offsetof(RndScenePartialFramerateData, mShadowCullResults) == 64);
static_assert(offsetof(RndScenePartialFramerateData, mCullResultCount) == 76);
