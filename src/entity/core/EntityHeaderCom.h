#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;

// The component on an entity's root object that describes the entity:
// how its instances copy, drive and poll it, its statistics, the icon
// camera and data, and the editor capabilities. Its class id is
// "EntityHeader" ("Common specifications for Entities"), with the alias
// "EntityDefinitionCom". The map's older build keeps these properties on
// InstanceCom; this build moves them into a class of their own, in an
// object the map does not name (0x1D1E90 to 0x1D8D37, with its static
// initializer at 0x1D8C80). The vtable at 0x18EA8E8 has Component's 41
// slots. The object is 528 bytes. Name not in the reference map; the field
// names follow the registry's properties (0x1D21D0).
class EntityHeaderCom : public Component {
public:
    // The edits counted against one layer, which decide whether it is
    // modified (0x114D50). Name and field names not in the reference map;
    // the evidence is weak.
    struct LayerChanges {
        int mCount;
        bool mDirty;
    };

    // The per-layer editor state, one element per layer (kMaxLayers). The
    // accessors at 0x1D52E0-0x1D57CF have no callers in this build and are
    // not reconstructed. Name and field names not in the reference map;
    // the meanings of the first two flags, which start set, are weakly
    // supported.
    struct RuntimeData {
        RuntimeData();  // 0x1D1F60

        eastl::vector<bool> mLayerVisible;
        eastl::vector<bool> mLayerSelectable;
        // Which layers are modified, and how many are; a change is
        // announced with "layer_is_modified_changed" (0x1D5430).
        eastl::vector<bool> mLayerModified;
        long mNumModifiedLayers;
        eastl::vector<LayerChanges> mLayerChanges;
        // Asks a poll-suppressed TransEntityResource to poll its entity
        // once (0x1BB660), which clears it. _OnResourcesLoaded sets it for a
        // "static" entity.
        bool mPollRequested;
    };

    EntityHeaderCom();  // 0x1D1E90
    // Copies the properties for an imprint, "icon_data" through
    // PropArrayBase::_Copy; the run-time state starts afresh. Inlined into
    // _Imprint (0x1D5880).
    EntityHeaderCom(const EntityHeaderCom& other);
    // Slots 0-1: 0x1D50E0 (which jumps to the body at 0x1D5D00) and
    // 0x1D57E0.
    ~EntityHeaderCom() override;
    Symbol GetId() const override;          // slot 4: 0x1D5800
    Symbol GetClassName() const override;   // slot 5: 0x1D5810
    int CurrentRev() const override;        // slot 7: 0x1D5820
    bool IsA(Symbol type) const override;   // slot 8: 0x1D5840
    Component* AsComponent() override;      // slot 9: 0x1D5870
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x1D5880
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x1D5A20
    ComMetaData& _GetMetaData() override;       // slot 23: 0x1D5A30
    // Slot 29 at 0x1D51C0: in a TransEntityResource, moves "icon_data" into
    // the resource and applies "static".
    bool _OnResourcesLoaded() override;

    // "copy_on_instance". InstanceCom reads it through this out-of-line
    // accessor. Name not in the reference map.
    bool CopyOnInstance() const;  // 0x1D51B0

    // Describes the class and registers its properties, among them the
    // "poll_order" view of the entity's poll order (ten objects from a
    // start index kept at 0x19E4CE8). Not reconstructed: the property
    // metadata and the std::function handlers it binds are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x1D21D0

    // "EntityHeader", constructed by the static initializer at 0x1D8C80.
    static Symbol sId;  // 0x19E4A80
    // Also "EntityHeader"; the class symbol GameObject::CreateComponent
    // takes. Name not in the reference map.
    static Symbol sClassName;           // 0x19E4A88
    static PropRegistry sPropRegistry;  // 0x19E4A90
    static ComMetaData sMetaData;       // 0x19E4B30

    // Field names are not in the reference map.
    // The statistics: "num_instances", "num_meshes", "num_particles",
    // "num_propanims", "num_lights", "num_verts", "num_faces" and
    // "changelist".
    int mNumInstances;
    int mNumMeshes;
    int mNumParticles;
    int mNumPropanims;
    int mNumLights;
    int mNumVerts;
    int mNumFaces;
    int mChangelist;
    // The icon camera: "icon_cam_initialized", "icon_cam_xfm" (the
    // identity), "icon_cam_near" (0.1) and "icon_cam_far" (1000).
    bool mIconCamInitialized;
    Transform mIconCamXfm;
    float mIconCamNear;
    float mIconCamFar;
    // "instance_polling_mode": always, only when showing, or only when on
    // screen.
    int mInstancePollingMode;
    // "copy_on_instance": each instance gets its own copy of the entity
    // (Entity::Copy) rather than sharing the resource's.
    bool mCopyOnInstance;
    // "drives_parent": the instanced entity's root transform drives the
    // instancing object's (InstanceCom::IsDrivenByChild). Entity::Enter and
    // TransEntityResource::_LoadEntity read it on the root's component.
    bool mDrivesParent;
    // "static": the TransEntityResource suppresses its entity's poll.
    bool mStatic;
    // "capabilities": Manipulate Layers, Create Objects, Delete Objects
    // and Rename Objects, all set by default.
    unsigned int mCapabilities;
    // "show_entity": a property RndEntityCom polls to show or hide the
    // entity.
    PropRef mShowEntity;
    // "icon_data"; TransEntityResource::_LoadRoot copies it into the root
    // data of revisions before 12, and _OnResourcesLoaded moves it into the
    // resource.
    PropArray<unsigned char> mIconData;
    RuntimeData mRuntime;
};

static_assert(sizeof(EntityHeaderCom::LayerChanges) == 8);
static_assert(offsetof(EntityHeaderCom::RuntimeData, mNumModifiedLayers) == 96);
static_assert(offsetof(EntityHeaderCom::RuntimeData, mLayerChanges) == 104);
static_assert(offsetof(EntityHeaderCom::RuntimeData, mPollRequested) == 136);
static_assert(offsetof(EntityHeaderCom, mNumInstances) == 24);
static_assert(offsetof(EntityHeaderCom, mChangelist) == 52);
static_assert(offsetof(EntityHeaderCom, mIconCamInitialized) == 56);
static_assert(offsetof(EntityHeaderCom, mIconCamXfm) == 60);
static_assert(offsetof(EntityHeaderCom, mIconCamNear) == 108);
static_assert(offsetof(EntityHeaderCom, mInstancePollingMode) == 116);
static_assert(offsetof(EntityHeaderCom, mCopyOnInstance) == 120);
static_assert(offsetof(EntityHeaderCom, mDrivesParent) == 121);
static_assert(offsetof(EntityHeaderCom, mStatic) == 122);
static_assert(offsetof(EntityHeaderCom, mCapabilities) == 124);
static_assert(offsetof(EntityHeaderCom, mShowEntity) == 128);
static_assert(offsetof(EntityHeaderCom, mIconData) == 344);
static_assert(offsetof(EntityHeaderCom, mRuntime) == 384);
static_assert(sizeof(EntityHeaderCom) == 528);

// Whether the entity or one of the entities around it is "static". Emitted
// in entity/Utl.o (0x114E20) and used by RndEntityInstanceCom::_Enter. Name
// not in the reference map.
bool IsStaticEntity(const Entity* entity);  // 0x114E20
