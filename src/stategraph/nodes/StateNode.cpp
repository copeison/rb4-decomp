// stategraph/StateNode.o (0x708410 to 0x70AD77). The class's registration
// and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNode.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "os/memory/MemMgr.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "stategraph/transitions/StateTransition.h"

namespace {

// The id of no object, at 0x1AB2CB8, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The array index the last node of the accessor's path names. Inlined into
// the array hooks. Name not in the reference map.
unsigned long LastIndex(const PropAccessorArgs& args) {
    const PropPath& path = *static_cast<const PropPath*>(args.mPath);
    return path[-1].mValue;
}

// The "transitions" hooks keep the resolved transitions in step with the
// array: an insert adds the transition the inserted id names (the value
// arrives through mOldValue), a removal drops it, and a change resolves it
// again. Names not in the reference map.
// Reconstructed from eboot.elf at 0x70A050.
void InsertTransitionHook(const PropAccessorArgs& args) {
    StateNode* const node = static_cast<StateNode*>(args.mComponent);
    const unsigned long index = LastIndex(args);
    const GameObjectId* const inserted = static_cast<const GameObjectId*>(args.mOldValue);
    GameObjectId id = inserted != nullptr ? *inserted : gNullObjectId;
    node->mTransitions._Insert(index, &id);
    StateTransition* const transition = node->_GetObjectBaseCom<StateTransition>(id, true);
    node->mRuntime.mTransitions.insert(node->mRuntime.mTransitions.begin() + index, transition);
}

// Reconstructed from eboot.elf at 0x70A380. The element is destroyed and
// the rest moved down through the array's vtable.
void RemoveTransitionHook(const PropAccessorArgs& args) {
    StateNode* const node = static_cast<StateNode*>(args.mComponent);
    PropArray<GameObjectId>& transitions = node->mTransitions;
    const unsigned long index = LastIndex(args);
    transitions.Destruct(1, transitions.ElementAt(index));
    if (static_cast<unsigned int>(index) < transitions.mSize - 1) {
        transitions.Move(transitions.mSize - 1 - index, transitions.ElementAt(index), transitions.ElementAt(index + 1));
    }
    --transitions.mSize;
    node->mRuntime.mTransitions.erase(node->mRuntime.mTransitions.begin() + index);
}

// Reconstructed from eboot.elf at 0x70A450.
void OnTransitionChanged(const PropAccessorArgs& args) {
    StateNode* const node = static_cast<StateNode*>(args.mComponent);
    node->mRuntime.mTransitions.resize(node->mTransitions.size());
    const unsigned long index = LastIndex(args);
    node->mRuntime.mTransitions[index] =
        node->_GetObjectBaseCom<StateTransition>(node->mTransitions[index], true);
}

// "folder" sets the folder through the node. Name not in the reference map.
// Reconstructed from eboot.elf at 0x70A5F0.
void SetFolderAccessor(const PropAccessorArgs& args) {
    StateNode* const node = static_cast<StateNode*>(args.mComponent);
    const GameObjectId folder = *static_cast<const GameObjectId*>(args.mValue);
    node->mFolder = folder;
    node->mRuntime.mFolderNode = node->_GetObjectCom<StateNodeFolder>(folder, true);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x70ACC0).
Symbol StateNode::sId("StateNode");
Symbol StateNode::sClassName("StateNode");
PropRegistry StateNode::sPropRegistry;
ComMetaData StateNode::sMetaData;

// Reconstructed from eboot.elf at 0x708410.
StateNode::StateNode()
    : mActiveAt(kActivityUnset),
      mActiveUntil(kActivityUnset),
      mRect(0.0F, 0.0F, 10.0F, 10.0F),
      mTransitions(),
      mFolder(gNullObjectId),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x7084C0.
StateNode::RuntimeData::RuntimeData() : mFolderNode(nullptr), mGroup(nullptr), mTransitions() {}

// Reconstructed from eboot.elf at 0x7084E0. "active_at" and
// "active_until" are hidden for nodes without activity of their own; the
// rectangle, the transitions and the folder are read-only debug
// properties. Revision 1 picks the activity defaults of older nodes.
void StateNode::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNode prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    IntMetadata<int>& activeAt = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "active_at", offsetof(StateNode, mActiveAt), kPropertyInt, 4).mMetadata.Get());
    activeAt.mDefault = prototype.mActiveAt;
    static_cast<void>(Symbol("prop"));
    activeAt.mHelp = String(
        "Controls when this node becomes active, i.e. begins doing its work. Allows the user to delay "
        "activation of nodes that would otherwise blend in, or to immediately activate nodes that would "
        "otherwise not become active until the incoming transition ended. For instantaneous transitions, "
        "this property has no effect.");
    activeAt.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kActiveAtStartOfIncoming,
            String("Start of Incoming Transition"),
            String(
                "This node will activate as soon as the transition into it begins, whether it is capable of "
                "blending or not.")},
        AllowedValue<int>{
            kActiveAtEndOfIncoming,
            String("End of Incoming Transition"),
            String(
                "This node will activate only when the transition into it ends, and will perform no blending "
                "regardless of blending capability. If the transition is interrupted prior to completion, "
                "this node will not activate at all.")}};
    // At 0x70A6A0.
    activeAt.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNode&>(args.mComponent).HasActivity();
    });

    IntMetadata<int>& activeUntil = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "active_until", offsetof(StateNode, mActiveUntil), kPropertyInt, 4).mMetadata.Get());
    activeUntil.mDefault = prototype.mActiveUntil;
    static_cast<void>(Symbol("prop"));
    activeUntil.mHelp = String(
        "Controls when this node becomes inactive, i.e. stops doing its work.. Allows the user to immediately "
        "deactivate nodes that would otherwise blend out, or to delay deactivation of nodes that would "
        "otherwise deactivate immediately at the beginning of the outgoing transition. For instantaneous "
        "transitions, this property has no effect.");
    activeUntil.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kActiveUntilStartOfOutgoing,
            String("Start of Outgoing Transition"),
            String(
                "This node will deactivate as soon as the transition out of it begins, and will perform no "
                "blending regardless of blending capability.")},
        AllowedValue<int>{
            kActiveUntilEndOfOutgoing,
            String("End of Outgoing Transition"),
            String(
                "This node will deactivate only when the transition out of it ends, whether it is capable of "
                "blending or not.")}};
    // At 0x70A7D0.
    activeUntil.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNode&>(args.mComponent).HasActivity();
    });

    PropInfo& rectInfo = registry.RegisterProp("rect", offsetof(StateNode, mRect), kPropertyStruct, sizeof(Hmx::Rect));
    GroupMetadata& rect = static_cast<GroupMetadata&>(*rectInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Hmx::Rect zero(0.0F, 0.0F, 0.0F, 0.0F);
    FloatMetadata& x = TypeSpecificMetadata(rect.mRegistry.RegisterProp("x", offsetof(Hmx::Rect, x), kPropertyFloat, 4), zero.x);
    x.mDefault = zero.x;
    FloatMetadata& y = TypeSpecificMetadata(rect.mRegistry.RegisterProp("y", offsetof(Hmx::Rect, y), kPropertyFloat, 4), zero.y);
    y.mDefault = zero.y;
    FloatMetadata& w = TypeSpecificMetadata(rect.mRegistry.RegisterProp("w", offsetof(Hmx::Rect, w), kPropertyFloat, 4), zero.w);
    w.mDefault = zero.w;
    FloatMetadata& h = TypeSpecificMetadata(rect.mRegistry.RegisterProp("h", offsetof(Hmx::Rect, h), kPropertyFloat, 4), zero.h);
    h.mDefault = zero.h;
    rect.mHelp = String("Rectangle of the node.");
    rect.mDebug = true;
    rect.mReadOnly = true;

    PropInfo& transitionsInfo =
        registry.RegisterPropArray("transitions", offsetof(StateNode, mTransitions), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& transitions = static_cast<ArrayMetadata&>(*transitionsInfo.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    transitions.mDebug = true;
    transitions.mReadOnly = true;
    transitions.mInsert = InsertTransitionHook;
    transitions.mHook2 = reinterpret_cast<void*>(&RemoveTransitionHook);
    PropRegistry::RegisterPropArrayItem(transitionsInfo, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId itemType = {0xFFFFFFFFU};
    GameObjectIdMetadata& item = TypeSpecificMetadata(transitions.mItemInfo, itemType);
    static_cast<void>(Symbol("item"));
    item.mOnChanged = OnTransitionChanged;

    GameObjectIdMetadata& folder = TypeSpecificMetadata(
        registry.RegisterProp("folder", offsetof(StateNode, mFolder), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mFolder);
    folder.mDefault = prototype.mFolder;
    static_cast<void>(Symbol("prop"));
    folder.mDebug = true;
    folder.mReadOnly = true;
    folder.mSetter = SetFolderAccessor;

    // At 0x70A900.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev <= 0) {
                StateNode* const node = static_cast<StateNode*>(args.mComponent);
                const bool canBlend = node->CanBlend();
                node->mActiveAt = !canBlend;
                node->mActiveUntil = canBlend;
            }
        };
    if (&own == &registry) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    }

    StateBase::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Base component for all StateGraph nodes; doesn't do anything directly (i.e. a No-Op).";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mInterface = sClassName;
    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol("true");
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("n");
}

// Reconstructed from eboot.elf at 0x709350. The deleting destructor is at
// 0x709CB0.
StateNode::~StateNode() {}

// Reconstructed from eboot.elf at 0x7093E0. A node that blends activates
// at the start of the transition into it and deactivates at the end of the
// one out of it; one that does not the other way round.
void StateNode::_PostCreate() {
    const bool canBlend = CanBlend();
    mActiveAt = !canBlend;
    mActiveUntil = canBlend;
}

// Reconstructed from eboot.elf at 0x709410.
void StateNode::ResetActivity() {
    const bool canBlend = CanBlend();
    mActiveAt = !canBlend;
    mActiveUntil = canBlend;
}

// Reconstructed from eboot.elf at 0x709440.
void StateNode::_Fixup() {
    mRuntime.mFolderNode = _GetObjectCom<StateNodeFolder>(mFolder, true);
    mRuntime.mTransitions.resize(mTransitions.size());
    for (unsigned long index = 0; index < mRuntime.mTransitions.size(); ++index) {
        mRuntime.mTransitions[index] = _GetObjectBaseCom<StateTransition>(mTransitions[index], true);
    }
}

// Reconstructed from eboot.elf at 0x709520.
void StateNode::AddTransition(GameObjectId transition) {
    unsigned long index = 0;
    for (; index < mTransitions.size(); ++index) {
        if (mTransitions[index].mId == transition.mId) {
            return;
        }
    }
    mTransitions._Insert(index, &transition);
    mRuntime.mTransitions.push_back(_GetObjectBaseCom<StateTransition>(transition, true));
}

// Reconstructed from eboot.elf at 0x709680.
bool StateNode::RemoveTransition(GameObjectId transition) {
    unsigned long index = 0;
    for (;; ++index) {
        if (index >= mTransitions.size()) {
            return false;
        }
        if (mTransitions[index].mId == transition.mId) {
            break;
        }
    }
    mTransitions.Destruct(1, mTransitions.ElementAt(index));
    if (static_cast<unsigned int>(index) < mTransitions.mSize - 1) {
        mTransitions.Move(mTransitions.mSize - 1 - index, mTransitions.ElementAt(index), mTransitions.ElementAt(index + 1));
    }
    --mTransitions.mSize;
    StateTransition* const removed = mRuntime.mTransitions[index];
    mRuntime.mTransitions.erase(mRuntime.mTransitions.begin() + index);
    return removed != nullptr;
}

// Reconstructed from eboot.elf at 0x709770.
void StateNode::SetFolder(GameObjectId folder) {
    mFolder = folder;
    mRuntime.mFolderNode = _GetObjectCom<StateNodeFolder>(folder, true);
}

// Reconstructed from eboot.elf at 0x7097A0.
const StateNodeTiming* StateNode::GetTimingSource() const {
    return GetTiming();
}

// Reconstructed from eboot.elf at 0x7097B0.
const StateNodeTiming* StateNode::GetTiming() const {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x7097C0.
bool StateNode::HasValidTiming() const {
    const StateNodeTiming* const timing = GetTiming();
    return timing != nullptr && timing->IsValid();
}

// Reconstructed from eboot.elf at 0x7097E0.
bool StateNode::HasActivity() const {
    if (GetId() == sId) {
        return false;
    }
    return _HasActivity();
}

// Reconstructed from eboot.elf at 0x709820.
StateNodeInstance* StateNode::CreateInstance(StateGraphInstance& graph) const {
    return _CreateInstance(graph);
}

// Reconstructed from eboot.elf at 0x709830.
StateNodeInstance* StateNode::_CreateInstance(StateGraphInstance& graph) const {
    return new StateNodeInstance(*this, graph);
}

// Reconstructed from eboot.elf at 0x709880.
void StateNode::SetGroup(StateNodeGroup* group) {
    mRuntime.mGroup = group;
}

// Reconstructed from eboot.elf at 0x709890.
void StateNode::ClearGroup() {
    mRuntime.mGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x7098A0.
StateNodeInstance::StateNodeInstance(const StateNode& node, StateGraphInstance& graph)
    : mNode(&node), mGraph(&graph), mEntered(false), mFinished(false), mExited(false) {}

// Reconstructed from eboot.elf at 0x7098D0. The deleting destructor at
// 0x7098E0 returns the instance to the pool.
StateNodeInstance::~StateNodeInstance() {}

// Reconstructed from eboot.elf at 0x7098F0.
void StateNodeInstance::Enter() {
    if (mNode->mActiveAt != StateNode::kActiveAtEndOfIncoming || mGraph->mNodeInstances.size() <= 1) {
        _Enter();
        mEntered = true;
    }
}

// Reconstructed from eboot.elf at 0x709930.
void StateNodeInstance::_UpdateEntered() {
    _Enter();
    mEntered = true;
}

// Reconstructed from eboot.elf at 0x709950.
void StateNodeInstance::SetWeights(float weight, float blend) {
    if (mEntered && !mExited) {
        _SetWeight(weight);
        _SetBlend(blend);
    }
}

// Reconstructed from eboot.elf at 0x7099A0.
bool StateNodeInstance::Poll() {
    if (mExited) {
        return false;
    }
    if (!mEntered) {
        if (mNode->mActiveAt == StateNode::kActiveAtEndOfIncoming && mGraph->_IsBlendingIn()) {
            return true;
        }
        _Enter();
        mEntered = true;
    }
    bool finished = true;
    if (!mFinished) {
        finished = !_Poll();
        mFinished = finished;
    }
    if (mNode->mActiveUntil == StateNode::kActiveUntilStartOfOutgoing) {
        if (mGraph->_IsBlendingOut()) {
            Exit();
            return false;
        }
        finished = mFinished;
    }
    return !finished;
}

// Reconstructed from eboot.elf at 0x709A40.
void StateNodeInstance::Exit() {
    if (mEntered && !mExited) {
        _Exit();
        mExited = true;
    }
}

// Reconstructed from eboot.elf at 0x709A70.
bool StateNodeInstance::WantsBlendPartner() const {
    return _WantsBlendPartner();
}

// Reconstructed from eboot.elf at 0x709A80.
bool StateNodeInstance::TakeBlendPartner(StateNodeInstance* previous) {
    return _TakeBlendPartner(previous);
}

// Reconstructed from eboot.elf at 0x709A90.
StateNodeInstance* StateNodeInstance::FindBlendPartner(const StateNodeInstance& other) {
    if (mNode->GetId() == other.mNode->GetId() && _IsBlendPartner(other)) {
        return this;
    }
    return _FindBlendPartner(other);
}

// Reconstructed from eboot.elf at 0x709B00.
bool StateNodeInstance::IsBlendPartner(const StateNodeInstance& other) const {
    if (mNode->GetId() != other.mNode->GetId()) {
        return false;
    }
    return _IsBlendPartner(other);
}

// Reconstructed from eboot.elf at 0x709B60.
StateNodeInstance* StateNodeInstance::FindInstance(const GameObject* object) {
    return _FindInstance(object);
}

// Reconstructed from eboot.elf at 0x709B70.
float StateNodeInstance::GetTimingPosition() {
    const StateNodeTiming* const timing = mNode->GetTiming();
    if (timing != nullptr && timing->IsValid()) {
        return _GetTimingPosition();
    }
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x709BC0.
void StateNodeInstance::SetTimingPosition(float position) {
    const StateNodeTiming* const timing = mNode->GetTiming();
    if (timing != nullptr && timing->IsValid()) {
        _SetTimingPosition(position);
    }
}

// Reconstructed from eboot.elf at 0x709C10. The report is compiled out;
// its arguments remain.
float StateNodeInstance::_GetTimingPosition() {
    static_cast<void>(mNode->MakeErrorName());
    static_cast<void>(mNode->GetId());
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x709C40.
void StateNodeInstance::_SetTimingPosition(float position) {
    static_cast<void>(position);
    static_cast<void>(mNode->MakeErrorName());
    static_cast<void>(mNode->GetId());
}

// Reconstructed from eboot.elf at 0x709C70.
Entity* StateNodeInstance::_GetContextEntity() const {
    return mGraph->mEntity;
}

// Reconstructed from eboot.elf at 0x709C80.
bool StateNodeInstance::UsesObject(GameObjectId object) const {
    if (mNode->mObject->mId.mId == object.mId) {
        return true;
    }
    return _UsesObject(object);
}

// Reconstructed from eboot.elf at 0x709CA0.
bool StateNodeInstance::_UsesObject(GameObjectId object) const {
    static_cast<void>(object);
    return false;
}

// Reconstructed from eboot.elf at 0x709D50.
Symbol StateNode::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x709D60.
Symbol StateNode::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x709D70.
int StateNode::CurrentRev() const {
    return const_cast<StateNode*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x709D90.
bool StateNode::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x709DC0.
Component* StateNode::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x709DD0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNode::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNode*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNode(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x709F80.
PropRegistry& StateNode::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x709F90.
ComMetaData& StateNode::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x709FA0.
bool StateNode::CanBlend() const {
    return false;
}

// Reconstructed from eboot.elf at 0x709FE0.
bool StateNodeInstance::_Poll() {
    return false;
}

// Reconstructed from eboot.elf at 0x70A000.
bool StateNodeInstance::_WantsBlendPartner() const {
    return false;
}

// Reconstructed from eboot.elf at 0x70A010.
bool StateNodeInstance::_TakeBlendPartner(StateNodeInstance* previous) {
    static_cast<void>(previous);
    return false;
}

// Reconstructed from eboot.elf at 0x70A020.
StateNodeInstance* StateNodeInstance::_FindBlendPartner(const StateNodeInstance& other) {
    static_cast<void>(other);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x70A030.
bool StateNodeInstance::_IsBlendPartner(const StateNodeInstance& other) const {
    static_cast<void>(other);
    return false;
}

// Reconstructed from eboot.elf at 0x70A040.
StateNodeInstance* StateNodeInstance::_FindInstance(const GameObject* object) {
    static_cast<void>(object);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x70E940. The binary emits it outside
// StateNode.o, among the node subclasses.
void StateNode::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateNode", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x6FEE90. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNode::_Create() {
    return new StateNode();
}

// Reconstructed from eboot.elf at 0x6F6AF0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNode::Init() {
    InitComponentClass<StateNode>();
}
