#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector4.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class ComMetaData;
class PropRegistry;
class TransCom;

// The component that places an object in the scene's draw hierarchy
// (render/RndDrawNodeCom.o, 0x6C3D40 to 0x6C944F): "Holds hierarchical
// drawing-related info (bounding volume, environment, show/hide". Its class
// id is "DrawNode". Each poll it rebuilds the world bounding sphere from the
// local one, inherits the show/hide flags and the clip planes of its parent
// draw node, resolves the light environment index and keeps the inverse
// world transform the drawables and lights work in. The vtable at
// 0x1937DD0 has 41 slots. The object is 296 bytes. Field names are not in
// the reference map; the property members take the names of the
// properties the registry (0x6C4060) binds to their offsets.
class RndDrawNodeCom : public Component {
public:
    // "inherit_from": where the show/hide state and the clip planes come
    // from. Enumerator names not in the reference map; they follow the
    // registry's labels.
    enum InheritFrom : int {
        // "Transform Parent": the nearest draw node up the transform
        // parents.
        kInheritTransformParent = 1,
        // "Entity Root": the entity root's draw node.
        kInheritEntityRoot = 2,
    };

    // The show/hide flag bits. Only the bit the reconstructed code tests
    // is named; the inheritance masks are kept as values. Name not in the
    // reference map.
    enum ShowHideFlags : unsigned int {
        // Set while the object is hidden ("showing" is its inverse).
        kHidden = 1,
        // The bits a draw node takes from its parent draw node.
        kParentInheritedFlags = 0xBBF,
        // The bits it takes from the draw node its environment comes from.
        kEnvironInheritedFlags = 0x440,
    };

    // The runtime state after the properties (constructor 0x6C3E60,
    // inlined into RndDrawNodeCom's constructor). The map names the type;
    // its field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x6C3E60

