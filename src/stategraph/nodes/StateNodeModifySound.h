#pragma once

#include <cstddef>

#include "entity/waveform/Waveform.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The node that changes a sound a StateNodePlaySound started
// (stategraph/StateNodeModifySound.o, 0x780B20-0x785086; class id
// "StateNodeModifySound"). It finds the sound by its instance name in the
// driver's local or global table, then sets or blends a parameter, mutes
// or unmutes it, fades its volume and pauses, continues or stops it. The
// vtable at 0x19449F0 has 50 slots. The object is 728 bytes.
class StateNodeModifySound : public StateNode {
public:
    // What "action" does to the sound. The labels are the allowed values';
    // the enumerator names are not in the reference map.
    enum Action : int {
        kActionNone = 0,
        kActionPause = 1,
        kActionContinue = 2,
        kActionStop = 3,
    };

    // The "parameter" struct: a parameter set at once or blended over a
    // duration along a curve. The map's StateNodeModifySound::ParamSettings
    // (304 bytes). Field names are not in the reference map; they follow
    // the properties.
    struct ParamSettings {
        ParamSettings();  // 0x782D00

        bool mEnable;              // "enable"
        Symbol mName;              // "name"
        DrivenProp<float> mValue;  // "value"
        // "persistent": sets the value again every frame.
        bool mPersistent;
        float mDuration;  // "duration"
        // "units": the TimeUnits of the duration; 4 takes the context
        // entity's (StateGraphInstance::ResolveTimeUnits).
        int mUnits;
        Waveform<float> mBlendCurve;  // "blend_curve"
    };

    // The "mute" struct. Name and field names not in the reference map.
    struct MuteSettings {
        MuteSettings() : mEnable(false), mSet(false) {}

        bool mEnable;  // "enable"
        bool mSet;     // "set": mute (true) or unmute.
    };

    // The "volume" struct: a fade to a level in dB. Name and field names
    // not in the reference map.
    struct VolumeSettings {
        VolumeSettings() : mEnable(false), mValue(), mSeconds(0.0F), mOption(0) {}

        bool mEnable;              // "enable"
        DrivenProp<float> mValue;  // "value", in dBFS.
        float mSeconds;            // "seconds"
        // "option": what the sound does when the fade ends
        // (AudioGenerator::PostFadeOption).
        int mOption;
    };

    // The node's run-time state: the sound's handle and the parameter
    // blend. The map's StateNodeModifySound::Instance. The vtable at
    // 0x1944B90 has 15 slots. The object is 56 bytes, from the small-block
    // pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x7835A0).
        Instance(const StateNodeModifySound& node, StateGraphInstance& graph);  // 0x782E10
        // Slots 0-1 at 0x783610 (StateNodeInstance's destructor) and
        // 0x783620.
        ~Instance() override {}

        // Slot 2: finds the sound and applies the parameter, the mute, the
        // volume and the action.
        void _Enter() override;  // 0x782E50
        // Slot 5: blends the parameter, or keeps setting a persistent one;
        // false once there is nothing left to do. The map's _Poll(float).
        bool _Poll() override;  // 0x783180

        // Field names are not in the reference map.
        const StateNodeModifySound* mModifySound;
        // The sound, from the driver's table.
        unsigned int mHandle;
        // Set while the parameter blends from mStartValue.
        bool mBlending;
        // Set when the parameter is set every frame.
        bool mPersistent;
        // The parameter's value when the node entered.
        float mStartValue;
        // The blend's time so far, in the parameter's units.
        float mElapsed;
    };

    StateNodeModifySound();  // 0x780B20
    // Copies the node for an imprint; the links start empty and the blend
    // curve draws a new evaluation state.
    StateNodeModifySound(const StateNodeModifySound& other);  // 0x783660
    // Slots 0-1: 0x782C30, 0x783320.
    ~StateNodeModifySound() override {}

    Symbol GetId() const override;          // slot 4: 0x7833F0
    Symbol GetClassName() const override;   // slot 5: 0x783400
    int CurrentRev() const override;        // slot 7: 0x783410
    bool IsA(Symbol type) const override;   // slot 8: 0x783430
    Component* AsComponent() override;      // slot 9: 0x783460
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x783470
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x783580
    ComMetaData& _GetMetaData() override;       // slot 23: 0x783590
    // Slot 42: the instance name.
    const char* GetEditorText() const override;  // 0x782CF0
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x7835A0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FB2B0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700610.
    static Component* _Create();
    // Revision 1 made the parameter and volume values driven.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x780BB0

    static Symbol sId;  // 0x1AB8CA8, "StateNodeModifySound"
    // Also "StateNodeModifySound". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB8CB0
    static PropRegistry sPropRegistry;  // 0x1AB8CC0
    static ComMetaData sMetaData;       // 0x1AB8D60

    // Field names are not in the reference map; they follow the
    // properties.
    // "instance_name": the name the sound was played under.
    Symbol mInstanceName;
    // "instance_name_is_global": looks in the global table.
    bool mInstanceNameIsGlobal;
    int mAction;  // "action" (Action)
    ParamSettings mParameter;  // "parameter"
    MuteSettings mMute;        // "mute"
    VolumeSettings mVolume;    // "volume"
};

static_assert(offsetof(StateNodeModifySound::ParamSettings, mName) == 8);
static_assert(offsetof(StateNodeModifySound::ParamSettings, mValue) == 16);
static_assert(offsetof(StateNodeModifySound::ParamSettings, mPersistent) == 248);
static_assert(offsetof(StateNodeModifySound::ParamSettings, mDuration) == 252);
static_assert(offsetof(StateNodeModifySound::ParamSettings, mUnits) == 256);
static_assert(offsetof(StateNodeModifySound::ParamSettings, mBlendCurve) == 264);
static_assert(sizeof(StateNodeModifySound::ParamSettings) == 304);
static_assert(sizeof(StateNodeModifySound::MuteSettings) == 2);
static_assert(offsetof(StateNodeModifySound::VolumeSettings, mValue) == 8);
static_assert(offsetof(StateNodeModifySound::VolumeSettings, mSeconds) == 240);
static_assert(offsetof(StateNodeModifySound::VolumeSettings, mOption) == 244);
static_assert(sizeof(StateNodeModifySound::VolumeSettings) == 248);
static_assert(offsetof(StateNodeModifySound, mInstanceName) == 152);
static_assert(offsetof(StateNodeModifySound, mInstanceNameIsGlobal) == 160);
static_assert(offsetof(StateNodeModifySound, mAction) == 164);
static_assert(offsetof(StateNodeModifySound, mParameter) == 168);
static_assert(offsetof(StateNodeModifySound, mMute) == 472);
static_assert(offsetof(StateNodeModifySound, mVolume) == 480);
static_assert(sizeof(StateNodeModifySound) == 728);
static_assert(offsetof(StateNodeModifySound::Instance, mModifySound) == 32);
static_assert(offsetof(StateNodeModifySound::Instance, mHandle) == 40);
static_assert(offsetof(StateNodeModifySound::Instance, mBlending) == 44);
static_assert(offsetof(StateNodeModifySound::Instance, mPersistent) == 45);
static_assert(offsetof(StateNodeModifySound::Instance, mStartValue) == 48);
static_assert(offsetof(StateNodeModifySound::Instance, mElapsed) == 52);
static_assert(sizeof(StateNodeModifySound::Instance) == 56);
