// stategraph/StateNodeGroup.o (0x71AC90 to 0x71E667). The class's
// registration and factory, and the group's destructor, are emitted in
// stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeGroup.h"

#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "math/random/Rand.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/nodes/StateNodeRoot.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/containers/Map.h"
#include "utl/messages/Message.h"
#include "utl/text/MakeString.h"

// The instance of a group in kSimultaneous mode: every child's instance
// runs, and the group works while any child does. The vtable at 0x193C088
// has 15 slots. The object is 80 bytes, from the small-block pool.
class SimultaneousInstance : public StateNodeInstance {
public:
    POOL_OVERLOAD(SimultaneousInstance)

    // Instances of the set children.
    SimultaneousInstance(const StateNodeGroup& group, StateGraphInstance& graph);  // 0x71CB30
    ~SimultaneousInstance() override;  // slots 0-1: 0x71CCB0, 0x71CD30

    void _Enter() override;                    // slot 2: 0x71CDC0
    void _SetWeight(float weight) override;    // slot 3: 0x71CE00
    void _SetBlend(float blend) override;      // slot 4: 0x71CE10
    // Slot 5: polls every child with the group's weight and blend.
    bool _Poll() override;  // 0x71CE20
    void _Exit() override;  // slot 6: 0x71CE90
    // Slots 7-8: the children take the partner once.
    bool _WantsBlendPartner() const override;                     // 0x71CED0
    bool _TakeBlendPartner(StateNodeInstance* previous) override;  // 0x71CEF0
    // Slot 9: the first child's partner.
    StateNodeInstance* _FindBlendPartner(const StateNodeInstance& other) override;  // 0x71CF70
    StateNodeInstance* _FindInstance(const GameObject* object) override;  // slot 11: 0x71CFC0
    bool _UsesObject(GameObjectId object) const override;  // slot 14: 0x71D010

    // Field names are not in the reference map.
    eastl::vector<StateNodeInstance*> mChildren;
    // Set once the children took the blend partner and every child that
    // wanted one got it. The name is weak.
    bool mTookBlendPartner;
    float mWeight;
    float mBlend;
};

static_assert(offsetof(SimultaneousInstance, mChildren) == 32);
static_assert(offsetof(SimultaneousInstance, mTookBlendPartner) == 64);
static_assert(offsetof(SimultaneousInstance, mWeight) == 68);
static_assert(offsetof(SimultaneousInstance, mBlend) == 72);
static_assert(sizeof(SimultaneousInstance) == 80);

// The instance of a group in kSequence mode: one child's instance at a
// time, in the children's order or a shuffled one, starting over when the
// group repeats. The vtable at 0x193C110 has 15 slots. The object is 96
// bytes, from the small-block pool.
class SequenceInstance : public StateNodeInstance {
public:
    POOL_OVERLOAD(SequenceInstance)

    // Draws the shuffled order from gRand.
    SequenceInstance(const StateNodeGroup& group, StateGraphInstance& graph);  // 0x71D060
    ~SequenceInstance() override;  // slots 0-1: 0x71D270, 0x71D2D0

    // Slot 2: enters the current child, creating the first one.
    void _Enter() override;                    // 0x71D340
    void _SetWeight(float weight) override;    // slot 3: 0x71D380
    void _SetBlend(float blend) override;      // slot 4: 0x71D390
    // Slot 5: polls the current child and moves on to the next one when it
    // finishes; false once no child is left.
    bool _Poll() override;  // 0x71D3A0
    void _Exit() override;  // slot 6: 0x71D440
    bool _WantsBlendPartner() const override;                     // slot 7: 0x71D450
    bool _TakeBlendPartner(StateNodeInstance* previous) override;  // slot 8: 0x71D470
    StateNodeInstance* _FindBlendPartner(const StateNodeInstance& other) override;  // slot 9: 0x71D480
    StateNodeInstance* _FindInstance(const GameObject* object) override;  // slot 11: 0x71D4A0
    bool _UsesObject(GameObjectId object) const override;  // slot 14: 0x71D4C0

