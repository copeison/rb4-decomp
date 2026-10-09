#include "render/drawing/RndSceneDrawer.h"

#include <cmath>
#include <cstring>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "render/atmosphere/RndAtmosphereCom.h"
#include "render/atmosphere/RndFogCom.h"
#include "render/atmosphere/RndSkyCom.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraCom.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/debug/RndBufferInspection.h"
#include "render/debug/RndShaderDisplayShadingMode.h"
#include "render/drawing/RndBasicCuller.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/volumetric/RndVolumetricScatteringCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/meshes/RndDynamicGpuData.h"
#include "render/postprocessing/antialiasing/RndCMAACom.h"
#include "render/postprocessing/chain/RndPostProcCom.h"
#include "render/postprocessing/output/RndShaderOutputConversion.h"
#include "render/queries/RndOcclusionQueryMgr.h"
#include "render/scene/RndEntityCom.h"
#include "render/shaders/nodes/RndShaderNodeSceneGlobalColor.h"
#include "render/shaders/nodes/RndShaderNodeSceneGlobalFloat.h"
#include "render/shaders/nodes/RndShaderNodeTime.h"
#include "render/shaders/nodes/RndShaderNodeTransInfo.h"
#include "render/scene/RndSceneCom.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/system/RndFactory.h"
#include "render/system/RndFence.h"
#include "render/system/RndUtl.h"
#include "render/textures/RndTextureBase.h"
#include "render/targets/RndBufferCollection.h"
#include "render/targets/RndScenePartialFramerateData.h"
#include "utl/containers/Sort.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"

std::atomic<int> RndSceneDrawer::sNumDrawers;

namespace {

// The immediate context, once the device's pending frame has begun. Name
// not in the reference map.
RndContext* ImmediateContext() {
    RndDevice* device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    return device->mImmediateContext;
}

// The drawer of the scene or RndEntity component on the entity's root
// object, or of the entities that instance it. Both FindForEntity copies
// inline it. Name not in the reference map.
RndSceneDrawer* FindDrawer(const Entity* entity) {
    for (; entity != nullptr; entity = entity->mParentObject != nullptr
             ? entity->mParentObject->mEntity
             : nullptr) {
        const GameObject* root = entity->GetRoot();
        if (RndSceneCom* scene = root->GetCom<RndSceneCom>()) {
            return scene->mSceneDrawer;
        }
        if (RndEntityCom* com = root->GetCom<RndEntityCom>()) {
            if (com->mRuntimeData.mSceneDrawer != nullptr) {
                return com->mRuntimeData.mSceneDrawer;
            }
        }
    }
    return nullptr;
}

// A bucket set's elements, which the drawer refills through the culler's
// const view. Name not in the reference map.
template <typename T>
PodVector<T>* Buckets(const VectorAdapter<PodVector<T>>& buckets) {
    return const_cast<PodVector<T>*>(buckets.mData);
}

// A drawer bucket set as the view the culler and the camera data take.
template <typename T>
VectorAdapter<T> Adapter(const FixedVector<T, kNumDrawBuckets>& buckets) {
    return VectorAdapter<T>{buckets.mData, buckets.mSize};
}

// Resource states the drawer's barriers use that RndResourceState does not
// name: any shader read, and a copy source. Names not in the reference map.
constexpr RndResourceState kStateShaderResource = static_cast<RndResourceState>(0xC0);
constexpr RndResourceState kStateCopySource = static_cast<RndResourceState>(0x800);

// What a partial-framerate scene's data holds once its results are stored:
// the FNV-1 offset basis. Name not in the reference map.
constexpr unsigned int kStoredResultsTag = 0x811C9DC5;

// Binds the texture alone as the target and clears it to the scene's clear
// color. Inlined into _DrawPartialFramerate. Name not in the reference map.
void ClearTarget(RndContext& context, const RndSceneInternalContext& internal, RndTextureBase& texture) {
    RndContext::RenderTargetParams targets;
    targets.mClearColor = internal.mClearColor;
    RndContext::RenderTargetParams::Target target;
    target.mTexture = &texture;
    target.mClearMode = 1;
    targets.mTargets.push_back(target);
    context.SetRenderTargets(targets);
}

}  // namespace

// Reconstructed from eboot.elf at 0x4194D0, inlined into the drawer's
// destructor and ResetRegistrations. The binary destroys the light lists
// first.
RndSceneCullResults::~RndSceneCullResults() {
    for (PodVector<RndDrawInstance>& bucket : mDrawInstances) {
        bucket.Free();
    }
}

LinkedList::Node& RndSceneDrawer::NewlyRegisteredNode::ToNode(RndDrawInstanceCom& com) {
    return com.mRuntime.mSceneDrawerLink;
}

RndDrawInstanceCom* RndSceneDrawer::NewlyRegisteredNode::FromNode(LinkedList::Node* node) {
    return reinterpret_cast<RndDrawInstanceCom*>(
        reinterpret_cast<char*>(node) - offsetof(RndDrawInstanceCom, mRuntime)
        - offsetof(RndDrawInstanceCom::RuntimeData, mSceneDrawerLink));
}

// Reconstructed from eboot.elf at 0x419170.
RndSceneDrawer* RndSceneDrawer::FindForEntity(const Entity* entity) {
    return FindDrawer(entity);
}

// Reconstructed from eboot.elf at 0x419260.
RndSceneDrawer* RndSceneDrawer::FindForEntity(Entity* entity) {
    return FindDrawer(entity);
}

// Reconstructed from eboot.elf at 0x419350. The drawer polls on the main
// thread. Every bucket set gets its 22 empty buckets, and the job graph,
// the threaded draw's context and twelve fences are created up front.
RndSceneDrawer::RndSceneDrawer()
    : mIndex(-1),
      mDrawing(false),
      mNumDeRegistered(0),
      mCuller(nullptr),
      mSceneCBuffer(nullptr),
      mDynamicGpuDataMgr(nullptr),
      mOcclusionQueryMgr(nullptr),
      mNeedsReserve(false),
      mStoredCullResults(nullptr),
      mDrawJobs(nullptr),
      mStereoDrawJobs(nullptr),
      mThreadedContext(nullptr),
      mFences{} {
    SetThreaded(false);
    mIndex = ++sNumDrawers;
    mCuller = new RndBasicCuller();
    mSceneCBuffer = RndShaderCBuffer::New(*gRndDevice->mShaderMgr.mSceneCBuffer, 0);
    mDynamicGpuDataMgr = new RndDynamicGpuDataMgr();
    mOcclusionQueryMgr = new RndOcclusionQueryMgr();
    mPendingGrowCounts[0] = 0;
    mPendingGrowCounts[1] = 0;
    mPendingGrowCounts[2] = 0;
    mThreadedContext = new RndSceneInternalContext();
    for (auto& buckets : mDrawInstances) {
        buckets.resize(kNumDrawBuckets);
    }
    for (auto& buckets : mSortableInstances) {
        buckets.resize(kNumDrawBuckets);
    }
    mDrawJobs = new RndSceneDrawJobs(this, 1);
    for (RndFence*& fence : mFences) {
        fence = gRndDevice->mFactory->CreateFence();
    }
}

// Reconstructed from eboot.elf at 0x419AC0 (the deleting destructor at
// 0x419F80). The bucket buffers are freed after the owned objects, last
// set first, as the binary's member destructors do.
RndSceneDrawer::~RndSceneDrawer() {
    for (RndFence*& fence : mFences) {
        delete fence;
        fence = nullptr;
    }
    delete mCuller;
    mCuller = nullptr;
    RndShaderCBuffer::SafeDelete(mSceneCBuffer);
    delete mDynamicGpuDataMgr;
    mDynamicGpuDataMgr = nullptr;
    delete mOcclusionQueryMgr;
    mOcclusionQueryMgr = nullptr;
    delete mStoredCullResults;
    mStoredCullResults = nullptr;
    delete mThreadedContext;
    mThreadedContext = nullptr;
    delete mDrawJobs;
    mDrawJobs = nullptr;
    delete mStereoDrawJobs;
    mStereoDrawJobs = nullptr;
    for (int set = 2; set >= 0; --set) {
        for (PodVector<RndDrawInstance*>& bucket : mSortableInstances[set]) {
            bucket.Free();
        }
    }
    for (int set = 2; set >= 0; --set) {
        for (PodVector<RndDrawInstance>& bucket : mDrawInstances[set]) {
            bucket.Free();
        }
    }
}

// Reconstructed from eboot.elf at 0x419FA0. The map's EnterPrelude only
// calls _PrepareToRegister, which this build inlines.
void RndSceneDrawer::EnterPrelude(Entity* entity) {
    static_cast<void>(entity);
    _PrepareToRegister();
}

// Reconstructed from eboot.elf at 0x41A040. The queued components are
// dropped: the culler is cleared and they register again as the scene
// enters.
void RndSceneDrawer::_PrepareToRegister() {
    ScopedCritSec lock(mCritSec);
    while (!mNewlyRegistered.empty()) {
        mNewlyRegistered.remove(mNewlyRegistered.front());
    }
    mPendingGrowCounts[0] = 0;
    mPendingGrowCounts[1] = 0;
    mPendingGrowCounts[2] = 0;
    mCuller->Clear();
}

// Reconstructed from eboot.elf at 0x41A0E0.
void RndSceneDrawer::EnterCoda(Entity* entity) {
    static_cast<void>(entity);
    _ProcessNewlyRegistered();
    mOcclusionQueryMgr->PostEnter();
}

// Reconstructed from eboot.elf at 0x41A1C0. The culler makes room for all
// the queued instances at once, then takes each component; the buckets are
// reserved for the culler's new sizes.
void RndSceneDrawer::_ProcessNewlyRegistered() {
    ScopedCritSec lock(mCritSec);
    if (!mNewlyRegistered.empty()) {
        mCuller->PrepareToGrowBy(mPendingGrowCounts);
        while (!mNewlyRegistered.empty()) {
            RndDrawInstanceCom& com = mNewlyRegistered.front();
            mNewlyRegistered.remove(com);
            mCuller->RegisterDrawInstancesForCom(com);
        }
        mPendingGrowCounts[0] = 0;
        mPendingGrowCounts[1] = 0;
        mPendingGrowCounts[2] = 0;
        _ReserveDrawInstances();
    }
}

// Reconstructed from eboot.elf at 0x41A290.
void RndSceneDrawer::ExitPrelude(Entity* entity) {
    static_cast<void>(entity);
    _PrepareToRegister();
}

// Reconstructed from eboot.elf at 0x41A330.
void RndSceneDrawer::ResetRegistrations() {
    delete mStoredCullResults;
    mStoredCullResults = nullptr;
    for (auto& buckets : mDrawInstances) {
        for (PodVector<RndDrawInstance>& bucket : buckets) {
            bucket.Free();
        }
    }
    for (auto& buckets : mSortableInstances) {
        for (PodVector<RndDrawInstance*>& bucket : buckets) {
            bucket.Free();
        }
    }
    if (mCuller != nullptr) {
        mCuller->TrimPools();
    }
    mNeedsReserve = true;
}

// Reconstructed from eboot.elf at 0x41A5B0. The binary also creates the
// thread's poll state (PollMgr's thread-local context) without using it.
void RndSceneDrawer::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    _DoPoll();
    ThreadPollContext& context = gEntityThreadState;
    static_cast<void>(context);
    _AddPostPoll();
}

// Reconstructed from eboot.elf at 0x41A650.
void RndSceneDrawer::_DoPoll() {
    mSceneCamera.Clear();
    mCuller->Poll();
}

// Reconstructed from eboot.elf at 0x41A680.
void RndSceneDrawer::PostPoll() {
    _DoPostPoll();
}

// Reconstructed from eboot.elf at 0x41A690.
void RndSceneDrawer::_DoPostPoll() {
    _ProcessNewlyRegistered();
    mOcclusionQueryMgr->Poll();
}

// Reconstructed from eboot.elf at 0x41A770.
const char* RndSceneDrawer::GetPollName() const {
    FormatString format("Scene Drawer %d");
    format << mIndex;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x41A7E0. A valid camera context makes
// the binary build the camera's error name, which it does not use.
void RndSceneDrawer::SetSceneCamera(RndCameraCom* camera, const Hmx::Rect& projectionRect) {
    const auto* cameraObject = reinterpret_cast<const GameObject*>(camera);
    if (mSceneCamera.mValid) {
        static_cast<void>(mSceneCamera.GetCameraSettings().mOwner->mEntity->MakeErrorName());
    }
    mSceneCamera.SetCamera(cameraObject);
    mSceneCamera.SetRenderTargetInfo(kTargetMode2D, Vector2{16.0F, 9.0F}, Vector2{0.0F, 1.0F});
    mSceneCamera.SetProjectionRect(projectionRect);
}

// Reconstructed from eboot.elf at 0x41A890.
void RndSceneDrawer::RegisterDrawInstanceCom(RndDrawInstanceCom& com) {
    ScopedCritSec lock(mCritSec);
    mNewlyRegistered.push_back(com);
    const unsigned long numLods = 2UL * gRndDevice->mSettings->mUseLod + 1;
    for (unsigned long lod = 0; lod != numLods; ++lod) {
        mPendingGrowCounts[lod] += com._GetNumDrawInstancesImpl(static_cast<RndSceneLod>(lod));
    }
}

// Reconstructed from eboot.elf at 0x41A950.
void RndSceneDrawer::DeRegisterDrawInstanceCom(RndDrawInstanceCom& com) {
    ScopedCritSec lock(mCritSec);
    ++mNumDeRegistered;
    for (RndDrawInstanceCom& queued : mNewlyRegistered) {
        if (&queued == &com) {
            mNewlyRegistered.remove(com);
            return;
        }
    }
    mCuller->DeRegisterDrawInstancesForCom(com);
}

// Reconstructed from eboot.elf at 0x41AA10. Each buffer collection is drawn
// on its own after a fresh frame and the scene constants are selected; a
// stereo pair with the stereo optimizations is drawn together. Previous
// targets are reused with their frame state reset.
FixedVector<RndSceneDrawTarget, 2> RndSceneDrawer::Draw(
    Entity* entity,
    RndSceneDrawParams& params,
    const FixedVector<RndSceneDrawTarget, 2>* previous) {
    mDrawing = true;
    if (mNeedsReserve) {
        mNeedsReserve = false;
        _ReserveDrawInstances();
    }
    RndSceneInternalContext internal;
    _ExtractSingletonsAndFinalizeParams(entity, params, internal);
    RndContext* context = params.mContext;
    if (context == nullptr) {
        context = ImmediateContext();
    }

    FixedVector<RndSceneDrawTarget, 2> targets;
    if (!internal.mStereoOptimized) {
        _UpdateShaderGraphGlobals(*context, internal);
        if (previous != nullptr) {
            targets = *previous;
            for (RndSceneDrawTarget& target : targets) {
                target.mDepthTargetIndex = 0;
                target.mScatteringResolution = -1;
            }
        } else {
            const unsigned long count = internal.mParams.mBuffers.size();
            targets.resize(count);
            for (unsigned long i = 0; i != count; ++i) {
                targets[i].mBuffersIndex = i;
            }
        }
        for (unsigned long i = 0; i < internal.mParams.mBuffers.size(); ++i) {
            context->BeginFrame(0);
            mSceneCBuffer->_SelectImpl(*context);
            if (internal.mPartialFramerate) {
                _DrawPartialFramerate(*context, internal, targets[i]);
            } else {
                _DrawFullFramerate(*context, internal, targets[i]);
            }
        }
    } else {
        targets = _DrawStereo(*context, internal, previous);
    }
    mDrawing = false;
    return targets;
}

// Reconstructed from eboot.elf at 0x41AE00. Every bucket of the first two
// camera sets, and bucket 2 of the third, can hold the culler's largest
// count.
void RndSceneDrawer::_ReserveDrawInstances() {
    if (mCuller == nullptr) {
        return;
    }
    FixedVector<unsigned long, kNumDrawBuckets> sizes;
    mCuller->GetMaxBucketSizes(sizes);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        const unsigned long size = sizes[bucket];
        mDrawInstances[0][bucket].reserve(size);
        mSortableInstances[0][bucket].reserve(size);
        if (bucket == 2) {
            mDrawInstances[2][bucket].reserve(size);
            mSortableInstances[2][bucket].reserve(size);
        }
    }
}

