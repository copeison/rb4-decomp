#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "render/scene/RndDrawableEntityCom.h"
#include "render/scene/RndSceneCom.h"
#include "utl/text/Symbol.h"

class PollGroup;
class RndSceneDrawer;

// The entity-wide rendering settings on the root object of an entity that
// RndEntityInstanceCom instances (render/RndEntityCom.o, 0x6CA2D0-0x6CAE7B).
// Its class id is "RndEntity". It holds the entity's preview LOD and, at run
// time, the entity's poll groups and scene drawer, which the instance
// component selects and draws through. The vtable at 0x1938278 has 41
// slots. The object is 72 bytes.
class RndEntityCom : public RndDrawableEntityCom {
public:
    // The per-component state, created empty and not copied by _Imprint.
    // Field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x6CA310

        // Created on demand by ObtainPollMgr and owned.
        PollGroup* mPollGroup;
        // Created on demand by ObtainPollGroups and owned.
        RndSceneCom::PollGroups* mPollGroups;
        // The drawer the entity draws through: an ancestor's, or one the
        // component created and owns (mOwnsSceneDrawer).
        RndSceneDrawer* mSceneDrawer;
        bool mOwnsSceneDrawer;
        // Set when the instance enters the entity without entering it
        // immediately (0x6CC9E7, 0x6CCF1D); the instance's next poll then
        // enters it immediately under the poll groups (0x6CCD20), which
        // clears it.
        bool mNeedsImmediateEnter;
    };

    RndEntityCom();            // 0x6CA2D0
    ~RndEntityCom() override;  // slots 0-1: 0x6CA320, 0x6CA3A0

    Symbol GetId() const override;         // slot 4: 0x6CABC0
    Symbol GetClassName() const override;  // slot 5: 0x6CABD0
    int CurrentRev() const override;       // slot 7: 0x6CABE0
    bool IsA(Symbol type) const override;  // slot 8: 0x6CAC00
    Component* AsComponent() override;     // slot 9: 0x6CAC30
    // Slot 10. The map's _Imprint(char*, Component*&, bool). The copy keeps
    // the preview LOD and starts with empty runtime data. Not
    // reconstructed: the thread's imprint state is not modelled.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6CAC40
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6CAD90
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6CADA0
    // Slots 32 and 37: forget a drawer the component does not own and
    // destroy the poll groups. The map lists _Fixup(bool) for the class
    // instead; this build has neither _Fixup nor these in the map.
    void _Exit(DestroyType type) override;      // 0x6CAB20
    void _EditExit(DestroyType type) override;  // 0x6CAB70

    // The entity's poll group ("<file>"), created on first use. The map's
    // ObtainPollMgr() returns a PollMgr; this build's returns the group.
    PollGroup* ObtainPollMgr();  // 0x6CA430
    // The entity's two nested poll managers, created on first use. Name
    // not in the reference map.
    RndSceneCom::PollGroups* ObtainPollGroups();  // 0x6CA480
    // Uses the scene drawer of the nearest entity up the tree, without
    // owning it. The map's signature is FindAncestorSceneDrawer(ObjPtr
    // const&); this build reads the entity through the component's owner,
    // and its only caller (0x6CC991) ignores any result.
    void FindAncestorSceneDrawer();  // 0x6CA4D0
    // The scene drawer: an ancestor's when there is one, else a new owned
    // one. The map's signature is ObtainSceneDrawer(ObjPtr const&).
    RndSceneDrawer* ObtainSceneDrawer();  // 0x6CA500
    // The scene drawer, creating an owned one when there is none.
    RndSceneDrawer* ObtainOwnedSceneDrawer();  // 0x6CA560

    // Registers the class. Inline in the map (render/RndInit.o's
    // RndEntityCom::Init); this build emits it at 0x3ED3A0. Not
    // reconstructed.
    static void Init();
    // The class factory. Emitted in render/RndInit.o at 0x4042A0; the
    // in-place construction beside it (0x4042D0) is not modelled.
    static Component* _Create();
    // Describes the class ("Entity-wide rendering settings", by Daniel
    // Sproul): a rendering class with no editor restrictions, flagged
    // mLightweight and allowed in RndEntityResources, under
    // RndDrawableEntityCom (_InitAsSuperclass). Registers "preview_lod" at
    // +0x20 with its help text and the three LOD names (0x4423A0) as its
    // values. Not reconstructed: the int property metadata it fills is not
    // modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6CA5A0

    static Symbol sId;  // 0x1AB0AE0, "RndEntity"
    // Also "RndEntity". Name not in the reference map.
    static Symbol sClassName;           // 0x1AB0AE8
    static PropRegistry sPropRegistry;  // 0x1AB0AF0
    static ComMetaData sMetaData;       // 0x1AB0B90

    // "preview_lod": the level of detail to draw when viewing the entity on
    // its own. Name not in the reference map.
    int mPreviewLod;
    RuntimeData mRuntimeData;
};

static_assert(offsetof(RndEntityCom::RuntimeData, mPollGroups) == 0x8);
static_assert(offsetof(RndEntityCom::RuntimeData, mSceneDrawer) == 0x10);
static_assert(offsetof(RndEntityCom::RuntimeData, mOwnsSceneDrawer) == 0x18);
static_assert(offsetof(RndEntityCom::RuntimeData, mNeedsImmediateEnter) == 0x19);
static_assert(sizeof(RndEntityCom::RuntimeData) == 0x20);
static_assert(offsetof(RndEntityCom, mPreviewLod) == 0x20);
static_assert(offsetof(RndEntityCom, mRuntimeData) == 0x28);
static_assert(sizeof(RndEntityCom) == 0x48);