    // Creates the instance of the next child that has one; true when there
    // is a current child.
    bool _MoveToNextChildInstance();  // 0x71D4E0

    // Field names are not in the reference map.
    const StateNodeGroup* mGroup;
    // The shuffled order of the children's indices; empty when the
    // children run in order.
    eastl::vector<int> mOrder;
    StateNodeInstance* mCurrent;
    float mWeight;
    float mBlend;
    // The position in the order; -1 before the first child.
    int mIndex;
};

static_assert(offsetof(SequenceInstance, mGroup) == 32);
static_assert(offsetof(SequenceInstance, mOrder) == 40);
static_assert(offsetof(SequenceInstance, mCurrent) == 72);
static_assert(offsetof(SequenceInstance, mWeight) == 80);
static_assert(offsetof(SequenceInstance, mIndex) == 88);
static_assert(sizeof(SequenceInstance) == 96);

// The instance of a group in kPickOne mode: the instance of one set child,
// drawn from gRand. The vtable at 0x193C198 has 15 slots. The object is 48
// bytes, from the small-block pool.
class PickOneInstance : public StateNodeInstance {
public:
    POOL_OVERLOAD(PickOneInstance)

    // The map has it out of line; this build inlines it into
    // StateNodeGroup::_CreateInstance (0x71C030).
    PickOneInstance(const StateNodeGroup& group, StateGraphInstance& graph)
        : StateNodeInstance(group, graph), mChild(nullptr), mWeight(0.0F), mBlend(0.0F) {
        unsigned int count = 0;
        for (const StateNode* child : group.mChildren) {
            if (child != nullptr) {
                ++count;
            }
        }
        const unsigned int pick = static_cast<unsigned int>(gRand.Int() % count);
        unsigned int index = 0;
        for (unsigned int i = 0; i < group.mChildNodes.size(); ++i) {
            const StateNode* const child = group.mChildren[i];
            if (child != nullptr) {
                if (index == pick) {
                    mChild = child->CreateInstance(graph);
                    return;
                }
                ++index;
            }
        }
    }
    ~PickOneInstance() override;  // slots 0-1: 0x71D5E0, 0x71D620

    void _Enter() override;                    // slot 2: 0x71D670
    void _SetWeight(float weight) override;    // slot 3: 0x71D680
    void _SetBlend(float blend) override;      // slot 4: 0x71D690
    bool _Poll() override;                     // slot 5: 0x71D6A0
    void _Exit() override;                     // slot 6: 0x71D6E0
    bool _WantsBlendPartner() const override;                     // slot 7: 0x71D6F0
    bool _TakeBlendPartner(StateNodeInstance* previous) override;  // slot 8: 0x71D710
    StateNodeInstance* _FindBlendPartner(const StateNodeInstance& other) override;  // slot 9: 0x71D720
    StateNodeInstance* _FindInstance(const GameObject* object) override;  // slot 11: 0x71D740
    bool _UsesObject(GameObjectId object) const override;  // slot 14: 0x71D760

    // Field names are not in the reference map.
    StateNodeInstance* mChild;
    float mWeight;
    float mBlend;
};

static_assert(offsetof(PickOneInstance, mChild) == 32);
static_assert(offsetof(PickOneInstance, mWeight) == 40);
static_assert(offsetof(PickOneInstance, mBlend) == 44);
static_assert(sizeof(PickOneInstance) == 48);

