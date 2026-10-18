#pragma once

#include <cstddef>
#include <functional>

#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "entity/resources/ResourceMetaData.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/threading/PollDep.h"

class BinStream;
class ComMetaData;
class RndBufferCollection;
class Component;
class DataArray;
class Entity;
class RndSceneInstanceCom;

// A UI screen: an entity of scene instances, each a RndSceneInstanceCom on
// one of its objects, with a UILayoutCom on its root that orders their
// polls and draws (ui/UILayoutResource.o, 0x8AAA40 to 0x8B1BAF). Scenes are
// added and removed through queues that the layout applies between polls.
// The UI manager enters, polls, draws and exits the current layout. Its
// metadata names "TransEntityResource" as the parent class, while the C++
// base is the drawable entity resource. The vtable at 0x195D1C0 has 39
// slots; the object is 1248 bytes.
class UILayoutResource : public RndDrawableEntityResource {
public:
    // Clears the draw's buffer collections when the layout draws through
    // jobs (slot 34), before the scenes draw. Its constructor is inlined
    // into the layout's; an out-of-line copy is at 0x8B1860. The vtable is
    // at 0x195D308. Name not in the reference map.
    class ClearJob : public PollDepBase {
    public:
        ClearJob() {
            SetThreaded(false);
        }
        ~ClearJob() override {}  // slots 0-1: 0x8ABA10, 0x8B19F0
        // Slot 3: clears the buffer collections of the parameters.
        void ThreadPoll(const int& thread) override;  // 0x8B18A0

        // The parameters of the draw the job clears for.
        RndSceneDrawParams mParams;
    };

    // Draws one showing scene's masking scenes before the scene's own draw
    // jobs, when the layout draws through jobs (_StartDrawJobs). The
    // layout keeps one per scene in mSceneDrawJobs. Its constructor is
    // inlined into _StartDrawJobs. The vtable is at 0x195D368. Name not in
    // the reference map.
    class SceneDrawJob : public PollDepBase {
    public:
        explicit SceneDrawJob(UILayoutResource* layout) : mLayout(layout), mEntity(nullptr) {
            SetThreaded(false);
        }
        ~SceneDrawJob() override {}  // slots 0-1: 0x8B1B10, 0x8B1B60
        // Slot 3: draws the masking scenes.
        void ThreadPoll(const int& thread) override;  // 0x8B1980

        // Field names are not in the reference map.
        UILayoutResource* mLayout;
        // The layout's entity.
        Entity* mEntity;
        // The scene's draw parameters.
        RndSceneDrawParams mParams;
        // The scene resource's path; the draw does not read it.
        ResourcePath mScenePath;
        // The scene's showing masking scenes (_GetMaskingScenes).
        eastl::vector<GameObjectId> mMaskingScenes;
    };

    // Where a queued scene goes in the layout's poll and draw orders
    // (mQueuedScenes). Names not in the reference map.
    enum QueuedSceneMode : int {
        // Where the new object falls, in front (AddSceneToFront).
        kQueuedSceneFront = 0,
        // Before or after another scene object (AddSceneAtDrawOrder,
        // AddSceneAfterObject).
        kQueuedSceneBefore = 1,
        kQueuedSceneAfter = 2,
    };

    // A scene waiting to be added. Name and field names are not in the
    // reference map.
    struct QueuedScene {
        ResourcePath mPath;
        QueuedSceneMode mMode;
        // The scene object the new one goes next to; -1 for the front.
        GameObjectId mAnchor;
        // Bit 1 of the caller's flags: the scene instance's
        // "use_threaded_poll".
        bool mUseThreadedPoll;
    };

    // Scene object ids over storage the caller provides, which the draw
    // functions take from the stack. Name and field names are not in the
    // reference map.
    struct SceneIdList {
        GameObjectId* mData;
        unsigned long mSize;
        unsigned long mCapacity;
    };

    // Flags of the scene queue functions. Names not in the reference map.
    enum SceneQueueFlags : unsigned int {
        // Applies the queue at once instead of before the next poll.
        kSceneQueueNow = 1,
        // Lets the scene poll threaded (QueuedScene::mUseThreadedPoll).
        kSceneQueueThreadedPoll = 2,
    };

