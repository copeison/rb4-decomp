#pragma once

#include <cstddef>

#include "render/shaders/RndShaderIncludeChecksums.h"
#include "render/shaders/RndShaderDefines.h"

class RndShaderError;
class RndShaderBasic;
class RndShaderBinkConvert;
class RndShaderBloom;
class RndShaderBlur;
class RndShaderFXAA;
class RndShaderDOFSprite;
class RndShaderDisplayShadingMode;
class RndShaderDisplaySphereMap;
class RndShaderDisplayTextureCube;
class RndShaderDownsample;
class RndShaderLinearizeDepth;
class RndShaderOutputConversion;
class RndShaderRefineSceneMask;
class RndShaderStencilSceneMask;
class RndShaderTestPattern;
class RndCShaderBlurClassify;
class RndCShaderCalcDepthRange;
class RndCShaderClearBuffer;
class RndCShaderCopyBuffer;
class RndCShaderDOFDiscBlur;
class RndCShaderVScatCalcDensityInscattering;
class RndCShaderVScatAccumScattering;
class RndCShaderVScatDeferred;
class RndCShaderSSAOGen;
class RndCShaderCMAAEdgeDetect;
class RndCShaderCMAAEdgePrune;
class RndCShaderCMAAShapeFit;
class RndCShaderCMAAFinalProcess;
class RndCShaderLinearizeDepth;
class RndCShaderSignedDistance;
class RndCShaderSignedDistanceClassify;
class RndShaderRenderTestSimple;
class RndCShaderRenderTestCompute;
class RndShaderCBufferConfig;
class RndTextureArray1D;
struct RndShaderLink;

// Owns the shader configuration every shader shares: the global defines,
// the fixed defines, the engine constant buffers, the function-table
// texture, and the built-in shaders. Shaders register in its list and are
// initialized by Init. Embedded in RndDevice.
class RndShaderMgr {
public:
    RndShaderMgr();   // 0x63F180
    ~RndShaderMgr();  // 0x63F350

    // Creates the configuration and the built-in shaders.
    void PreInit();               // 0x63F400
    // Initializes every registered shader and builds the function table.
    void Init();                  // 0x641370
    void Terminate();             // 0x641740
    void ReloadAll();             // 0x641F30
    void _InitFixedDefines();     // 0x63F920
    void _InitDefinesGroups();    // 0x640BF0
    void _CreateCBufferConfigs(); // 0x640D60