// Reconstructed from eboot.elf at 0x41B060. The context starts from a fresh
// one and takes a copy of the parameters, which are then finalized: a
// stereo pair gets a bucket set per eye, the shading mode and the device
// settings switch passes off, a partial-framerate scene keeps the
// parameters of its interval's first frame, the camera falls back to the
// scene's cameras and the device's, every collection gets a camera
// context, and the scene's singleton components are collected.
void RndSceneDrawer::_ExtractSingletonsAndFinalizeParams(
    Entity* entity,
    const RndSceneDrawParams& params,
    RndSceneInternalContext& internal) const {
    internal = RndSceneInternalContext();
    internal.mEntity = entity;
    RndSceneDrawParams& p = internal.mParams;
    p = params;
    internal.mCameras.resize(p.mBuffers.size());

    const RndConfig* config = gRndDevice->mSettings;
    internal.mStereoPair = p.mBuffers.size() == 2 && p.mBuffers[0]->mTargetMode == kTargetModeLeftEye
        && p.mBuffers[1]->mTargetMode == kTargetModeRightEye;
    if (internal.mStereoPair) {
        // Each eye culls into its own bucket set.
        internal.mStereoOptimized = config->mStereoOptimizationsEnabled;
        if (p.mDepthOnly || p.mDrawToTextures || p.mSkipDraw || p.mWireframeOnly) {
            internal.mStereoOptimized = false;
        }
        internal.mCameras[0].mDrawInstances = Adapter(mDrawInstances[0]);
        internal.mCameras[0].mSortableInstances = Adapter(mSortableInstances[0]);
        internal.mCameras[1].mDrawInstances = Adapter(mDrawInstances[1]);
        internal.mCameras[1].mSortableInstances = Adapter(mSortableInstances[1]);
    } else {
        internal.mCameras[0].mDrawInstances = Adapter(mDrawInstances[0]);
        internal.mCameras[0].mSortableInstances = Adapter(mSortableInstances[0]);
    }

    RndBufferCollection* buffers = p.mBuffers[0];
    if (p.mShadingMode == -1) {
        p.mShadingMode = static_cast<int>(buffers->mShadingMode);
    }
    // The debug shading modes draw without shadows, and most of them
    // without the atmosphere and the post-processing.
    const unsigned int mode = static_cast<unsigned int>(p.mShadingMode);
    const bool debugMode = mode - 18 < 8 || (mode - 1 <= 30 && ((0x7E00001FU >> (mode - 1)) & 1) != 0);
    if (debugMode || (mode - 12 <= 5 && ((0x37U >> (mode - 12)) & 1) != 0)) {
        p.mDrawShadows = false;
    }
    if (debugMode || (mode | 1) == 17) {
        p.mDrawAtmosphere = false;
        p.mDrawPostProc = false;
    }
    if (!config->mShadowsEnabled) {
        p.mDrawShadows = false;
    }
    if (!config->mPostProcEnabled) {
        p.mDrawPostProc = false;
    }
    if (!config->mVolumetricScatteringEnabled) {
        p.mDrawAtmosphere = false;
    }
    internal.mAsyncCopy = config->mAsyncCopyEnabled;

    RndSceneCom* scene = entity->GetRoot()->GetCom<RndSceneCom>();
    internal.mSceneCom = scene;
    internal.mPartialFramerate =
        scene != nullptr && scene->GetFramerateType() != 0 && (buffers->mFlags & 0x40000000) != 0;
    if (internal.mStereoPair && internal.mPartialFramerate) {
        internal.mPartialFramerate = false;
    } else if (internal.mPartialFramerate) {
        RndScenePartialFramerateData* data =
            buffers->ObtainPartialFramerateData(static_cast<unsigned long>(scene->mPartialFramerateScene));
        if (mStoredCullResults == nullptr) {
            mStoredCullResults = new RndSceneCullResults();
        }
        if (scene->mFrameInterval != 0) {
            p.mShowHide = data->mShowHide;
            p.mShadingMode = data->mShadingMode;
            p.mDrawAtmosphere = data->mDrawAtmosphere;
            p.mDrawPostProc = data->mDrawPostProc;
            p.mDrawSceneMask = data->mDrawSceneMask;
        } else {
            data->mShowHide = p.mShowHide;
            data->mShadingMode = p.mShadingMode;
            data->mDrawAtmosphere = p.mDrawAtmosphere;
            data->mDrawPostProc = p.mDrawPostProc;
            data->mDrawSceneMask = p.mDrawSceneMask;
        }
    }

    // A camera of another entity is dropped. Without a camera the scene's
    // second camera (unless the pair is stereo), then its first, then the
    // device's default camera is used. Drawing from the scene's first or
    // second camera hides the instances flagged 2 or 4.
    if (p.mCamera != nullptr && p.mCameraEntity != nullptr && p.mCameraEntity != entity) {
        p.mCamera = nullptr;
    }
    GameObject* first = nullptr;
    GameObject* second = nullptr;
    if (scene != nullptr) {
        first = reinterpret_cast<GameObject*>(scene->GetCamera(0));
        second = reinterpret_cast<GameObject*>(scene->GetCamera(1));
    }
    if (p.mCamera == nullptr && !internal.mStereoPair) {
        p.mCamera = second;
    }
    if (p.mCamera == nullptr) {
        p.mCamera = first;
        if (first == nullptr) {
            p.mCamera = reinterpret_cast<GameObject*>(gRndDevice->mDefaults.mCamera);
        }
    }
    if (p.mCamera == nullptr || p.mCamera == first) {
        p.mShowHide.mHideFlags |= 2;
    } else if (p.mCamera == second) {
        p.mShowHide.mHideFlags |= 4;
    }

    internal.mHasClearColor = _ExtractClearColor(p, scene, internal.mClearColor);

    unsigned int lodMask;
    float lodDistances[3];
    RndUtl::ExtractLodSettings(entity, lodMask, lodDistances);
    for (unsigned long i = 0; i < p.mBuffers.size(); ++i) {
        RndCameraContext& camera = internal.mCameras[i].mCamera;
        const RndBufferCollection* target = p.mBuffers[i];
        camera.Clear();
        camera.SetRenderTargetInfo(
            static_cast<RndTargetMode>(target->mTargetMode),
            Vector2{static_cast<float>(target->mSize.x), static_cast<float>(target->mSize.y)},
            Vector2{0.0F, 1.0F});
        camera.SetProjectionRect(p.mProjectionRect);
        camera.SetLodSettings(lodMask, lodDistances);
        camera.SetCamera(p.mCamera);
    }
    if (internal.mStereoPair) {
        // The shared work culls with the camera of both eyes.
        RndCameraContext& camera = internal.mCullCamera;
        camera.SetCamera(p.mCamera);
        camera.SetRenderTargetInfo(
            kTargetModeStereo,
            Vector2{static_cast<float>(buffers->mSize.x), static_cast<float>(buffers->mSize.y)},
            Vector2{0.0F, 1.0F});
        camera.SetProjectionRect(p.mProjectionRect);
        camera.SetLodSettings(lodMask, lodDistances);
    } else {
        internal.mCullCamera = internal.mCameras[0].mCamera;
    }

    if (scene != nullptr) {
        internal.mLightMgr = scene->GetLightMgr();
    }
    if (internal.mLightMgr == nullptr || internal.mLightMgr->mLights.empty()) {
        internal.mLightMgr = gRndDevice->mDefaults.mLightMgr;
    }
    if (p.mDrawAtmosphere && scene != nullptr) {
        internal.mSky = scene->GetSky();
        RndAtmosphereCom* atmosphere = scene->GetAtmosphere();
        if (atmosphere != nullptr && atmosphere->mEnabled) {
            if (atmosphere->IsA(RndFogCom::sClassName)) {
                internal.mFog = static_cast<RndFogCom*>(atmosphere);
            } else if (
                atmosphere->IsA(RndVolumetricScatteringCom::sClassName)
                && static_cast<RndVolumetricScatteringCom*>(atmosphere)->IsEnabled(config->mQualityLevel)
                && (gRndDevice->mCapabilities[7].mFeatureFlags & 0x10) != 0) {
                internal.mVolumetricScattering = static_cast<RndVolumetricScatteringCom*>(atmosphere);
            }
        }
    }
    if (p.mDrawPostProc && scene != nullptr) {
        internal.mPostProc[0] = scene->GetPostProc(0);
        internal.mPostProc[1] = scene->GetPostProc(1);
    }
    if (scene != nullptr) {
        RndCMAACom* antialiasing = scene->GetAntialiasing();
        if (antialiasing != nullptr && antialiasing->IsEnabled(config->mQualityLevel)) {
            internal.mAntialiasing = antialiasing;
        }
    }
    if (!internal.mCameras.empty() && p.mDrawAtmosphere) {
        // Fog and volumetric scattering read the linear depth.
        const bool needsDepth = internal.mFog != nullptr || internal.mVolumetricScattering != nullptr;
        for (RndSceneInternalContext::CameraData& camera : internal.mCameras) {
            camera.mNeedsLinearDepth = camera.mNeedsLinearDepth || needsDepth;
        }
    }
}

// Reconstructed from eboot.elf at 0x41D040. One collection's draw: the
// depth-only, texture and wireframe modes have paths of their own;
// otherwise the two intervals, the output conversion and the buffer
// inspection draw. The binary builds the scene's file name for a report
// that is compiled out.
void RndSceneDrawer::_DrawFullFramerate(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) {
    static_cast<void>(FileGetName(internal.mEntity->mResource->mPath.mPath.Str()));
    const RndSceneDrawParams& p = internal.mParams;
    if (p.mDepthOnly) {
        _DrawDepthOnly(context, internal, target.mBuffersIndex);
    } else if (p.mDrawToTextures) {
        _DrawToTextures(context, internal, target);
    } else if (!p.mSkipDraw) {
        if (p.mWireframeOnly) {
            _DrawWireframe(context, internal, target);
        } else {
            _DrawInterval0(context, internal, target);
            _DrawInterval1(context, internal, target);
            _DrawOutputConversion(context, internal, target);
            if (p.mOutputToBackBuffer) {
                RndBufferInspection::Draw(context, *p.mBuffers[target.mBuffersIndex], internal.mEntity, target);
            }
        }
    }
    target.mResult = nullptr;
}

// Reconstructed from eboot.elf at 0x41D4B0. Empties the buckets, adds the
// camera's extra clip planes unless culling is off, culls and points the
// sortable lists at the results.
void RndSceneDrawer::_Cull(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
    RndCullerParams& params) const {
    PodVector<RndDrawInstance>* buckets = Buckets(instances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        buckets[bucket].clear();
    }
    if (!params.mNoCulling) {
        for (const Vector4& plane : camera.mDerivedCache) {
            params.mClipPlanes.push_back(plane);
        }
    }
    mCuller->Cull(sceneDrawId, sceneDrawIndex, camera, showHide, instances, params);
    _FillSortableBuckets(instances, sortable);
}

// Reconstructed from eboot.elf at 0x41D640. The stereo graph is never
// created, so a stereo pair would hook in a null graph.
void RndSceneDrawer::StartDrawJobs(
    Entity* entity,
    const RndSceneDrawParams& params,
    RndSceneDrawer* after,
    PollDepBase* startDep,
    PollDepBase* contextDep,
    PollDepBase* endAfter,
    eastl::vector<PollDepBase*>& jobs) {
    _ExtractSingletonsAndFinalizeParams(entity, params, *mThreadedContext);
    if (mThreadedContext->mStereoOptimized) {
        mStereoDrawJobs->Start(
            *mThreadedContext,
            after != nullptr ? after->mStereoDrawJobs : nullptr,
            startDep,
            contextDep,
            endAfter,
            jobs);
    } else {
        mDrawJobs->Start(
            *mThreadedContext,
            after != nullptr ? after->mDrawJobs : nullptr,
            startDep,
            contextDep,
            endAfter,
            jobs);
    }
}

// Reconstructed from eboot.elf at 0x41D6D0.
FixedVector<RndSceneDrawTarget, 2> RndSceneDrawer::FinishDrawJobs() {
    RndSceneDrawJobs* jobs = mThreadedContext->mStereoOptimized ? mStereoDrawJobs : mDrawJobs;
    return jobs->Finish();
}