    // The class symbol, "UILayoutResource", kept in a guarded static
    // (0x1AC7DC0) that is built on first use.
    static Symbol Id() {
        static Symbol sId;
        if (sId == Symbol()) {
            sId = Symbol("UILayoutResource");
        }
        return sId;
    }

    // Registers the class: its metadata and its factory. Inline in the
    // map's ui/UI.o (UILayoutResource::Init()); emitted in ui/UI.o at
    // 0x891A30.
    static void Init() {
        _Init(sMetaData);
        const Symbol id = Id();
        Resource::sFactory[id] = &_Create;
        sMetaData.Init(id, Symbol("TransEntityResource"), true);
    }
    // The factory. Emitted in ui/UI.o at 0x894FA0.
    static Resource* _Create() {
        return new UILayoutResource;
    }
    // Names the type's extension ("layout").
    static void _Init(ResourceMetaData& metaData);  // 0x8ABA50

    UILayoutResource();  // 0x8AAA40

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x8B1A20
    Symbol GetId() const override;                   // slot 1: 0x8B1A30
    bool IsA(Symbol type) const override;            // slot 2: 0x8B1AD0
    // Slot 5: writes the entity, then the layout's revision, 4.
    void Save(BinStream& stream, bool cached) override;  // 0x8AC0F0
    // Slots 10-11: 0x8AAB30, 0x8ABA30. Applies the scene queues, releases
    // the scenes' poll ranges and deletes the scene draw jobs.
    ~UILayoutResource() override;
    // Slot 13: adds the state graph driver, the animation driver and the
    // layout component to the new root.
    Entity* CreateEntity() override;  // 0x8AC150
    // Slot 14: reads the layout's revision after the entity and upgrades
    // older layouts.
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x8ABB50
    // Slot 15: checks the scenes after the base class's post-load.
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x8ABF60
    // Slots 16-18. Exit releases the scenes' poll range and the layout's
    // poll groups first. The poll applies the scene queues; an active
    // game-mode layout then polls its poll groups under its manager, with
    // the audio lock that "use_pollmgr_audio_lock" asks for, and runs the
    // post-polls of the scene groups (the scene's, the threaded scenes'
    // and the drawers'); last, the partial-framerate scenes get distinct
    // intervals.
    void _EnterEntity(Entity* entity, unsigned int flags) override;  // 0x8AC810
    void _ExitEntity(Entity* entity, unsigned int flags) override;   // 0x8ACBC0
    void _PollEntity(Entity* entity) override;                        // 0x8ACC60
    // Slot 19: a scene entity that becomes ready is told through its scene
    // resource and kept for the next sync of the poll range, which is
    // released meanwhile; the layout's own entity releases it unless it
    // is released already.
    void _OnEntityReady(Entity* root, Entity* entity) override;  // 0x8ADA20
    // Slot 20: once the poll range was released, enters the ready scenes
    // at once and syncs the range again.
    void _EnterImmediately(Entity* entity) override;  // 0x8AD8F0
    // Slot 23: a scene instance also needs the draw node component.
    bool CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x8AFC20
    // Slot 24: removing the draw node also removes the scene instance.
    void DestroyDependentComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x8AFC80
    // Slot 25: a new scene instance is appended to the layout's poll and
    // draw orders.
    void _OnComponentCreated(Component* component) override;  // 0x8AFCE0
    // Slot 26 at 0x8B1B00: false, as for an entity resource.
    bool IsTransient() const override;
    // Slots 33-38: draw the showing scenes in the layout's draw order, at
    // once or through jobs, and their texture renderers. Each scene draws
    // its masking scenes first, clears only as the first scene and outputs
    // to the back buffer only as the last; without scenes the buffers are
    // cleared. RndConfig's first and last drawn scene limit the immediate
    // draw.
    FixedVector<RndSceneDrawTarget, 2> _Draw(
        Entity* entity,
        RndSceneDrawParams& params,
        const FixedVector<RndSceneDrawTarget, 2>* previous) override;  // 0x8AFDC0
    void _StartDrawJobs(
        Entity* entity,
        const RndSceneDrawParams& params,
        Entity* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs) override;  // 0x8B05E0
    FixedVector<RndSceneDrawTarget, 2> _FinishDrawJobs(Entity* entity) override;  // 0x8B1100
    void _DrawTexRenderers(Entity* entity) override;  // 0x8B1350
    void _StartTexRendererJobs(
        Entity* entity,
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs) override;  // 0x8B14A0
    void _FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) override;  // 0x8B1600

    // Sends the message to the scenes' entities, the last in poll order
    // first. With `stopAtHandled` it stops at the first scene that handles
    // it and returns true; otherwise every scene receives it and the
    // result is false. The map's HandleMsg(DataArray*, bool).
    bool HandleMsg(DataArray* msg, bool stopAtHandled);  // 0x8ADB70
    // Queue a scene for adding: in front, or before or after another
    // scene object in the poll and draw orders. The map's
    // AddSceneToFront(ResourcePath const&) and
    // AddSceneAtDrawOrder(ResourcePath const&, int); this build passes the
    // id of the scene object to go before, which callers find through
    // _FindSceneObject, and adds the flags (SceneQueueFlags).
    // AddSceneAfterObject is not in the reference map.
    void AddSceneToFront(const ResourcePath& path, unsigned int flags);  // 0x8ADCD0
    void AddSceneAtDrawOrder(const ResourcePath& path, GameObjectId before, unsigned int flags);  // 0x8ADE80
    void AddSceneAfterObject(const ResourcePath& path, GameObjectId after, unsigned int flags);  // 0x8AE040
    // Drops the path's queued additions and queues its removal. The map's
    // RemoveScene(ResourcePath const&); this build adds the flags.
    void RemoveScene(const ResourcePath& path, unsigned int flags);  // 0x8AE270
    // Whether the path's removal is queued. Name not in the reference map.
    bool IsSceneQueuedForRemoval(const ResourcePath& path) const;  // 0x8AE430
    // The object whose scene instance instances the path. Name not in the
    // reference map.
    GameObject* _FindSceneObject(const ResourcePath& path) const;  // 0x8AE480
    // The scene instance that instances the path, or null.
    RndSceneInstanceCom* GetSceneInstance(const ResourcePath& path) const;  // 0x8AE560
    // Shows or hides the scene whose instanced entity is the path's, through
    // the hidden bit of its draw node.
    void SetSceneShowing(const ResourcePath& path, bool showing);  // 0x8AE670
    // Activates or deactivates every scene's entity. Name not in the
    // reference map.
    void SetScenesActive(bool active);  // 0x8AE790
    // The scene instances, the last in poll order first. Name not in the
    // reference map.
    eastl::vector<RndSceneInstanceCom*> GetSceneInstances() const;  // 0x8AE960
    // Calls the function with each scene instance, the last in poll order
    // first.
    void ForEachScene(std::function<void(RndSceneInstanceCom*)> function) const;  // 0x8AE860
    // Plays every scene's exit transition (UIInputCom::Exit).
    void Exit();  // 0x8AEB90
    // Whether a scene is still playing its exit or enter transition, or
    // the layout's own flag is set.
    bool IsExiting() const;   // 0x8AECC0
    bool IsEntering() const;  // 0x8AEE40
    // Takes one frame of the pending skipped draws; true when the frame is
    // to be skipped.
    bool UpdateSkipDrawState();  // 0x8B1840
    // Whether the scene object's instanced entity is active and neither
    // the object's draw node nor the entity root's hides it in the world.
    // The map's _IsSceneInstanceShowing(EntityPtr const&, GameObjectId)
    // const.
    bool _IsSceneInstanceShowing(Entity* entity, GameObjectId id) const;  // 0x8B1750
    // Appends the showing scenes of the draw order, or of the HMD mirror
    // overlays, to the list. Name not in the reference map.
    void _GetShowingScenes(Entity* entity, SceneIdList& scenes, bool hmdMirrorOverlays) const;  // 0x8AF080

    // Create the queued scenes' objects in the layout's poll and draw
    // orders, and destroy the objects of the queued removals.
    void _AddQueuedScenes();     // 0x8AAC60
    void _RemoveQueuedScenes();  // 0x8AB2D0
    // Releases the scenes' threaded poll range: marks it released and,
    // when a scene group has jobs, clears the layout's groups and unlinks
    // the threaded scenes' groups from the layout's group and from each
    // other. Name not in the reference map; the evidence is weak.
    void _ClearThreadedPollRange(Entity* entity);  // 0x8AB6E0
    // Rebuilds the poll range: the instanced scenes poll in order, a scene
    // that can poll threaded alongside the threaded ones before it and
    // after the last serial scene, a serial one after all of them; the
    // layout's entity polls in the layout's group after every scene. The
    // map's _SyncThreadedPollRange(EntityPtr const&); the match rests on
    // the callees.
    void _SyncThreadedPollRange(Entity* entity);  // 0x8AC1E0
    // Gives the partial-framerate scenes of the poll order distinct
    // RndSceneCom::mPartialFramerateScene values, through a bit vector
    // (vtable 0x195D3C8) that is not modelled. Name not in the reference
    // map. Not reconstructed.
    static void _AssignPartialFramerateScenes(Entity* entity);  // 0x8AD420
    // Draws the showing HMD mirror overlays into the buffer collection
    // through the immediate path, without clearing. No caller is found.
    // Name not in the reference map.
    void DrawHmdMirrorOverlays(RndBufferCollection* buffers);  // 0x8AF240
    // The scene instance's masking scenes that are showing, when the
    // renderer enables scene masks. Names not in the reference map.
    void _GetMaskingScenes(
        Entity* entity,
        const RndSceneInstanceCom* instance,
        SceneIdList& scenes) const;  // 0x8AF6D0
    void _GetMaskingScenes(
        Entity* entity,
        const RndSceneInstanceCom* instance,
        eastl::vector<GameObjectId>& scenes) const;  // 0x8B0E90
    // Sets the projection of the scene's draw parameters, and whether it
    // clears (the first scene), outputs to the back buffer (the last) and
    // is masked. No caller is found; the draws inline it. Name not in the
    // reference map.
    void _SetSceneDrawParams(
        RndSceneDrawParams& params,
        const RndSceneInstanceCom* instance,
        unsigned long index,
        unsigned long count,
        unsigned long numMaskingScenes) const;  // 0x8AFB90
    // Reads each scene's light manager; the checks around the reads are
    // compiled out. The map's _ValidateSceneData(EntityPtr const&); the
    // match rests on the size and the place of its call.
    void _ValidateSceneData(Entity* entity);  // 0x8ABF90

    static ResourceMetaData sMetaData;  // 0x1AC8FB8

    // Field names are not in the reference map.
    // The scenes to add before the next poll, and the paths to remove.
    eastl::vector<QueuedScene> mQueuedScenes;
    eastl::vector<ResourcePath> mQueuedRemovals;
    // IsEntering and IsExiting return them when no scene is in a
    // transition. Cleared by the constructor; no writer is identified.
    bool mEntering;
    bool mExiting;
    ClearJob mClearJob;
    // The scenes' draw jobs (SceneDrawJob), deleted by the destructor.
    eastl::vector<PollDepBase*> mSceneDrawJobs;
    // Frames whose draw is skipped (UpdateSkipDrawState).
    int mFramesToSkipDraw;
    // Cleared on enter; while it is nonzero the poll counts it down
    // instead of resetting the poll manager's threading. The meaning is
    // not identified; the name is weak.
    int mSerialPollFrames;
};

