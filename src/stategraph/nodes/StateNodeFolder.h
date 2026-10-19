#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// A node that holds other nodes, an internal subgraph, and runs from its
// "start" node (stategraph/StateNodeFolder.o, after the map's build,
// 0x719A40-0x71AC87; class id "StateNodeFolder"). A new folder creates its
// two plain nodes, "Entry Point" and "Parent Folder", and destroys them
// with itself; the nodes inside it name it as their folder. The graph
// runtime enters the start node in its place, so the folder has no
// instance. The vtable at 0x193BD48 has 50 slots. The object is 176 bytes.
class StateNodeFolder : public StateNode {
public:
    // The nodes of mStart and mExit, resolved by _Fixup and not copied into
    // imprints; StateNode's RuntimeData keeps the base node's links. Name
    // and field names not in the reference map.
    struct FolderRuntimeData {
        FolderRuntimeData();  // 0x719A90

        StateNode* mStartNode;
        StateNode* mExitNode;
    };

    StateNodeFolder();  // 0x719A40
    // Copies the folder for an imprint; the links start empty. Inlined into
    // _Imprint (0x71A850).
    StateNodeFolder(const StateNodeFolder& other)
        : StateNode(other), mStart(other.mStart), mExit(other.mExit), mFolderRuntime() {}
    // Slots 0-1: 0x719E60, 0x71A730.
    ~StateNodeFolder() override {}

    Symbol GetId() const override;          // slot 4: 0x71A7D0
    Symbol GetClassName() const override;   // slot 5: 0x71A7E0
    int CurrentRev() const override;        // slot 7: 0x71A7F0
    bool IsA(Symbol type) const override;   // slot 8: 0x71A810
    Component* AsComponent() override;      // slot 9: 0x71A840
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x71A850
    // Slot 19: creates the "Entry Point" and "Parent Folder" nodes inside
    // the folder, which the editor may not move to another layer or edit.
    void _PostCreate() override;  // 0x719EF0
    // Slot 20: destroys the two nodes with the object or the component.
    void _PreDestroy(DestroyType type) override;  // 0x71A170
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x71AA00
    ComMetaData& _GetMetaData() override;       // slot 23: 0x71AA10
    // Slot 41: resolves the start and exit nodes.
    void _Fixup() override;  // 0x71A120
    // Slot 43: the nodes in the folder with their transitions and
    // conditions, their own child nodes, and the comment blocks in it.
    void GetChildNodes(eastl::vector<GameObjectId>& children) const override;  // 0x71A1B0
    // Slot 45: none; the graph enters the start node instead.
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x71A720
    bool CanBlend() const override;      // slot 46: 0x71AA20, true
    bool _HasActivity() const override;  // slot 49: 0x71AA30, false

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F96C0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FFD80.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x719AA0

    static Symbol sId;  // 0x1AB3BA0, "StateNodeFolder"
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3BA8
    static PropRegistry sPropRegistry;  // 0x1AB3BB0
    static ComMetaData sMetaData;       // 0x1AB3C50

    // Field names are not in the reference map; they follow the
    // properties.
    // "start": the node the folder starts at, its "Entry Point".
    GameObjectId mStart;
    // "exit": the node that leaves the folder, its "Parent Folder".
    GameObjectId mExit;
    FolderRuntimeData mFolderRuntime;
};

static_assert(sizeof(StateNodeFolder::FolderRuntimeData) == 16);
static_assert(offsetof(StateNodeFolder, mStart) == 152);
static_assert(offsetof(StateNodeFolder, mExit) == 156);
static_assert(offsetof(StateNodeFolder, mFolderRuntime) == 160);
static_assert(sizeof(StateNodeFolder) == 176);
