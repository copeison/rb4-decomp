#pragma once

#include <atomic>
#include <cstddef>

#include "math/color/Color.h"
#include "math/geometry/Rect.h"
#include "os/threading/CritSec.h"
#include "render/context/RndCameraContext.h"
#include "render/drawing/PodVector.h"
#include "render/drawing/RndCuller.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/drawing/RndSceneInternalContext.h"
#include "render/lighting/RndLightMgrCom.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"
#include "utl/containers/VectorAdapter.h"
#include "utl/threading/PollDep.h"

class Entity;
class RndBufferCollection;
class RndCameraCom;
class RndContext;
class RndDrawInstanceCom;
class RndDynamicGpuDataMgr;
class RndFence;
class RndOcclusionQueryMgr;
class RndPostProcCom;
class RndSceneCom;
class RndSceneDrawer;
class RndShaderCBuffer;
class RndTexture2D;
class RndTextureBase;
struct RndInstanceData;

// The scene's draw job graph for the multithreaded draw: a 1168-byte
// object of PollDepBase jobs ("draw start", "draw init immed context",
// "draw cull", "draw cull analysis", "draw post-cull", "draw bucket", "draw
// light culling", "draw tiled light culling", "draw post-light-cull", "draw
// deferred light", "draw submit occ queries", "draw capture scenetex",
// "draw postproc", "draw sync dynamic gpu data", "draw end" and the
// per-shadowmap and per-collection jobs) that runs the passes of
// _DrawFullFramerate on worker threads. It is emitted in
// render/RndSceneDrawer.o (0x430610-0x4349DF) but is not reconstructed:
// only the entry points the drawer calls are declared. Name not in the
// reference map.
class RndSceneDrawJobs {
public:
    // Builds the graph for the drawer; the drawer passes 1.
    RndSceneDrawJobs(RndSceneDrawer* drawer, unsigned long numCollections);  // 0x430610
    ~RndSceneDrawJobs();  // 0x4314B0

    // Hooks the graph after `after`'s graph, after `startDep` and
    // `contextDep`, before `endAfter`, and appends its jobs to `jobs`.
    // Name not in the reference map.
    void Start(
        RndSceneInternalContext& context,
        RndSceneDrawJobs* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs);  // 0x4317E0
    // Unhooks the graph and returns the draw targets of its collection
    // jobs. Name not in the reference map.
    FixedVector<RndSceneDrawTarget, 2> Finish();  // 0x431970
};

// The buckets and culled lights a partial-framerate scene keeps from the
// first frame of an interval (808 bytes; RndSceneDrawer::
// mStoredCullResults). The light manager stores and restores its part
// (RndLightMgrCom::StoreCullResults, 0x484D80, and RestoreCullResults,
// 0x485220). Name not in the reference map.
struct RndSceneCullResults {
    // Frees the buckets. The binary releases the light lists first.
    ~RndSceneCullResults();  // Inlined at 0x419CB0 and 0x41A33E.

    FixedVector<PodVector<RndDrawInstance>, kNumDrawBuckets> mDrawInstances;
    // A copy of the light manager's culled lights (RndLightMgrCom
    // +0x490). Its destructor (0x4274E0) is shared with the light manager.
    RndLightMgrCom::CullResults mLights;
};

static_assert(offsetof(RndSceneCullResults, mLights) == 552);
static_assert(sizeof(RndSceneCullResults) == 808);

// What every batch of a draw passes its material: the target, the
// scene's lighting and fog, and the occlusion queries. Built inline by
// the flush templates and by RndSceneDrawer::_BuildBatchContext (0x41DF90).
// Name not in the reference map.
struct RndSceneBatchContext {
    RndBufferCollection* mBuffers;
    RndSceneDrawTarget mTarget;
    RndLightMgrCom* mLightMgr;
    // The scene component's property hysteresis texture.
    RndTexture2D* mPropHysteresisTexture;
    // With fog, the sky's atmosphere texture of the collection, or the
    // default texture 5 (0x452690).
    RndTextureBase* mAtmosphereTexture;
    // The occlusion query manager's current queries (+8). The type is
    // not recovered.
    void* mOcclusionQueries;
};

