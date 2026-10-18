#pragma once

#include <cstddef>

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The initial state of a state graph (stategraph/StateNodeRoot.o,
// 0x78CF90-0x78DA27; class id "StateNodeRoot"). StateGraphResource creates
// one on the root object of every graph; its properties decide whether the
// graph starts when its context entity enters and how often it polls. The
// vtable at 0x1945AC0 has 50 slots. The object is 160 bytes.
class StateNodeRoot : public StateNode {
public:
    // How often the graph polls ("poll_frequency"). The labels are the
    // allowed values'; the enumerator names are not in the reference map.
    enum PollFrequency : int {
        kPollAlways = 0,
        kPollEveryOtherFrame = 1,
    };

    StateNodeRoot();  // 0x78CF90
    // Copies the root for an imprint. Inlined into _Imprint (0x78D770).
    StateNodeRoot(const StateNodeRoot& other)
        : StateNode(other), mStartOnEnter(other.mStartOnEnter), mPollFrequency(other.mPollFrequency) {}
    // Slots 0-1: 0x78D580, 0x78D650.
    ~StateNodeRoot() override {}

    Symbol GetId() const override;          // slot 4: 0x78D6F0
    Symbol GetClassName() const override;   // slot 5: 0x78D700
    int CurrentRev() const override;        // slot 7: 0x78D710
    bool IsA(Symbol type) const override;   // slot 8: 0x78D730
    Component* AsComponent() override;      // slot 9: 0x78D760
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x78D770
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x78D920
    ComMetaData& _GetMetaData() override;       // slot 23: 0x78D930
    // Slot 42: "Start on Enter" for a graph that starts on enter.
    const char* GetEditorText() const override;  // 0x78D630
    // Slot 49: the root does no work of its own.
    bool _HasActivity() const override;  // 0x78D940

    // Sets "start_on_enter" and redraws the node when it changed. Name not
    // in the reference map.
    void SetStartOnEnter(bool startOnEnter);  // 0x78D610

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F6F50.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEEE0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x78CFD0

    static Symbol sId;  // 0x1AB91B0, "StateNodeRoot"
    // Also "StateNodeRoot". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB91B8
    static PropRegistry sPropRegistry;  // 0x1AB91C0
    static ComMetaData sMetaData;       // 0x1AB9260

    // Field names are not in the reference map; they follow the
    // properties.
    bool mStartOnEnter;  // "start_on_enter"
    int mPollFrequency;  // "poll_frequency" (PollFrequency)
};

static_assert(offsetof(StateNodeRoot, mStartOnEnter) == 152);
static_assert(offsetof(StateNodeRoot, mPollFrequency) == 156);
static_assert(sizeof(StateNodeRoot) == 160);