    // Field names are not in the reference map.
    // HX_BT709_TO_BT2020, HX_NUM_RT_SLICES, HX_SHADING_MODE, HX_GEO_TYPE.
    RndShaderDefInfo mGlobalDefInfos[4];
    bool mPreInitialized;
    bool mInitialized;
    RndShaderCBufferConfig* mSceneCBuffer;
    unsigned long mTime;
    unsigned long mSmoothnessDecay;
    unsigned long mSGraphTransInfos;
    unsigned long mSceneGlobalFloats;
    unsigned long mSceneGlobalColors;
    unsigned long mTiledLightingParams;
    unsigned long mFogParams;
    unsigned long mVolumetricParams0;
    unsigned long mVolumetricParams1;
    RndShaderCBufferConfig* mRenderTargetCBuffer;
    unsigned long mTargetDimensions;
    RndShaderCBufferConfig* mCameraCBuffer;
    unsigned long mCameraNearFarParams;
    unsigned long mCameraMiscParams;
    unsigned long mCameraViewExtents;
    // Two more camera constant offsets that the constructor sets to -1 and
    // _CreateCBufferConfigs never adds, so they stay -1; nothing reads them.
    // Named after the camera transforms the shaders now fetch from
    // gCameraRTSlicedData (GetCameraXfm, GetCameraXfmInverse); the names
    // are uncertain.
    unsigned long mCameraXfm;
    unsigned long mCameraXfmInverse;
    unsigned long mCameraRTSlicedData;
    RndShaderCBufferConfig* mClipPlanesCBuffer;
    unsigned long mClipPlanes;
    RndShaderCBufferConfig* mSkeletonCBuffer;
    unsigned long mSkeletonBoneXfms;
    RndShaderCBufferConfig* mMiscDrawStateCBuffer;
    unsigned long mEnvironIndex;
    unsigned long mSolidColor;
    RndShaderCBufferConfig* mOcclusionQueryCBuffer;
    unsigned long mOcclusionQueryCoverageParams;
    RndShaderCBufferConfig* mDebugCBuffer;
    unsigned long mDebugModes;
    unsigned long mDebugColor;
    unsigned long mBatchInfo;
    unsigned long mPreviewNodeIndex;
    RndShaderCBufferConfig* mTransientCBuffers[3];
    RndShaderFixedDefines* mFixedDefines;
    RndShaderDefinesGroup* mDefinesGroup;
    RndTextureArray1D* mFunctionTable;
    RndShaderError* mErrorShader;
    RndShaderBasic* mBasicShader;
    RndShaderBinkConvert* mBinkConvertShader;
    RndShaderBloom* mBloomShader;
    RndShaderBlur* mBlurShader;
    RndShaderFXAA* mFXAAShader;
    RndShaderDOFSprite* mDOFSpriteShader;
    RndShaderDisplayShadingMode* mDisplayShadingModeShader;
    RndShaderDisplaySphereMap* mDisplaySphereMapShader;
    RndShaderDisplayTextureCube* mDisplayTextureCubeShader;
    RndShaderDownsample* mDownsampleShader;
    RndShaderLinearizeDepth* mLinearizeDepthShader;
    RndShaderOutputConversion* mOutputConversionShader;
    RndShaderRefineSceneMask* mRefineSceneMaskShader;
    RndShaderStencilSceneMask* mStencilSceneMaskShader;
    RndShaderTestPattern* mTestPatternShader;
    // A shader slot that PreInit never fills in this build; Terminate still
    // deletes it through its virtual destructor (0x641740). Its type is
    // unknown.
    void* mReservedShader;
    RndCShaderBlurClassify* mBlurClassifyCShader;
    RndCShaderCalcDepthRange* mCalcDepthRangeCShader;
    RndCShaderClearBuffer* mClearBufferCShader;
    RndCShaderCopyBuffer* mCopyBufferCShader;
    RndCShaderDOFDiscBlur* mDOFDiscBlurCShader;
    RndCShaderVScatCalcDensityInscattering* mVScatCalcDensityInscatteringCShader;
    RndCShaderVScatAccumScattering* mVScatAccumScatteringCShader;
    RndCShaderVScatDeferred* mVScatDeferredCShader;
    RndCShaderSSAOGen* mSSAOGenCShader;
    RndCShaderCMAAEdgeDetect* mCMAAEdgeDetectCShader;
    RndCShaderCMAAEdgePrune* mCMAAEdgePruneCShader;
    RndCShaderCMAAShapeFit* mCMAAShapeFitCShader;
    RndCShaderCMAAFinalProcess* mCMAAFinalProcessCShader;
    RndCShaderLinearizeDepth* mLinearizeDepthCShader;
    RndCShaderSignedDistance* mSignedDistanceCShader;
    RndCShaderSignedDistanceClassify* mSignedDistanceClassifyCShader;
    RndShaderRenderTestSimple* mRenderTestSimpleShader;
    RndCShaderRenderTestCompute* mRenderTestComputeCShader;

    unsigned long mFixedDefinesChecksum;  // Low 32 bits: FNV-1a of the fixed defines.
    RndShaderIncludeChecksums* mIncludeChecksums;
    RndShaderLink* mShaders;
    RndShaderLink* mShaderGraphs;
};

static_assert(offsetof(RndShaderMgr, mPreInitialized) == 80);
static_assert(offsetof(RndShaderMgr, mSceneCBuffer) == 88);
static_assert(offsetof(RndShaderMgr, mRenderTargetCBuffer) == 168);
static_assert(offsetof(RndShaderMgr, mClipPlanesCBuffer) == 240);
static_assert(offsetof(RndShaderMgr, mMiscDrawStateCBuffer) == 272);
static_assert(offsetof(RndShaderMgr, mOcclusionQueryCBuffer) == 296);
static_assert(offsetof(RndShaderMgr, mTransientCBuffers) == 352);
static_assert(offsetof(RndShaderMgr, mDefinesGroup) == 384);
static_assert(offsetof(RndShaderMgr, mErrorShader) == 400);
static_assert(offsetof(RndShaderMgr, mFixedDefinesChecksum) == 680);
static_assert(offsetof(RndShaderMgr, mShaders) == 696);
static_assert(sizeof(RndShaderMgr) == 712);