static_assert(offsetof(RndSceneBatchContext, mLightMgr) == 56);
static_assert(sizeof(RndSceneBatchContext) == 88);

// Draws a scene entity (render/RndSceneDrawer.o, 0x419170-0x4349FF; the
// static initializer at 0x4349E0). Each scene component owns one
// (RndSceneCom::mSceneDrawer). Draw instance components register with the
// drawer of their scene; registrations made while the scene enters are
// queued and handed to the culler at once when it has entered. Every frame
// the drawer polls as a PollDepBase job, and Draw culls the registered
// instances into per-bucket lists for each buffer collection's camera,
// sorts them and flushes them through the passes, at the full framerate or
// at a partial framerate that reuses the previous frame's cull results.
// The vtable at 0x19007A8 has ten slots. The object is 13008 bytes. Field
// names are not in the reference map.
class RndSceneDrawer : public PollDepBase {
public:
    // The render state a batch was flushed with, kept across the batches
    // of a bucket so unchanged state is not set again (104 bytes; the map's
    // RndSceneDrawer::FlushContext). Its constructor (0x4A bytes in the
    // map) is inlined into every _FlushBucket.
    struct FlushContext {
        FlushContext()
            : mNumBatches(0),
              mDebugTag(-1),
              mReserved8(-1),
              mEnvironIndex(6),
              mMaterial(nullptr),
              mHasInstanceCBuffer(-1),
              mCullMode(-1),
              mBlendMode(-1),
              mBlendColor(Hmx::Color::GetWhite()),
              mCounterClockwise(true),
              mUsesSceneTex(false),
              mUsesSceneDepth(false),
              mReceiveAtmosphere(true),
              mReceiveDecals(1),
              mStencilSet(false),
              mClipPlanes{} {}

        // Batches flushed in the batch-visualization shading mode (18).
        unsigned int mNumBatches;
        int mDebugTag;
        // Set to -1 and never read.
        long mReserved8;
        int mEnvironIndex;
        RndMaterialRuntimeData* mMaterial;
        // Whether the material was selected with an instance buffer.
        int mHasInstanceCBuffer;
        int mCullMode;
        // The blend state the blended passes set.
        int mBlendMode;
        Hmx::Color mBlendColor;
        bool mCounterClockwise;
        bool mUsesSceneTex;
        bool mUsesSceneDepth;
        bool mReceiveAtmosphere;
        unsigned char mReceiveDecals;
        // The stencil state was set at least once.
        bool mStencilSet;
        Vector4 mClipPlanes[2];
    };

    // What the batches of a _FlushBucket instantiation set besides the
    // common state; the instantiations are numbered in the binary's order
    // (0x4268D0 to 0x42F3F0) since the map's values are not recovered. Name
    // and enumerator names not in the reference map.
    enum FlushType : unsigned int {
        // The deferred lit and emissive passes: the environment, atmosphere
        // and decal bits go to the stencil (0x42C710).
        kFlushGBuffer = 0,
        // The depth passes: the material is reselected only for a new
        // instance buffer, or a new material with scene flags (0x42A680).
        kFlushDepth = 1,
        // Plain draws (0x42B020).
        kFlushBasic = 2,
        // The deferred unlit pass: the atmosphere and decal bits only
        // (0x42C3E0).
        kFlushGBufferUnlit = 3,
        // The decals: the environment and atmosphere test (0x42D0E0), with
        // the material's blend state (0x42DAB0), or the atmosphere test and
        // the material's blend mode alone (0x42E570).
        kFlushDecal = 4,
        kFlushDecalBlended = 5,
        kFlushDecalUnlit = 6,
        // The forward passes: the environment goes to the draw state
        // constants and the atmosphere selects the shading mode
        // (0x42F010), with the blend mode and the fog texture for the
        // transparent ones (0x42FA70).
        kFlushForward = 7,
        kFlushTransparent = 8,
    };

    // The batch context, at namespace scope so that other headers can
    // forward-declare it.
    using BatchContext = RndSceneBatchContext;

