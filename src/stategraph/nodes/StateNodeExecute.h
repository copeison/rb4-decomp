#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The node that runs a script command (stategraph/StateNodeExecute.o,
// 0x718F00-0x719A37; class id "StateNodeExecute"). The command ("cmd") is
// parsed when the node is entered and run once on entry, or on every poll
// while the node is active when it is "persistent". The vtable at 0x193BB20
// has 50 slots. The object is 168 bytes.
class StateNodeExecute : public StateNode {
public:
    // The node's run-time state: the parsed command. The map's
    // StateNodeExecute::Instance. The vtable at 0x193BCC0 has 15 slots. The
    // object is 48 bytes, from the small-block pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Also inlined into _CreateInstance (0x719840).
        Instance(const StateNodeExecute& node, StateGraphInstance& graph);  // 0x7193B0
        ~Instance() override;  // slots 0-1: 0x7198B0, 0x719910

        // Slot 2: runs the command once unless it is persistent.
        void _Enter() override;  // 0x7193F0
        // Slot 5: runs the persistent command; the node works for as long as
        // it is persistent.
        bool _Poll() override;  // 0x719490

        // Field names are not in the reference map.
        // The command, parsed from the node's "cmd".
        DataArrayPtr mCommand;
        const StateNodeExecute* mExecuteNode;
    };

    StateNodeExecute();  // 0x718F00
    // Copies the node for an imprint. Inlined into _Imprint (0x719660).
    StateNodeExecute(const StateNodeExecute& other)
        : StateNode(other), mCmd(other.mCmd), mPersistent(other.mPersistent) {}
    // Slots 0-1: 0x719310, 0x719540.
    ~StateNodeExecute() override {}

    Symbol GetId() const override;          // slot 4: 0x7195E0
    Symbol GetClassName() const override;   // slot 5: 0x7195F0
    int CurrentRev() const override;        // slot 7: 0x719600
    bool IsA(Symbol type) const override;   // slot 8: 0x719620
    Component* AsComponent() override;      // slot 9: 0x719650
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x719660
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x719820
    ComMetaData& _GetMetaData() override;       // slot 23: 0x719830
    // Slot 42: the command.
    const char* GetEditorText() const override;  // 0x7193A0
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x719840

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F9250.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FFD30.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x718F40

    static Symbol sId;  // 0x1AB3940, "StateNodeExecute"
    // Also "StateNodeExecute". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB3948
    static PropRegistry sPropRegistry;  // 0x1AB3950
    static ComMetaData sMetaData;       // 0x1AB39F0

    // Field names are not in the reference map; they follow the
    // properties.
    // "cmd": the script command's text.
    Symbol mCmd;
    // "persistent": run the command on every poll instead of once.
    bool mPersistent;
};

static_assert(offsetof(StateNodeExecute, mCmd) == 152);
static_assert(offsetof(StateNodeExecute, mPersistent) == 160);
static_assert(sizeof(StateNodeExecute) == 168);
static_assert(offsetof(StateNodeExecute::Instance, mCommand) == 32);
static_assert(offsetof(StateNodeExecute::Instance, mExecuteNode) == 40);
static_assert(sizeof(StateNodeExecute::Instance) == 48);
