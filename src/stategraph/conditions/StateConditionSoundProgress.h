#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition that compares the progress of a sound the graph plays
// (stategraph/StateConditionSoundProgress.o, 0x88F1B0-0x8903E6, after the
// map's build; class id "StateConditionSoundProgress"). It finds the sound
// by its instance name in the driver's local or global table and compares
// the seconds since its start, the seconds until its end or the percent
// played with its comparison values; without a playing sound it is met
// when "proceed_if_not_playing" says so. The vtable at 0x195BD30 has 59
// slots. The object is 536 bytes.
class StateConditionSoundProgress : public StateConditionComparison<float> {
public:
    // What the condition compares ("progress_mode"). The labels are the
    // allowed values'; the enumerator names are not in the reference map.
    enum ProgressMode : int {
        kProgressFromStart = 0,
        kProgressFromEnd = 1,
        kProgressPercent = 2,
    };

    // The condition's run-time state. The vtable at 0x195BF18 has nine
    // slots. The object is 56 bytes, from the small-block pool. Name not in
    // the reference map, as for the other conditions' instances.
    class Instance : public StateConditionComparison<float>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x8900D0).
        Instance(const StateConditionSoundProgress& condition, StateGraphInstance& graph);  // 0x88FAD0
        // Slots 0-1 at 0x8901C0 (StateConditionInstance's destructor) and
        // 0x8901D0.
        ~Instance() override {}

        // Slot 2: the comparison while the sound plays; otherwise
        // "proceed_if_not_playing".
        bool _IsMet(bool useCached) override;  // 0x88FB80
        // Slot 8: the sound's progress by the mode; zero without a sound.
        // Always true.
        bool _GetValueToCompare(float& value) const override;  // 0x88FD30

        // The sound under the instance name, or zero. Inlined into _IsMet
        // and _GetValueToCompare. Name not in the reference map.
        unsigned int _GetSoundHandle() const;  // 0x88FC70

        // Field name not in the reference map.
        const StateConditionSoundProgress* mSoundProgress;
    };

    // Compares with <= by default.
    StateConditionSoundProgress();  // 0x88F1B0
    // Slots 0-1: 0x88F9C0 (Component's destructor) and 0x88FEA0.
    ~StateConditionSoundProgress() override {}

    Symbol GetId() const override;          // slot 4: 0x88FEC0
    Symbol GetClassName() const override;   // slot 5: 0x88FED0
    int CurrentRev() const override;        // slot 7: 0x88FEE0
    bool IsA(Symbol type) const override;   // slot 8: 0x88FF00
    Component* AsComponent() override;      // slot 9: 0x88FF30
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x88FF40
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8900B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8900C0
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x8900D0
    // Slot 52: kSoundProgress.
    int _GetType() const override;  // 0x8901B0
    // Slot 54: the progress mode, in the operators' help.
    const char* _GetValueToCompareGeneralDescription() const override;  // 0x88F9D0
    // Slot 55: the progress mode with the instance name.
    const char* _GetValueToCompareDescription() const override;  // 0x88FA00
    // Slot 56: an instance name is set.
    bool _HasValueToCompare() const override;  // 0x88FAA0
    // Slot 57: the ordering operators and "Between".
    bool _AllowOperator(ComparisonOperator op) const override;  // 0x88FAC0

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FDC70.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x7012F0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x88F250

    static Symbol sId;  // 0x1AC7800, "StateConditionSoundProgress"
    // Also "StateConditionSoundProgress". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC7808
    static PropRegistry sPropRegistry;  // 0x1AC7810
    static ComMetaData sMetaData;       // 0x1AC78B0

    // Field names are not in the reference map; they follow the
    // properties.
    Symbol mInstanceName;        // "instance_name"
    bool mInstanceNameIsGlobal;  // "instance_name_is_global"
    int mProgressMode;           // "progress_mode" (ProgressMode)
    bool mProceedIfNotPlaying;   // "proceed_if_not_playing"
};

static_assert(offsetof(StateConditionSoundProgress, mInstanceName) == 512);
static_assert(offsetof(StateConditionSoundProgress, mInstanceNameIsGlobal) == 520);
static_assert(offsetof(StateConditionSoundProgress, mProgressMode) == 524);
static_assert(offsetof(StateConditionSoundProgress, mProceedIfNotPlaying) == 528);
static_assert(sizeof(StateConditionSoundProgress) == 536);
static_assert(offsetof(StateConditionSoundProgress::Instance, mSoundProgress) == 48);
static_assert(sizeof(StateConditionSoundProgress::Instance) == 56);