// Reconstructed from eboot.elf at 0x41D960. The scene component's clear
// type decides: 1 clears to its color, 0 to the parameters' color unless
// it is black or clearing is off, and any other type does not clear.
// Without a scene the parameters' color is used. Shading modes 3, 16 and
// 17 never clear.
bool RndSceneDrawer::_ExtractClearColor(
    const RndSceneDrawParams& params,
    const RndSceneCom* scene,
    Hmx::Color& color) const {
    const unsigned int mode = static_cast<unsigned int>(params.mShadingMode) - 3;
    if (mode <= 14 && ((0x6001U >> mode) & 1) != 0) {
        return false;
    }
    if (scene == nullptr) {
        color = params.mClearColor;
        return true;
    }
    if (scene->mClearType == 1) {
        color = scene->mClearColor;
        return true;
    }
    if (scene->mClearType != 0 || !params.mClear) {
        return false;
    }
    const Hmx::Color& black = Hmx::Color::GetBlack();
    if (std::fabs(params.mClearColor.red - black.red) < 1.0e-4F
        && std::fabs(params.mClearColor.green - black.green) < 1.0e-4F
        && std::fabs(params.mClearColor.blue - black.blue) < 1.0e-4F) {
        return false;
    }
    color = params.mClearColor;
    return true;
}

// Reconstructed from eboot.elf at 0x41DF90. The flush templates build the
// same context inline.
RndSceneDrawer::BatchContext RndSceneDrawer::_BuildBatchContext(
    const RndSceneInternalContext& internal,
    const RndSceneDrawTarget& target) const {
    BatchContext batch{};
    if (!internal.mParams.mBuffers.empty()) {
        batch.mBuffers = internal.mParams.mBuffers[target.mBuffersIndex];
        batch.mTarget = target;
    }
    batch.mLightMgr = internal.mLightMgr;
    if (internal.mSceneCom != nullptr) {
        batch.mPropHysteresisTexture = internal.mSceneCom->mPropHysteresisTexture;
    }
    if (internal.mFog != nullptr) {
        RndTextureBase* texture =
            internal.mSky != nullptr ? internal.mSky->GetAtmosphereTexture(*batch.mBuffers) : nullptr;
        batch.mAtmosphereTexture = internal.mFog->TextureOrDefault(texture, 5);
    }
    batch.mOcclusionQueries = mOcclusionQueryMgr->mQueries;
    return batch;
}

// Reconstructed from eboot.elf at 0x425FC0. Each sortable bucket grows to
// its instance bucket's capacity and points at its instances.
void RndSceneDrawer::_FillSortableBuckets(
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable) const {
    PodVector<RndDrawInstance>* from = Buckets(instances);
    PodVector<RndDrawInstance*>* to = Buckets(sortable);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        to[bucket].reserve(from[bucket].capacity());
        to[bucket].reserve(from[bucket].size());
        to[bucket].mSize = from[bucket].size();
        for (unsigned long i = 0; i != from[bucket].size(); ++i) {
            to[bucket][i] = &from[bucket][i];
        }
    }
}

// Reconstructed from eboot.elf at 0x41FFC0. After a partial-framerate
// scene's first frame of an interval drew, its cull analysis goes to the
// collection's partial-framerate data and its buckets and culled lights to
// mStoredCullResults.
void RndSceneDrawer::_StoreCullResults(RndSceneInternalContext& internal, unsigned long camera) const {
    RndBufferCollection* buffers = internal.mParams.mBuffers[camera];
    RndScenePartialFramerateData* data =
        buffers->mFrameIntervals.mData[buffers->mActiveFrameInterval].mPartialFramerateData;
    const RndSceneInternalContext::CameraData& cameraData = internal.mCameras[camera];
    data->mHasInstances = cameraData.mHasInstances;
    data->mNeedsLightCulling = cameraData.mNeedsLightCulling;
    data->mSceneTexUsage = cameraData.mSceneTexUsage;
    data->mSceneDepthUsage = cameraData.mSceneDepthUsage;
    data->mSceneTexCaptureUsage = cameraData.mSceneTexCaptureUsage;
    data->mMaterialsNeedLinearDepth = cameraData.mMaterialsNeedLinearDepth;
    data->mNeedsLinearDepth = cameraData.mNeedsLinearDepth;

    FixedVector<PodVector<RndDrawInstance>, kNumDrawBuckets>& stored = mStoredCullResults->mDrawInstances;
    stored.resize(kNumDrawBuckets);
    const PodVector<RndDrawInstance>* buckets = Buckets(cameraData.mDrawInstances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        PodVector<RndDrawInstance>& to = stored[bucket];
        const PodVector<RndDrawInstance>& from = buckets[bucket];
        to.reserve(from.capacity());
        to.reserve(from.size());
        to.mSize = from.size();
        if (from.size() != 0) {
            memcpy(to.mData, from.mData, from.size() * sizeof(RndDrawInstance));
        }
    }
    internal.mLightMgr->StoreCullResults(*data, *mStoredCullResults);
}

// Reconstructed from eboot.elf at 0x420240. The following frames of the
// interval take the stored analysis, buckets and culled lights instead of
// culling.
void RndSceneDrawer::_RestoreCullResults(
    RndContext& context,
    RndSceneInternalContext& internal,
    unsigned long camera) const {
    RndBufferCollection* buffers = internal.mParams.mBuffers[camera];
    RndScenePartialFramerateData* data =
        buffers->mFrameIntervals.mData[buffers->mActiveFrameInterval].mPartialFramerateData;
    RndSceneInternalContext::CameraData& cameraData = internal.mCameras[camera];
    cameraData.mHasInstances = data->mHasInstances;
    cameraData.mNeedsLightCulling = data->mNeedsLightCulling;
    cameraData.mSceneTexUsage = data->mSceneTexUsage;
    cameraData.mSceneDepthUsage = data->mSceneDepthUsage;
    cameraData.mSceneTexCaptureUsage = data->mSceneTexCaptureUsage;
    cameraData.mMaterialsNeedLinearDepth = data->mMaterialsNeedLinearDepth;
    cameraData.mNeedsLinearDepth = data->mNeedsLinearDepth;

    PodVector<RndDrawInstance>* buckets = Buckets(cameraData.mDrawInstances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        const PodVector<RndDrawInstance>& from = mStoredCullResults->mDrawInstances[bucket];
        PodVector<RndDrawInstance>& to = buckets[bucket];
        to.reserve(from.size());
        to.mSize = from.size();
        if (from.size() != 0) {
            memcpy(to.mData, from.mData, from.size() * sizeof(RndDrawInstance));
        }
    }
    _FillSortableBuckets(cameraData.mDrawInstances, cameraData.mSortableInstances);
    internal.mLightMgr->RestoreCullResults(context, cameraData.mCamera, *data, *mStoredCullResults);
}

// Reconstructed from eboot.elf at 0x420400. Records which kinds of work
// the camera's buckets need: any instance at all, the light culling (the
// lit buckets 4, 5, 7, 8 and 13, or a lit material in buckets 13-20), the
// scene texture and depth levels of the materials outside buckets 0-3 and
// 21, and the linear depth.
void RndSceneDrawer::_AnalyzeCullResults(RndSceneInternalContext& internal, unsigned long camera) const {
    RndSceneInternalContext::CameraData& cameraData = internal.mCameras[camera];
    cameraData.mHasInstances = false;
    cameraData.mNeedsLightCulling = false;
    cameraData.mSceneTexUsage = -1;
    cameraData.mSceneDepthUsage = -1;
    cameraData.mSceneTexCaptureUsage = -1;
    cameraData.mMaterialsNeedLinearDepth = false;
    const PodVector<RndDrawInstance>* buckets = Buckets(cameraData.mDrawInstances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        if (buckets[bucket].size() != 0) {
            cameraData.mHasInstances = true;
        }
    }
    if (buckets[7].size() != 0 || buckets[4].size() != 0 || buckets[8].size() != 0
        || buckets[5].size() != 0 || buckets[13].size() != 0) {
        cameraData.mNeedsLightCulling = true;
    }
    for (unsigned long bucket = 13; bucket != 21; ++bucket) {
        for (const RndDrawInstance& instance : buckets[bucket]) {
            const RndMaterialRuntimeData* material = instance.mMaterial;
            if (material == nullptr) {
                continue;
            }
            if (!cameraData.mNeedsLightCulling && material->mRootFlags[0]) {
                cameraData.mNeedsLightCulling = true;
            }
            if (!cameraData.mMaterialsNeedLinearDepth && material->UsesLinearDepth()) {
                cameraData.mMaterialsNeedLinearDepth = true;
            }
            if (cameraData.mNeedsLightCulling && cameraData.mMaterialsNeedLinearDepth) {
                break;
            }
        }
    }
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        if (bucket <= 3 || bucket == 21) {
            continue;
        }
        for (const RndDrawInstance& instance : buckets[bucket]) {
            const RndMaterialRuntimeData* material = instance.mMaterial;
            if (material == nullptr) {
                continue;
            }
            const int sceneTex = material->GetSceneTexUsage();
            if (cameraData.mSceneTexUsage < sceneTex) {
                cameraData.mSceneTexUsage = sceneTex;
            }
            if (cameraData.mSceneDepthUsage == -1) {
                cameraData.mSceneDepthUsage = material->GetSceneDepthUsage();
            } else if (material->GetSceneDepthUsage() != -1) {
                const int sceneDepth = material->GetSceneDepthUsage();
                if (sceneDepth <= cameraData.mSceneDepthUsage) {
                    cameraData.mSceneDepthUsage = sceneDepth;
                }
            }
            const int capture = material->GetSceneTexCaptureUsage();
            if (cameraData.mSceneTexCaptureUsage < capture) {
                cameraData.mSceneTexCaptureUsage = capture;
            }
        }
    }
    cameraData.mNeedsLinearDepth = cameraData.mNeedsLinearDepth || cameraData.mNeedsLightCulling
        || cameraData.mMaterialsNeedLinearDepth;
    if (buckets[10].size() != 0 || buckets[11].size() != 0 || buckets[12].size() != 0) {
        cameraData.mNeedsLinearDepth = true;
    }
}

// Reconstructed from eboot.elf at 0x41BD40. A stereo pair shares the work
// that does not depend on the eye: the shader globals, the dynamic GPU data,
// the light culling and the shadow maps run once, between the two eyes'
// culling and opaque geometry and their lit passes.
FixedVector<RndSceneDrawTarget, 2> RndSceneDrawer::_DrawStereo(
    RndContext& context,
    RndSceneInternalContext& internal,
    const FixedVector<RndSceneDrawTarget, 2>* previous) {
    static_cast<void>(FileGetName(internal.mEntity->mResource->mPath.mPath.Str()));
    _UpdateShaderGraphGlobals(context, internal);
    context.BeginFrame(0);
    mSceneCBuffer->_SelectImpl(context);
    mDynamicGpuDataMgr->ProcessQueue(context);
    if (internal.mSceneCom != nullptr) {
        internal.mSceneCom->UpdatePropHysteresisTexture(internal.mEntity, context);
    }

    FixedVector<RndSceneDrawTarget, 2> targets;
    if (previous != nullptr) {
        targets = *previous;
        for (RndSceneDrawTarget& target : targets) {
            target.mDepthTargetIndex = 0;
            target.mScatteringResolution = -1;
        }
    } else {
        const unsigned long count = internal.mParams.mBuffers.size();
        targets.resize(count);
        for (unsigned long i = 0; i != count; ++i) {
            targets[i].mBuffersIndex = i;
        }
    }

    const RndSceneDrawParams& p = internal.mParams;
    for (unsigned long eye = 0; eye != 2; ++eye) {
        RndSceneInternalContext::CameraData& camera = internal.mCameras[eye];
        RndCullerParams params;
        params.mNoCulling = internal.mSceneCom != nullptr && internal.mSceneCom->mNeverCull;
        _Cull(
            context.mSceneDrawId,
            *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
            camera.mCamera,
            p.mShowHide,
            camera.mDrawInstances,
            camera.mSortableInstances,
            params);
        _AnalyzeCullResults(internal, eye);
        mOcclusionQueryMgr->Submit(camera.mCamera);
        _DrawPostCull(context, internal, targets[eye]);
    }

    RndLightMgrCom* lightMgr = internal.mLightMgr;
    RndBufferCollection& leftEye = *p.mBuffers[0];
    RndBufferCollection& rightEye = *p.mBuffers[1];
    if (internal.mCameras[0].mNeedsLightCulling || internal.mCameras[1].mNeedsLightCulling) {
        static Symbol sLightCulling;
        if (sLightCulling == Symbol()) {
            sLightCulling = Symbol("Light Culling");
        }
        RndScopedGpuStatBlock statBlock(context, sLightCulling.Str());
        lightMgr->CullLights(
            context.mSceneDrawId,
            *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
            internal.mCullCamera,
            p);
        lightMgr->_CullTiledLights(context, internal.mCullCamera, p, leftEye, &rightEye);
        if (p.mDrawShadows) {
            VectorAdapter<PodVector<RndDrawInstance>> instances = Adapter(mDrawInstances[2]);
            VectorAdapter<PodVector<RndDrawInstance*>> sortable = Adapter(mSortableInstances[2]);
            for (unsigned long i = 0; i < lightMgr->mCullResults.mShadowLights.size(); ++i) {
                lightMgr->DrawShadowMaps(context, *this, internal.mCullCamera, p.mShowHide, instances, sortable, i);
            }
        }
    }
    if (internal.mCameras[0].mNeedsLightCulling) {
        lightMgr->_CullTiledLightsStereo(
            context, p, internal.mCullCamera, internal.mCameras[0].mCamera, leftEye, leftEye);
    }
    _DrawPostLightCull(context, internal, targets[0]);
    _DrawInterval1(context, internal, targets[0]);
    _DrawOutputConversion(context, internal, targets[0]);
    if (p.mOutputToBackBuffer) {
        RndBufferInspection::Draw(context, leftEye, internal.mEntity, targets[0]);
    }
    targets[0].mResult = nullptr;
    if (internal.mCameras[1].mNeedsLightCulling) {
        lightMgr->_CullTiledLightsStereo(
            context, p, internal.mCullCamera, internal.mCameras[1].mCamera, leftEye, rightEye);
    }
    _DrawPostLightCull(context, internal, targets[1]);
    _DrawInterval1(context, internal, targets[1]);
    _DrawOutputConversion(context, internal, targets[1]);
    if (p.mOutputToBackBuffer) {
        RndBufferInspection::Draw(context, rightEye, internal.mEntity, targets[1]);
    }
    targets[1].mResult = nullptr;
    return targets;
}

