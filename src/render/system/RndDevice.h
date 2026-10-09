#pragma once

#include <cstddef>
#include <_pthread.h>

#include "os/threading/CritSec.h"
#include "render/core/platform/render_platform_config.h"
#include "render/debug/RndGpuStatsMgr.h"
#include "render/resources/lighting/render_lighting_resources.h"
#include "render/defaults/RndDefaults.h"
#include "render/shaders/RndShaderMgr.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"

class RndContext;
class RndFactory;
class RndBufferCollection;
class RndShaderCBuffer;
class RndShaderFogDeferred;

class RndWindow;

namespace rb4 {
struct AudioAnalysisTextureSet;
struct RenderPrimitiveMeshSet;
struct RenderSettings;
}  // namespace rb4

// Startup options copied into the device by Init. The game builds them in
// its startup code. Field names are not in the reference map.
struct RndInitParams {
    bool mUnknown0;
    bool mInitRendering;  // Load the default resources and shaders.
    bool mUnknown2;
    unsigned long mUnknown8;
};

static_assert(sizeof(RndInitParams) == 16);

// Per-material GPU data released at a frame boundary. Only the destructor is
// referenced here; the class has not been reconstructed.
class RndMaterialRuntimeData {
public:
    ~RndMaterialRuntimeData();  // 0x4F7580
};

// Value returned by vtable slot 15. Name not in the reference map.
struct RndDeviceSlot15Result {
    void* mUnknown0;
    void* mUnknown8;
    void* mUnknown16;
};

// Render device: owns the immediate and deferred contexts, the main window,
// frame timing, the per-platform settings, the default resources, and the
// pending-free queue. The vtable is at 0x18FF528; PS4Device derives from it.
class RndDevice {
public:
    enum ConsoleState : unsigned int {};

    RndDevice();             // 0x3DD410
    virtual ~RndDevice();    // 0x3DD790, 0x3DDAC0

    virtual void _ProcessDeferredDeletion();               // slot 2 at 0x3DEF50
    virtual void _InitImpl(const RndInitParams* params) = 0;  // slot 3
    // Called at the end of Init. Name not in the reference map.
    virtual void _PostInitImpl() = 0;                      // slot 4
    virtual void _TerminateImpl() = 0;                     // slot 5
    // The map's signature is _BeginFrameImpl().
    virtual void _BeginFrameImpl(bool offscreen) = 0;      // slot 6
    virtual void _EndFrameImpl(
        FixedVector<RndWindow*, 6>& windows,
        bool offscreen) = 0;                               // slot 7
    // Slots 8 to 10 and 14 to 16 are not in the reference map.
    virtual void _AcquireDeferredContextImpl(RndContext* context);  // slot 8 at 0x3DEF60
    virtual void _UnknownSlot9Impl();                      // slot 9 at 0x3DEF70
    virtual void _ReleaseDeferredContextImpl(RndContext* context);  // slot 10 at 0x3DEF80
    virtual void _ExecuteDeferredContextImpl(RndContext* context);  // slot 11 at 0x3DEF90
    // The map's signature is
    // _SetConsoleStateImpl(ConsoleState, ConsoleState).
    virtual void _SetConsoleStateImpl(ConsoleState state);  // slot 12 at 0x3DEFA0
    virtual int _GetGpuBlockingBehaviorImpl() const;       // slot 13 at 0x3DEFB0
    virtual int _UnknownSlot14Impl();                      // slot 14 at 0x3DEFC0
    virtual RndDeviceSlot15Result _UnknownSlot15Impl();    // slot 15 at 0x3DEFD0
    virtual void _UnknownSlot16Impl();                     // slot 16 at 0x3DEFF0

    // The map's signature is Init(RndInitParams*).
    void Init(const RndInitParams& params);                // 0x3DDAE0
    void Terminate();                                      // 0x3DDE60
    void Lock();                                           // 0x3DE080
    void Unlock();                                         // 0x3DE0B0
    void PollMainWindow();                                 // 0x3DE0E0
    bool BeginMainWindowFrame();                           // 0x3DE130
    void EndMainWindowFrame();                             // 0x3DE7C0
    void BeginOffscreenFrame(RndBufferCollection* buffers);  // 0x3DE8F0
    void EndOffscreenFrame();                              // 0x3DE9E0
    void ForceIncrementFrameCount();                       // 0x3DEAA0
    RndContext* AcquireDeferredContext(unsigned long index);  // 0x3DEB20
    // Name not in the reference map.
    void ReleaseDeferredContext(unsigned long index);      // 0x3DEB60
    // The map's parameter is RndContext&.
    void ExecuteDeferredContext(unsigned long index);      // 0x3DEB80
    ConsoleState GetConsoleState() const;                  // 0x3DEC00
    void SetConsoleState(ConsoleState state);              // 0x3DEC10
    void SyncFreeMaterialData(RndMaterialRuntimeData* data);  // 0x3DEC20
    void _InstallMainWindow(RndWindow* window);  // 0x3DED70
    void _DestroyMainWindow();                             // 0x3DED80
    void _InstallFactory(RndFactory* factory);             // 0x3DEDB0
    void _InstallImmediateContext(RndContext* context);    // 0x3DEDC0
    void _DestroyContexts();                               // 0x3DEEA0
    // Starts the frame Init or ExecuteDeferredContext left pending. Name not
    // in the reference map.
    void _FlushPendingBeginFrame();                        // 0x3DEF20