    RndSceneDrawer();  // 0x419350
    // Slots 0-1: 0x419AC0, 0x419F80.
    ~RndSceneDrawer() override;
    // Slot 3 at 0x41A5B0: _DoPoll on a worker, then queues the post-poll.
    // The map's signature is ThreadPoll().
    void ThreadPoll(const int& thread) override;
    // Slot 4 at 0x41A680: _DoPostPoll.
    void PostPoll() override;
    // Slot 5 at 0x41A770: "Scene Drawer <index>".
    const char* GetPollName() const override;

    // The drawer of the entity's scene: the scene component's or the
    // RndEntity component's (0x1AB0AE0, +56) on the entity's root object,
    // searched up through the entities that instance it. The map's
    // signature is FindForEntity(EntityPtr const&).
    static RndSceneDrawer* FindForEntity(Entity* entity);  // 0x419260
    // An identical copy that nothing calls. Name not in the reference
    // map; the overload is a guess.
    static RndSceneDrawer* FindForEntity(const Entity* entity);  // 0x419170

    // Called around the scene resource's EnterEntity: the prelude drops the
    // queued registrations and clears the culler, the coda hands the queued
    // registrations to the culler and steps the occlusion queries. The map's
    // signatures take the scene EntityPtr const&; this build passes the
    // entity.
    void EnterPrelude(Entity* entity);  // 0x419FA0
    void EnterCoda(Entity* entity);     // 0x41A0E0
    // The same as EnterPrelude, called before the scene resource's
    // ExitEntity. Name not in the reference map.
    void ExitPrelude(Entity* entity);  // 0x41A290
    // Forgets every registration and frees the bucket buffers; the next
    // Draw reserves them again. Called by the scene component (0x411AF0).
    // Name not in the reference map.
    void ResetRegistrations();  // 0x41A330
    // Points the drawer's own camera context at the camera, for 16:9
    // targets. The map's signature is SetSceneCamera(ObjPtr).
    void SetSceneCamera(RndCameraCom* camera, const Hmx::Rect& projectionRect);  // 0x41A7E0
    // Queues the component and adds its instance counts per level. The
    // map's signature starts with an ObjPtr const&.
    void RegisterDrawInstanceCom(RndDrawInstanceCom& com);  // 0x41A890
    // Unqueues a queued component, or deregisters it from the culler. The
    // map's signature starts with an ObjPtr const&.
    void DeRegisterDrawInstanceCom(RndDrawInstanceCom& com);  // 0x41A950
    // Draws the scene into each of the parameters' buffer collections and
    // returns what was drawn into each; `previous` are the targets a
    // previous draw returned, or null. The map's signature is
    // Draw(EntityPtr const&, RndSceneDrawParams&).
    FixedVector<RndSceneDrawTarget, 2> Draw(
        Entity* entity,
        RndSceneDrawParams& params,
        const FixedVector<RndSceneDrawTarget, 2>* previous);  // 0x41AA10
    // Starts the multithreaded draw: finalizes the parameters into
    // mThreadedContext and hooks the job graph in. Names not in the
    // reference map.
    void StartDrawJobs(
        Entity* entity,
        const RndSceneDrawParams& params,
        RndSceneDrawer* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs);  // 0x41D640
    // Finishes the multithreaded draw and returns its targets.
    FixedVector<RndSceneDrawTarget, 2> FinishDrawJobs();  // 0x41D6D0
    // Draws the shadow casters (bucket 2) of the instances the context's
    // first camera sees into the bound depth target, with the light's
    // show/hide flags: a light for flagged casters only shows flag 0x100,
    // others hide flag 8. The lights' shadow-map draws call it with the
    // drawer's third bucket set. The map's signature is
    // DrawShadowDepth(RndContext&, RndCameraContext const&, bool); the
    // camera is not read.
    void DrawShadowDepth(
        RndContext& context,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        bool flaggedCastersOnly,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable);  // 0x41D130

