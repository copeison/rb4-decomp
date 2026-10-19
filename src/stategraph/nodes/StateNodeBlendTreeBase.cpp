// stategraph/StateNodeBlendTreeBase.o (0x717900 to 0x718EFA): the blend
// tree nodes StateNodeBlendTreeBase, StateNodeClipBlendTree and
// StateNodeClipJukebox, with one static initializer. The classes'
// registrations and factories, the subclasses' methods and the base's
// inline slots are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeBlendTreeBase.h"

#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "os/memory/MemMgr.h"
#include "stategraph/blendtrees/ClipBlendTreeCom.h"
#include "stategraph/blendtrees/ClipJukeboxCom.h"
#include "stategraph/nodes/StateNodeClipBlendTree.h"
#include "stategraph/nodes/StateNodeClipJukebox.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x718D60).
// The id of no object it first sets (0x1AB3208) comes from a shared header
// and is not used here.
Symbol StateNodeBlendTreeBase::sId("StateNodeBlendTreeBase");
Symbol StateNodeBlendTreeBase::sClassName("StateNodeBlendTreeBase");
PropRegistry StateNodeBlendTreeBase::sPropRegistry;
ComMetaData StateNodeBlendTreeBase::sMetaData;
Symbol StateNodeClipBlendTree::sId("StateNodeClipBlendTree");
Symbol StateNodeClipBlendTree::sClassName("StateNodeClipBlendTree");
PropRegistry StateNodeClipBlendTree::sPropRegistry;
ComMetaData StateNodeClipBlendTree::sMetaData;
Symbol StateNodeClipJukebox::sId("StateNodeClipJukebox");
Symbol StateNodeClipJukebox::sClassName("StateNodeClipJukebox");
PropRegistry StateNodeClipJukebox::sPropRegistry;
ComMetaData StateNodeClipJukebox::sMetaData;

// Reconstructed from eboot.elf at 0x717900. The node has no properties of
// its own; the subclasses' metadata is creatable, the base's is not.
void StateNodeBlendTreeBase::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeBlendTreeBase prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    StateNode::_InitAsSuperclass(registry, metadata);
    const char* const description =
        FormatString(
            "Base class for StateGraph nodes that own and operate instances of various BlendTree subclasses.")
            .Str();
    metadata.mDescription = description;
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol(&metadata == &sMetaData ? "false" : "true");
}

// Reconstructed from eboot.elf at 0x717BC0. The deleting destructor is at
// 0x718A60.
StateNodeBlendTreeBase::~StateNodeBlendTreeBase() {}

// Reconstructed from eboot.elf at 0x717C50.
void StateNodeBlendTreeBase::GetChildNodes(eastl::vector<GameObjectId>& children) const {
    GetBlendTree()->GetChildObjects(children, true);
}

// Reconstructed from eboot.elf at 0x717CD0.
bool StateNodeBlendTreeBase::IsValid() const {
    return true;
}

// Reconstructed from eboot.elf at 0x717CF0.
const char* StateNodeBlendTreeBase::GetName() const {
    return StateBase::GetName().Str();
}

// Reconstructed from eboot.elf at 0x717D10.
float StateNodeBlendTreeBase::GetScale(int units, float rate) const {
    return GetBlendTree()->GetScale(units, rate);
}

// Reconstructed from eboot.elf at 0x717DF0.
bool StateNodeBlendTreeBase::GetLength(float& length) const {
    length = 0.0f;
    return false;
}

// Reconstructed from eboot.elf at 0x717E10.
int StateNodeBlendTreeBase::GetUnits() const {
    return GetBlendTree()->GetTimeUnits();
}

// Reconstructed from eboot.elf at 0x717EF0.
void StateNodeBlendTreeBase::Refresh() const {
    GetBlendTree()->Refresh();
}

// Reconstructed from eboot.elf at 0x717FD0. The tree's instance plays in
// the game, not the editor's preview.
StateNodeBlendTreeBase::Instance::Instance(const StateNodeBlendTreeBase& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph),
      mTreeInstance(node.GetBlendTree()->CreateInstance(graph.mEntity, false)),
      mWeight(0.0f) {}

// Reconstructed from eboot.elf at 0x718070. The deleting destructor at
// 0x7180B0 returns the instance to the pool.
StateNodeBlendTreeBase::Instance::~Instance() {
    delete mTreeInstance;
    mTreeInstance = nullptr;
}