static_assert(sizeof(UILayoutResource::ClearJob) == 352);
static_assert(offsetof(UILayoutResource::ClearJob, mParams) == 144);
static_assert(offsetof(UILayoutResource::SceneDrawJob, mLayout) == 144);
static_assert(offsetof(UILayoutResource::SceneDrawJob, mParams) == 160);
static_assert(offsetof(UILayoutResource::SceneDrawJob, mScenePath) == 368);
static_assert(offsetof(UILayoutResource::SceneDrawJob, mMaskingScenes) == 376);
static_assert(sizeof(UILayoutResource::SceneDrawJob) == 408);
static_assert(sizeof(UILayoutResource::QueuedScene) == 24);
static_assert(offsetof(UILayoutResource, mQueuedScenes) == 784);
static_assert(offsetof(UILayoutResource, mQueuedRemovals) == 816);
static_assert(offsetof(UILayoutResource, mEntering) == 848);
static_assert(offsetof(UILayoutResource, mExiting) == 849);
static_assert(offsetof(UILayoutResource, mClearJob) == 856);
static_assert(offsetof(UILayoutResource, mSceneDrawJobs) == 1208);
static_assert(offsetof(UILayoutResource, mFramesToSkipDraw) == 1240);
static_assert(sizeof(UILayoutResource) == 1248);
