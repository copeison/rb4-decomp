#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/waveform/Waveform.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class StateGraphInstance;

// An FMOD event parameter a StateNodePlaySound sets when its sound starts:
// at once, or blended from the event's default over a duration along a
// curve (304 bytes). StateGraph::Init registers it as the property struct
// "DrivenAudioParamSettings" (RegisterPropStruct, 0x6FB100), the element
// type of "params". The constructor is inline; the binary emits it in
// stategraph/StateGraph.o at 0x700D60. The type is newer than the reference
// map, whose StateNodePlaySound has no parameters; the name is the
// registered type name. Field names follow the properties.
struct DrivenAudioParamSettings {
    DrivenAudioParamSettings()
        : mName(""),
          mValue(),
          mPersistent(false),
          mDuration(0.0F),
          mUnits(kTimeUnitsSeconds),
          mBlendCurve(),
          mUseDefinedParam(true) {}

    // "name", or "defined_name" when mUseDefinedParam picks it from the
    // event's parameters.
    Symbol mName;
    DrivenProp<float> mValue;  // "value"
    // "persistent": a driven value is followed every frame.
    bool mPersistent;
    float mDuration;  // "duration"
    // "units": the TimeUnits of the duration; 4 takes the context entity's.
    int mUnits;
    Waveform<float> mBlendCurve;  // "blend_curve"
    // "use_defined_param": the name is one of the event's parameters, and
    // the value is clamped to its range.
    bool mUseDefinedParam;
};

static_assert(offsetof(DrivenAudioParamSettings, mValue) == 8);
static_assert(offsetof(DrivenAudioParamSettings, mPersistent) == 240);
static_assert(offsetof(DrivenAudioParamSettings, mDuration) == 244);
static_assert(offsetof(DrivenAudioParamSettings, mUnits) == 248);
static_assert(offsetof(DrivenAudioParamSettings, mBlendCurve) == 256);
static_assert(offsetof(DrivenAudioParamSettings, mUseDefinedParam) == 296);
static_assert(sizeof(DrivenAudioParamSettings) == 304);
static_assert(alignof(DrivenAudioParamSettings) == 8);

// The node that plays a sound (stategraph/StateNodePlaySound.o,
// 0x785090-0x78CF86; class id "StateNodePlaySound"). It plays a sound event,
// or one from a preloaded bank, on an audio emitter of the entity or the
// default 2D emitter, as plain sound, master music or dialog, with initial
// parameters and volume. A named sound goes into the driver's local or
// global table for the modify nodes; the sound may stop when the node
// exits. The vtable at 0x1945118 has 50 slots. The object is 728 bytes.
class StateNodePlaySound : public StateNode {
public:
    // The "volume" struct: the sound's initial level in dB, which a driven
    // persistent value keeps following. Name and field names not in the
    // reference map.
    struct VolumeSettings {
        VolumeSettings() : mEnable(false), mValue(), mPersistent(false), mSeconds(0.0F), mOption(0) {}

        bool mEnable;              // "enable"
        DrivenProp<float> mValue;  // "value", in dBFS.
        bool mPersistent;          // "persistent"
        float mSeconds;            // "seconds"
        // "option" (AudioGenerator::PostFadeOption).
        int mOption;
    };

    // The state of one of the node's parameters in a running sound. Name and
    // field names not in the reference map.
    struct ParamState {
        // The current value is left unset.
        ParamState()
            : mValid(false), mBlending(false), mDone(false), mStartValue(0.0F), mTarget(0.0F), mElapsed(0.0F) {}

        // The sound has the parameter.
        bool mValid;
        bool mBlending;
        // The blend reached its target.
        bool mDone;
        // The value the blend started from, the driven value it goes to and
        // the value it set last.
        float mStartValue;
        float mTarget;
        float mCurrent;
        // The blend's time so far, in the parameter's units.
        float mElapsed;
    };

    // The node's run-time state: the sound's handle and the parameter
    // blends. The map's StateNodePlaySound::Instance. The vtable at
    // 0x19452B8 has 15 slots. The object is 88 bytes, from the small-block
    // pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x789E30).
        Instance(const StateNodePlaySound& node, StateGraphInstance& graph);  // 0x7887E0
        // Slots 0-1: 0x788880, 0x788960. Stops the sound and forgets its
        // name when the node stops on exit.
        ~Instance() override;

        // Slot 2: plays the sound.
        void _Enter() override;  // 0x788990
        // Slot 5: blends the parameters and follows a persistent driven
        // volume; false once the sound ended. The map's _Poll(float).
        bool _Poll() override;  // 0x7896A0