        // Set on the entity root's draw node when a draw node of its entity
        // moved; the root then rebuilds its local sphere around them.
        bool mEntityRootSphereDirty;
        // Set when the local sphere changed (SetLocalSphere).
        bool mLocalSphereDirty;
        // Set by the poll that rebuilt the world sphere.
        bool mWorldSphereChanged;
        // "world_sphere": the local sphere in world space, its radius
        // scaled by "sphere_radius_mult".
        Sphere mWorldSphere;
        // "world_show_hide_flags": "show_hide_flags" with the inherited
        // bits. Bit 0 hides the object.
        unsigned int mWorldShowHideFlags;
        // "environ_index": the light manager's index of the environment, or
        // RndLightEnvironCom::kNoEnvironIndex. The draw instances take it as
        // their stencil group (RndMeshCom::_SyncDrawInstancesImpl,
        // 0x5CA2D0).
        int mEnvironIndex;
        // Set when "nested_light_environments" changed; the next _EditPoll
        // re-registers the nested lights.
        bool mNestedLightEnvironsDirty;
        // Set by the registry's upgrade of revision 0 data; the next
        // _OnResourcesLoaded takes "environment" from the scene's default
        // environment. Weak evidence.
        bool mUseDefaultEnviron;
        // "world_clip_plane_0" and "world_clip_plane_1": the planes through
        // the "clip_plane" objects, facing their z axes, or the parent's.
        Vector4 mWorldClipPlanes[2];
        // Lets an instancing draw node take its local sphere from the
        // instanced entity's root. Set by the constructor; no other writer
        // was found, so the name is weak.
        bool mUseInstancedSphere;
        // Lets the entity root inherit from the draw node of the object
        // that instances its entity. Set by the constructor; no other
        // writer was found, so the name is weak.
        bool mInheritFromParentEntity;
        // "editor_show_hide_flags".
        unsigned int mEditorShowHideFlags;
        // Cleared by the constructor; no reader was found.
        bool mReserved;
        // Set when an entity around the object polls only while on screen
        // (InstanceCom "instance_polling_mode" 2): the entity root then
        // keeps a local sphere around its entity's draw nodes. Always set in
        // the edit mode.
        bool mNeedsEntityRootSphere;
        // Set when a transform between the object and its parent draw node
        // can change its parent (the TransCom byte at 64); the parent is
        // then looked up every poll. Weak evidence.
        bool mParentDynamic;
        // Whether the world transform keeps the winding (a non-negative
        // determinant); the draw instances' front-face winding
        // (0x5CA2D0).
        bool mCounterClockwise;
        // The inverse of the TransCom's world transform, rebuilt when it
        // changes. The light components move their clip planes and
        // directions into the object's space with it (0x46D680, 0x47445B).
        Transform mInvWorldXfm;
        // The node pointers _CacheNodePointers caches.
        TransCom* mTransCom;
        // The object's instance component ("Instance") and instance pool
        // ("InstancePool"); neither class is modelled.
        Component* mInstance;
        Component* mInstancePool;
        // The draw node of the entity's root object.
        RndDrawNodeCom* mEntityRoot;
        // The draw node the show/hide flags and clip planes come from.
        RndDrawNodeCom* mParent;
        // The draw node the environment index comes from when the entity
        // is not a scene.
        RndDrawNodeCom* mEnvironParent;
        // The root draw node of the entity the object instances.
        RndDrawNodeCom* mInstancedRoot;
        // The instance pool of the object that instances the entity; the
        // entity root's sphere then encloses the pool's objects.
        Component* mParentInstancePool;
    };

    RndDrawNodeCom();  // 0x6C3D40
    // Copies the properties and starts a fresh runtime state; inlined into
    // _Imprint (0x6C8A70). Name not in the reference map.
    RndDrawNodeCom(const RndDrawNodeCom& other);

    // Slots 0-1: 0x6C3EF0, 0x6C3F60.
    ~RndDrawNodeCom() override;
    Symbol GetId() const override;          // slot 4: 0x6C89F0
    Symbol GetClassName() const override;   // slot 5: 0x6C8A00
    int CurrentRev() const override;        // slot 7: 0x6C8A10
    bool IsA(Symbol type) const override;   // slot 8: 0x6C8A30
    Component* AsComponent() override;      // slot 9: 0x6C8A60
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6C8A70
    // Slots 17-18: the component polls after the instance components and
    // follows TransCom in the component order, which the instance
    // components follow.
    void _GetPollOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x6C61A0
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x6C5EE0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6C8C80
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6C8C90
    // Slot 27: the object polls after its environment and clip-plane
    // objects.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;  // 0x6C5C80
    // Slot 29: caches the node pointers; always true.
    bool _OnResourcesLoaded() override;  // 0x6C63B0
    void _Enter() override;                   // slot 31: 0x6C6B50
    void _Exit(DestroyType type) override;    // slot 32: 0x6C7350
    // Slot 33 at 0x6C7390 runs _PollDrawNode.
    void _Poll() override;
    void _EditEnter() override;                 // slot 36: 0x6C7B60
    void _EditExit(DestroyType type) override;  // slot 37: 0x6C8380
    void _EditPoll() override;                  // slot 38: 0x6C8250

    // Replaces the bounding sphere and marks it for the next poll when it
    // changed. Inlined into the light components, for example
    // RndLightProbeCom::_Poll at 0x49C856. Name not in the reference map.
    void SetLocalSphere(const Sphere& sphere) {
        if (mLocalSphere.center.x == sphere.center.x &&
            mLocalSphere.center.y == sphere.center.y &&
            mLocalSphere.center.z == sphere.center.z &&
            mLocalSphere.radius == sphere.radius) {
            return;
        }
        mLocalSphere = sphere;
        mRuntime.mLocalSphereDirty = true;
    }
    // SetLocalSphere for a sphere in world space: moves it into the
    // object's space with mInvWorldXfm first. Only RndParticleCom::
    // _UpdateBoundingSphere calls it. Name not in the reference map.
    void SetLocalSphereFromWorld(const Sphere& worldSphere);  // 0x6C3FE0

    // Re-registers the lights of the instanced entity with the
    // environments in "nested_light_environments", or empties the array
    // without an instance component. RndSceneInstanceCom calls it
    // (0x43A923). The map's _SyncNestedLightEnvirons(ObjPtr const&).
    void _SyncNestedLightEnvirons();  // 0x6C82F0
    // Whether the entity's root object holds a RndSceneCom. The map's
    // _IsInScene(ObjPtr const&) const.
    bool _IsInScene() const;  // 0x6C83A0
    // Whether the instanced entities under the object's instance component
    // hold a light; no instance component means none. The map's
    // _ContainsNestedLights(ObjPtr const&) const.
    bool _ContainsNestedLights() const;  // 0x6C8400
    bool _ContainsNestedLightsRecur(Component& instance) const;  // 0x6C8470
    // The number of lights in those entities. The map's
    // _GetNumNestedLights(ObjPtr const&) const.
    int _GetNumNestedLights() const;  // 0x6C8590
    int _GetNumNestedLightsRecur(Component& instance) const;  // 0x6C8600
    // Sets the local sphere around the world spheres of the entity's other
    // draw nodes. Inlined into the polls; the copy has no caller. The
    // map's _ComputeEntityRootLocalSphere(ObjPtr const&).
    void _ComputeEntityRootLocalSphere();  // 0x6C8730
    // The same around the draw nodes of the parent instance pool's
    // objects. Inlined into the polls; the copy has no caller. Name not in
    // the reference map.
    void _ComputeInstancePoolLocalSphere();  // 0x6C88A0
    // The draw node to inherit from: for the entity root, the instancing
    // object's (when mInheritFromParentEntity is set); otherwise the
    // nearest up the transform parents for kInheritTransformParent, or
    // the entity root. Name not in the reference map.
    RndDrawNodeCom* _FindParent() const;  // 0x6C68C0
    // The draw node the environment index comes from: the entity root, or
    // the root's parent for the root itself. Inlined into
    // _OnResourcesLoaded; the copy has no caller. Name not in the
    // reference map.
    RndDrawNodeCom* _FindEnvironParent() const;  // 0x6C6A30
    // The root draw node of the entity the instance component or the
    // instance pool's resource instances. Inlined into _OnResourcesLoaded;
    // the copy has no caller. Name not in the reference map.
    RndDrawNodeCom* _FindInstancedRoot() const;  // 0x6C6A50

    // Registers the class. Emitted in render/RndInit.o at 0x3EE650. Not
    // reconstructed: the registration helpers it inlines are not
    // modelled.
    static void Init();
    // The class factory, emitted with the other render factories. The
    // placement factory beside it (0x404460) is not modelled.
    static Component* _Create();  // 0x404430
    // Registers the class description and the properties, with the
    // callbacks of "showing", "environment" and
    // "nested_light_environments" and the upgrade of the show/hide flags
    // (the functor vtables at 0x1937F18 to 0x1938008). Not reconstructed:
    // the property metadata's attributes are not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x6C4060

    // The class symbol, "DrawNode", constructed by the static initializer at
    // 0x6C93CA.
    static Symbol sId;  // 0x1AB05D8
    // The class's second symbol, also "DrawNode", which
    // GameObject::CreateComponent and the component-order lists take. Name
    // not in the reference map.
    static Symbol sClassName;  // 0x1AB05E0
    static PropRegistry sPropRegistry;  // 0x1AB05F0
    static ComMetaData sMetaData;       // 0x1AB0690

    // "local_sphere": the bounding sphere in the object's space; the
    // drawables set it.
    Sphere mLocalSphere;
    // "sphere_radius_mult".
    float mSphereRadiusMult;
    // "show_hide_flags".
    unsigned int mShowHideFlags;
    // "inherit_from", an InheritFrom.
    int mInheritFrom;
    // "environment": the RndLightEnvironCom object the object receives
    // light from, or the null id.
    GameObjectId mEnvironment;
    // "nested_light_environments": the environments the lights of an
    // instanced entity contribute to (RndLightCom::_InitBookkeeping,
    // 0x46CE60).
    PropArray<GameObjectId> mNestedLightEnvironments;
    // "clip_plane_0" and "clip_plane_1": the objects whose z axes clip the
    // object and its children, or the null id.
    GameObjectId mClipPlanes[2];
    // The map's RndDrawNodeCom::RuntimeData.
    RuntimeData mRuntime;

