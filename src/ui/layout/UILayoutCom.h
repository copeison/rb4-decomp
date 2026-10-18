#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "render/scene/RndDrawableEntityCom.h"
#include "render/scene/RndSceneCom.h"
#include "utl/containers/List.h"
#include "utl/text/Symbol.h"

class Entity;
class PollGroup;
class PollMgr;
class RndSceneInstanceCom;
class RndSceneResource;

// The layout-wide rendering settings on a UILayoutResource's root
// ("Layout-wide rendering settings"): the order the layout's scenes poll
// and draw in, and the scenes drawn only over the HMD mirror. Its class
// symbol is "UILayout". It lies before UILayoutResource in the binary
// (0x8A9340 to 0x8AAA3F, with the static initializer at 0x8AA970); the
// map, which is older, has no object for it. The vtable at 0x195D018 has 41
// slots. The object is 280 bytes. Name not in the reference map.
class UILayoutCom : public RndDrawableEntityCom {
public:
    // A scene entity that became ready while the layout's poll range was
    // released (UILayoutResource::_OnEntityReady). Name and field names are
    // not in the reference map.
    struct ReadyScene {
        RndSceneResource* mResource;
        Entity* mEntity;
    };

    UILayoutCom();  // 0x8A9400
    // The imprint's copy keeps the orders; the poll state starts afresh.
    // Inlined into _Imprint. Not in the reference map.
    UILayoutCom(const UILayoutCom& other)
        : RndDrawableEntityCom(other),
          mPollOrder(),
          mDrawOrder(),
          mHmdMirrorOverlays(),
          mPollMgr(nullptr),
          mPollGroup(nullptr),
          mScenePollGroups(nullptr),
          mThreadedScenes(),
          mStartPollGroups(),
          mReadyScenes(),
          mPollRangeReleased(false) {
        mPollOrder._Copy(other.mPollOrder);
        mDrawOrder._Copy(other.mDrawOrder);
        mHmdMirrorOverlays._Copy(other.mHmdMirrorOverlays);
    }
    // Slots 0-1: 0x8A9600, 0x8A98B0. Destroys the poll groups and the poll
    // manager.
    ~UILayoutCom() override;

    Symbol GetId() const override;         // slot 4: 0x8AA510
    Symbol GetClassName() const override;  // slot 5: 0x8AA520
    int CurrentRev() const override;       // slot 7: 0x8AA530
    bool IsA(Symbol type) const override;  // slot 8: 0x8AA550
    Component* AsComponent() override;     // slot 9: 0x8AA580
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8AA590
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8AA830
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8AA840

    // Registers the class (InitComponentClass), emitted in ui/UI.o.
    static void Init();  // 0x892360
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x8950A0) is InitComponentClass's.
    static Component* _Create();  // 0x895070
    // Describes the class after RndDrawableEntityCom::_InitAsSuperclass and
    // registers the three scene orders.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8A9E30

    // Removes the object from the poll, draw and HMD mirror orders. Name
    // not in the reference map.
    void RemoveSceneObject(GameObjectId id);  // 0x8A9B00
    // Resets and destroys the scenes' poll groups and the layout's poll
    // group. Name not in the reference map.
    void _DestroyPollGroups();  // 0x8A9830
    // Creates the poll manager the layout's scenes are selected into,
    // named for the layout's file, unless there is one. Name not in the
    // reference map.
    void _ObtainPollMgr();  // 0x8A98D0
    // Creates the scenes' poll groups and the layout's poll group, named
    // for the layout's file. Name not in the reference map.
    void _CreatePollGroups();  // 0x8A9960

    static Symbol sId;                  // 0x1AC8D50, "UILayout"
    // The class symbol GameObject::CreateComponent takes; also
    // "UILayout". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC8D58
    static PropRegistry sPropRegistry;  // 0x1AC8D60
    static ComMetaData sMetaData;       // 0x1AC8E00

    // Field names are not in the reference map.
    // "poll_order": the scenes in poll order, the reverse of the order
    // they handle input in.
    PropArray<GameObjectId> mPollOrder;
    // "draw_order": the scenes in draw order.
    PropArray<GameObjectId> mDrawOrder;
    // "hmd_mirror_overlays": the scenes drawn only over the HMD mirror, in
    // draw order.
    PropArray<GameObjectId> mHmdMirrorOverlays;
    // The poll manager (_ObtainPollMgr), and the poll group and the
    // scenes' poll groups (_CreatePollGroups).
    PollMgr* mPollMgr;
    PollGroup* mPollGroup;
    RndSceneCom::PollGroups* mScenePollGroups;
    // The scenes of the threaded poll range: the instanced scenes in poll
    // order (UILayoutResource::_SyncThreadedPollRange).
    eastl::list<RndSceneInstanceCom*> mThreadedScenes;
    // Groups the layout's poll starts before its own and empties after;
    // nothing found adds to it.
    eastl::list<PollGroup*> mStartPollGroups;
    // The scenes to enter at once when the poll range is synced again.
    eastl::list<ReadyScene> mReadyScenes;
    // Set while the threaded poll range is released
    // (UILayoutResource::_ClearThreadedPollRange).
    bool mPollRangeReleased;
};

static_assert(offsetof(UILayoutCom, mPollOrder) == 32);
static_assert(offsetof(UILayoutCom, mDrawOrder) == 72);
static_assert(offsetof(UILayoutCom, mHmdMirrorOverlays) == 112);
static_assert(offsetof(UILayoutCom, mPollMgr) == 152);
static_assert(offsetof(UILayoutCom, mPollGroup) == 160);
static_assert(offsetof(UILayoutCom, mScenePollGroups) == 168);
static_assert(offsetof(UILayoutCom, mThreadedScenes) == 176);
static_assert(offsetof(UILayoutCom, mStartPollGroups) == 208);
static_assert(offsetof(UILayoutCom, mReadyScenes) == 240);
static_assert(offsetof(UILayoutCom, mPollRangeReleased) == 272);
static_assert(sizeof(UILayoutCom) == 280);
