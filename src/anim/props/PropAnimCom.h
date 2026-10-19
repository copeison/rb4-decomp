#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

// The component on the root object of a property animation's entity that
// describes the animation (anim/PropAnimCom.o, 0x2FBC00-0x2FCAB1; class id
// "PropAnim", alias "PropAnimCom"): the frame range its keys span, the time
// units, looping and the order of the objects it targets. The vtable at
// 0x18F6FB0 has 41 slots. The object is 80 bytes. Field names are not in
// the reference map.
class PropAnimCom : public Component {
public:
    PropAnimCom();  // 0x2FBC00
    // Slots 0-1: 0x2FC1E0, 0x2FC250.
    ~PropAnimCom() override {}

    Symbol GetId() const override;          // slot 4: 0x2FC6D0
    Symbol GetClassName() const override;   // slot 5: 0x2FC6E0
    int CurrentRev() const override;        // slot 7: 0x2FC6F0
    bool IsA(Symbol type) const override;   // slot 8: 0x2FC710
    Component* AsComponent() override;      // slot 9: 0x2FC740
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2FC750
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2FC8C0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2FC8D0
    // Slot 31: measures the frame range, except in regression tests.
    void _Enter() override;  // 0x2FC6A0
    // Slot 33: nothing. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0x2FC8E0
    // Slot 38: polls as in the game mode.
    void _EditPoll() override;  // 0x2FC6C0

    // Sets the frame range to span the first and last keys of every key
    // component on the entity's objects other than the root. Name not in
    // the reference map (PropAnimResource::UpdateFrameRange calls it).
    void UpdateFrameRange();  // 0x2FC2D0
    // Replace and copy out "target_order". Names not in the reference map.
    void SetTargetOrder(const eastl::vector<GameObjectId>& order);  // 0x2FC3E0
    void GetTargetOrder(eastl::vector<GameObjectId>& order) const;  // 0x2FC4F0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A7C70, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B1E10.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2FBC70

    // "PropAnim", constructed by the static initializer at 0x2FCA00, which
    // first sets a shared header's null object id to -1 (0x19F96D8).
    static Symbol sId;  // 0x19F96E0
    // Also "PropAnim". Name not in the reference map.
    static Symbol sClassName;           // 0x19F96E8
    static PropRegistry sPropRegistry;  // 0x19F96F0
    static ComMetaData sMetaData;       // 0x19F9790

    // "frame_range_start" and "frame_range_end": an empty range (the
    // largest float to its negation) until measured. The constructor calls
    // an out-of-line initializer of the pair (0x1183320, beside its
    // widening at 0x1183300, which UpdateFrameRange calls).
    float mFrameRangeStart;
    float mFrameRangeEnd;
    // "time_units": a timeline (0-3), the same counted in frames (4-7), or
    // by default 8, the "default_timeline" of the AnimDriverCom of the
    // entity resource the animation is inlined in
    // (PropAnimResource::GetUnits).
    int mTimeUnits;
    // "is_looping".
    bool mLooping;
    // "target_order": the animated objects, in the editor's order.
    PropArray<GameObjectId> mTargetOrder;
};

static_assert(offsetof(PropAnimCom, mFrameRangeStart) == 24);
static_assert(offsetof(PropAnimCom, mTimeUnits) == 32);
static_assert(offsetof(PropAnimCom, mLooping) == 36);
static_assert(offsetof(PropAnimCom, mTargetOrder) == 40);
static_assert(sizeof(PropAnimCom) == 80);
