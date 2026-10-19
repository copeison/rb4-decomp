#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNodeSetValue.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The node that applies one frame of an animation
// (stategraph/StateNodeSetAnimFrame.o, 0x78DA30-0x7913FF; class id
// "StateNodeSetAnimFrame"). The frame is the float value of its
// StateNodeSetValue<float> base, so it can be driven, converted and
// blended like a property; the instance starts the animation ("anim", or
// the clip of "clip"'s CharClipCom, as "anim_type" chooses) on the graph's
// AnimDriverCom with its range set to that frame. The vtable at 0x1945C60
// has 52 slots. The object is 480 bytes. The object also holds the copies
// of StateNodeSetValue<float>'s functions the map lists with it; this
// build emits those with the property nodes.
class StateNodeSetAnimFrame : public StateNodeSetValue<float> {
public:
    // A node in a running graph: the frame's animation instance (the map's
    // StateNodeSetAnimFrame::Instance). The vtable at 0x1945E10 has 20
    // slots. The object is 104 bytes, from the small-block pool.
    class Instance : public StateNodeSetValue<float>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        Instance(const StateNodeSetAnimFrame& node, StateGraphInstance& graph);  // 0x78EBE0
        // Slots 0-1 at 0x78F130 and 0x78F160.
        ~Instance() override {}

        // Slot 15: the current frame of the playback started under the
        // node's instance name. The map's _GetCurrentValue(float&) const.
        bool _GetCurrentValue(float& value) const override;  // 0x78EC70
        // Slot 17: starts the animation at the frame unless that frame is
        // applied already. The map's _ApplyValue(float const&) const.
        void _ApplyValue(const float& value) const override;  // 0x78ED00
        // Slot 18: whether the other instance plays the same animation. The
        // map has StateNodeSetAnimFrame::Instance::_IsBlendPartner
        // (StateNodeInstance const&) const in this place.
        bool _HasSameTarget(const StateNodeInstance& other) const override;  // 0x78EC30
        // Slot 19: finished once the playback no longer runs. The map's
        // _IsFinishedApplyingValue() const.
        bool _IsFinishedApplyingValue() const override;  // 0x78EDF0

        // Field names are not in the reference map.
        const StateNodeSetAnimFrame* mAnimFrameNode;
        // The playback _ApplyValue last started, and its frame (1e30 until
        // then).
        mutable AnimInstID mAnimId;
        mutable float mAppliedFrame;
    };

    StateNodeSetAnimFrame();  // 0x78DA30
    // Copies the node for an imprint. Inlined into _Imprint (0x78EF70).
    StateNodeSetAnimFrame(const StateNodeSetAnimFrame& other)
        : StateNodeSetValue<float>(other),
          mAnimType(other.mAnimType),
          mClip(other.mClip),
          mAnim(other.mAnim),
          mInstanceName(other.mInstanceName) {}
    // Slots 0-1: 0x78E680, 0x78EE20.
    ~StateNodeSetAnimFrame() override {}

    Symbol GetId() const override;          // slot 4: 0x78EEF0
    Symbol GetClassName() const override;   // slot 5: 0x78EF00
    int CurrentRev() const override;        // slot 7: 0x78EF10
    bool IsA(Symbol type) const override;   // slot 8: 0x78EF30
    Component* AsComponent() override;      // slot 9: 0x78EF60
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x78EF70
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x78F0A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x78F0B0
    // Slot 42: "<animation>: frame = <value>", or nothing without an
    // animation.
    const char* GetEditorText() const override;  // 0x78E7A0
    // Slot 45.
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x78F0C0

    // The path of the animation: "anim", or the clip resource of "clip"'s
    // CharClipCom in the context entity's prototype. Name not in the
    // reference map.
    ResourcePath GetAnimPath() const;  // 0x78EA70

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F9F10.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FFF30.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x78DC10

    static Symbol sId;  // 0x1AB9410, "StateNodeSetAnimFrame"
    // Also "StateNodeSetAnimFrame". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB9418
    static PropRegistry sPropRegistry;  // 0x1AB9420
    static ComMetaData sMetaData;       // 0x1AB94C0

    // Field names are not in the reference map; they follow the
    // properties.
    // "anim_type": the resource class of the animation.
    Symbol mAnimType;
    // "clip": the object whose CharClipCom defines the clip.
    GameObjectId mClip;
    // "anim": the PropAnimResource or CharAnimResource.
    ResourcePath mAnim;
    // "anim_instance_name": the name the playback is started under.
    Symbol mInstanceName;
};

static_assert(offsetof(StateNodeSetAnimFrame, mAnimType) == 448);
static_assert(offsetof(StateNodeSetAnimFrame, mClip) == 456);
static_assert(offsetof(StateNodeSetAnimFrame, mAnim) == 464);
static_assert(offsetof(StateNodeSetAnimFrame, mInstanceName) == 472);
static_assert(sizeof(StateNodeSetAnimFrame) == 480);
static_assert(offsetof(StateNodeSetAnimFrame::Instance, mAnimFrameNode) == 72);
static_assert(offsetof(StateNodeSetAnimFrame::Instance, mAnimId) == 80);
static_assert(offsetof(StateNodeSetAnimFrame::Instance, mAppliedFrame) == 96);
static_assert(sizeof(StateNodeSetAnimFrame::Instance) == 104);