    // Name not in the reference map.
    void _InitBuiltinCBuffers();                           // 0x3DDC20
    void _ProcessPendingFrees();                           // 0x3DDFF0
    // The map's signature is _DoBeginFrame().
    void _DoBeginFrame(bool offscreen);                    // 0x3DE170
    // The map's parameter is RndWindow&.
    bool _DoBeginDrawingWindow(RndWindow& window);  // 0x3DE3A0
    void _DoEndFrame(bool offscreen);                      // 0x3DE4A0
    void _DoEndDrawingBufferCollection();                  // 0x3DE8E0

    // Field names are not in the reference map.
    CritSec mCritSec;
    ScePthread mLockOwner;
    bool mInitialized;
    RndInitParams mInitParams;
    RndContext* mImmediateContext;
    bool mBeginFramePending;
    unsigned int mBeginFrameFlags;
    eastl::vector<RndContext*> mDeferredContexts;
    unsigned int mHdrOutputMode;
    RndWindow* mMainWindow;
    RndWindow* mCurrentWindow;
    eastl::vector<RndBufferCollection*> mCurrentTargets;
    unsigned long mFrameCount;
    unsigned long mOffscreenFrameCount;
    bool mInFrame;
    bool mTerminating;
    FixedVector<RndWindow*, 6> mFrameWindows;
    unsigned long mLastFrameCycles;
    unsigned long mPendingFrameCycles;
    unsigned int mFrameTimerState;
    long mGpuTotalStat;
    float mFrameRate;
    float mSmoothedFrameRate;
    rb4::RenderSettings* mSettings;
    RndFactory* mFactory;
    rb4::RenderPlatformConfig mPlatformConfigs[13];
    RndDefaults mDefaults;
    RndShaderMgr mShaderMgr;
    rb4::RenderLightingResources mLighting;
    RndShaderFogDeferred* mFogDeferred;
    rb4::RenderPrimitiveMeshSet* mPrimitiveMeshes;
    rb4::AudioAnalysisTextureSet* mAudioTextures;
    RndGpuStatsMgr mGpuStats;
    RndShaderCBuffer* mBuiltinCBuffers[4];
    CritSec mPendingFreeCritSec;
    eastl::vector<RndMaterialRuntimeData*> mPendingFrees;
    void* mUnknown3792;
    ConsoleState mConsoleState;
};

static_assert(offsetof(RndDevice, mCritSec) == 8);
static_assert(offsetof(RndDevice, mLockOwner) == 24);
static_assert(offsetof(RndDevice, mInitialized) == 32);
static_assert(offsetof(RndDevice, mInitParams) == 40);
static_assert(offsetof(RndDevice, mImmediateContext) == 56);
static_assert(offsetof(RndDevice, mBeginFramePending) == 64);
static_assert(offsetof(RndDevice, mBeginFrameFlags) == 68);
static_assert(offsetof(RndDevice, mDeferredContexts) == 72);
static_assert(offsetof(RndDevice, mHdrOutputMode) == 104);
static_assert(offsetof(RndDevice, mMainWindow) == 112);
static_assert(offsetof(RndDevice, mCurrentWindow) == 120);
static_assert(offsetof(RndDevice, mCurrentTargets) == 128);
static_assert(offsetof(RndDevice, mFrameCount) == 160);
static_assert(offsetof(RndDevice, mOffscreenFrameCount) == 168);
static_assert(offsetof(RndDevice, mInFrame) == 176);
static_assert(offsetof(RndDevice, mTerminating) == 177);
static_assert(offsetof(RndDevice, mFrameWindows) == 184);
static_assert(offsetof(RndDevice, mLastFrameCycles) == 256);
static_assert(offsetof(RndDevice, mFrameTimerState) == 272);
static_assert(offsetof(RndDevice, mGpuTotalStat) == 280);
static_assert(offsetof(RndDevice, mFrameRate) == 288);
static_assert(offsetof(RndDevice, mSettings) == 296);
static_assert(offsetof(RndDevice, mFactory) == 304);
static_assert(offsetof(RndDevice, mPlatformConfigs) == 312);
static_assert(offsetof(RndDevice, mDefaults) == 1976);
static_assert(offsetof(RndDevice, mShaderMgr) == 2544);
static_assert(offsetof(RndDevice, mLighting) == 3256);
static_assert(offsetof(RndDevice, mFogDeferred) == 3560);
static_assert(offsetof(RndDevice, mPrimitiveMeshes) == 3568);
static_assert(offsetof(RndDevice, mAudioTextures) == 3576);
static_assert(offsetof(RndDevice, mGpuStats) == 3584);
static_assert(offsetof(RndDevice, mBuiltinCBuffers) == 3712);
static_assert(offsetof(RndDevice, mPendingFreeCritSec) == 3744);
static_assert(offsetof(RndDevice, mPendingFrees) == 3760);
static_assert(offsetof(RndDevice, mUnknown3792) == 3792);
static_assert(offsetof(RndDevice, mConsoleState) == 3800);

// The active device, set by the constructor and cleared by the destructor.
// Names not in the reference map.
extern RndDevice* gRndDevice;

inline RndDevice* TheRndDevice() {
    return gRndDevice;
}

namespace Rnd {
// Creates the platform device. Defined by each platform's Init object.
RndDevice* PlatformCreateDevice();
}  // namespace Rnd