private:
    // The body the polls share: the entity root's sphere, the world
    // sphere, the inherited flags and clip planes and the environment
    // index. Inlined into _Enter, _PollDrawNode and _EditEnter. Name not
    // in the reference map.
    void _SyncWorldState();
    // Rebuilds mInvWorldXfm and mCounterClockwise from the TransCom.
    // Inlined into the same. Name not in the reference map.
    void _UpdateInvWorldXfm();
    // Caches the node pointers; inlined into _OnResourcesLoaded. The map's
    // _CacheNodePointers(ObjPtr const&).
    void _CacheNodePointers();
    // The game-mode poll: refreshes the node pointers that can change, then
    // _SyncWorldState. _EditPoll runs it too. The map's _Poll(ObjPtr
    // const&); this build keeps it apart from slot 33.
    void _PollDrawNode();  // 0x6C73A0
};

static_assert(offsetof(RndDrawNodeCom::RuntimeData, mWorldSphere) == 0x04);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mWorldShowHideFlags) == 0x14);
static_assert(offsetof(RndDrawNodeCom::RuntimeData, mEnvironIndex) == 0x18);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mNestedLightEnvironsDirty) ==
    0x1C);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mUseDefaultEnviron) == 0x1D);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mWorldClipPlanes) == 0x20);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mUseInstancedSphere) == 0x40);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mEditorShowHideFlags) == 0x44);
static_assert(offsetof(RndDrawNodeCom::RuntimeData, mReserved) == 0x48);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mCounterClockwise) == 0x4B);
static_assert(offsetof(RndDrawNodeCom::RuntimeData, mInvWorldXfm) == 0x4C);
static_assert(offsetof(RndDrawNodeCom::RuntimeData, mTransCom) == 0x80);
static_assert(offsetof(RndDrawNodeCom::RuntimeData, mEntityRoot) == 0x98);
static_assert(
    offsetof(RndDrawNodeCom::RuntimeData, mParentInstancePool) == 0xB8);
static_assert(sizeof(RndDrawNodeCom::RuntimeData) == 0xC0);
static_assert(offsetof(RndDrawNodeCom, mLocalSphere) == 0x18);
static_assert(offsetof(RndDrawNodeCom, mSphereRadiusMult) == 0x28);
static_assert(offsetof(RndDrawNodeCom, mShowHideFlags) == 0x2C);
static_assert(offsetof(RndDrawNodeCom, mInheritFrom) == 0x30);
static_assert(offsetof(RndDrawNodeCom, mEnvironment) == 0x34);
static_assert(offsetof(RndDrawNodeCom, mNestedLightEnvironments) == 0x38);
static_assert(offsetof(RndDrawNodeCom, mClipPlanes) == 0x60);
static_assert(offsetof(RndDrawNodeCom, mRuntime) == 0x68);
static_assert(sizeof(RndDrawNodeCom) == 0x128);