// Reconstructed from eboot.elf at 0x41C760. A partial-framerate scene
// draws the opaque passes and the lighting (interval 0) on the first frame
// of an interval and keeps the cull results and the lit scene; the
// following frame draws the rest (interval 1) on a copy of it. A frame
// that does not follow on (another frame, drawer, deregistration or light
// removal came between) only clears the target and copies it. The
// collection's partial-framerate buffers are selected for the draw.
void RndSceneDrawer::_DrawPartialFramerate(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) {
    RndBufferCollection* buffers = internal.mParams.mBuffers[target.mBuffersIndex];
    const unsigned long interval = internal.mSceneCom->mFrameInterval;
    static_cast<void>(FileGetName(internal.mEntity->mResource->mPath.mPath.Str()));
    buffers->SelectPartialFramerateBuffers(internal.mSceneCom->mPartialFramerateScene, static_cast<long>(interval));
    const RndSceneDrawParams& p = internal.mParams;
    if (p.mWireframeOnly) {
        if (!p.mSkipDraw) {
            _DrawWireframe(context, internal, target);
        }
    } else if (!p.mSkipDraw) {
        RndBufferCollection::FrameIntervalBuffers& frame =
            buffers->mFrameIntervals.mData[buffers->mActiveFrameInterval];
        RndScenePartialFramerateData* data = frame.mPartialFramerateData;
        const unsigned long storedInterval = data->mFrameInterval;
        bool followsOn = storedInterval == (interval ^ 1) && data->mFrame == gRndDevice->mFrameCount - 1
            && data->mDrawerIndex == mIndex;
        if (interval != 0) {
            if (followsOn
                && (data->mNumDeRegistered != mNumDeRegistered
                    || data->mLightRemovalCount != internal.mLightMgr->mLightRemovalCount)) {
                followsOn = false;
            }
            RndTextureBase* scene = buffers->mActiveSceneContext != 0
                ? buffers->mLightAccum[target.mSrcLightAccum]
                : frame.mPartialLightAccum;
            FixedVector<RndResourceBarrier, 2> barriers;
            barriers.resize(2);
            barriers[0].mResource = scene;
            barriers[0].mSubresource = ~0UL;
            barriers[0].mBefore = kStateShaderResource;
            barriers[1].mResource = frame.mPartialLightAccum;
            barriers[1].mSubresource = ~0UL;
            barriers[1].mBefore = kStateShaderResource;
            if (followsOn) {
                _RestoreCullResults(context, internal, target.mBuffersIndex);
                if (data->mResultsTag != kStoredResultsTag) {
                    static_cast<void>(internal.mEntity->MakeErrorName());
                }
                _DrawInterval1(context, internal, target);
                scene = buffers->mActiveSceneContext != 0
                    ? buffers->mLightAccum[target.mSrcLightAccum]
                    : frame.mPartialLightAccum;
                barriers[0].mResource = scene;
                barriers[0].mBefore = RndResourceState::kRenderTarget;
            } else {
                // The scene is not drawn: the target is cleared and
                // copied.
                barriers[0].mAfter = RndResourceState::kRenderTarget;
                barriers[1].mPhase = RndResourceBarrierPhase::kBegin;
                barriers[1].mBefore = storedInterval == 0 ? RndResourceState::kRenderTarget : kStateShaderResource;
                barriers[1].mAfter = RndResourceState::kCopyDestination;
                context._ResourceBarrierImpl(barriers.size(), barriers.mData);
                if (internal.mHasClearColor) {
                    ClearTarget(context, internal, *scene);
                }
                barriers[0].mBefore = RndResourceState::kRenderTarget;
                barriers[1].mPhase = RndResourceBarrierPhase::kEnd;
            }
            barriers[0].mAfter = kStateCopySource;
            barriers[1].mAfter = RndResourceState::kCopyDestination;
            context._ResourceBarrierImpl(barriers.size(), barriers.mData);
            frame.mPartialLightAccum->GpuCopyFrom(context, *scene);
            barriers[0].mBefore = kStateCopySource;
            barriers[0].mAfter = RndResourceState::kRenderTarget;
            barriers[1].mPhase = RndResourceBarrierPhase::kImmediate;
            barriers[1].mBefore = RndResourceState::kCopyDestination;
            barriers[1].mAfter = static_cast<RndResourceState>(0);
            context._ResourceBarrierImpl(barriers.size(), barriers.mData);
            target.mResult = frame.mPartialLightAccum;
        } else {
            RndTextureBase* scene = buffers->mLightAccum[target.mSrcLightAccum];
            FixedVector<RndResourceBarrier, 3> barriers;
            barriers.resize(1);
            barriers[0].mResource = scene;
            barriers[0].mSubresource = ~0UL;
            barriers[0].mBefore = kStateShaderResource;
            if (storedInterval == 0) {
                barriers.resize(2);
                barriers[1].mResource = frame.mPartialLightAccum;
                barriers[1].mSubresource = ~0UL;
                barriers[1].mBefore = RndResourceState::kRenderTarget;
            }
            if (followsOn) {
                // The previous interval left a scene: start from it.
                barriers[0].mAfter = RndResourceState::kCopyDestination;
                context._ResourceBarrierImpl(barriers.size(), barriers.mData);
                static Symbol sCopyLastFramebuffer;
                if (sCopyLastFramebuffer == Symbol()) {
                    sCopyLastFramebuffer = Symbol("Copy Last Framebuffer");
                }
                RndScopedGpuStatBlock statBlock(context, sCopyLastFramebuffer.Str());
                scene->GpuCopyFrom(context, *frame.mPartialLightAccum);
                barriers.resize(2);
                barriers[0].mBefore = RndResourceState::kCopyDestination;
                barriers[0].mAfter = RndResourceState::kRenderTarget;
                barriers[1].mType = RndResourceBarrierType::kTransition;
                barriers[1].mPhase = RndResourceBarrierPhase::kImmediate;
                barriers[1].mResource = frame.mPartialLightAccum;
                barriers[1].mSubresource = ~0UL;
                barriers[1].mBefore = kStateCopySource;
                barriers[1].mAfter = kStateShaderResource;
                context._ResourceBarrierImpl(barriers.size(), barriers.mData);
            } else {
                barriers[0].mAfter = RndResourceState::kRenderTarget;
                if (storedInterval != 0) {
                    barriers.resize(2);
                    barriers[1].mResource = frame.mPartialLightAccum;
                    barriers[1].mSubresource = ~0UL;
                    barriers[1].mBefore = kStateCopySource;
                }
                barriers[1].mAfter = kStateShaderResource;
                context._ResourceBarrierImpl(barriers.size(), barriers.mData);
                if (internal.mHasClearColor) {
                    ClearTarget(context, internal, *scene);
                }
            }
            _DrawInterval0(context, internal, target);
            _StoreCullResults(internal, target.mBuffersIndex);
            data->mResultsTag = kStoredResultsTag;
            target.mResult = nullptr;
        }
        _DrawOutputConversion(context, internal, target);
        if (p.mOutputToBackBuffer) {
            RndBufferInspection::Draw(context, *buffers, internal.mEntity, target);
        }
        data->mFrame = gRndDevice->mFrameCount;
        data->mFrameInterval = interval;
        data->mDrawerIndex = mIndex;
        data->mNumDeRegistered = mNumDeRegistered;
        data->mLightRemovalCount = internal.mLightMgr->mLightRemovalCount;
    }
    buffers->SelectPartialFramerateBuffers(-1, -1);
}

// Reconstructed from eboot.elf at 0x41C640. The scene constant buffer gets
// the shader-graph globals (time, the material smoothness adjustment, the
// trans infos, the scene's global floats and colors), the forward lighting
// constants and the fog's and volumetric scattering's, or their defaults,
// and is synced once.
void RndSceneDrawer::_UpdateShaderGraphGlobals(RndContext& context, RndSceneInternalContext& internal) const {
    RndShaderCBuffer& cbuffer = *mSceneCBuffer;
    RndShaderNodeTime::SetGlobalConstants(cbuffer);
    static_cast<float*>(cbuffer.mData)[4 * gRndDevice->mShaderMgr.mSmoothnessDecay] =
        internal.mLightMgr->mMaterialSmoothnessAdjustment;
    cbuffer.mSyncPending = true;
    RndShaderNodeTransInfo::SetGlobalConstants(cbuffer, internal.mEntity);
    RndShaderNodeSceneGlobalFloat::SetGlobalConstants(cbuffer, internal.mEntity);
    RndShaderNodeSceneGlobalColor::SetGlobalConstants(cbuffer, internal.mEntity);
    internal.mLightMgr->SetFwdLightingConstants(internal.mParams.mBuffers[0]->mSize, cbuffer);
    if (internal.mFog != nullptr) {
        internal.mFog->SetFwdShadingConstants(cbuffer);
    } else {
        RndFogCom::SetNoAtmosphereFwdShadingConstants(cbuffer);
    }
    if (internal.mVolumetricScattering != nullptr) {
        internal.mVolumetricScattering->SetFwdShadingConstants(internal.mCullCamera, cbuffer);
    } else {
        RndVolumetricScatteringCom::SetNoAtmosphereFwdShadingConstants(cbuffer);
    }
    if (cbuffer.mSyncPending) {
        cbuffer._SyncImpl(context, 0, cbuffer.mNumElements);
        cbuffer.mSyncPending = false;
    }
}