        // Field names are not in the reference map.
        const StateNodePlaySound* mPlaySound;
        unsigned int mHandle;
        // One for each of the node's "params".
        eastl::vector<ParamState> mParamStates;
        // The volume last set, in dB.
        float mVolumeDb;
    };

    StateNodePlaySound();  // 0x785090
    // Copies the node for an imprint; the links start empty. Inlined into
    // _Imprint (0x789BF0).
    StateNodePlaySound(const StateNodePlaySound& other)
        : StateNode(other),
          mUsePreloadedBank(other.mUsePreloadedBank),
          mPreloadedSound(other.mPreloadedSound),
          mSound(other.mSound),
          mAudioEmitter(other.mAudioEmitter),
          mInstanceName(other.mInstanceName),
          mInstanceNameIsGlobal(other.mInstanceNameIsGlobal),
          mIsMasterMusic(other.mIsMasterMusic),
          mIsDialog(other.mIsDialog),
          mIsInterruptible(other.mIsInterruptible),
          mStopOnExit(other.mStopOnExit),
          mParams(other.mParams),
          mStartMuted(other.mStartMuted),
          mVolume(other.mVolume) {}
    // Slots 0-1: 0x7886A0, 0x789A70.
    ~StateNodePlaySound() override {}

    Symbol GetId() const override;          // slot 4: 0x789B70
    Symbol GetClassName() const override;   // slot 5: 0x789B80
    int CurrentRev() const override;        // slot 7: 0x789B90
    bool IsA(Symbol type) const override;   // slot 8: 0x789BB0
    Component* AsComponent() override;      // slot 9: 0x789BE0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x789BF0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x789E10
    ComMetaData& _GetMetaData() override;       // slot 23: 0x789E20
    // Slot 42: the sound event's name.
    const char* GetEditorText() const override;  // 0x788790
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x789E30

    // The sound event as the editor names it: the preloaded sound, the
    // driving property's description, or the event. Inlined into
    // GetEditorText and the tag functions. Name not in the reference map.
    const char* GetSoundEventName() const {
        if (mUsePreloadedBank) {
            return mPreloadedSound.Str();
        }
        if (mSound.mDriven) {
            return mSound.mProp.ValueToString(_GetContextEntityPrototype());
        }
        return mSound.mValue.Str();
    }

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FB760.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700660.
    static Component* _Create();
    // Revision 1 made the sound driven.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x785190

    static Symbol sId;  // 0x1AB8F10, "StateNodePlaySound"
    // Also "StateNodePlaySound". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB8F18
    static PropRegistry sPropRegistry;  // 0x1AB8F20
    static ComMetaData sMetaData;       // 0x1AB8FC0

    // Field names are not in the reference map; they follow the
    // properties.
    bool mUsePreloadedBank;    // "use_preloaded_bank"
    Symbol mPreloadedSound;    // "preloaded_sound"
    DrivenProp<Symbol> mSound;  // "sound"
    // "audio_emitter": the object of the AudioEmitterCom to play on.
    GameObjectId mAudioEmitter;
    Symbol mInstanceName;        // "instance_name"
    bool mInstanceNameIsGlobal;  // "instance_name_is_global"
    bool mIsMasterMusic;         // "is_master_music"
    bool mIsDialog;              // "is_dialog"
    bool mIsInterruptible;       // "is_interruptible"
    bool mStopOnExit;            // "stop_on_exit"
    PropArray<DrivenAudioParamSettings> mParams;  // "params"
    bool mStartMuted;        // "start_muted"
    VolumeSettings mVolume;  // "volume"
};

static_assert(offsetof(StateNodePlaySound::VolumeSettings, mValue) == 8);
static_assert(offsetof(StateNodePlaySound::VolumeSettings, mPersistent) == 240);
static_assert(offsetof(StateNodePlaySound::VolumeSettings, mSeconds) == 244);
static_assert(offsetof(StateNodePlaySound::VolumeSettings, mOption) == 248);
static_assert(sizeof(StateNodePlaySound::VolumeSettings) == 256);
static_assert(offsetof(StateNodePlaySound::ParamState, mStartValue) == 4);
static_assert(offsetof(StateNodePlaySound::ParamState, mCurrent) == 12);
static_assert(offsetof(StateNodePlaySound::ParamState, mElapsed) == 16);
static_assert(sizeof(StateNodePlaySound::ParamState) == 20);
static_assert(offsetof(StateNodePlaySound, mUsePreloadedBank) == 152);
static_assert(offsetof(StateNodePlaySound, mPreloadedSound) == 160);
static_assert(offsetof(StateNodePlaySound, mSound) == 168);
static_assert(offsetof(StateNodePlaySound, mAudioEmitter) == 400);
static_assert(offsetof(StateNodePlaySound, mInstanceName) == 408);
static_assert(offsetof(StateNodePlaySound, mInstanceNameIsGlobal) == 416);
static_assert(offsetof(StateNodePlaySound, mIsInterruptible) == 419);
static_assert(offsetof(StateNodePlaySound, mStopOnExit) == 420);
static_assert(offsetof(StateNodePlaySound, mParams) == 424);
static_assert(offsetof(StateNodePlaySound, mStartMuted) == 464);
static_assert(offsetof(StateNodePlaySound, mVolume) == 472);
static_assert(sizeof(StateNodePlaySound) == 728);
static_assert(offsetof(StateNodePlaySound::Instance, mPlaySound) == 32);
static_assert(offsetof(StateNodePlaySound::Instance, mHandle) == 40);
static_assert(offsetof(StateNodePlaySound::Instance, mParamStates) == 48);
static_assert(offsetof(StateNodePlaySound::Instance, mVolumeDb) == 80);
static_assert(sizeof(StateNodePlaySound::Instance) == 88);
