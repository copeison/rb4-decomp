#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/geometry/Rect.h"
#include "math/vector/Vector2.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class PollGroup;
class PollMgr;
class Vector2i;

// The component through which a parent entity instances a scene
// (render/RndSceneInstanceCom.o, 0x4362E0 to 0x438200): an InstanceCom of a
// RndSceneResource ("Scene instance settings") with the scene's draw order
// and placement in its parent's viewport, and the scenes that mask it. Its
// class id is "SceneInstance". The vtable at 0x1901170 has 50 slots:
// InstanceCom's 49 and the DynamicComResource override; the vtables at +24
// and +64 are 0x1901310 and 0x1901328. The object is 296 bytes. Field names
// are not in the reference map; the properties are named after the
// registry (0x436AB0).
class RndSceneInstanceCom : public InstanceCom {
public:
    // The run-time state, which imprints start afresh. The map names the
    // type; the field names are not in the reference map.
    struct RuntimeData {
        // Set unless the instanced scene polls only while it shows; the
        // poll then follows the draw node's show/hide flag. _Enter sets it.
        bool mAlwaysPolls;
        // Set by the "masked_by" properties' handlers (0x437FC0,
        // 0x437FD0); the next poll rebuilds mMaskingScenes.
        bool mMaskingScenesDirty;
        // Set by "unique_resource"'s handler (0x437FB0); the next edit poll
        // reloads the resources.
        bool mReloadPending;
        // The scene instances that mask this one: those of
        // "masked_by_scenes" and the dynamically loaded ones whose files
        // "masked_by_dynamic_scenes" names.
        PropArray<GameObjectId> mMaskingScenes;
    };

    RndSceneInstanceCom();  // 0x4362E0
    // Copies the properties for an imprint, the arrays through
    // PropArrayBase::_Copy; the run-time state starts afresh. Inlined into
    // _Imprint (0x437C90).
    RndSceneInstanceCom(const RndSceneInstanceCom& other);
    // Slots 0-1: 0x436480, 0x4365C0.
    ~RndSceneInstanceCom() override;
    Symbol GetId() const override;          // slot 4: 0x437C10
    Symbol GetClassName() const override;   // slot 5: 0x437C20
    int CurrentRev() const override;        // slot 7: 0x437C30
    bool IsA(Symbol type) const override;   // slot 8: 0x437C50
    Component* AsComponent() override;      // slot 9: 0x437C80
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x437C90
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x437F60
    ComMetaData& _GetMetaData() override;       // slot 23: 0x437F70
    // Slot 27: the poll dependencies the root's UI layout component gives
    // the object. Not reconstructed: that component (0x8A9CD0) is not
    // modelled. The map's signature starts with an EntityPtr.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;  // 0x437B50
    // Slot 29 at 0x4379C0: InstanceCom's, then the masking scenes.
    bool _OnResourcesLoaded() override;
    // Slot 31 at 0x4379F0: whether the scene always polls, then
    // InstanceCom's enter.
    void _Enter() override;
    // Slot 33 at 0x437A70: activates the scene while its draw node shows,
    // unless it always polls, and rebuilds dirty masking scenes.
    void _Poll() override;
    // Slot 38 at 0x437B00: a pending reload, the masking scenes, then
    // InstanceCom's edit poll.
    void _EditPoll() override;
    // Slot 41 at 0x437BE0: "reenter_shared_children".
    bool _ReenterSharedChildren() const override;
    // Slots 42-43 at 0x437BF0 and 0x437C00: false.
    bool _HasParentPollDep() const override;
    bool _CanBeDrivenByChild() const override;
    // Slot 49 at 0x437BC0 (thunk 0x437BD0): "unique_resource".
    bool _ShouldLoadUniqueResource() const override;

