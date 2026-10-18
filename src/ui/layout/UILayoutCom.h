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
class RndSceneResource;

// The layout-wide rendering settings on a UILayoutResource's root
// ("Layout-wide rendering settings"): the order the layout's scenes poll
// and draw in, and the scenes drawn only over the HMD mirror. Its class
// symbol is "UILayout". It lies before UILayoutResource in the binary
// (0x8A9340 to 0x8AAA3F); the map, which is older, has no object for it.
// Only the members the layout uses are declared, and only its poll
// managers are reconstructed. The vtable is at 0x195D018; the object is 280
// bytes. Name not in the reference map.
class UILayoutCom : public RndDrawableEntityCom {
public:
    // A scene entity that became ready while the layout's poll range was
    // released (UILayoutResource::_OnEntityReady). Name and field names are
    // not in the reference map.
    struct ReadyScene {
        RndSceneResource* mResource;
        Entity* mEntity;
    };

    // Registers the class: its property registry, metadata and factory.
    // Emitted in ui/UI.o at 0x892360. Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();

    // Removes the object from the poll, draw and HMD mirror orders. Name
    // not in the reference map.
    void RemoveSceneObject(GameObjectId id);  // 0x8A9B00
    // Resets and destroys the scenes' poll groups and nested poll manager.
    // Name not in the reference map.
    void _DestroyPollMgrs();  // 0x8A9830
    // Creates the poll group the layout's scenes are selected into, named
    // for the layout's file, unless there is one. Name not in the
    // reference map.
    void _ObtainPollGroup();  // 0x8A98D0
    // Creates the scenes' poll groups and nested poll manager, named for
    // the layout's file. Name not in the reference map.
    void _CreatePollMgrs();  // 0x8A9960

    static Symbol sId;         // 0x1AC8D50, "UILayout"
    // The class symbol GameObject::CreateComponent takes; also
    // "UILayout". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC8D58

    // Field names are not in the reference map.
    // "poll_order": the scenes in poll order, the reverse of the order
    // they handle input in.
    PropArray<GameObjectId> mPollOrder;
    // "draw_order": the scenes in draw order.
    PropArray<GameObjectId> mDrawOrder;
    // "hmd_mirror_overlays": the scenes drawn only over the HMD mirror, in
    // draw order.
    PropArray<GameObjectId> mHmdMirrorOverlays;
    // The poll group (_ObtainPollGroup), and the nested poll manager and
    // poll groups (_CreatePollMgrs) of the layout's scenes. The manager is
    // a small-pool block.
    PollGroup* mPollGroup;
    PollMgr* mPollMgr;
    RndSceneCom::PollGroups* mScenePollGroups;
    // Two lists the threaded poll range keeps: the scene instances whose
    // poll groups follow the layout's, and the poll jobs the layout polls.
    // Not modelled.
    unsigned char mPollRangeLists[64];
    // The scenes to enter at once when the poll range is synced again.
    eastl::list<ReadyScene> mReadyScenes;
    // Set while the threaded poll range is released
    // (UILayoutResource::_ClearThreadedPollRange).
    bool mPollRangeReleased;
};

static_assert(offsetof(UILayoutCom, mPollOrder) == 32);
static_assert(offsetof(UILayoutCom, mDrawOrder) == 72);
static_assert(offsetof(UILayoutCom, mHmdMirrorOverlays) == 112);
static_assert(offsetof(UILayoutCom, mPollGroup) == 152);
static_assert(offsetof(UILayoutCom, mPollMgr) == 160);
static_assert(offsetof(UILayoutCom, mScenePollGroups) == 168);
static_assert(offsetof(UILayoutCom, mReadyScenes) == 240);
static_assert(offsetof(UILayoutCom, mPollRangeReleased) == 272);
static_assert(sizeof(UILayoutCom) == 280);