namespace {

// The id of no object, at 0x1AB3DF8, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The array index the last node of the accessor's path names. Inlined into
// the array hooks. Name not in the reference map.
unsigned long LastIndex(const PropAccessorArgs& args) {
    const PropPath& path = *static_cast<const PropPath*>(args.mPath);
    return path[-1].mValue;
}

// The "child_nodes" hooks keep the resolved children in step with the
// array: an insert adds the node the inserted id names (the value arrives
// through mOldValue) and makes this its group; a removal tells the entity
// ("state_node_group_removed" <removed id> <group>), redraws the group and
// drops the child. Names not in the reference map.
// Reconstructed from eboot.elf at 0x71C5F0.
void InsertChildHook(const PropAccessorArgs& args) {
    StateNodeGroup* const group = static_cast<StateNodeGroup*>(args.mComponent);
    const unsigned long index = LastIndex(args);
    const GameObjectId* const inserted = static_cast<const GameObjectId*>(args.mOldValue);
    GameObjectId id = inserted != nullptr ? *inserted : gNullObjectId;
    group->mChildNodes._Insert(index, &id);
    StateNode* const child = group->_GetObjectBaseCom<StateNode>(id, true);
    group->mChildren.insert(group->mChildren.begin() + index, child);
    if (child != nullptr) {
        child->SetGroup(group);
    }
}

// Reconstructed from eboot.elf at 0x71C810.
void RemoveChildHook(const PropAccessorArgs& args) {
    StateNodeGroup* const group = static_cast<StateNodeGroup*>(args.mComponent);
    Message<2> msg(Symbol("state_node_group_removed"));
    DataNode removed;
    removed.mType = kDataGameObjectId;
    removed.mValue.var = static_cast<const GameObjectId*>(args.mOldValue)->mId;
    msg.mData->Node(2) = removed;
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = group;
    msg.mData->Node(3) = self;
    group->mObject->mEntity->Export(msg.mData);
    group->_SendDisplayTextUpdated();

    PropArray<GameObjectId>& children = group->mChildNodes;
    const unsigned long index = LastIndex(args);
    children.Destruct(1, children.ElementAt(index));
    if (static_cast<unsigned int>(index) < children.mSize - 1) {
        children.Move(children.mSize - 1 - index, children.ElementAt(index), children.ElementAt(index + 1));
    }
    --children.mSize;
    StateNode* const child = group->mChildren[index];
    group->mChildren.erase(group->mChildren.begin() + index);
    if (child != nullptr) {
        child->ClearGroup();
    }
}

// The element setter of "child_nodes". The binary's body is only ud2:
// every path of the original is undefined behaviour, so what it meant to do
// is lost. Name not in the reference map; the evidence is weak.
// Reconstructed from eboot.elf at 0x71CB20.
void SetChildNodeAccessor(const PropAccessorArgs& args) {
    static_cast<void>(args);
    __builtin_trap();
}

// The objects a group's child may not be: the nodes outside the group's
// folder, the roots, the folders and the folder's own start and exit
// nodes, every node another group runs (or this one, except the element
// being edited), and the outermost group that runs this one. Name not in
// the reference map.
eastl::vector<GameObjectId> ExcludedChildNodes(const TagArgs& args) {
    const StateNodeGroup& self = static_cast<const StateNodeGroup&>(args.mComponent);
    Entity* const entity = self.mObject->mEntity;
    const StateNodeFolder* const folderNode = self.mRuntime.mFolderNode;
    const GameObjectId current = self.mChildNodes[args.mPath[1].mValue];
    const GameObjectId folder = folderNode != nullptr ? folderNode->mObject->mId : gNullObjectId;
    eastl::vector<GameObjectId> excluded;
    // The group that runs each group, by the group's object.
    eastl::map<GameObjectId, StateNodeGroup*> parents;
    for (const GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const StateNode* const node = object->GetBaseCom<StateNode>();
        if (node == nullptr) {
            continue;
        }
        const GameObjectId id = object->mId;
        if (node->mFolder.mId != folder.mId || object->GetCom<StateNodeRoot>() != nullptr ||
            object->GetCom<StateNodeFolder>() != nullptr ||
            (folderNode != nullptr && (id.mId == folderNode->mStart.mId || id.mId == folderNode->mExit.mId))) {
            excluded.push_back(id);
            continue;
        }
        StateNodeGroup* const group = object->GetCom<StateNodeGroup>();
        if (group == nullptr) {
            continue;
        }
        for (unsigned int i = 0; i < group->mChildNodes.size(); ++i) {
            const StateNode* const child = group->mChildren[i];
            if (child == nullptr) {
                continue;
            }
            const GameObjectId childId = child->mObject->mId;
            if (childId.mId != current.mId) {
                excluded.push_back(childId);
            }
            if (child->GetId() == StateNodeGroup::sId) {
                parents[childId] = group;
            }
        }
    }
    const StateNodeGroup* outermost = &self;
    while (StateNodeGroup* const parent = parents[outermost->mObject->mId]) {
        outermost = parent;
    }
    if (outermost != &self) {
        excluded.push_back(outermost->mObject->mId);
    }
    return excluded;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x71E5B0).
Symbol StateNodeGroup::sId("StateNodeGroup");
Symbol StateNodeGroup::sClassName("StateNodeGroup");
PropRegistry StateNodeGroup::sPropRegistry;
ComMetaData StateNodeGroup::sMetaData;

// Reconstructed from eboot.elf at 0x71AC90.
StateNodeGroup::StateNodeGroup()
    : mChildNodes(), mMode(kSimultaneous), mShuffle(false), mRepeat(false), mChildren() {}

// Reconstructed from eboot.elf at 0x71AD50. "shuffle" and "repeat" are
// hidden unless the group is a sequence; the children must be state nodes
// other than the group itself.
void StateNodeGroup::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeGroup prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    IntMetadata<int>& mode = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("mode", offsetof(StateNodeGroup, mMode), kPropertyInt, 4).mMetadata.Get());
    mode.mDefault = prototype.mMode;
    static_cast<void>(Symbol("prop"));
    mode.mHelp = String("The manner in which the child nodes will be activated.");
    mode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kSimultaneous,
            String("Simultaneous"),
            String(
                "All child nodes will execute simultaneously; the group is finished when all children are "
                "finished.")},
        AllowedValue<int>{
            kSequence,
            String("Sequence"),
            String(
                "All child nodes will execute in sequence, optionally shuffled; the group is finished when the "
                "last child is finished, unless looping is enabled.")},
        AllowedValue<int>{
            kPickOne,
            String("Pick One"),
            String(
                "A single child node is chosen at random and activated; the group is finished when the child is "
                "finished.")}};

    BoolMetadata& shuffle = TypeSpecificMetadata(
        registry.RegisterProp("shuffle", offsetof(StateNodeGroup, mShuffle), kPropertyBool, 1), prototype.mShuffle);
    shuffle.mDefault = prototype.mShuffle;
    static_cast<void>(Symbol("prop"));
    shuffle.mHelp = String("If true, the children will be executed in a shuffled order.");
    // At 0x71D800.
    shuffle.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeGroup&>(args.mComponent).mMode != kSequence;
    });

    BoolMetadata& repeat = TypeSpecificMetadata(
        registry.RegisterProp("repeat", offsetof(StateNodeGroup, mRepeat), kPropertyBool, 1), prototype.mRepeat);
    repeat.mDefault = prototype.mRepeat;
    static_cast<void>(Symbol("prop"));
    repeat.mHelp = String("If true, the sequence will loop indefinitely.");
    // At 0x71D900.
    repeat.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeGroup&>(args.mComponent).mMode != kSequence;
    });

    PropInfo& childNodesInfo = registry.RegisterPropArray(
        "child_nodes", offsetof(StateNodeGroup, mChildNodes), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& childNodes = static_cast<ArrayMetadata&>(*childNodesInfo.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    childNodes.mHelp = String("The nodes that will be executed as members of this group.");
    childNodes.mExtraFlag0 = false;
    childNodes.mInsert = InsertChildHook;
    childNodes.mHook2 = reinterpret_cast<void*>(&RemoveChildHook);
    PropRegistry::RegisterPropArrayItem(childNodesInfo, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId itemType = {0xFFFFFFFFU};
    GameObjectIdMetadata& item = TypeSpecificMetadata(childNodes.mItemInfo, itemType);
    static_cast<void>(Symbol("item"));
    item.mRequiredInterfaces = {StateNode::sClassName};
    item.mAllowsThis = false;
    // At 0x71DA00.
    item.mExcludedObjects._Set(ExcludedChildNodes);
    item.mSetter = SetChildNodeAccessor;

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that simultaneously executes a group of subordinate states.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("g");
}

// Reconstructed from eboot.elf at 0x6FFE20. The complete destructor at
// 0x71B750 jumps here and the deleting one is at 0x71C2E0.
StateNodeGroup::~StateNodeGroup() {}

// Reconstructed from eboot.elf at 0x71B760.
void StateNodeGroup::_Fixup() {
    for (StateNode* const child : mChildren) {
        if (child != nullptr) {
            child->ClearGroup();
        }
    }
    StateNode::_Fixup();
    mChildren.resize(mChildNodes.size());
    for (unsigned long i = 0; i < mChildren.size(); ++i) {
        mChildren[i] = _GetObjectBaseCom<StateNode>(mChildNodes[i], true);
    }
    for (StateNode* const child : mChildren) {
        if (child != nullptr) {
            child->SetGroup(this);
        }
    }
}

// Reconstructed from eboot.elf at 0x71B8C0.
bool StateNodeGroup::UsesColorBlending() const {
    for (const StateNode* const child : mChildren) {
        if (child != nullptr && child->UsesColorBlending()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x71B910.
bool StateNodeGroup::UsesRotationBlending() const {
    for (const StateNode* const child : mChildren) {
        if (child != nullptr && child->UsesRotationBlending()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x71B960.
void StateNodeGroup::AddChild(GameObjectId child) {
    for (unsigned long i = 0; i < mChildNodes.size(); ++i) {
        if (mChildNodes[i].mId == child.mId) {
            return;
        }
    }
    mChildNodes._Insert(mChildNodes.size(), &child);
    StateNode* const node = _GetObjectBaseCom<StateNode>(child, true);
    mChildren.push_back(node);
    if (node != nullptr) {
        node->SetGroup(this);
    }
}

// Reconstructed from eboot.elf at 0x71BB20.
void StateNodeGroup::RemoveChild(GameObjectId child) {
    for (unsigned long index = 0; index < mChildNodes.size(); ++index) {
        if (mChildNodes[index].mId != child.mId) {
            continue;
        }
        mChildNodes.Destruct(1, mChildNodes.ElementAt(index));
        if (static_cast<unsigned int>(index) < mChildNodes.mSize - 1) {
            mChildNodes.Move(
                mChildNodes.mSize - 1 - index, mChildNodes.ElementAt(index), mChildNodes.ElementAt(index + 1));
        }
        --mChildNodes.mSize;
        StateNode* const node = mChildren[index];
        mChildren.erase(mChildren.begin() + index);
        if (node != nullptr) {
            node->ClearGroup();
        }
        return;
    }
}

// Reconstructed from eboot.elf at 0x71BCA0.
void StateNodeGroup::SetChild(unsigned long index, GameObjectId child) {
    StateNode* const old = mChildren[index];
    if (old != nullptr) {
        old->ClearGroup();
    }
    mChildNodes[index] = child;
    StateNode* const node = _GetObjectBaseCom<StateNode>(child, true);
    mChildren[index] = node;
    if (node != nullptr) {
        node->SetGroup(this);
    }
}

// Reconstructed from eboot.elf at 0x71BD10.
const char* StateNodeGroup::GetEditorText() const {
    unsigned long count = 0;
    for (unsigned long i = 0; i < mChildNodes.size(); ++i) {
        if (mChildNodes[i].mId != gNullObjectId.mId) {
            ++count;
        }
    }
    FormatString text("%u nodes");
    text << count;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x71BEB0.
void StateNodeGroup::GetChildNodes(eastl::vector<GameObjectId>& children) const {
    for (const StateNode* const child : mChildren) {
        if (child != nullptr) {
            children.push_back(child->mObject->mId);
            child->GetChildNodes(children);
        }
    }
}

// Reconstructed from eboot.elf at 0x71BFF0.
void StateNodeGroup::ForgetChildNode(const StateNode* child) {
    for (StateNode*& node : mChildren) {
        if (node == child) {
            node = nullptr;
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x71C030.
StateNodeInstance* StateNodeGroup::_CreateInstance(StateGraphInstance& graph) const {
    switch (mMode) {
    case kSimultaneous:
        return new SimultaneousInstance(*this, graph);
    case kSequence:
        return new SequenceInstance(*this, graph);
    case kPickOne:
        return new PickOneInstance(*this, graph);
    default:
        return nullptr;
    }
}

// Reconstructed from eboot.elf at 0x71C300.
Symbol StateNodeGroup::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x71C310.
Symbol StateNodeGroup::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x71C320.
int StateNodeGroup::CurrentRev() const {
    return const_cast<StateNodeGroup*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x71C340.
bool StateNodeGroup::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x71C370.
Component* StateNodeGroup::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x71C380. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeGroup::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeGroup*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeGroup(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x71C5B0.
PropRegistry& StateNodeGroup::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x71C5C0.
ComMetaData& StateNodeGroup::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x71C5D0.
bool StateNodeGroup::CanBlend() const {
    return true;
}

// Reconstructed from eboot.elf at 0x71C5E0.
bool StateNodeGroup::_HasActivity() const {
    return false;
}

// Reconstructed from eboot.elf at 0x71CB30.
SimultaneousInstance::SimultaneousInstance(const StateNodeGroup& group, StateGraphInstance& graph)
    : StateNodeInstance(group, graph), mChildren(), mTookBlendPartner(false), mWeight(0.0F), mBlend(0.0F) {
    for (unsigned int i = 0; i < group.mChildNodes.size(); ++i) {
        const StateNode* const child = group.mChildren[i];
        if (child != nullptr) {
            mChildren.push_back(child->CreateInstance(graph));
        }
    }
}

// Reconstructed from eboot.elf at 0x71CCB0. The deleting destructor at
// 0x71CD30 returns the instance to the pool.
SimultaneousInstance::~SimultaneousInstance() {
    for (StateNodeInstance* const child : mChildren) {
        delete child;
    }
    mChildren.clear();
}

// Reconstructed from eboot.elf at 0x71CDC0.
void SimultaneousInstance::_Enter() {
    for (StateNodeInstance* const child : mChildren) {
        child->Enter();
    }
}

// Reconstructed from eboot.elf at 0x71CE00.
void SimultaneousInstance::_SetWeight(float weight) {
    mWeight = weight;
}

// Reconstructed from eboot.elf at 0x71CE10.
void SimultaneousInstance::_SetBlend(float blend) {
    mBlend = blend;
}

// Reconstructed from eboot.elf at 0x71CE20.
bool SimultaneousInstance::_Poll() {
    bool finished = true;
    for (StateNodeInstance* const child : mChildren) {
        child->SetWeights(mWeight, mBlend);
        if (child->Poll()) {
            finished = false;
        }
    }
    return !finished;
}

// Reconstructed from eboot.elf at 0x71CE90.
void SimultaneousInstance::_Exit() {
    for (StateNodeInstance* const child : mChildren) {
        child->Exit();
    }
}

// Reconstructed from eboot.elf at 0x71CED0.
bool SimultaneousInstance::_WantsBlendPartner() const {
    return !mChildren.empty() && !mTookBlendPartner;
}

// Reconstructed from eboot.elf at 0x71CEF0.
bool SimultaneousInstance::_TakeBlendPartner(StateNodeInstance* previous) {
    mTookBlendPartner = true;
    for (StateNodeInstance* const child : mChildren) {
        const bool took = child->WantsBlendPartner() ? child->TakeBlendPartner(previous) : true;
        mTookBlendPartner = mTookBlendPartner && took;
    }
    return mTookBlendPartner;
}

// Reconstructed from eboot.elf at 0x71CF70.
StateNodeInstance* SimultaneousInstance::_FindBlendPartner(const StateNodeInstance& other) {
    for (StateNodeInstance* const child : mChildren) {
        StateNodeInstance* const partner = child->FindBlendPartner(other);
        if (partner != nullptr) {
            return partner;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x71CFC0.
StateNodeInstance* SimultaneousInstance::_FindInstance(const GameObject* object) {
    for (StateNodeInstance* const child : mChildren) {
        StateNodeInstance* const instance = child->FindInstance(object);
        if (instance != nullptr) {
            return instance;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x71D010.
bool SimultaneousInstance::_UsesObject(GameObjectId object) const {
    for (StateNodeInstance* const child : mChildren) {
        if (child->UsesObject(object)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x71D060. The order is shuffled as
// eastl::random_shuffle does, with gRand.
SequenceInstance::SequenceInstance(const StateNodeGroup& group, StateGraphInstance& graph)
    : StateNodeInstance(group, graph), mGroup(&group), mOrder(), mCurrent(nullptr), mWeight(0.0F), mBlend(0.0F),
      mIndex(-1) {
    if (group.mMode != StateNodeGroup::kSequence || !group.mShuffle) {
        return;
    }
    const unsigned int count = group.mChildNodes.size();
    for (unsigned int i = 0; i != count; ++i) {
        mOrder.push_back(static_cast<int>(i));
    }
    if (mOrder.empty()) {
        return;
    }
    int* const first = mOrder.begin();
    for (int* element = first + 1; element != mOrder.end(); ++element) {
        const unsigned long other = gRand.Int() % static_cast<unsigned long>(element - first + 1);
        const int value = *element;
        *element = first[other];
        first[other] = value;
    }
}

// Reconstructed from eboot.elf at 0x71D270. The deleting destructor at
// 0x71D2D0 returns the instance to the pool.
SequenceInstance::~SequenceInstance() {
    delete mCurrent;
    mCurrent = nullptr;
}

// Reconstructed from eboot.elf at 0x71D340.
void SequenceInstance::_Enter() {
    if (mCurrent == nullptr) {
        _MoveToNextChildInstance();
    }
    if (mCurrent != nullptr) {
        mCurrent->Enter();
    }
}

// Reconstructed from eboot.elf at 0x71D380.
void SequenceInstance::_SetWeight(float weight) {
    mWeight = weight;
}

// Reconstructed from eboot.elf at 0x71D390.
void SequenceInstance::_SetBlend(float blend) {
    mBlend = blend;
}

// Reconstructed from eboot.elf at 0x71D3A0. A child that finishes at once
// hands over to the next one in the same poll; the search stops when the
// new instance is the first one polled.
bool SequenceInstance::_Poll() {
    StateNodeInstance* const first = mCurrent;
    StateNodeInstance* current = first;
    while (current != nullptr) {
        mCurrent->SetWeights(mWeight, mBlend);
        if (mCurrent->Poll()) {
            return mCurrent != nullptr;
        }
        mCurrent->Exit();
        delete mCurrent;
        mCurrent = nullptr;
        if (_MoveToNextChildInstance()) {
            mCurrent->Enter();
        }
        current = mCurrent;
        if (current == first) {
            break;
        }
    }
    return current != nullptr;
}

// Reconstructed from eboot.elf at 0x71D440.
void SequenceInstance::_Exit() {
    if (mCurrent != nullptr) {
        mCurrent->Exit();
    }
}

// Reconstructed from eboot.elf at 0x71D450.
bool SequenceInstance::_WantsBlendPartner() const {
    return mCurrent != nullptr && mCurrent->WantsBlendPartner();
}

// Reconstructed from eboot.elf at 0x71D470. Only called after
// _WantsBlendPartner, so the current child is set.
bool SequenceInstance::_TakeBlendPartner(StateNodeInstance* previous) {
    return mCurrent->TakeBlendPartner(previous);
}

// Reconstructed from eboot.elf at 0x71D480.
StateNodeInstance* SequenceInstance::_FindBlendPartner(const StateNodeInstance& other) {
    return mCurrent != nullptr ? mCurrent->FindBlendPartner(other) : nullptr;
}

// Reconstructed from eboot.elf at 0x71D4A0.
StateNodeInstance* SequenceInstance::_FindInstance(const GameObject* object) {
    return mCurrent != nullptr ? mCurrent->FindInstance(object) : nullptr;
}

// Reconstructed from eboot.elf at 0x71D4C0.
bool SequenceInstance::_UsesObject(GameObjectId object) const {
    return mCurrent != nullptr && mCurrent->UsesObject(object);
}

// Reconstructed from eboot.elf at 0x71D4E0. A repeating sequence wraps
// around; at most one pass over the children is tried.
bool SequenceInstance::_MoveToNextChildInstance() {
    const int count = static_cast<int>(mGroup->mChildNodes.size());
    if (count == 0) {
        return false;
    }
    if (count - 1 <= mIndex) {
        mIndex = count - 1;
    }
    if (mCurrent == nullptr) {
        for (unsigned long attempt = 1; mIndex < count; ++attempt) {
            ++mIndex;
            if (mGroup->mMode == StateNodeGroup::kSequence && mGroup->mRepeat) {
                mIndex %= count;
            }
            if (mIndex < count) {
                const int child = mOrder.empty() ? mIndex : mOrder[mIndex];
                const StateNode* const node = mGroup->mChildren[child];
                if (node != nullptr) {
                    mCurrent = node->CreateInstance(*mGraph);
                    if (mCurrent != nullptr) {
                        break;
                    }
                }
            }
            if (attempt >= static_cast<unsigned long>(count)) {
                break;
            }
        }
    }
    return mCurrent != nullptr;
}

// Reconstructed from eboot.elf at 0x71D5E0. The deleting destructor at
// 0x71D620 returns the instance to the pool.
PickOneInstance::~PickOneInstance() {
    delete mChild;
    mChild = nullptr;
}

// Reconstructed from eboot.elf at 0x71D670.
void PickOneInstance::_Enter() {
    if (mChild != nullptr) {
        mChild->Enter();
    }
}

// Reconstructed from eboot.elf at 0x71D680.
void PickOneInstance::_SetWeight(float weight) {
    mWeight = weight;
}

// Reconstructed from eboot.elf at 0x71D690.
void PickOneInstance::_SetBlend(float blend) {
    mBlend = blend;
}

// Reconstructed from eboot.elf at 0x71D6A0.
bool PickOneInstance::_Poll() {
    if (mChild == nullptr) {
        return false;
    }
    mChild->SetWeights(mWeight, mBlend);
    return mChild->Poll();
}

// Reconstructed from eboot.elf at 0x71D6E0.
void PickOneInstance::_Exit() {
    if (mChild != nullptr) {
        mChild->Exit();
    }
}

// Reconstructed from eboot.elf at 0x71D6F0.
bool PickOneInstance::_WantsBlendPartner() const {
    return mChild != nullptr && mChild->WantsBlendPartner();
}

// Reconstructed from eboot.elf at 0x71D710. Only called after
// _WantsBlendPartner, so the child is set.
bool PickOneInstance::_TakeBlendPartner(StateNodeInstance* previous) {
    return mChild->TakeBlendPartner(previous);
}

// Reconstructed from eboot.elf at 0x71D720.
StateNodeInstance* PickOneInstance::_FindBlendPartner(const StateNodeInstance& other) {
    return mChild != nullptr ? mChild->FindBlendPartner(other) : nullptr;
}

// Reconstructed from eboot.elf at 0x71D740.
StateNodeInstance* PickOneInstance::_FindInstance(const GameObject* object) {
    return mChild != nullptr ? mChild->FindInstance(object) : nullptr;
}

// Reconstructed from eboot.elf at 0x71D760.
bool PickOneInstance::_UsesObject(GameObjectId object) const {
    return mChild != nullptr && mChild->UsesObject(object);
}

// Reconstructed from eboot.elf at 0x6FFDD0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeGroup::_Create() {
    return new StateNodeGroup();
}

// Reconstructed from eboot.elf at 0x6F9B30. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeGroup::Init() {
    InitComponentClass<StateNodeGroup>();
}