namespace {

// Whether two clip planes are the same, compared as floats.
bool PlaneEqual(const Vector4& a, const Vector4& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

// Orders two different clip planes by their two 64-bit halves, as the
// sorts' inlined comparisons do. Name not in the reference map.
bool PlaneLess(const Vector4& a, const Vector4& b) {
    unsigned long left[2];
    unsigned long right[2];
    memcpy(left, &a, sizeof(left));
    memcpy(right, &b, sizeof(right));
    return left[0] != right[0] ? left[0] < right[0] : left[1] < right[1];
}

// The map's RndDrawInstance::StateCompare: orders by the sorting hint, the
// debug tag, the environment, the render state, the material, the
// drawable, the instance buffer and the clip planes, and then, for the
// passes that ask for it, front to back. This build passes the distance
// flag in a functor (operator() at 0x42BA90). Name not in the reference
// map.
struct StateCompare {
    bool operator()(const RndDrawInstance* a, const RndDrawInstance* b) const {
        if (a->mSortingHint != b->mSortingHint) {
            return a->mSortingHint < b->mSortingHint;
        }
        if (a->mDebugTag != b->mDebugTag) {
            return a->mDebugTag < b->mDebugTag;
        }
        if (a->mEnvironIndex != b->mEnvironIndex) {
            return a->mEnvironIndex < b->mEnvironIndex;
        }
        if (a->mCounterClockwise != b->mCounterClockwise) {
            return a->mCounterClockwise < b->mCounterClockwise;
        }
        if (a->mCullMode != b->mCullMode) {
            return a->mCullMode < b->mCullMode;
        }
        if (a->mReceiveAtmosphere != b->mReceiveAtmosphere) {
            return a->mReceiveAtmosphere < b->mReceiveAtmosphere;
        }
        if (a->mReceiveDecals != b->mReceiveDecals) {
            return a->mReceiveDecals < b->mReceiveDecals;
        }
        if (a->mMaterial != b->mMaterial) {
            return a->mMaterial < b->mMaterial;
        }
        if (a->mDrawable != b->mDrawable) {
            return a->mDrawable < b->mDrawable;
        }
        if (a->mInstanceCBuffer != b->mInstanceCBuffer) {
            return a->mInstanceCBuffer < b->mInstanceCBuffer;
        }
        if (!PlaneEqual(a->mClipPlanes[0], b->mClipPlanes[0])) {
            return PlaneLess(a->mClipPlanes[0], b->mClipPlanes[0]);
        }
        if (!PlaneEqual(a->mClipPlanes[1], b->mClipPlanes[1])) {
            return PlaneLess(a->mClipPlanes[1], b->mClipPlanes[1]);
        }
        return mFrontToBack && a->mDistance < b->mDistance;
    }

    bool mFrontToBack;
};

// The map's RndDrawInstance::DepthOnlyStateCompare: the state that
// matters to a depth pass, inlined into its sort (0x428A90).
bool DepthOnlyStateCompare(const RndDrawInstance* a, const RndDrawInstance* b) {
    if (a->mCullMode != b->mCullMode) {
        return a->mCullMode < b->mCullMode;
    }
    if (a->mUsesSceneTex != b->mUsesSceneTex) {
        return a->mUsesSceneTex < b->mUsesSceneTex;
    }
    if (a->mUsesSceneDepth != b->mUsesSceneDepth) {
        return a->mUsesSceneDepth < b->mUsesSceneDepth;
    }
    if (a->mMaterial != b->mMaterial) {
        return a->mMaterial < b->mMaterial;
    }
    if (a->mDrawable != b->mDrawable) {
        return a->mDrawable < b->mDrawable;
    }
    if (a->mInstanceCBuffer != b->mInstanceCBuffer) {
        return a->mInstanceCBuffer < b->mInstanceCBuffer;
    }
    if (!PlaneEqual(a->mClipPlanes[0], b->mClipPlanes[0])) {
        return PlaneLess(a->mClipPlanes[0], b->mClipPlanes[0]);
    }
    if (!PlaneEqual(a->mClipPlanes[1], b->mClipPlanes[1])) {
        return PlaneLess(a->mClipPlanes[1], b->mClipPlanes[1]);
    }
    return false;
}

// The map's RndDrawInstance::DistanceCompareBackToFront, inlined into its
// sort (0x42FF90): the farther first, then by address.
bool DistanceCompareBackToFront(const RndDrawInstance* a, const RndDrawInstance* b) {
    if (a->mDistance != b->mDistance) {
        return a->mDistance > b->mDistance;
    }
    return a < b;
}

// Sorts a whole bucket.
template <typename Compare>
void SortBucket(PodVector<RndDrawInstance*>& bucket, Compare compare) {
    eastl::sort(bucket.begin(), bucket.end(), compare);
}

// The stencil reference of an environment: -2 and -1 are 1 and 0, the
// others follow from 2. Name not in the reference map.
unsigned int EnvironStencil(int environ) {
    if (environ == -2) {
        return 1;
    }
    if (environ == -1) {
        return 0;
    }
    return static_cast<unsigned int>(environ + 2);
}

// Whether two consecutive instances draw in one batch: the same drawable,
// material and instance buffer, and the same state. Inlined into every
// _FlushBucket. Name not in the reference map.
bool CanBatch(const RndDrawInstance& previous, const RndDrawInstance& instance) {
    return previous.mDrawable == instance.mDrawable && previous.mCullMode != -1
        && previous.mCullMode == instance.mCullMode && previous.mCounterClockwise == instance.mCounterClockwise
        && previous.mMaterial != nullptr && previous.mMaterial == instance.mMaterial
        && previous.mEnvironIndex == instance.mEnvironIndex
        && previous.mReceiveAtmosphere == instance.mReceiveAtmosphere
        && previous.mReceiveDecals == instance.mReceiveDecals && previous.mDebugTag == instance.mDebugTag
        && previous.mInstanceCBuffer == instance.mInstanceCBuffer
        && PlaneEqual(previous.mClipPlanes[0], instance.mClipPlanes[0])
        && PlaneEqual(previous.mClipPlanes[1], instance.mClipPlanes[1]);
}

// The blend mode a blended pass draws a material with: the batch-
// visualization shading mode (18) shows some debug modes opaque. The first
// form is 0x42DAB0's, the second 0x42E570's and 0x42FA70's. Names not in the
// reference map.
int DecalBlendMode(const RndContext& context, const RndMaterialRuntimeData& material) {
    const int blend = material.mBlendMode != 0 ? material.mBlendMode : 11;
    if (context.mShadingMode == 18 && context.mDebugMiscMode != -1) {
        const unsigned int mode = static_cast<unsigned int>(context.mDebugMiscMode) - 3;
        if (mode <= 14 && ((0x6001U >> mode) & 1) != 0) {
            return 6;
        }
    }
    return blend;
}

int TransparentBlendMode(const RndContext& context, const RndMaterialRuntimeData& material) {
    const int blend = material.mBlendMode;
    if (context.mShadingMode == 18 && context.mDebugMiscMode != -1) {
        const int debug = context.mDebugMiscMode;
        const unsigned int mode = static_cast<unsigned int>(debug) - 3;
        if (mode <= 14 && ((0x6001U >> mode) & 1) != 0) {
            return 6;
        }
        const unsigned int value = static_cast<unsigned int>(debug);
        const bool debugMode =
            value - 18 < 8 || (value - 1 <= 30 && ((0x7E00001FU >> (value - 1)) & 1) != 0);
        if (debugMode && debug != 2) {
            return 5;
        }
    }
    return blend;
}

// Sets one of the context's two user clip planes for a batch. A zero plane
// only disables a plane that is on.
void SyncClipPlane(RndContext& context, unsigned int index, const Vector4& plane) {
    RndContext::ClipPlane& clip = context.mClipPlanes[index];
    if (plane.x == 0.0F && plane.y == 0.0F && plane.z == 0.0F && plane.w == 0.0F) {
        if (!clip.mEnabled) {
            return;
        }
        clip.mEnabled = false;
    } else {
        clip.mEnabled = true;
        memcpy(clip.mPlane, &plane, sizeof(clip.mPlane));
    }
    context._SyncClipPlanes(1U << index);
}

}  // namespace

// Reconstructed from eboot.elf at 0x4268D0. The instantiations at 0x42A000,
// 0x42A9A0, 0x42BD60, 0x42CA60, 0x42D430, 0x42DEF0, 0x42E990 and 0x42F3F0
// differ only in their _FlushBatch and their 'disable_batching' index.
// Consecutive instances that can batch are drawn together, at most 256 (or
// one with the "disable_batching" variable set); afterwards the front face,
// and the clip planes the batches enabled, are reset.
template <unsigned int N>
void RndSceneDrawer::_FlushBucket(
    RndContext& context,
    RndSceneInternalContext& internal,
    const RndSceneDrawTarget& target,
    PodVector<RndDrawInstance*>& bucket,
    unsigned long begin,
    unsigned long end) const {
    FixedVector<RndInstanceData, 256> instances;
    static const unsigned long disable_batchingIndex = DataVarIndex(Symbol("disable_batching"), DataNode(0));
    const unsigned long maxBatch = DataVariable(disable_batchingIndex).mValue.integer == 0 ? 256 : 1;
    BatchContext batch = _BuildBatchContext(internal, target);
    FlushContext flush;
    if (end > bucket.size()) {
        end = bucket.size();
    }
    RndDrawInstance* previous = nullptr;
    for (unsigned long i = begin; i < end; ++i) {
        RndDrawInstance* instance = bucket[i];
        if (previous != nullptr && (!CanBatch(*previous, *instance) || instances.size() == maxBatch)) {
            _FlushBatch<N>(
                context,
                internal,
                batch,
                flush,
                *previous,
                VectorAdapter<RndInstanceData>{instances.mData, instances.size()});
            instances.clear();
        }
        instances.push_back(instance->mInstanceData);
        previous = instance;
    }
    if (previous != nullptr) {
        _FlushBatch<N>(
            context,
            internal,
            batch,
            flush,
            *previous,
            VectorAdapter<RndInstanceData>{instances.mData, instances.size()});
    }
    if (!flush.mCounterClockwise) {
        context._SetFrontFaceImpl(true);
    }
    if (context.mClipPlanes[0].mEnabled) {
        context.mClipPlanes[0].mEnabled = false;
        context._SyncClipPlanes(1);
    }
    if (context.mClipPlanes[1].mEnabled) {
        context.mClipPlanes[1].mEnabled = false;
        context._SyncClipPlanes(2);
    }
}

// Reconstructed from eboot.elf at 0x42C710. The instantiations at 0x42A680,
// 0x42B020, 0x42C3E0, 0x42D0E0, 0x42DAB0, 0x42E570, 0x42F010 and 0x42FA70
// differ in the state of their FlushType: the stencil, the blend mode, the
// forward environment and atmosphere shading. Each draws one batch: it
// selects the instance buffer, sets the state that changed since the last
// batch, reselects the material, and draws the instances. The map's
// signature has no batch context.
template <unsigned int N>
void RndSceneDrawer::_FlushBatch(
    RndContext& context,
    RndSceneInternalContext& internal,
    BatchContext& batch,
    FlushContext& flush,
    RndDrawInstance& instance,
    const VectorAdapter<RndInstanceData>& instances) const {
    int hasInstanceCBuffer = 0;
    if (instance.mInstanceCBuffer != nullptr) {
        instance.mInstanceCBuffer->_SelectImpl(context);
        hasInstanceCBuffer = 1;
    }

    // The stencil, or the forward passes' environment and atmosphere.
    if (N == kFlushGBuffer) {
        if (!flush.mStencilSet || instance.mReceiveAtmosphere != flush.mReceiveAtmosphere
            || instance.mReceiveDecals != flush.mReceiveDecals
            || instance.mEnvironIndex != flush.mEnvironIndex) {
            const unsigned int reference = (instance.mReceiveAtmosphere ? 0U : 8U)
                | EnvironStencil(instance.mEnvironIndex)
                | (static_cast<unsigned int>(instance.mReceiveDecals) << 4);
            context._SetStencilModeImpl(3, static_cast<unsigned char>(reference), 6, 5);
            flush.mStencilSet = true;
        }
    } else if (N == kFlushGBufferUnlit) {
        if (!flush.mStencilSet || instance.mReceiveAtmosphere != flush.mReceiveAtmosphere
            || instance.mReceiveDecals != flush.mReceiveDecals) {
            const unsigned int reference = (instance.mReceiveAtmosphere ? 0U : 8U)
                | (static_cast<unsigned int>(instance.mReceiveDecals) << 4);
            context._SetStencilModeImpl(3, static_cast<unsigned char>(reference), 6, 5);
            flush.mStencilSet = true;
        }
    } else if (N == kFlushDecal || N == kFlushDecalBlended) {
        if (!flush.mStencilSet || instance.mReceiveAtmosphere != flush.mReceiveAtmosphere
            || instance.mEnvironIndex != flush.mEnvironIndex) {
            const unsigned int reference =
                EnvironStencil(instance.mEnvironIndex) | (instance.mReceiveAtmosphere ? 0x10U : 0x18U);
            context._SetStencilModeImpl(3, static_cast<unsigned char>(reference), 8, 4);
            flush.mStencilSet = true;
        }
    } else if (N == kFlushDecalUnlit) {
        if (!flush.mStencilSet || instance.mReceiveAtmosphere != flush.mReceiveAtmosphere) {
            context._SetStencilModeImpl(3, instance.mReceiveAtmosphere ? 0x10 : 0x18, 8, 4);
            flush.mStencilSet = true;
        }
    } else if (N == kFlushForward || N == kFlushTransparent) {
        if (instance.mEnvironIndex != flush.mEnvironIndex) {
            RndShaderCBuffer& state = *context.mCBuffers[3];
            static_cast<float*>(state.mData)[4 * gRndDevice->mShaderMgr.mEnvironIndex] =
                static_cast<float>(instance.mEnvironIndex);
            state.mSyncPending = true;
            state._SyncImpl(context, 0, state.mNumElements);
            state.mSyncPending = false;
            state._SelectImpl(context);
        }
        if ((internal.mFog != nullptr || internal.mVolumetricScattering != nullptr)
            && instance.mReceiveAtmosphere != flush.mReceiveAtmosphere) {
            if (instance.mReceiveAtmosphere) {
                if (context.mShadingMode == 0) {
                    context.SetShadingMode(static_cast<RndShadingMode>(
                        internal.mFog != nullptr ? 1 : (internal.mVolumetricScattering != nullptr ? 2 : 0)));
                }
            } else if (static_cast<unsigned int>(context.mShadingMode) - 1 <= 1) {
                context.SetShadingMode(static_cast<RndShadingMode>(0));
            }
        }
    }
    flush.mEnvironIndex = instance.mEnvironIndex;
    flush.mReceiveAtmosphere = instance.mReceiveAtmosphere;
    flush.mReceiveDecals = instance.mReceiveDecals;

    // The material, with the blend state of the blended passes.
    RndMaterialRuntimeData* material = instance.mMaterial;
    if (N == kFlushDepth) {
        if (material != nullptr
            && (hasInstanceCBuffer != flush.mHasInstanceCBuffer
                || (material != flush.mMaterial
                    && (instance.mUsesSceneTex || flush.mUsesSceneTex || instance.mUsesSceneDepth
                        || flush.mUsesSceneDepth)))) {
            material->SelectShader(context, batch, static_cast<RndShaderGeoType>(hasInstanceCBuffer));
        }
    } else if (N == kFlushDecalBlended || N == kFlushDecalUnlit || N == kFlushTransparent) {
        int blend = flush.mBlendMode;
        Hmx::Color color = flush.mBlendColor;
        if (material != nullptr && (material != flush.mMaterial || hasInstanceCBuffer != flush.mHasInstanceCBuffer)) {
            bool changed;
            if (N == kFlushDecalBlended) {
                blend = DecalBlendMode(context, *material);
                color = material->mBlendFactor;
                changed = blend != flush.mBlendMode || color.red != flush.mBlendColor.red
                    || color.green != flush.mBlendColor.green || color.blue != flush.mBlendColor.blue
                    || color.alpha != flush.mBlendColor.alpha;
            } else {
                blend = TransparentBlendMode(context, *material);
                color = Hmx::Color::GetWhite();
                changed = blend != flush.mBlendMode;
            }
            if (changed) {
                context.mBlendMode = static_cast<RndBlendMode>(blend);
                context._SetBlendModeImpl(static_cast<RndBlendMode>(blend), color);
                if (N == kFlushTransparent && internal.mFog != nullptr) {
                    RndTextureBase* texture = internal.mSky != nullptr
                        ? internal.mSky->GetAtmosphereTexture(*batch.mBuffers)
                        : nullptr;
                    batch.mAtmosphereTexture =
                        internal.mFog->TextureOrDefault(texture, static_cast<unsigned int>(blend));
                }
            }
            material->SelectShader(context, batch, static_cast<RndShaderGeoType>(hasInstanceCBuffer));
        }
        flush.mBlendMode = material != nullptr ? blend : -1;
        if (N == kFlushDecalBlended) {
            flush.mBlendColor = color;
        }
    } else {
        if (material != nullptr && (material != flush.mMaterial || hasInstanceCBuffer != flush.mHasInstanceCBuffer)) {
            material->SelectShader(context, batch, static_cast<RndShaderGeoType>(hasInstanceCBuffer));
        }
    }
    flush.mDebugTag = instance.mDebugTag;
    flush.mMaterial = material;
    flush.mUsesSceneTex = instance.mUsesSceneTex;
    flush.mUsesSceneDepth = instance.mUsesSceneDepth;
    flush.mHasInstanceCBuffer = hasInstanceCBuffer;

    // The common state.
    if (instance.mCounterClockwise != flush.mCounterClockwise) {
        context._SetFrontFaceImpl(instance.mCounterClockwise);
    }
    flush.mCounterClockwise = instance.mCounterClockwise;
    if (instance.mCullMode != -1 && instance.mCullMode != flush.mCullMode) {
        context._SetCullModeImpl(static_cast<RndCullMode>(instance.mCullMode));
    }
    flush.mCullMode = instance.mCullMode;
    for (unsigned int i = 0; i != 2; ++i) {
        if (!PlaneEqual(instance.mClipPlanes[i], flush.mClipPlanes[i])) {
            SyncClipPlane(context, i, instance.mClipPlanes[i]);
            flush.mClipPlanes[i] = instance.mClipPlanes[i];
        }
    }
    if (context.mShadingMode == 18) {
        context.SetBatchInfo(flush.mNumBatches++, instances.mSize);
    }
    instance.mDrawable->_DrawBatchImpl(context, instances, RndDrawable::DrawRange{0, RndDrawable::DrawRange::kAllFaces});
}

namespace {

// Whether a shading mode is one of the debug views, which draw through
// RndContext::SetShaderDebugMode. Inlined everywhere. Name not in the
// reference map.
bool IsDebugShadingMode(int shadingMode) {
    const unsigned int mode = static_cast<unsigned int>(shadingMode);
    return mode - 18 < 8 || (mode - 1 <= 30 && ((0x7E00001FU >> (mode - 1)) & 1) != 0);
}

// The buffers of the collection's active frame interval.
RndBufferCollection::FrameIntervalBuffers& ActiveFrame(RndBufferCollection& buffers) {
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
}

// The buffer the lit scene accumulates in: the scene context's light
// accumulation of a partial-framerate scene, else the interval's.
RndTextureBase* LightAccum(RndBufferCollection& buffers, const RndSceneDrawTarget& target) {
    return buffers.mActiveSceneContext != 0 ? buffers.mLightAccum[target.mSrcLightAccum]
                                             : ActiveFrame(buffers).mPartialLightAccum;
}

// The cached GPU-timer label of a pass, as each pass's function-local
// symbol is built on first use.
const char* PassName(Symbol& symbol, const char* name) {
    if (symbol == Symbol()) {
        symbol = Symbol(name);
    }
    return symbol.Str();
}

// The deferred passes blend opaque, or into the depth-only debug view.
void SetDeferredBlend(RndContext& context, const RndSceneDrawParams& params) {
    const RndBlendMode blend = static_cast<RndBlendMode>(params.mShadingMode == 3 ? 6 : 5);
    context.mBlendMode = blend;
    context._SetBlendModeImpl(blend, Hmx::Color::GetWhite());
}

// Binds a list of color targets with the depth target and enables writes
// to all of them.
template <unsigned long N>
void SetColorTargets(RndContext& context, FixedVector<RndTextureBase*, N>& targets, RndTextureBase* depth) {
    context.SetRenderTargets(VectorAdapter<RndTextureBase*>{targets.mData, targets.size()}, depth);
    context._SetColorWriteMaskImpl(
        static_cast<unsigned char>((1U << targets.size()) - 1), static_cast<RndWriteMaskChannelSet>(0));
}

}  // namespace

// Reconstructed from eboot.elf at 0x422320. Lays down the depth of bucket
// 0, sorted for depth, with the stencil cleared.
void RndSceneDrawer::_DrawZPrepass(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Z Prepass"));
    RndBufferCollection& buffers = *internal.mParams.mBuffers[target.mBuffersIndex];
    SortBucket(bucket, DepthOnlyStateCompare);
    context.SetShadingMode(kShadingModeDepthOnly);
    context._SetDepthModeImpl(1);
    context.SetRenderTargets(nullptr, ActiveFrame(buffers).mDepthStencil);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushDepth>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x4224B0. The G-buffer's color and
// normal targets move to render targets first (a barrier for each that
// exists); bucket 1 then fills the normals and the depth.
void RndSceneDrawer::_DrawNormalsZPrepass(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    RndBufferCollection& buffers = *internal.mParams.mBuffers[target.mBuffersIndex];
    RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
    const RndResourceState before = gRndDevice->mSettings->mUseTiledLighting
        ? kStateShaderResource
        : RndResourceState::kPixelShaderResource;
    FixedVector<RndResourceBarrier, 3> barriers;
    RndTextureBase* textures[] = {frame.mGBufferColor, frame.mGBufferPixelNormals, frame.mGBufferVertexNormals};
    for (RndTextureBase* texture : textures) {
        if (texture != nullptr) {
            RndResourceBarrier barrier;
            barrier.mResource = texture;
            barrier.mSubresource = ~0UL;
            barrier.mBefore = before;
            barrier.mAfter = RndResourceState::kRenderTarget;
            barriers.push_back(barrier);
        }
    }
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Normals Z Prepass"));
    SortBucket(bucket, DepthOnlyStateCompare);
    context.SetShadingMode(kShadingModeDeferredNormalsAndZFill);
    context._SetDepthModeImpl(1);
    context.SetRenderTargets(frame.mGBufferPixelNormals, frame.mDepthStencil);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushBasic>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x4227A0. Fills the G-buffer from the
// lit deferred bucket: bucket 4 before the lighting writes the depth too
// (and initializes the light accumulation, before the pass in the debug
// views and after it otherwise); bucket 7 after it tests the depth. The
// bucket is sorted and flushed in runs of 256. The debug views draw into
// the light accumulation instead.
void RndSceneDrawer::_DrawOpaqueDeferredLit(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    const RndConfig* config = gRndDevice->mSettings;
    const bool firstPass = bucketIndex == 4;
    const RndShadingMode shading = firstPass ? kShadingModeDeferredLitAndZFill : kShadingModeDeferredLit;
    if (firstPass && IsDebugShadingMode(p.mShadingMode)) {
        _InitLightAccum(context, internal, target);
    }
    if (!bucket.empty()) {
        static Symbol sName;
        RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Deferred Lit"));
        RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
        if (IsDebugShadingMode(p.mShadingMode)) {
            context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), shading);
            FixedVector<RndTextureBase*, 2> targets;
            targets.push_back(LightAccum(buffers, target));
            if (config->mUseGBufferVertexNormals) {
                targets.push_back(frame.mGBufferVertexNormals);
            }
            SetColorTargets(context, targets, frame.mDepthStencil);
        } else {
            context.SetShadingMode(shading);
            context._DeselectAllReadWriteTexturesImpl(1U << kShaderProgramPixel);
            context.mInputSlotLimits[kShaderProgramPixel] = 0;
            FixedVector<RndTextureBase*, 8> targets;
            targets.push_back(frame.mGBufferColor);
            targets.push_back(frame.mGBufferPixelNormals);
            if (config->mUseGBufferVertexNormals) {
                targets.push_back(frame.mGBufferVertexNormals);
            }
            SetColorTargets(context, targets, frame.mDepthStencil);
        }
        SetDeferredBlend(context, p);
        context._SetDepthModeImpl(firstPass ? 1 : 2);
        for (unsigned long begin = 0; begin < bucket.size(); begin += 256) {
            RndDrawInstance** first = bucket.begin() + begin;
            RndDrawInstance** last = begin + 256 < bucket.size() ? first + 256 : bucket.end();
            eastl::sort(first, last, StateCompare{true});
            _FlushBucket<kFlushGBuffer>(context, internal, target, bucket, begin, begin + 256);
        }
        context._SetStencilModeImpl(0, 0, 0, 0);
        context._SetColorWriteMaskImpl(0xF, static_cast<RndWriteMaskChannelSet>(0));
        context.SetShadingMode(kShadingModeStandard);
    }
    if (firstPass && !IsDebugShadingMode(p.mShadingMode)) {
        _InitLightAccum(context, internal, target);
    }
}

// Reconstructed from eboot.elf at 0x422D60. The emissive deferred bucket
// draws into the light accumulation and the G-buffer; bucket 5 before the
// lighting writes the depth, bucket 8 after it tests it.
void RndSceneDrawer::_DrawOpaqueDeferredEmissive(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Deferred Emissive"));
    const bool firstPass = bucketIndex == 5;
    const RndShadingMode shading =
        firstPass ? kShadingModeDeferredLitEmissiveAndZFill : kShadingModeDeferredLitEmissive;
    const RndSceneDrawParams& p = internal.mParams;
    const RndConfig* config = gRndDevice->mSettings;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
    if (IsDebugShadingMode(p.mShadingMode)) {
        context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), shading);
        FixedVector<RndTextureBase*, 2> targets;
        targets.push_back(LightAccum(buffers, target));
        if (config->mUseGBufferVertexNormals) {
            targets.push_back(frame.mGBufferVertexNormals);
        }
        SetColorTargets(context, targets, frame.mDepthStencil);
    } else {
        context.SetShadingMode(shading);
        FixedVector<RndTextureBase*, 8> targets;
        targets.push_back(LightAccum(buffers, target));
        targets.push_back(frame.mGBufferColor);
        targets.push_back(frame.mGBufferPixelNormals);
        if (config->mUseGBufferVertexNormals) {
            targets.push_back(frame.mGBufferVertexNormals);
        }
        SetColorTargets(context, targets, frame.mDepthStencil);
    }
    SortBucket(bucket, StateCompare{true});
    SetDeferredBlend(context, p);
    context._SetDepthModeImpl(firstPass ? 1 : 2);
    _FlushBucket<kFlushGBuffer>(context, internal, target, bucket, 0, bucket.size());
    context._SetStencilModeImpl(0, 0, 0, 0);
    context._SetColorWriteMaskImpl(0xF, static_cast<RndWriteMaskChannelSet>(0));
    context.SetShadingMode(kShadingModeStandard);
}

