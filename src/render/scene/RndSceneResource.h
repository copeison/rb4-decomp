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
    // manager and a "default_env" light environment, and the entity
    // header's "copy_on_instance" property set. Not reconstructed: the
    // property write it inlines is not modelled.
    Entity* CreateEntity() override;  // 0x439350
    // Slot 14: after TransEntityResource's load, reads the scene revision
    // (from resource revision 13) and upgrades older scenes: the root's
    // state-graph driver, the objects' entity-instance components, the
    // audio emitters' editor data, the transforms' editor draw, the gaze
    // input lists' instance pools, the fog's replacement by the sky and the
    // UI lists' editor data. Not reconstructed: most of the classes it
    // names are not modelled.
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x438B60
    // Slot 15: a non-cached load whose TransEntityResource post-load
    // failed is repaired by 0x50E4D0 on the entity and succeeds. Not
    // reconstructed: that repair is not modelled.
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x439300
    // Slot 16: enters the entity between the scene drawer's EnterPrelude
    // and EnterCoda. The map's EnterEntity(EntityPtr).
    void _EnterEntity(Entity* entity, unsigned int flags) override;  // 0x439820
    // Slot 17: the scene drawer's ExitPrelude, then the exit.
    void _ExitEntity(Entity* entity, unsigned int flags) override;  // 0x4398D0
    // Slot 18: polls the entity through the scene component's split jobs,
    // steps its frame interval and records the frame each interval starts
    // on. The map's PollEntity(EntityPtr). Not reconstructed: the poll
    // context and the lambdas it queues are not modelled.
    void _PollEntity(Entity* entity) override;  // 0x43A130
    // Slot 19: the first time, unhooks the scene's poll groups and split
    // jobs and destroys the jobs.
    void _OnEntityReady(Entity* root, Entity* entity) override;  // 0x439950
    // Slot 20: creates the scene's poll groups on the first immediate
    // enter, queues the frame-interval step and the instance update under
    // them, and enters with the group selected. Not reconstructed: the
    // queued lambdas (0x43AC70 on) and the group selection are not
    // modelled.
    void _EnterImmediately(Entity* entity) override;  // 0x438670
    // Slot 22. Not reconstructed.
    void _DestroyLayerEntity(Entity* entity) override;  // 0x439AE0
    // Slot 23: an instance or instance-pool component also needs a
    // drawable node. The map's CreateRequiredComponents(ObjPtr&,
    // ComMetaData const&). Not reconstructed: the instance classes'
    // symbols are not modelled.
    bool CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x43A600
    // Slot 24: destroying a drawable node destroys the instance and
    // instance-pool components first. The map's
    // DestroyDependentComponents(ObjPtr&, ComMetaData const&). Not
    // reconstructed, for the same reason.
    void DestroyDependentComponents(GameObject* object, const ComMetaData& metaData) override;  // 0x43A660
    // Slot 25: the map's PostCreateComponent(ObjPtr&, ComMetaData const&).
    // Not reconstructed.
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