    // "use_threaded_poll". The map's CanUseThreadedPoll() const.
    bool CanUseThreadedPoll() const;  // 0x4365E0
    // The rectangle of the parent's viewport the instanced scene projects
    // into, for a target of the given size: "projection_rect" around the
    // centre, moved by "projection_offset" (horizontally scaled from the
    // output's aspect to the target's).
    Hmx::Rect CalcProjectionRect(const Vector2i& size) const;  // 0x4365F0
    // The poll manager of the instanced scene's RndSceneCom. Name not in
    // the reference map.
    PollMgr* GetScenePollMgr() const;  // 0x436670
    // The drawer's (key 0) and the scene's (key 1) poll groups of the
    // instanced scene, or null while no scene is instanced. Names not in
    // the reference map.
    PollGroup* GetSceneDrawerPollGroup() const;  // 0x4366C0
    PollGroup* GetScenePollGroup() const;        // 0x4366E0
    // Rebuilds mMaskingScenes. Not reconstructed: the dynamically loaded
    // scenes are found through the root's UI layout component, which is
    // not modelled. Name not in the reference map.
    void _SyncMaskingScenes();  // 0x436710

    // Registers the class (InitComponentClass), emitted in
    // render/RndInit.o.
    static void Init();  // 0x3F2800
    // The class factory, emitted in render/RndInit.o.
    static Component* _Create();  // 0x404C10
    // Registers "draw_order", "unique_resource", "use_threaded_poll",
    // "reenter_shared_children", "projection_offset", "projection_rect",
    // "masked_by_scenes", "masked_by_dynamic_scenes" and
    // "disable_load_tag" after InstanceCom's, and gives the file property
    // the scene resource class.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x436AB0
    // Whether instances with the tag are loaded; true for every tag in this
    // build. GameStartupCom asks it before preloading tagged scenes.
    static bool ShouldLoadInstanceWithTag(Symbol tag);  // 0x436A90

    static Symbol sId;  // 0x1A72AD8, "SceneInstance"
    // Also "SceneInstance". Name not in the reference map.
    static Symbol sClassName;           // 0x1A72AE0
    static PropRegistry sPropRegistry;  // 0x1A72AF0
    static ComMetaData sMetaData;       // 0x1A72B90

    // "draw_order": the scene's order within its layout.
    int mDrawOrder;
    // "unique_resource": load a private copy of the scene.
    bool mUniqueResource;
    // "use_threaded_poll": the layout may poll the scene in parallel.
    bool mUseThreadedPoll;
    // "projection_offset" (normalized) and "projection_rect" (width and
    // height, 1 by default).
    Vector2 mProjectionOffset;
    Vector2 mProjectionRect;
    // "masked_by_scenes": the scenes that mask this one.
    PropArray<GameObjectId> mMaskedByScenes;
    // "masked_by_dynamic_scenes": the files of dynamically loaded scenes
    // that mask this one.
    PropArray<ResourcePath> mMaskedByDynamicScenes;
    // "disable_load_tag": a tag of the config's disable_scene_loading
    // block that keeps the scene from loading.
    Symbol mDisableLoadTag;
    // "reenter_shared_children".
    bool mReenterSharedChildren;
    RuntimeData mRuntimeData;
};

static_assert(offsetof(RndSceneInstanceCom, mDrawOrder) == 0x80);
static_assert(offsetof(RndSceneInstanceCom, mUseThreadedPoll) == 0x85);
static_assert(offsetof(RndSceneInstanceCom, mProjectionOffset) == 0x88);
static_assert(offsetof(RndSceneInstanceCom, mProjectionRect) == 0x90);
static_assert(offsetof(RndSceneInstanceCom, mMaskedByScenes) == 0x98);
static_assert(offsetof(RndSceneInstanceCom, mMaskedByDynamicScenes) == 0xC0);
static_assert(offsetof(RndSceneInstanceCom, mDisableLoadTag) == 0xE8);
static_assert(offsetof(RndSceneInstanceCom, mReenterSharedChildren) == 0xF0);
static_assert(offsetof(RndSceneInstanceCom, mRuntimeData) == 0xF8);
static_assert(offsetof(RndSceneInstanceCom::RuntimeData, mMaskingScenes) == 8);
static_assert(sizeof(RndSceneInstanceCom) == 0x128);