// Reconstructed from eboot.elf at 0x423220. The unlit deferred bucket draws
// into the light accumulation; bucket 6 before the lighting writes the
// depth, bucket 9 after it tests it.
void RndSceneDrawer::_DrawOpaqueDeferredUnlit(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    if (bucket.empty()) {
        return;
    }
    const bool firstPass = bucketIndex == 6;
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Deferred Unlit"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    SortBucket(bucket, StateCompare{true});
    const RndShadingMode shading =
        firstPass ? kShadingModeDeferredUnlitAndZFill : kShadingModeDeferredUnlit;
    if (IsDebugShadingMode(p.mShadingMode)) {
        context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), shading);
    } else {
        context.SetShadingMode(shading);
    }
    SetDeferredBlend(context, p);
    context._SetDepthModeImpl(firstPass ? 1 : 2);
    _FlushBucket<kFlushGBufferUnlit>(context, internal, target, bucket, 0, bucket.size());
    context._SetStencilModeImpl(0, 0, 0, 0);
    context.SetShadingMode(kShadingModeStandard);
}

// Reconstructed from eboot.elf at 0x423AF0. The lit opaque decals (bucket
// 10) draw into the light accumulation and the G-buffer, testing the depth
// without writing it.
void RndSceneDrawer::_DrawDecalsLitOpaque(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Decals Lit Opaque"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
    if (IsDebugShadingMode(p.mShadingMode)) {
        context.SetShaderDebugMode(
            static_cast<RndUserShadingMode>(p.mShadingMode), kShadingModeDeferredLitEmissive);
        context.SetRenderTargets(LightAccum(buffers, target), frame.mDepthStencil);
    } else {
        context.SetShadingMode(kShadingModeDeferredLitEmissive);
        FixedVector<RndTextureBase*, 8> targets;
        targets.push_back(LightAccum(buffers, target));
        targets.push_back(frame.mGBufferColor);
        targets.push_back(frame.mGBufferPixelNormals);
        context.SetRenderTargets(VectorAdapter<RndTextureBase*>{targets.mData, targets.size()}, frame.mDepthStencil);
    }
    SortBucket(bucket, StateCompare{true});
    context.mBlendMode = static_cast<RndBlendMode>(5);
    context._SetBlendModeImpl(static_cast<RndBlendMode>(5), Hmx::Color::GetWhite());
    context._SetDepthModeImpl(3);
    _FlushBucket<kFlushDecal>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x423E40. The lit transparent decals
// (bucket 11) blend into the light accumulation and the G-buffer's color
// and normals without writing their alpha. The vertex normals move to a
// shader resource first, and afterwards the color and pixel normals too.
void RndSceneDrawer::_DrawDecalsLitTransparent(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
    const RndResourceState after = gRndDevice->mSettings->mUseTiledLighting
        ? kStateShaderResource
        : RndResourceState::kPixelShaderResource;
    FixedVector<RndResourceBarrier, 3> barriers;
    auto addBarrier = [&](RndTextureBase* texture) {
        RndResourceBarrier barrier;
        barrier.mResource = texture;
        barrier.mSubresource = ~0UL;
        barrier.mBefore = RndResourceState::kRenderTarget;
        barrier.mAfter = after;
        barriers.push_back(barrier);
    };
    if (frame.mGBufferVertexNormals != nullptr) {
        addBarrier(frame.mGBufferVertexNormals);
    }
    if (!bucket.empty()) {
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        static Symbol sName;
        RndScopedGpuStatBlock statBlock(context, PassName(sName, "Decals Lit Transparent"));
        if (IsDebugShadingMode(p.mShadingMode)) {
            context.SetShaderDebugMode(
                static_cast<RndUserShadingMode>(p.mShadingMode), kShadingModeDeferredDecalTransparent);
            context.SetRenderTargets(LightAccum(buffers, target), frame.mDepthStencil);
        } else {
            context.SetShadingMode(kShadingModeDeferredDecalTransparent);
            FixedVector<RndTextureBase*, 8> targets;
            targets.push_back(LightAccum(buffers, target));
            targets.push_back(frame.mGBufferColor);
            targets.push_back(frame.mGBufferPixelNormals);
            context.SetRenderTargets(
                VectorAdapter<RndTextureBase*>{targets.mData, targets.size()}, frame.mDepthStencil);
        }
        SortBucket(bucket, StateCompare{true});
        context._SetDepthModeImpl(3);
        context._SetColorWriteMaskImpl(0xF, kWriteNone);
        _FlushBucket<kFlushDecalBlended>(context, internal, target, bucket, 0, bucket.size());
        context.SetShadingMode(kShadingModeStandard);
        context._SetStencilModeImpl(0, 0, 0, 0);
        context._SetColorWriteMaskImpl(0xF, kWriteRGBA);
    }
    if (frame.mGBufferColor != nullptr) {
        addBarrier(frame.mGBufferColor);
    }
    if (frame.mGBufferPixelNormals != nullptr) {
        addBarrier(frame.mGBufferPixelNormals);
    }
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}

// Reconstructed from eboot.elf at 0x4242B0. The unlit decals (bucket 12)
// draw into the light accumulation through the debug-view setter, which
// selects the standard shading outside the debug views.
void RndSceneDrawer::_DrawDecalsUnlit(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Decals Unlit"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), kShadingModeStandard);
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    SortBucket(bucket, StateCompare{true});
    context._SetDepthModeImpl(3);
    _FlushBucket<kFlushDecalUnlit>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

namespace {

// The forward shading of the scene's atmosphere: fog, volumetric
// scattering or none. Name not in the reference map.
RndShadingMode AtmosphereShading(const RndSceneInternalContext& internal) {
    if (internal.mFog != nullptr) {
        return kShadingModeStandardFog;
    }
    return internal.mVolumetricScattering != nullptr ? kShadingModeStandardVScat : kShadingModeStandard;
}

}  // namespace

// Reconstructed from eboot.elf at 0x424470. The unlit forward bucket (14)
// draws into the light accumulation with the atmosphere's forward shading,
// writing the depth.
void RndSceneDrawer::_DrawOpaqueFwdUnlit(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Fwd Unlit"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    SortBucket(bucket, StateCompare{true});
    const RndShadingMode shading = AtmosphereShading(internal);
    if (IsDebugShadingMode(p.mShadingMode)) {
        context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), shading);
    } else {
        context.SetShadingMode(shading);
    }
    SetDeferredBlend(context, p);
    context._SetDepthModeImpl(1);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushForward>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x424710. The lit forward bucket (13)
// draws into the light accumulation with forward lighting, writing the
// depth. Shading modes 3, 16 and 17 draw opaque.
void RndSceneDrawer::_DrawOpaqueFwdLit(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Fwd Lit"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    SortBucket(bucket, StateCompare{true});
    context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), kShadingModeFwdLitOpaque);
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    const unsigned int mode = static_cast<unsigned int>(p.mShadingMode) - 3;
    const RndBlendMode blend =
        static_cast<RndBlendMode>(mode <= 14 && ((0x6001U >> mode) & 1) != 0 ? 6 : 5);
    context.mBlendMode = blend;
    context._SetBlendModeImpl(blend, Hmx::Color::GetWhite());
    context._SetDepthModeImpl(1);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushForward>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x424980. A transparent bucket (15, 17,
// 19 or 20) is drawn back to front over the target's depth; runs of
// instances with the same order-independent blend mode (above 3) are
// sorted by state inside the run so that they batch.
void RndSceneDrawer::_DrawTransparent(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Transparent"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    RndTextureBase* const* depthTargets = &ActiveFrame(buffers).mDepthStencil;
    context.SetRenderTargets(LightAccum(buffers, target), depthTargets[target.mDepthTargetIndex]);
    SortBucket(bucket, DistanceCompareBackToFront);
    RndDrawInstance** runStart = nullptr;
    int runBlend = -1;
    for (RndDrawInstance** it = bucket.begin(); it != bucket.end(); ++it) {
        const RndMaterialRuntimeData* material = (*it)->mMaterial;
        const int blend = material != nullptr ? material->mBlendMode : -1;
        if (blend == runBlend) {
            continue;
        }
        if (runBlend != -1) {
            eastl::sort(runStart, it, StateCompare{false});
        }
        if (blend > 3) {
            runStart = it;
            runBlend = blend;
        } else {
            runBlend = -1;
        }
    }
    if (runBlend != -1) {
        eastl::sort(runStart, bucket.end(), StateCompare{false});
    }
    context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), AtmosphereShading(internal));
    context._SetDepthModeImpl(3);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushTransparent>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x424C20. Transparent-material instances