    // Steps of the draw. Names not in the reference map are marked.
    // Hands the queued registrations to the culler. The map has it out of
    // line (0x41A1C0); EnterCoda and _DoPostPoll inline it.
    void _ProcessNewlyRegistered();  // 0x41A1C0
    // Drops the queued registrations and clears the culler. EnterPrelude
    // and ExitPrelude inline it.
    void _PrepareToRegister();  // 0x41A040
    // Clears the drawer's camera and polls the culler.
    void _DoPoll();  // 0x41A650
    // Processes the queued registrations and steps the occlusion queries.
    void _DoPostPoll();  // 0x41A690
    // Reserves every bucket for the culler's largest bucket sizes. Name not
    // in the reference map.
    void _ReserveDrawInstances();  // 0x41AE00
    // Copies the parameters into the context, sets up the cameras and
    // collects the scene's components. The map's signature also has the
    // output RndSceneDrawParams&; this build finalizes the context's copy.
    void _ExtractSingletonsAndFinalizeParams(
        Entity* entity,
        const RndSceneDrawParams& params,
        RndSceneInternalContext& context) const;  // 0x41B060
    // Draws a stereo pair with shared culling and lighting. Name not in the
    // reference map.
    FixedVector<RndSceneDrawTarget, 2> _DrawStereo(
        RndContext& context,
        RndSceneInternalContext& internal,
        const FixedVector<RndSceneDrawTarget, 2>* previous);  // 0x41BD40
    // Uploads the scene's shader graph globals. The map's signature is
    // _UpdateShaderGraphGlobals(EntityPtr const&, RndSceneInternalContext&).
    void _UpdateShaderGraphGlobals(
        RndContext& context,
        RndSceneInternalContext& internal) const;  // 0x41C640
    // The map's signatures take the EntityPtr and the parameters; this
    // build passes the context and one collection's target.
    void _DrawPartialFramerate(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target);  // 0x41C760
    void _DrawFullFramerate(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target);  // 0x41D040
    // The map's signature also has the EntityPtr.
    void _Cull(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        RndCullerParams& params) const;  // 0x41D4B0
    bool _ExtractClearColor(
        const RndSceneDrawParams& params,
        const RndSceneCom* scene,
        Hmx::Color& color) const;  // 0x41D960
    // Builds the batch context of a target. Name not in the reference map.
    BatchContext _BuildBatchContext(
        const RndSceneInternalContext& internal,
        const RndSceneDrawTarget& target) const;  // 0x41DF90
    // The draw paths the parameters' modes select. Names not in the
    // reference map.
    void _DrawDepthOnly(
        RndContext& context,
        RndSceneInternalContext& internal,
        unsigned long target) const;  // 0x41E090
    void _DrawToTextures(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x41E420
    // The map's signature is
    // _DrawWireframe(RndContext&, RndSceneDrawParams&, RndSceneInternalContext&).
    void _DrawWireframe(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x41ECC0
    // The opaque passes and the lighting, and everything after it. The
    // map's signatures take the EntityPtr and the parameters.
    void _DrawInterval0(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target);  // 0x41F0C0
    void _DrawInterval1(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target);  // 0x41F590
    // The map's signature is
    // _DrawOutputConversion(RndContext&, RndSceneDrawParams&).
    void _DrawOutputConversion(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x41FC30
    // Save and restore the cull results of a partial-framerate scene. The
    // map's signatures differ: this build stores into mStoredCullResults.
    void _StoreCullResults(
        RndSceneInternalContext& internal,
        unsigned long camera) const;  // 0x41FFC0
    void _RestoreCullResults(
        RndContext& context,
        RndSceneInternalContext& internal,
        unsigned long camera) const;  // 0x420240
    // The map's signature is _AnalyzeCullResults(VectorAdapter<PodVector<
    // RndDrawInstance>> const&, RndSceneDrawParams const&,
    // RndSceneInternalContext&) const.
    void _AnalyzeCullResults(
        RndSceneInternalContext& internal,
        unsigned long camera) const;  // 0x420400
    // The passes the job graph names "draw post-cull" (the scene texture
    // capture, the depth clear, the scene mask, the prepasses, the
    // deferred geometry and the linear depth) and "draw post-light-cull"
    // (the sky, the lit deferred passes, the decals, the deferred lighting
    // and the volumetric scattering, on the async-compute fences). Names
    // not in the reference map.
    void _DrawPostCull(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x420740
    void _DrawPostLightCull(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x420AE0
    // The map's signature is _CaptureSceneTex(RndContext&, RndSceneDrawParams&).
    void _CaptureSceneTex(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x420FF0
    // Inlines the map's _RefineSceneMask.
    void _DrawSceneMask(
        RndContext& context,
        RndSceneInternalContext& internal,
        unsigned long target) const;  // 0x421AE0
    // The passes; each sorts and flushes its buckets.
    void _DrawZPrepass(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x422320
    void _DrawNormalsZPrepass(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x4224B0
    void _DrawOpaqueDeferredLit(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x4227A0
    void _DrawOpaqueDeferredEmissive(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x422D60
    void _DrawOpaqueDeferredUnlit(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x423220
    // "Analyze Depth".
    void _GenerateLinearDepth(
        RndContext& context,
        RndSceneInternalContext& internal,
        unsigned long target) const;  // 0x4234C0
    void _DrawDecalsLitOpaque(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x423AF0
    void _DrawDecalsLitTransparent(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x423E40
    void _DrawDecalsUnlit(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x4242B0
    void _DrawOpaqueFwdUnlit(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x424470
    void _DrawOpaqueFwdLit(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x424710
    void _DrawTransparent(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x424980
    void _DrawOpaqueTransparent(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x424C20
    void _DrawMaskBuffer(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x424E10
    void _DrawPostProc(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long index) const;  // 0x425510
    void _DrawShadingModeDisplay(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x425A00
    void _DrawOverlays(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target,
        unsigned long bucketIndex,
        PodVector<RndDrawInstance*>& bucket) const;  // 0x425E50
    // Points each bucket's sortable list at its instances.
    void _FillSortableBuckets(
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable) const;  // 0x425FC0
    void _InitLightAccum(
        RndContext& context,
        RndSceneInternalContext& internal,
        RndSceneDrawTarget& target) const;  // 0x426160
    void _SetDepthOnlyTargets(
        RndContext& context,
        RndSceneInternalContext& internal,
        unsigned long target,
        bool clear) const;  // 0x427040

    // Flush a range of a sorted bucket, batching consecutive instances
    // with the same state. The instantiations are at 0x4268D0, 0x42A000,
    // 0x42A9A0, 0x42BD60, 0x42CA60, 0x42D430, 0x42DEF0, 0x42E990 and
    // 0x42F3F0, each with its 'disable_batching' DataVariable index; they
    // differ only in the _FlushBatch they call.
    template <unsigned int N>
    void _FlushBucket(
        RndContext& context,
        RndSceneInternalContext& internal,
        const RndSceneDrawTarget& target,
        PodVector<RndDrawInstance*>& bucket,
        unsigned long begin,
        unsigned long end) const;
    // At 0x42C710, 0x42A680, 0x42B020, 0x42C3E0, 0x42D0E0, 0x42DAB0,
    // 0x42E570, 0x42F010 and 0x42FA70.
    template <unsigned int N>
    void _FlushBatch(
        RndContext& context,
        RndSceneInternalContext& internal,
        BatchContext& batch,
        FlushContext& flush,
        RndDrawInstance& instance,
        const VectorAdapter<RndInstanceData>& instances) const;

    // Field names are not in the reference map.
    // "Scene Drawer <index>": one more than the drawers built before.
    int mIndex;
    // Set while Draw runs.
    bool mDrawing;
    CritSec mCritSec;
    // Components registered while the scene entered, through the link in
    // their runtime data.
    struct NewlyRegisteredNode {
        static LinkedList::Node& ToNode(RndDrawInstanceCom& com);
        static RndDrawInstanceCom* FromNode(LinkedList::Node* node);
    };
    LinkedList::List<RndDrawInstanceCom, NewlyRegisteredNode> mNewlyRegistered;
    // The instance counts per scene level the queued registrations add.
    unsigned long mPendingGrowCounts[3];
    // Deregistrations handed to the culler; the partial-framerate scenes
    // compare it to tell whether the cull results are still valid.
    unsigned long mNumDeRegistered;
    RndCuller* mCuller;
    // The scene constant buffer the shader graph globals go to.
    RndShaderCBuffer* mSceneCBuffer;
    RndDynamicGpuDataMgr* mDynamicGpuDataMgr;
    RndOcclusionQueryMgr* mOcclusionQueryMgr;
    // The cull buckets of the first and second camera and of the third
    // (reserved only for bucket 2), and their sortable lists.
    FixedVector<PodVector<RndDrawInstance>, kNumDrawBuckets> mDrawInstances[3];
    FixedVector<PodVector<RndDrawInstance*>, kNumDrawBuckets> mSortableInstances[3];
    // Set when the buckets were freed; Draw reserves them again.
    bool mNeedsReserve;
    // The drawer's own camera (SetSceneCamera); cleared every poll.
    RndCameraContext mSceneCamera;
    // The cull results a partial-framerate scene keeps between its frames,
    // created by the first such frame. Mutable: the const parameter
    // finalization creates it.
    mutable RndSceneCullResults* mStoredCullResults;
    RndSceneDrawJobs* mDrawJobs;
    // Never created in this build.
    RndSceneDrawJobs* mStereoDrawJobs;
    RndSceneInternalContext* mThreadedContext;
    // Fences from the factory (RndFactory::CreateFence); their users are
    // the job graph.
    RndFence* mFences[12];

    // The drawer count behind mIndex. Name not in the reference map.
    static std::atomic<int> sNumDrawers;  // 0x1A725AC
};

static_assert(offsetof(RndSceneDrawer, mIndex) == 0x90);
static_assert(offsetof(RndSceneDrawer, mDrawing) == 0x94);
static_assert(offsetof(RndSceneDrawer, mCritSec) == 0x98);
static_assert(offsetof(RndSceneDrawer, mNewlyRegistered) == 0xA8);
static_assert(offsetof(RndSceneDrawer, mPendingGrowCounts) == 0xB8);
static_assert(offsetof(RndSceneDrawer, mNumDeRegistered) == 0xD0);
static_assert(offsetof(RndSceneDrawer, mCuller) == 0xD8);
static_assert(offsetof(RndSceneDrawer, mSceneCBuffer) == 0xE0);
static_assert(offsetof(RndSceneDrawer, mDynamicGpuDataMgr) == 0xE8);
static_assert(offsetof(RndSceneDrawer, mOcclusionQueryMgr) == 0xF0);
static_assert(offsetof(RndSceneDrawer, mDrawInstances) == 0xF8);
static_assert(offsetof(RndSceneDrawer, mSortableInstances) == 0x770);
static_assert(offsetof(RndSceneDrawer, mNeedsReserve) == 0xDE8);
static_assert(offsetof(RndSceneDrawer, mSceneCamera) == 0xDF0);
static_assert(offsetof(RndSceneDrawer, mStoredCullResults) == 0x3250);
static_assert(offsetof(RndSceneDrawer, mDrawJobs) == 0x3258);
static_assert(offsetof(RndSceneDrawer, mThreadedContext) == 0x3268);
static_assert(offsetof(RndSceneDrawer, mFences) == 0x3270);
static_assert(sizeof(RndSceneDrawer) == 0x32D0);
static_assert(offsetof(RndSceneDrawer::FlushContext, mEnvironIndex) == 16);
static_assert(offsetof(RndSceneDrawer::FlushContext, mMaterial) == 24);
static_assert(offsetof(RndSceneDrawer::FlushContext, mBlendMode) == 40);
static_assert(offsetof(RndSceneDrawer::FlushContext, mCounterClockwise) == 60);
static_assert(offsetof(RndSceneDrawer::FlushContext, mStencilSet) == 65);
static_assert(offsetof(RndSceneDrawer::FlushContext, mClipPlanes) == 68);
static_assert(sizeof(RndSceneDrawer::FlushContext) == 104);
