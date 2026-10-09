#pragma once

#include <cstddef>
#include <_pthread.h>

#include "os/threading/CritSec.h"
#include "render/debug/RndGpuStatsMgr.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/defaults/RndDefaults.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndCapabilities.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"

class RndContext;
class RndFactory;
class RndBufferCollection;
class RndConfig;
class RndMaterialRuntimeData;
class RndShaderCBuffer;
class RndShaderFogDeferred;

class RndWindow;
class RndAudioTextures;
class RndPrimitiveMeshes;

// Startup options copied into the device by Init. The game builds them in
// its startup code. Field names are not in the reference map.
struct RndInitParams {
    // Set by the startup code; nothing in this build reads it.
    bool mReservedOption;
    bool mInitRendering;  // Load the default resources and shaders.
    // Loads the shaders whose _GetLoadOption is 1, as mInitRendering does
    // for option 0; no shader in this build selects it.
    bool mInitOptionalShaders;
    // Zeroed by the startup code; nothing in this build reads it.
    unsigned long mReserved;
};

static_assert(sizeof(RndInitParams) == 16);

// The video memory in use, from vtable slot 15. The memory overlay shows
// both counts in thousands of bytes. Name not in the reference map; the
// field names are not either.
struct RndDeviceMemoryUsage {
    bool mValid;  // Clear when the device does not report its memory.
    long mLocalUsed;
    long mNonlocalUsed;
};

static_assert(offsetof(RndDeviceMemoryUsage, mLocalUsed) == 8);
static_assert(sizeof(RndDeviceMemoryUsage) == 24);

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
    // Slots 8, 10 and 14 to 16 are not in the reference map. Nothing in
    // this build calls slots 9, 14 or 16 and PS4Device gives slots 9 and
    // 16 no body, so their names are inferred. Slot 9 is taken to be the
    // map's ForceFlushResources(): a virtual whose base body is empty, that
    // the map emits ahead of _GetGpuBlockingBehaviorImpl, and the only
    // empty void slot before it.
    virtual void _AcquireDeferredContextImpl(RndContext* context);  // slot 8 at 0x3DEF60
    virtual void ForceFlushResources();                    // slot 9 at 0x3DEF70
    virtual void _ReleaseDeferredContextImpl(RndContext* context);  // slot 10 at 0x3DEF80
    virtual void _ExecuteDeferredContextImpl(RndContext* context);  // slot 11 at 0x3DEF90
    // The map's signature is
    // _SetConsoleStateImpl(ConsoleState, ConsoleState).
    virtual void _SetConsoleStateImpl(ConsoleState state);  // slot 12 at 0x3DEFA0
    virtual int _GetGpuBlockingBehaviorImpl() const;       // slot 13 at 0x3DEFB0
    // Returns 0 in the base and in PS4Device. Name not in the reference
    // map; inferred from its place between the GPU and memory queries.
    virtual int _GetDeviceStatusImpl();                    // slot 14 at 0x3DEFC0
    virtual RndDeviceMemoryUsage _GetMemoryUsageImpl();    // slot 15 at 0x3DEFD0
    // An empty hook. Name not in the reference map; a guess.
    virtual void _ResetDeviceStateImpl();                  // slot 16 at 0x3DEFF0

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
    RndConfig* mSettings;
    RndFactory* mFactory;
    RndCapabilities mCapabilities[13];
    RndDefaults mDefaults;
    RndShaderMgr mShaderMgr;
    RndLightGlobals mLighting;
    RndShaderFogDeferred* mFogDeferred;
    RndPrimitiveMeshes* mPrimitiveMeshes;
    RndAudioTextures* mAudioTextures;
    RndGpuStatsMgr mGpuStats;
    RndShaderCBuffer* mBuiltinCBuffers[4];
    CritSec mPendingFreeCritSec;
    eastl::vector<RndMaterialRuntimeData*> mPendingFrees;
    // The texture that material texture source 19 binds; the binder
    // (0x4FE850) falls back to the default error texture while it is
    // null, and nothing in this build sets it. Name not in the reference
    // map.
    void* mSourceTexture19;
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
static_assert(offsetof(RndDevice, mCapabilities) == 312);
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
static_assert(offsetof(RndDevice, mSourceTexture19) == 3792);
static_assert(offsetof(RndDevice, mConsoleState) == 3800);

// The active device, set by the constructor and cleared by the destructor.
// Names not in the reference map.
extern RndDevice* gRndDevice;

inline RndDevice* TheRndDevice() {
    return gRndDevice;
}