// drawn as opaque (bucket 16): sorted by state, testing the depth.
void RndSceneDrawer::_DrawOpaqueTransparent(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Opaque Transparent"));
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    SortBucket(bucket, StateCompare{true});
    context.SetShaderDebugMode(static_cast<RndUserShadingMode>(p.mShadingMode), AtmosphereShading(internal));
    context._SetDepthModeImpl(3);
    context._SetStencilModeImpl(2, 0, 6, 0);
    _FlushBucket<kFlushTransparent>(context, internal, target, bucket, 0, bucket.size());
    context.SetShadingMode(kShadingModeStandard);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x425E50. The overlays (bucket 21) draw
// last into the light accumulation, unsorted and with the standard
// shading.
void RndSceneDrawer::_DrawOverlays(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long bucketIndex,
    PodVector<RndDrawInstance*>& bucket) const {
    static_cast<void>(bucketIndex);
    if (bucket.empty()) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Overlays"));
    RndBufferCollection& buffers = *internal.mParams.mBuffers[target.mBuffersIndex];
    context.SetRenderTargets(LightAccum(buffers, target), ActiveFrame(buffers).mDepthStencil);
    context.SetShadingMode(kShadingModeStandard);
    _FlushBucket<kFlushBasic>(context, internal, target, bucket, 0, bucket.size());
}

// Reconstructed from eboot.elf at 0x427040. The depth-only draw renders
// into the collection's tiled scene mask (the first of mTiledSceneMask),
// whose size rounds the target up to whole tiles: the projection rectangle
// is scaled into the covered part, and the scene mask is cleared on
// request.
void RndSceneDrawer::_SetDepthOnlyTargets(
    RndContext& context,
    RndSceneInternalContext& internal,
    unsigned long target,
    bool clear) const {
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target];
    context.SetCamera(p.mCamera);
    RndTextureBase* mask = buffers.mTiledSceneMask[0];
    const int tileWidth = static_cast<int>(mask->mBaseDesc.mWidth);
    const int tileHeight = static_cast<int>(mask->mBaseDesc.mHeight);
    const int width = buffers.mSize.x;
    const int height = buffers.mSize.y;
    const int paddedWidth = (width / tileWidth + (width / tileWidth * tileWidth < width ? 1 : 0)) * tileWidth;
    const int paddedHeight = (height / tileHeight + (height / tileHeight * tileHeight < height ? 1 : 0)) * tileHeight;
    const float invHeight = 1.0F / static_cast<float>(paddedHeight);
    const float xScale = static_cast<float>(width) / static_cast<float>(paddedWidth);
    const float yScale = static_cast<float>(height) * invHeight;
    const Hmx::Rect rect(
        xScale * p.mProjectionRect.x,
        static_cast<float>(paddedHeight - height) * invHeight + yScale * p.mProjectionRect.y,
        xScale * p.mProjectionRect.w,
        yScale * p.mProjectionRect.h);
    context.SetProjectionRect(rect);
    RndContext::RenderTargetParams targets;
    RndContext::RenderTargetParams::Target color;
    color.mTexture = mask;
    color.mClearMode = clear;
    targets.mTargets.push_back(color);
    context.SetRenderTargets(targets);
    context.mBlendMode = static_cast<RndBlendMode>(9);
    context._SetBlendModeImpl(static_cast<RndBlendMode>(9), Hmx::Color::GetWhite());
}

// Reconstructed from eboot.elf at 0x41E090. Draws the shadow casters'
// depth (state flag 8, bucket 3) of the drawer's own camera set into the
// tiled scene mask in the scene-mask shading.
void RndSceneDrawer::_DrawDepthOnly(
    RndContext& context,
    RndSceneInternalContext& internal,
    unsigned long target) const {
    mDynamicGpuDataMgr->ProcessQueue(context);
    VectorAdapter<PodVector<RndDrawInstance>> instances = Adapter(mDrawInstances[0]);
    VectorAdapter<PodVector<RndDrawInstance*>> sortable = Adapter(mSortableInstances[0]);
    RndCullerParams params;
    params.mNoCulling = internal.mSceneCom != nullptr && internal.mSceneCom->mNeverCull;
    params.mStateFlagMask = 8;
    PodVector<RndDrawInstance>* buckets = Buckets(instances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        buckets[bucket].clear();
    }
    const RndCameraContext& camera = internal.mCameras[target].mCamera;
    if (!params.mNoCulling) {
        for (const Vector4& plane : camera.mDerivedCache) {
            params.mClipPlanes.push_back(plane);
        }
    }
    mCuller->Cull(
        context.mSceneDrawId,
        *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
        camera,
        internal.mParams.mShowHide,
        instances,
        params);
    _FillSortableBuckets(instances, sortable);
    _SetDepthOnlyTargets(context, internal, target, internal.mParams.mClear);
    const RndShadingMode shadingMode = context.mShadingMode;
    context.SetShadingMode(kShadingModeSceneMask);
    PodVector<RndDrawInstance*>& casters = Buckets(sortable)[3];
    SortBucket(casters, StateCompare{true});
    RndSceneDrawTarget drawTarget;
    drawTarget.mBuffersIndex = target;
    if (!casters.empty()) {
        _FlushBucket<kFlushBasic>(context, internal, drawTarget, casters, 0, casters.size());
    }
    context.SetShadingMode(shadingMode);
}

// Reconstructed from eboot.elf at 0x420740. With the scene not cleared and
// materials reading the scene texture at level 0, the old scene is
// captured first. A camera with instances clears the depth and stencil;
// then come the scene mask, the camera, the prepasses, the deferred
// geometry before the lighting (buckets 0, 1, 4, 5 and 6) and the linear
// depth.
void RndSceneDrawer::_DrawPostCull(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) const {
    const unsigned long index = target.mBuffersIndex;
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[index];
    RndSceneInternalContext::CameraData& camera = internal.mCameras[index];
    if (!p.mClear && camera.mSceneTexUsage == 0) {
        _CaptureSceneTex(context, internal, target);
    }
    if (camera.mHasInstances) {
        static Symbol sName;
        RndScopedGpuStatBlock statBlock(context, PassName(sName, "Clear Depth/Stencil"));
        RndContext::RenderTargetParams targets;
        targets.mDepthTexture = ActiveFrame(buffers).mDepthStencil;
        targets.mDepthClearMode = 1;
        context.SetRenderTargets(targets);
    }
    if (p.mDrawSceneMask) {
        _DrawSceneMask(context, internal, index);
    }
    context.SetCamera(p.mCamera);
    context.SetProjectionRect(p.mProjectionRect);
    PodVector<RndDrawInstance*>* sortable = Buckets(camera.mSortableInstances);
    _DrawZPrepass(context, internal, target, 0, sortable[0]);
    _DrawNormalsZPrepass(context, internal, target, 1, sortable[1]);
    _DrawOpaqueDeferredLit(context, internal, target, 4, sortable[4]);
    _DrawOpaqueDeferredEmissive(context, internal, target, 5, sortable[5]);
    _DrawOpaqueDeferredUnlit(context, internal, target, 6, sortable[6]);
    if (camera.mNeedsLinearDepth) {
        _GenerateLinearDepth(context, internal, index);
    }
}

// Reconstructed from eboot.elf at 0x41F0C0. The first interval of a scene
// draw: refreshes the dynamic GPU data and the property hysteresis
// texture, culls the collection's camera, analyses the buckets, submits the
// occlusion queries and draws the passes up to the lighting. When the
// buckets need lights, they are culled (the tiled culling on the compute
// pipe, signalling fence 9) and the shadow maps are drawn; then the passes
// after the light culling run.
void RndSceneDrawer::_DrawInterval0(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) {
    const unsigned long index = target.mBuffersIndex;
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[index];
    RndSceneInternalContext::CameraData& camera = internal.mCameras[index];
    mDynamicGpuDataMgr->ProcessQueue(context);
    if (internal.mSceneCom != nullptr) {
        internal.mSceneCom->UpdatePropHysteresisTexture(internal.mEntity, context);
    }
    RndCullerParams params;
    params.mNoCulling = internal.mSceneCom != nullptr && internal.mSceneCom->mNeverCull;
    _Cull(
        context.mSceneDrawId,
        *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
        camera.mCamera,
        p.mShowHide,
        camera.mDrawInstances,
        camera.mSortableInstances,
        params);
    _AnalyzeCullResults(internal, index);
    mOcclusionQueryMgr->Submit(camera.mCamera);
    _DrawPostCull(context, internal, target);
    if (camera.mNeedsLightCulling) {
        RndLightMgrCom* lightMgr = internal.mLightMgr;
        lightMgr->CullLights(
            context.mSceneDrawId,
            *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
            camera.mCamera,
            p);
        const RndPipeline pipeline = static_cast<RndPipeline>(context.mActivePipe);
        const unsigned long slot = context.mActiveComputeSlot;
        context.SetActivePipeline(kPipelineCompute, 0);
        lightMgr->_CullTiledLights(context, camera.mCamera, p, buffers, nullptr);
        if (mFences[9] != nullptr) {
            context._SignalFenceImpl(*mFences[9]);
        }
        context.SetActivePipeline(pipeline, slot);
        if (p.mDrawShadows) {
            VectorAdapter<PodVector<RndDrawInstance>> instances = Adapter(mDrawInstances[2]);
            VectorAdapter<PodVector<RndDrawInstance*>> sortable = Adapter(mSortableInstances[2]);
            for (unsigned long i = 0; i < lightMgr->mCullResults.mShadowLights.size(); ++i) {
                lightMgr->DrawShadowMaps(
                    context, *this, internal.mCullCamera, p.mShowHide, instances, sortable, i);
            }
        }
    }
    _DrawPostLightCull(context, internal, target);
}

// Reconstructed from eboot.elf at 0x41F590. The second interval: the
// deferred lighting (or, in the debug views, the partial-framerate copy of
// the first interval's scene), the atmosphere, the forward and transparent
// passes with the scene texture captures their materials ask for, the
// mask buffer and the two post-processing chains between the transparent
// buckets, the antialiasing (up to 1920x1080), the debug display and the
// overlays.
void RndSceneDrawer::_DrawInterval1(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) {
    const unsigned long index = target.mBuffersIndex;
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[index];
    RndBufferCollection::FrameIntervalBuffers& frame = ActiveFrame(buffers);
    RndSceneInternalContext::CameraData& camera = internal.mCameras[index];
    context.SetCamera(p.mCamera);
    context.SetProjectionRect(p.mProjectionRect);

    if (camera.mNeedsLightCulling && !IsDebugShadingMode(p.mShadingMode)) {
        RndTextureBase* source =
            internal.mPartialFramerate ? frame.mPartialLightAccum : buffers.mLightAccum[target.mSrcLightAccum];
        internal.mLightMgr->AccumDeferredLight(
            context, camera.mCamera, p, buffers, target, source, mFences[8]);
    } else {
        if (internal.mPartialFramerate) {
            // The first interval's scene is copied back.
            RndTextureBase* scene = buffers.mLightAccum[target.mSrcLightAccum];
            FixedVector<RndResourceBarrier, 2> barriers;
            barriers.resize(2);
            barriers[0].mResource = scene;
            barriers[0].mSubresource = ~0UL;
            barriers[0].mBefore = kStateShaderResource;
            barriers[0].mAfter = RndResourceState::kCopyDestination;
            barriers[1].mResource = frame.mPartialLightAccum;
            barriers[1].mSubresource = ~0UL;
            barriers[1].mBefore = RndResourceState::kRenderTarget;
            barriers[1].mAfter = kStateCopySource;
            context._ResourceBarrierImpl(barriers.size(), barriers.mData);
            {
                static Symbol sName;
                RndScopedGpuStatBlock statBlock(context, PassName(sName, "Copy Interval0 Framebuffer"));
                scene->GpuCopyFrom(context, *frame.mPartialLightAccum);
            }
            barriers[0].mBefore = RndResourceState::kCopyDestination;
            barriers[0].mAfter = RndResourceState::kRenderTarget;
            barriers[1].mBefore = kStateCopySource;
            barriers[1].mAfter = kStateShaderResource;
            context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        }
        if (camera.mNeedsLightCulling && mFences[8] != nullptr) {
            context._WaitFenceImpl(*mFences[8]);
        }
    }

    if (internal.mFog != nullptr) {
        RndTextureBase* texture =
            internal.mSky != nullptr ? internal.mSky->GetAtmosphereTexture(buffers) : nullptr;
        internal.mFog->ApplyDeferred(context, buffers, target, texture);
    } else if (internal.mVolumetricScattering != nullptr) {
        if (mFences[11] != nullptr) {
            context._WaitFenceImpl(*mFences[11]);
        }
        internal.mVolumetricScattering->EndAsyncUpdate(context, camera.mCamera, buffers, target);
        internal.mVolumetricScattering->ApplyDeferred(context, camera.mCamera, buffers, target, p.mDrawSceneMask);
    }

    PodVector<RndDrawInstance*>* sortable = Buckets(camera.mSortableInstances);
    if (camera.mSceneTexUsage == 1) {
        _CaptureSceneTex(context, internal, target);
    }
    _DrawOpaqueFwdUnlit(context, internal, target, 14, sortable[14]);
    _DrawOpaqueFwdLit(context, internal, target, 13, sortable[13]);
    mOcclusionQueryMgr->DrawQueries(context, *frame.mDepthStencil);
    _DrawTransparent(context, internal, target, 15, sortable[15]);
    _DrawZPrepass(context, internal, target, 16, sortable[16]);
    _DrawOpaqueTransparent(context, internal, target, 16, sortable[16]);
    if (camera.mSceneTexUsage == 2) {
        _CaptureSceneTex(context, internal, target);
    }
    _DrawTransparent(context, internal, target, 17, sortable[17]);
    _DrawMaskBuffer(context, internal, target, 18, sortable[18]);
    _DrawPostProc(context, internal, target, 0);
    _DrawTransparent(context, internal, target, 19, sortable[19]);
    _DrawPostProc(context, internal, target, 1);
    _DrawTransparent(context, internal, target, 20, sortable[20]);
    target.mDepthTargetIndex = 0;
    if (internal.mAntialiasing != nullptr && buffers.mSize.x <= 1920 && buffers.mSize.y <= 1080) {
        internal.mAntialiasing->Draw(context, buffers, target);
    }
    if (p.mOutputToBackBuffer) {
        _DrawShadingModeDisplay(context, internal, target);
    }
    _DrawOverlays(context, internal, target, 21, sortable[21]);
    context.SetShadingMode(kShadingModeStandard);
}