// Reconstructed from eboot.elf at 0x718100.
bool StateNodeBlendTreeBase::Instance::_Poll() {
    mTreeInstance->Poll(mWeight);
    return true;
}

// Reconstructed from eboot.elf at 0x718120.
void StateNodeBlendTreeBase::Instance::_Exit() {
    mTreeInstance->Stop();
    delete mTreeInstance;
    mTreeInstance = nullptr;
}

// Reconstructed from eboot.elf at 0x718160.
StateNodeInstance* StateNodeBlendTreeBase::Instance::_FindInstance(const GameObject* object) {
    if (!mTreeInstance->GetBlendTree()->ContainsObject(object)) {
        return nullptr;
    }
    return reinterpret_cast<StateNodeInstance*>(mTreeInstance);
}

// Reconstructed from eboot.elf at 0x7181A0.
float StateNodeBlendTreeBase::Instance::_GetTimingPosition() {
    return mTreeInstance->GetTimingPosition();
}

// Reconstructed from eboot.elf at 0x7181B0.
void StateNodeBlendTreeBase::Instance::_SetTimingPosition(float position) {
    mTreeInstance->SetTimingPosition(position);
}

// Reconstructed from eboot.elf at 0x7181C0.
void StateNodeClipBlendTree::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeClipBlendTree prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    StateNodeBlendTreeBase::_InitAsSuperclass(registry, metadata);
    const char* const description =
        FormatString(
            "Owns and operates an instance of a CharBlendTree. This allows blending between an arbitrarily deep "
            "set of nested character animation clips according to specified parameters.")
            .Str();
    metadata.mDescription = description;
    metadata.mAuthor = "Matt Sharpe";
    metadata.mRequiredComponents.push_back(ClipBlendTreeCom::sClassName);
    metadata.mAliases.push_back(Symbol("StateNodeBlendTree"));
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("b");
}

// Reconstructed from eboot.elf at 0x718610.
void StateNodeBlendTreeBase::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateNodeBlendTree", sMetaData);
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

// Reconstructed from eboot.elf at 0x7186C0.
void StateNodeClipJukebox::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeClipJukebox prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    StateNodeBlendTreeBase::_InitAsSuperclass(registry, metadata);
    const char* const description =
        FormatString(
            "Owns and operates an instance of a CharJukebox. This allows playing back a set of character "
            "animation clips in randomized order, crossfading from one to the next.")
            .Str();
    metadata.mDescription = description;
    metadata.mAuthor = "Matt Sharpe";
    metadata.mRequiredComponents.push_back(ClipJukeboxCom::sClassName);
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("j");
}

// Reconstructed from eboot.elf at 0x718B00.
Symbol StateNodeBlendTreeBase::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x718B10.
Symbol StateNodeBlendTreeBase::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x718B20.
int StateNodeBlendTreeBase::CurrentRev() const {
    return const_cast<StateNodeBlendTreeBase*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x718B40.
bool StateNodeBlendTreeBase::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x718B70.
Component* StateNodeBlendTreeBase::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x718B80. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeBlendTreeBase::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeBlendTreeBase*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeBlendTreeBase(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x718D30.
PropRegistry& StateNodeBlendTreeBase::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x718D40.
ComMetaData& StateNodeBlendTreeBase::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x6FF410. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeBlendTreeBase::_Create() {
    return new StateNodeBlendTreeBase();
}

// Reconstructed from eboot.elf at 0x6FF490. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeClipBlendTree::_Create() {
    return new StateNodeClipBlendTree();
}

// Reconstructed from eboot.elf at 0x6FF930. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeClipJukebox::_Create() {
    return new StateNodeClipJukebox();
}

// Reconstructed from eboot.elf at 0x6F84A0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeBlendTreeBase::Init() {
    InitComponentClass<StateNodeBlendTreeBase>();
}

// Reconstructed from eboot.elf at 0x6F8930. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeClipBlendTree::Init() {
    InitComponentClass<StateNodeClipBlendTree>();
}

// Reconstructed from eboot.elf at 0x6F8DC0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeClipJukebox::Init() {
    InitComponentClass<StateNodeClipJukebox>();
}
