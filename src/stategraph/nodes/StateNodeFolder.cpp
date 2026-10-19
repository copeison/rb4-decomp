// stategraph/StateNodeFolder.o (0x719A40 to 0x71AC87), newer than the
// reference map. The class's registration and factory are emitted in
// stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeFolder.h"

#include <new>

#include "entity/core/EditorCom.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "render/editor/RndCommentBlock.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/transitions/StateTransition.h"

namespace {

// The id of no object, at 0x1AB3B98, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Creates one of the folder's two nodes: a plain StateNode on a new object
// with the name, which the editor may neither edit nor move to another
// layer. The binary revokes the capabilities without checking that the
// object has an EditorCom. Inlined twice into _PostCreate. Name not in the
// reference map.
StateNode* CreateFolderNode(Entity* entity, const char* name, GameObject*& object) {
    object = entity->CreateObject(0, 0);
    object->SetName(Symbol(name));
    StateNode* const node = static_cast<StateNode*>(object->CreateComponent(StateNode::sClassName, false));
    EditorCom* const editor = object->GetCom<EditorCom>();
    editor->RevokeEditorCapability(object, EditorCom::kCanChangeProperties);
    editor->RevokeEditorCapability(object, EditorCom::kCanChangeLayer);
    return node;
}

// "start" and "exit" set the node and resolve it. Names not in the
// reference map.
// Reconstructed from eboot.elf at 0x71AA40.
void SetStartAccessor(const PropAccessorArgs& args) {
    StateNodeFolder* const folder = static_cast<StateNodeFolder*>(args.mComponent);
    const GameObjectId start = *static_cast<const GameObjectId*>(args.mValue);
    folder->mStart = start;
    folder->mFolderRuntime.mStartNode = folder->_GetObjectBaseCom<StateNode>(start, true);
}

// Reconstructed from eboot.elf at 0x71AA70.
void SetExitAccessor(const PropAccessorArgs& args) {
    StateNodeFolder* const folder = static_cast<StateNodeFolder*>(args.mComponent);
    const GameObjectId exit = *static_cast<const GameObjectId*>(args.mValue);
    folder->mExit = exit;
    folder->mFolderRuntime.mExitNode = folder->_GetObjectBaseCom<StateNode>(exit, true);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x71ABD0).
Symbol StateNodeFolder::sId("StateNodeFolder");
Symbol StateNodeFolder::sClassName("StateNodeFolder");
PropRegistry StateNodeFolder::sPropRegistry;
ComMetaData StateNodeFolder::sMetaData;

// Reconstructed from eboot.elf at 0x719A40.
StateNodeFolder::StateNodeFolder() : mStart(gNullObjectId), mExit(gNullObjectId), mFolderRuntime() {}

// Reconstructed from eboot.elf at 0x719A90. No caller in this build; the
// constructors inline it.
StateNodeFolder::FolderRuntimeData::FolderRuntimeData() : mStartNode(nullptr), mExitNode(nullptr) {}

// Reconstructed from eboot.elf at 0x719AA0. Both nodes are read-only debug
// properties.
void StateNodeFolder::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeFolder prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    GameObjectIdMetadata& start = TypeSpecificMetadata(
        registry.RegisterProp("start", offsetof(StateNodeFolder, mStart), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mStart);
    start.mDefault = prototype.mStart;
    static_cast<void>(Symbol("prop"));
    start.mDebug = true;
    start.mReadOnly = true;
    start.mSetter = SetStartAccessor;

    GameObjectIdMetadata& exit = TypeSpecificMetadata(
        registry.RegisterProp("exit", offsetof(StateNodeFolder, mExit), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mExit);
    exit.mDefault = prototype.mExit;
    static_cast<void>(Symbol("prop"));
    exit.mDebug = true;
    exit.mReadOnly = true;
    exit.mSetter = SetExitAccessor;

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "A state that represents a folder (i.e. an internal subgraph) within a larger state graph. The folder will "
        "operate as any normal state graph would, but can additionally exit to specific states elsewhere in the "
        "hierarchy. Likewise, states outside of this folder can directly transition to specific states within it, "
        "not just to the folder as a whole.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("f");
}

// Reconstructed from eboot.elf at 0x719EF0. The exit node is placed to the
// right of and above the start node.
void StateNodeFolder::_PostCreate() {
    Entity* const entity = mObject->mEntity;
    GameObject* startObject;
    mFolderRuntime.mStartNode = CreateFolderNode(entity, "Entry Point", startObject);
    mStart = startObject->mId;
    GameObject* exitObject;
    mFolderRuntime.mExitNode = CreateFolderNode(entity, "Parent Folder", exitObject);
    const Hmx::Rect& startRect = mFolderRuntime.mStartNode->mRect;
    mFolderRuntime.mExitNode->mRect =
        Hmx::Rect(startRect.x + 200.0F, startRect.y + -100.0F, startRect.w, startRect.h);
    mExit = exitObject->mId;
    startObject->_ReenterComponents();
    exitObject->_ReenterComponents();
    mFolderRuntime.mStartNode->SetFolder(mObject->mId);
    mFolderRuntime.mExitNode->SetFolder(mObject->mId);
}

// Reconstructed from eboot.elf at 0x71A120.
void StateNodeFolder::_Fixup() {
    StateNode::_Fixup();
    mFolderRuntime.mStartNode = _GetObjectBaseCom<StateNode>(mStart, true);
    mFolderRuntime.mExitNode = _GetObjectBaseCom<StateNode>(mExit, true);
}

// Reconstructed from eboot.elf at 0x71A170.
void StateNodeFolder::_PreDestroy(DestroyType type) {
    if (type == kDestroyObject || type == kDestroyComponent) {
        Entity* const entity = mObject->mEntity;
        entity->DestroyObject(mStart);
        entity->DestroyObject(mExit);
    }
}

// Reconstructed from eboot.elf at 0x71A1B0. A node's transitions follow
// it, each with its condition and the condition's descendants; the
// binary looks the transitions up without the null checks of
// GetBaseCom.
void StateNodeFolder::GetChildNodes(eastl::vector<GameObjectId>& children) const {
    Entity* const entity = mObject->mEntity;
    const GameObjectId folder = mObject->mId;
    for (const GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const StateNode* const node = object->GetBaseCom<StateNode>();
        if (node != nullptr && node->mFolder.mId == folder.mId) {
            children.push_back(object->mId);
            for (const GameObjectId& transitionId : node->mTransitions) {
                children.push_back(transitionId);
                const StateTransition* const transition =
                    entity->GetObject(transitionId)->GetBaseCom<StateTransition>();
                StateCondition* const condition = transition->mRuntime.mCondition;
                if (condition != nullptr) {
                    children.push_back(condition->mObject->mId);
                    condition->GetDescendants(children);
                }
            }
            node->GetChildNodes(children);
        }
        const RndCommentBlock* const comment = object->GetCom<RndCommentBlock>();
        if (comment != nullptr && comment->mFolder.mId == folder.mId) {
            children.push_back(object->mId);
        }
    }
}

// Reconstructed from eboot.elf at 0x71A720.
StateNodeInstance* StateNodeFolder::_CreateInstance(StateGraphInstance& graph) const {
    static_cast<void>(graph);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x71A7D0.
Symbol StateNodeFolder::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x71A7E0.
Symbol StateNodeFolder::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x71A7F0.
int StateNodeFolder::CurrentRev() const {
    return const_cast<StateNodeFolder*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x71A810.
bool StateNodeFolder::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x71A840.
Component* StateNodeFolder::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x71A850. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeFolder::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeFolder*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeFolder(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x71AA00.
PropRegistry& StateNodeFolder::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x71AA10.
ComMetaData& StateNodeFolder::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x71AA20.
bool StateNodeFolder::CanBlend() const {
    return true;
}

// Reconstructed from eboot.elf at 0x71AA30.
bool StateNodeFolder::_HasActivity() const {
    return false;
}

// Reconstructed from eboot.elf at 0x6FFD80. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeFolder::_Create() {
    return new StateNodeFolder();
}

// Reconstructed from eboot.elf at 0x6F96C0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeFolder::Init() {
    InitComponentClass<StateNodeFolder>();
}