// Reconstructed from eboot.elf at 0x420AE0. Updates the sky texture, then
// draws the lit deferred buckets after the light culling (7, 8 and 9) and
// the decals (10, 11 and 12) on the graphics pipe, waiting for fence 6 and
// signalling fence 7 between them; the ambient occlusion runs on the
// compute pipe after fence 7 and signals fence 8. The volumetric
// scattering starts on compute slot 3 after fences 10 and 9 and signals
// fence 11.
void RndSceneDrawer::_DrawPostLightCull(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) const {
    const unsigned long index = target.mBuffersIndex;
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[index];
    RndSceneInternalContext::CameraData& camera = internal.mCameras[index];
    context.SetCamera(p.mCamera);
    context.SetProjectionRect(p.mProjectionRect);
    RndTextureBase* skyTexture = nullptr;
    if (internal.mSky != nullptr) {
        const BatchContext batch = _BuildBatchContext(internal, target);
        internal.mSky->UpdateTexture(context, batch);
        skyTexture = internal.mSky->GetAtmosphereTexture(buffers);
    }
    if (mFences[10] != nullptr) {
        context._SignalFenceImpl(*mFences[10]);
    }
    if (mFences[6] != nullptr) {
        context._WaitFenceImpl(*mFences[6]);
    }
    PodVector<RndDrawInstance*>* sortable = Buckets(camera.mSortableInstances);
    _DrawOpaqueDeferredLit(context, internal, target, 7, sortable[7]);
    _DrawOpaqueDeferredEmissive(context, internal, target, 8, sortable[8]);
    _DrawOpaqueDeferredUnlit(context, internal, target, 9, sortable[9]);
    _DrawDecalsLitOpaque(context, internal, target, 10, sortable[10]);
    _DrawDecalsLitTransparent(context, internal, target, 11, sortable[11]);
    if (mFences[7] != nullptr) {
        context._SignalFenceImpl(*mFences[7]);
    }
    _DrawDecalsUnlit(context, internal, target, 12, sortable[12]);
    if (camera.mNeedsLightCulling) {
        const RndPipeline pipeline = static_cast<RndPipeline>(context.mActivePipe);
        const unsigned long slot = context.mActiveComputeSlot;
        context.SetActivePipeline(kPipelineCompute, 0);
        if (mFences[7] != nullptr) {
            context._WaitFenceImpl(*mFences[7]);
        }
        internal.mLightMgr->GenerateAmbientOcclusion(context, buffers, camera.mCamera, p);
        if (mFences[8] != nullptr) {
            context._SignalFenceImpl(*mFences[8]);
        }
        context.SetActivePipeline(pipeline, slot);
    }
    if (internal.mVolumetricScattering != nullptr) {
        const RndCameraContext* stereoCamera = nullptr;
        RndBufferCollection* stereoBuffers = nullptr;
        if (internal.mStereoOptimized) {
            stereoCamera = &internal.mCullCamera;
            stereoBuffers = p.mBuffers[0];
        }
        context.SetRenderTargets(nullptr, nullptr);
        const RndPipeline pipeline = static_cast<RndPipeline>(context.mActivePipe);
        const unsigned long slot = context.mActiveComputeSlot;
        context.SetActivePipeline(kPipelineCompute, 3);
        if (mFences[10] != nullptr) {
            context._WaitFenceImpl(*mFences[10]);
        }
        if (mFences[9] != nullptr) {
            context._WaitFenceImpl(*mFences[9]);
        }
        internal.mVolumetricScattering->BeginAsyncUpdate(
            context,
            camera.mCamera,
            stereoCamera,
            buffers,
            stereoBuffers,
            internal.mLightMgr,
            skyTexture,
            p.mDrawSceneMask);
        if (mFences[11] != nullptr) {
            context._SignalFenceImpl(*mFences[11]);
        }
        context.SetActivePipeline(pipeline, slot);
    }
}

// Reconstructed from eboot.elf at 0x41D130. Culls with the light's show
// and hide flags (the hide flags are the low seven bits of the combined
// flags, the show flags the next five) from the context's first camera and
// its extra clip planes, and flushes the sorted shadow casters (bucket 2)
// depth-only with a default context and target.
void RndSceneDrawer::DrawShadowDepth(
    RndContext& context,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    bool flaggedCastersOnly,
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable) {
    static_cast<void>(camera);
    const unsigned int flags = (flaggedCastersOnly ? 0x100U : 8U) | showHide.mShowFlags | showHide.mHideFlags;
    const RndShowHideContext lightShowHide{flags & 0xF80, flags & 0x7F};
    RndCullerParams params;
    PodVector<RndDrawInstance>* buckets = Buckets(instances);
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        buckets[bucket].clear();
    }
    const RndCameraContext& view = context.mCameras[0];
    for (const Vector4& plane : view.mDerivedCache) {
        params.mClipPlanes.push_back(plane);
    }
    mCuller->Cull(
        context.mSceneDrawId,
        *reinterpret_cast<unsigned int*>(context.mSceneDrawId),
        view,
        lightShowHide,
        instances,
        params);
    _FillSortableBuckets(instances, sortable);
    context.SetShadingMode(kShadingModeDepthOnly);
    context._SetDepthModeImpl(1);
    PodVector<RndDrawInstance*>& casters = Buckets(sortable)[2];
    SortBucket(casters, DepthOnlyStateCompare);
    RndSceneInternalContext internal;
    RndSceneDrawTarget target;
    if (!casters.empty()) {
        _FlushBucket<kFlushDepth>(context, internal, target, casters, 0, casters.size());
    }
    context.SetShadingMode(kShadingModeStandard);
}

// Reconstructed from eboot.elf at 0x41FC30. For draws that output to the
// back buffer, converts the target's light accumulation into the
// collection's back buffer with a full-screen quad, encoding for HDR
// output (RndDevice::mHdrOutputMode 1) with the perceptual quantizer.
void RndSceneDrawer::_DrawOutputConversion(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) const {
    if (!internal.mParams.mOutputToBackBuffer) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Output Conversion"));
    const RndResourceState shaderResource = gRndDevice->mSettings->mUseTiledLighting
        ? kStateShaderResource
        : RndResourceState::kPixelShaderResource;
    RndBufferCollection& buffers = *internal.mParams.mBuffers[target.mBuffersIndex];
    RndTextureBase* scene = buffers.mLightAccum[target.mSrcLightAccum];
    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.resize(2);
    barriers[0].mResource = buffers.mBackBuffer;
    barriers[0].mSubresource = ~0UL;
    barriers[0].mBefore = static_cast<RndResourceState>(0);
    barriers[0].mAfter = RndResourceState::kRenderTarget;
    barriers[1].mResource = scene;
    barriers[1].mSubresource = ~0UL;
    barriers[1].mBefore = RndResourceState::kRenderTarget;
    barriers[1].mAfter = shaderResource;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndContext::RenderTargetParams targets;
    RndContext::RenderTargetParams::Target backBuffer;
    backBuffer.mTexture = buffers.mBackBuffer;
    backBuffer.mClearMode = 2;
    targets.mTargets.push_back(backBuffer);
    context.SetRenderTargets(targets);

    RndShaderOutputConversion::Params params{};
    params.mSource = scene;
    if (gRndDevice->mHdrOutputMode == 1) {
        params.mPerceptualQuantizer = true;
    }
    gRndDevice->mShaderMgr.mOutputConversionShader->Select(context, params);
    RndDrawUtl::Quad2DParams quad;
    quad.mBlendMode = static_cast<RndBlendMode>(5);
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);
}

// Reconstructed from eboot.elf at 0x425510. Post-processing chain `index`
// (0 or 1): the light manager tonemaps before the chain it names (SDR
// output only), the scene's chain draws with the target's batch context
// and hands back the updated target, and when the scene clears the depth
// after the chain and a later transparent bucket (19 + index to 21) has
// instances, the target's depth and stencil are cleared.
void RndSceneDrawer::_DrawPostProc(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target,
    unsigned long index) const {
    const RndSceneDrawParams& p = internal.mParams;
    RndBufferCollection& buffers = *p.mBuffers[target.mBuffersIndex];
    RndSceneInternalContext::CameraData& camera = internal.mCameras[target.mBuffersIndex];
    RndLightMgrCom* lightMgr = internal.mLightMgr;
    if (static_cast<unsigned long>(lightMgr->mTonemapping) == index + 1 && p.mDrawPostProc
        && gRndDevice->mHdrOutputMode != 1) {
        lightMgr->TonemapScene(context, buffers, target);
    }
    if (RndPostProcCom* postProc = internal.mPostProc[index]) {
        static Symbol sName;
        RndScopedGpuStatBlock statBlock(context, PassName(sName, "PostProc"));
        BatchContext batch = _BuildBatchContext(internal, target);
        postProc->Draw(context, camera, p, batch);
        target = batch.mTarget;
    }
    if (!camera.mHasInstances || internal.mSceneCom == nullptr
        || !internal.mSceneCom->mClearDepthAfterPostProc[index]) {
        return;
    }
    PodVector<RndDrawInstance*>* sortable = Buckets(camera.mSortableInstances);
    for (unsigned long bucket = index + 19; bucket < kNumDrawBuckets; ++bucket) {
        if (!sortable[bucket].empty()) {
            static Symbol sClearName;
            RndScopedGpuStatBlock statBlock(context, PassName(sClearName, "Clear Depth/Stencil"));
            RndContext::RenderTargetParams targets;
            targets.mDepthTexture = (&ActiveFrame(buffers).mDepthStencil)[target.mDepthTargetIndex];
            targets.mDepthClearMode = 1;
            context.SetRenderTargets(targets);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x425A00. The overdraw and solid-color
// debug views (shading modes 3, 4, 5, 16 and 17) are shown by drawing the
// source light buffer through the display shader into the destination
// one; the target's light buffers then swap.
void RndSceneDrawer::_DrawShadingModeDisplay(
    RndContext& context,
    RndSceneInternalContext& internal,
    RndSceneDrawTarget& target) const {
    const int mode = internal.mParams.mShadingMode;
    const unsigned int index = static_cast<unsigned int>(mode) - 3;
    if (!((index <= 14 && ((0x6001U >> index) & 1) != 0) || (mode & ~1) == 4)) {
        return;
    }
    static Symbol sName;
    RndScopedGpuStatBlock statBlock(context, PassName(sName, "Display Shading Mode"));
    RndBufferCollection& buffers = *internal.mParams.mBuffers[target.mBuffersIndex];
    context.SetShaderDebugMode(static_cast<RndUserShadingMode>(mode), kShadingModeStandard);
    context._DeselectAllReadWriteTexturesImpl(1U << kShaderProgramPixel);
    context.mInputSlotLimits[kShaderProgramPixel] = 0;
    const RndResourceState shaderResource = gRndDevice->mSettings->mUseTiledLighting
        ? kStateShaderResource
        : RndResourceState::kPixelShaderResource;
    auto accum = [&](unsigned long light) {
        return buffers.mActiveSceneContext != 0 ? buffers.mLightAccum[light]
                                                 : ActiveFrame(buffers).mPartialLightAccum;
    };
    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.resize(2);
    barriers[0].mResource = accum(target.mSrcLightAccum);
    barriers[0].mSubresource = ~0UL;
    barriers[0].mBefore = RndResourceState::kRenderTarget;
    barriers[0].mAfter = shaderResource;
    barriers[1].mResource = accum(target.mDstLightAccum);
    barriers[1].mSubresource = ~0UL;
    barriers[1].mBefore = shaderResource;
    barriers[1].mAfter = RndResourceState::kRenderTarget;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    RndContext::RenderTargetParams targets;
    RndContext::RenderTargetParams::Target destination;
    destination.mTexture = accum(target.mDstLightAccum);
    destination.mClearMode = 2;
    targets.mTargets.push_back(destination);
    context.SetRenderTargets(targets);
    gRndDevice->mShaderMgr.mDisplayShadingModeShader->Select(context, buffers, target);
    RndDrawUtl::Quad2DParams quad;
    quad.mBlendMode = static_cast<RndBlendMode>(5);
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);
    const unsigned long source = target.mSrcLightAccum;
    target.mSrcLightAccum = target.mDstLightAccum;
    target.mDstLightAccum = source;
    target.mResult = nullptr;
    context.SetShadingMode(kShadingModeStandard);
}
