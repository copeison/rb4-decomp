#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"
#include "utl/threading/PollMgr.h"

class Component;
class ComMetaData;
class Entity;
class GameObject;

// A resource that holds an entity (entity/EntityResource.o,
// 0xFD630-0x102EDF). The entity's layers are separate files; resources
// inlined into a layer are kept with it. The vtable is at 0x18E6AF8 and has
// 33 slots; the object is 200 bytes. Slots 13-32 are this class's; the map
// names some of them, and the names of the rest are inferred from their
// bodies and their callers.
class EntityResource : public Resource {
public:
    // One layer: its file and the resources inlined into it. The type and
    // field names are not in the reference map.
    struct LayerInfo {
        ResourcePath mPath;
        eastl::vector<Resource*> mInlineResources;
    };

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19E2C28.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("EntityResource");
        }
        return id;
    }

    // Starts with the main layer.
    EntityResource();  // 0xFD9A0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x102620
    Symbol GetId() const override;                   // slot 1: 0x102630
    bool IsA(Symbol type) const override;            // slot 2: 0x1026D0
    // Slot 4. Loads the entity through _LoadEntity, then its resources,
    // timed as "EntityLoad" when entity load profiling is on. Not
    // reconstructed.
    bool Load(BinStream& stream, bool cached) override;  // 0xFE380
    // Slot 5. Not reconstructed.
    void Save(BinStream& stream, bool cached) override;  // 0x100760
    // Slot 6: a resource without an entity failed to load.
    bool Fail() const override;  // 0x102700
    // Slots 10-11: 0xFDB10, 0xFDDF0. Releases the inlined resources and
    // destroys the entity.
    ~EntityResource() override;

    // Slot 13: creates the entity with its "root" object.
    virtual Entity* CreateEntity();  // 0xFD6A0
    // Slot 14: reads the entity from the stream. Not reconstructed. Name not
    // in the reference map.
    virtual bool _LoadEntity(BinStream& stream, bool cached);  // 0xFEC20
    // Slot 15 at 0x5C290: runs after the entity and its resources loaded;
    // true here. Name not in the reference map.
    virtual bool _PostLoad(BinStream& stream, bool cached) {
        static_cast<void>(stream);
        static_cast<void>(cached);
        return true;
    }
    // Slots 16-18: enter, exit and poll the entity. The map has
    // EnterEntity(EntityPtr) and PollEntity(EntityPtr) as virtuals; this
    // build gives them these bodies and the non-virtual wrappers below.
    // Names not in the reference map.
    virtual void _EnterEntity(Entity* entity, unsigned int flags);  // 0xFD870
    virtual void _ExitEntity(Entity* entity, unsigned int flags);   // 0xFD8A0
    virtual void _PollEntity(Entity* entity);                        // 0xFD8D0
    // Slot 19 at 0xFD940: told about a ready entity by its topmost
    // ancestor's resource (ReadyEntity); empty here. Name not in the
    // reference map; the evidence is weak.
    virtual void _OnEntityReady(Entity* root, Entity* entity);
    // Slots 20-21: enter or poll the entity at once when the thread's poll
    // context asks for it. Names not in the reference map.
    virtual void _EnterImmediately(Entity* entity);  // 0xFD950
    virtual void _PollImmediately(Entity* entity);   // 0xFD960
    // Slot 22 at 0x5C2A0: empty here; DestroyLayerEntity calls it. Name not
    // in the reference map; the evidence is weak.
    virtual void _DestroyLayerEntity(Entity* entity) {
        static_cast<void>(entity);
    }
    // Slot 23: creates the components the class requires that the object
    // lacks. False when one cannot be created.
    virtual bool CreateRequiredComponents(GameObject* object, const ComMetaData& metaData);  // 0x101A20
    // Slot 24: destroys the object's components that depend on the class.
    virtual void DestroyDependentComponents(GameObject* object, const ComMetaData& metaData);  // 0x101AC0
    // Slot 25 at 0x5C2B0: told about each component GameObject creates;
    // empty here. Name not in the reference map.
    virtual void _OnComponentCreated(Component* component) {
        static_cast<void>(component);
    }
    // Slot 26 at 0x5C2C0: false here, true for TransEntityResource. Its
    // callers are not identified. Name not in the reference map; the
    // evidence is weak.
    virtual bool IsTransient() const {
        return false;
    }
    // Slot 27: sets up a new object, giving it the instance component.
    virtual void InitObject(GameObject* object);  // 0xFD7E0
    // Slot 28 at 0x5C2D0: told about a renamed object; empty here. Name not
    // in the reference map.
    virtual void _OnObjectRenamed(GameObject* object) {
        static_cast<void>(object);
    }
    // Slot 29: the class symbol of the component that instances the
    // entity.
    virtual Symbol GetInstanceComId() const;  // 0x101BD0
    // Slot 30 at 0x5C2E0: false here; _NewEntity stores it in bit 11 of
    // the entity's flags. Name not in the reference map; the evidence is
    // weak.
    virtual bool _IsEditorEntity() {
        return false;
    }
    // Slot 31 at 0x5C2F0: false here; _LoadEntity creates the entity's
    // poll, post-poll, enter and destroy timers only when it is false. Name
    // not in the reference map.
    virtual bool _SkipPerfTimers() {
        return false;
    }
    // Slot 32: reads the entity's root. The map's
    // _LoadRoot(BinStream&, EntityPtr, vector<unsigned char>&,
    // vector<ResourcePath>&); this build passes pointers.
    virtual void _LoadRoot(
        BinStream& stream,
        Entity* entity,
        eastl::vector<unsigned char>* rootData,
        eastl::vector<ResourcePath>* paths);  // 0xFD610

    // Builds the entity class's metadata.
    static void _Init(ResourceMetaData& metaData);  // 0xFDF10
    // Creates an empty entity owned by the resource. Name not in the
    // reference map.
    Entity* _NewEntity();  // 0xFD630
    // Destroys the entity.
    void DestroyEntity();  // 0xFD800
    // Takes the entity, which becomes owned by the resource, and returns
    // the old one.
    Entity* SwapEntity(Entity* entity);  // 0xFD830
    // Enter, exit and poll the entity when there is one, through slots
    // 16-18. The map has EnterEntity(EntityPtr) and PollEntity(EntityPtr)
    // as virtuals; ExitEntity's name is not in the reference map. Most
    // callers pass no flags (zero); the default argument stands for them.
    void EnterEntity(Entity* entity, unsigned int flags = 0);  // 0xFD850
    void ExitEntity(Entity* entity, unsigned int flags = 0);   // 0xFD880
    void PollEntity(Entity* entity);                        // 0xFD8B0
    // Tells the topmost ancestor's resource that an entered entity is
    // ready, through slot 19. Name not in the reference map.
    void ReadyEntity(Entity* entity);  // 0xFD8E0
    // Calls slot 22 when there is an entity. Name not in the reference
    // map.
    void DestroyLayerEntity(Entity* entity);  // 0xFD970
    // Loads the entity's resources.
    bool LoadResources();  // 0xFD990
    // Whether the resource is the one the calling thread is loading while
    // entity load profiling is on. Name not in the reference map.
    bool IsProfilingLoad() const;  // 0xFE530
    // Whether the layer exists; the main layer always does.
    bool LayerExists(unsigned long layer) const;  // 0xFFE80
    // Whether the component class may be created in the entity: true when
    // the resource is, or derives from, one of the resource classes the
    // component's metadata lists. The map's
    // IsAllowedComponent(ComMetaData const&) const.
    bool IsAllowedComponent(const ComMetaData& metaData) const;  // 0x101B80
    // Records a component class that could not be created. Name not in the
    // reference map.
    void AddMissingComponent(Symbol className);  // 0x101BE0
    // The layer's file, or the empty path past the last layer.
    ResourcePath GetLayerPath(unsigned long layer) const;  // 0x101CF0

    static ResourceMetaData sMetaData;  // 0x19E3008

    // Field names are not in the reference map.
    Entity* mEntity;
    eastl::vector<LayerInfo> mLayers;
    // Data read with the entity's root (_LoadRoot); TransEntityResource
    // reads it from revisions 12 and 13. The meaning is not recovered.
    eastl::vector<unsigned char> mRootData;
    // The entity resource this one is inlined in; Entity::MakeErrorName
    // (0xF0DD0) names it.
    EntityResource* mInlineOwner;
    // PerfTimerMgr indices of the "poll: ", "post_poll: ", "enter_exit: "
    // and "destroy: " timers, created on the first load (0xFEC20); -1
    // until then.
    long mPollTimer;
    long mPostPollTimer;
    long mEnterExitTimer;
    long mDestroyTimer;
    // The component classes that could not be created.
    eastl::vector<Symbol> mMissingComponents;
    // Set when the loaded revision is below 18; the stale-id fixup clears
    // it. Weakly supported.
    bool mHasStaleIds;
};

static_assert(sizeof(EntityResource::LayerInfo) == 40);
static_assert(offsetof(EntityResource, mEntity) == 48);
static_assert(offsetof(EntityResource, mLayers) == 56);
static_assert(offsetof(EntityResource, mRootData) == 88);
static_assert(offsetof(EntityResource, mInlineOwner) == 120);
static_assert(offsetof(EntityResource, mPollTimer) == 128);
static_assert(offsetof(EntityResource, mMissingComponents) == 160);
static_assert(offsetof(EntityResource, mHasStaleIds) == 192);
static_assert(sizeof(EntityResource) == 200);
