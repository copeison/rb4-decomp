#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/Symbol.h"

// The node that runs another state graph inside it
// (stategraph/StateNodeSubGraph.o, 0x7F2310-0x7F2E67; class id
// "StateNodeSubGraph"). "sub_graph" names a StateGraphResource inlined into
// the context entity's resource; the node's instance runs it as a
// StateGraphInstance of its own, from its root, for as long as that graph
// runs. A graph may not name itself. The vtable at 0x194E588 has 50 slots.
// The object is 160 bytes.
class StateNodeSubGraph : public StateNode {
public:
    // The node's run-time state: the running sub-graph. The map's
    // StateNodeSubGraph::Instance, whose _StartBlendingIn this build drops.
    // The vtable at 0x194E728 has 15 slots. The object is 216 bytes, from
    // the small-block pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // The sub-graph runs in the graph's entity on the graph's clock.
        // Inlined into _CreateInstance (0x7F2C80).
        Instance(const StateNodeSubGraph& node, StateGraphInstance& graph)
            : StateNodeInstance(node, graph), mSubGraph(graph.mEntity, graph.mClock, node.mSubGraph) {}
        ~Instance() override;  // slots 0-1: 0x7F2CF0, 0x7F2D20

        // Slot 2: starts the sub-graph at its root.
        void _Enter() override;  // 0x7F2950
        // Slot 5: polls the sub-graph; the node works while it runs.
        bool _Poll() override;  // 0x7F2960
        void _Exit() override;  // slot 6: 0x7F2970
        // Slot 11: the sub-graph's instance of the object's node.
        StateNodeInstance* _FindInstance(const GameObject* object) override;  // 0x7F2980

        // Name not in the reference map.
        StateGraphInstance mSubGraph;
    };

    StateNodeSubGraph();  // 0x7F2310
    // Copies the node for an imprint. Inlined into _Imprint (0x7F2AB0).
    StateNodeSubGraph(const StateNodeSubGraph& other) : StateNode(other), mSubGraph(other.mSubGraph) {}
    // Slots 0-1: 0x7F2750, 0x7F2990.
    ~StateNodeSubGraph() override {}

    Symbol GetId() const override;          // slot 4: 0x7F2A30
    Symbol GetClassName() const override;   // slot 5: 0x7F2A40
    int CurrentRev() const override;        // slot 7: 0x7F2A50
    bool IsA(Symbol type) const override;   // slot 8: 0x7F2A70
    Component* AsComponent() override;      // slot 9: 0x7F2AA0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7F2AB0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7F2C60
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7F2C70
    // Slot 42: the sub-graph's file name without its directory or
    // extension.
    const char* GetEditorText() const override;  // 0x7F27E0
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x7F2C80

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FA3B0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700520.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7F2350

    static Symbol sId;  // 0x1ABD998, "StateNodeSubGraph"
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1ABD9A0
    static PropRegistry sPropRegistry;  // 0x1ABD9B0
    static ComMetaData sMetaData;       // 0x1ABDA50

    // "sub_graph": the graph the node runs. Name not in the reference map.
    ResourcePath mSubGraph;
};

static_assert(offsetof(StateNodeSubGraph, mSubGraph) == 152);
static_assert(sizeof(StateNodeSubGraph) == 160);
static_assert(offsetof(StateNodeSubGraph::Instance, mSubGraph) == 32);
static_assert(sizeof(StateNodeSubGraph::Instance) == 216);
