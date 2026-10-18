#pragma once

#include <cstddef>

#include "render/scene/RndDrawableEntityResource.h"
#include "utl/text/Symbol.h"

class BinStream;
class ComMetaData;
class Component;
class GameObject;

// A scene file (render/RndSceneResource.o, 0x4384A0-0x43AEE0): a drawable
// entity resource whose root object carries the scene components. Its
// entity draws through the root's RndSceneCom: the scene drawer draws it,
// and the root's texture renderers draw first. The scene's poll is split
// into jobs under the scene component's nested poll managers. The vtable at
// 0x19013C0 has 39 slots. The object is 784 bytes.
class RndSceneResource : public RndDrawableEntityResource {
public:
    RndSceneResource();  // 0x4384A0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x43AB50
    Symbol GetId() const override;                   // slot 1: 0x43AB60
    bool IsA(Symbol type) const override;            // slot 2: 0x43AC00
    // Slot 5: TransEntityResource's data, then the scene revision, 7.
    void Save(BinStream& stream, bool cached) override;  // 0x438B00
    ~RndSceneResource() override;  // slots 10-11: 0x4384D0, 0x4384E0
    // Slot 13: the entity with the root's scene, drawable-node, animation
    // driver and state-graph driver components, a "main_camera" object 20
    // units back as the scene's camera, a "light_mgr" object as its light
    // manager and a "default_env" light environment, and the root
    // EntityHeaderCom's "copy_on_instance" property cleared.
    Entity* CreateEntity() override;  // 0x439350
    // Slot 14: after TransEntityResource's load, reads the scene revision
    // (from resource revision 13) and upgrades older scenes: the root's
    // state-graph driver and the instances' renderer side (before 1), the
    // audio emitters' editor data (2), the transforms' editor draw (3), the
    // gaze input lists' instance pools (4), the particle colliders (5), the
    // fog's replacement by the sky (6) and the UI lists' editor data (7).
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x438B60
    // Slot 15: a non-cached load that succeeded resolves the entity's
    // material sharing (0x50E4D0).
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x439300
    // Slot 16: enters the entity between the scene drawer's EnterPrelude
    // and EnterCoda. The map's EnterEntity(EntityPtr).
    void _EnterEntity(Entity* entity, unsigned int flags) override;  // 0x439820
    // Slot 17: the scene drawer's ExitPrelude, then the exit.
    void _ExitEntity(Entity* entity, unsigned int flags) override;  // 0x4398D0
    // Slot 18: polls the entity through the scene component's split jobs,
    // steps its frame interval and records the frame each interval starts
    // on; the timelines learn of partial frame rates (0x260720, 0x260520)
    // and the drawer polls around the entity. The map's
    // PollEntity(EntityPtr). Not reconstructed: the PollMgr and
    // TimelinesCom calls it makes are not declared.
    void _PollEntity(Entity* entity) override;  // 0x43A130
    // Slot 19: the first time, unhooks the scene's poll groups and split
    // jobs and destroys the jobs.
    void _OnEntityReady(Entity* root, Entity* entity) override;  // 0x439950
    // Slot 20: creates the scene's poll groups on the first immediate
    // enter and gives the scene manager (key 1) two callbacks
    // (PollMgr 0x24CF10): before its polls, step the frame interval and
    // either mark the entity (flag 0x2000) or record the interval's start
    // frame; after them, clear the mark. Then clears both managers' jobs
    // and enters with the group selected (PollGroup 0x24F900). Not
    // reconstructed: those two PollMgr and PollGroup entry points take
    // std::function and RndSceneCom::PollGroups and are not declared.
    void _EnterImmediately(Entity* entity) override;  // 0x438670
    // Slot 22: called at the end of Entity's poll (0xF575F through
    // EntityResource::DestroyLayerEntity), so the base name looks wrong.
    // When the thread's poll context has a manager and enters immediately,
    // queues the scene drawer on the context's key-0 manager, rebuilds the
    // scene's split poll jobs ("splitpoll %d") over the entity's poll order
    // and links them. Not reconstructed: the poll context's third member
    // (the selected poll groups) and the split job windows are not
    // modelled.
    void _DestroyLayerEntity(Entity* entity) override;  // 0x439AE0
    // Slot 23: an instance or instance-pool component also needs a
    // drawable node. The map's CreateRequiredComponents(ObjPtr&,
    // ComMetaData const&).
    bool CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x43A600
    // Slot 24: destroying a drawable node destroys the instance and
    // instance-pool components first. The map's
    // DestroyDependentComponents(ObjPtr&, ComMetaData const&).
    void DestroyDependentComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x43A660
    // Slot 25: a new light, draw node or entity instance joins the scene
    // light manager's default environment: a light through
    // RndLightCom::AddToEnviron (0x46CCD0), a draw node as its
    // "environment", and an instance (which also gets its
    // RndEntityInstanceCom) as a nested light environment. The map's
    // PostCreateComponent(ObjPtr&, ComMetaData const&). Not reconstructed:
    // RndLightCom::AddToEnviron is not declared yet.
    void _OnComponentCreated(Component* component) override;  // 0x43A6D0
    // Slot 29: scenes are instanced by RndSceneInstanceCom.
    Symbol GetInstanceComId() const override;  // 0x439330
    // Slot 30: true.
    bool _IsEditorEntity() override;  // 0x439340

    // Slots 33-38: the root's scene drawer and texture renderers.
    FixedVector<RndSceneDrawTarget, 2> _Draw(
        Entity* entity,
        RndSceneDrawParams& params,
        const FixedVector<RndSceneDrawTarget, 2>* previous) override;  // 0x43A950
    void _StartDrawJobs(
        Entity* entity,
        const RndSceneDrawParams& params,
        Entity* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs) override;  // 0x43A9B0
    FixedVector<RndSceneDrawTarget, 2> _FinishDrawJobs(Entity* entity) override;  // 0x43AA20
    void _DrawTexRenderers(Entity* entity) override;  // 0x43AA80
    void _StartTexRendererJobs(
        Entity* entity,
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs) override;  // 0x43AAC0
    void _FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) override;  // 0x43AB10

    // The class id, created on first use. Inlined into its users, for
    // example Resource::GetOrLoad<RndSceneResource> at 0x6C0160; the local
    // static is RndSceneResource::Id()::id in the map (0x19C7F20).
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndSceneResource");
        }
        return id;
    }

    // Describes the class: extension "scene", category "Scenes".
    static void _Init(ResourceMetaData& metaData);  // 0x4389E0

    static ResourceMetaData sMetaData;  // 0x1A72FD8
};

static_assert(sizeof(RndSceneResource) == 784);
