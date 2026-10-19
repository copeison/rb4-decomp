// stategraph/StateNodeSubGraph.o (0x7F2310 to 0x7F2E67). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeSubGraph.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "stategraph/resources/StateGraphResource.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x1ABD990, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// "sub_graph" refuses the graph the node belongs to. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x7F2D60.
void SetSubGraphAccessor(const PropAccessorArgs& args) {
    StateNodeSubGraph* const node = static_cast<StateNodeSubGraph*>(args.mComponent);
    const ResourcePath& subGraph = *static_cast<const ResourcePath*>(args.mValue);
    if (subGraph != node->_GetStateGraphResource()->mPath) {
        node->mSubGraph = subGraph;
    }
}

// "sub_graph" redraws the node, whose text is the graph's name. Name not
// in the reference map.
// Reconstructed from eboot.elf at 0x7F2DA0.
void OnSubGraphChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeSubGraph*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x7F2DB0).
Symbol StateNodeSubGraph::sId("StateNodeSubGraph");
Symbol StateNodeSubGraph::sClassName("StateNodeSubGraph");
PropRegistry StateNodeSubGraph::sPropRegistry;
ComMetaData StateNodeSubGraph::sMetaData;

// Reconstructed from eboot.elf at 0x7F2310.
StateNodeSubGraph::StateNodeSubGraph() : mSubGraph() {}

// Reconstructed from eboot.elf at 0x7F2350. The graph must be a state graph
// inlined into the context entity's resource; the property is left out of
// the property tests.
void StateNodeSubGraph::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeSubGraph prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    ResourcePathMetadata& subGraph = TypeSpecificMetadata(
        registry.RegisterProp(
            "sub_graph", offsetof(StateNodeSubGraph, mSubGraph), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mSubGraph);
    subGraph.mDefault = prototype.mSubGraph;
    static_cast<void>(Symbol("prop"));
    subGraph.mResourceId = StateGraphResource::Id();
    subGraph.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    subGraph.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    subGraph.mNoTest = true;
    subGraph.mSetter = SetSubGraphAccessor;
    subGraph.mOnChanged = OnSubGraphChanged;

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that encapsulates an entire subordinate state graph inside it.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("u");
}

// Reconstructed from eboot.elf at 0x7F27E0.
const char* StateNodeSubGraph::GetEditorText() const {
    char base[512] = {};
    FormatString text("%s");
    text << FileGetBase(mSubGraph.Str(), base);
    return text.Str();
}

// Reconstructed from eboot.elf at 0x7F2950.
void StateNodeSubGraph::Instance::_Enter() {
    mSubGraph.Activate(gNullObjectId);
}

// Reconstructed from eboot.elf at 0x7F2960.
bool StateNodeSubGraph::Instance::_Poll() {
    return mSubGraph.Poll();
}

// Reconstructed from eboot.elf at 0x7F2970.
void StateNodeSubGraph::Instance::_Exit() {
    mSubGraph.Deactivate();
}

// Reconstructed from eboot.elf at 0x7F2980.
StateNodeInstance* StateNodeSubGraph::Instance::_FindInstance(const GameObject* object) {
    return mSubGraph.FindNodeInstance(object);
}

// Reconstructed from eboot.elf at 0x7F2A30.
Symbol StateNodeSubGraph::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7F2A40.
Symbol StateNodeSubGraph::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7F2A50.
int StateNodeSubGraph::CurrentRev() const {
    return const_cast<StateNodeSubGraph*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7F2A70.
bool StateNodeSubGraph::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7F2AA0.
Component* StateNodeSubGraph::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7F2AB0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeSubGraph::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeSubGraph*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeSubGraph(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7F2C60.
PropRegistry& StateNodeSubGraph::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7F2C70.
ComMetaData& StateNodeSubGraph::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7F2C80.
StateNodeInstance* StateNodeSubGraph::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x7F2CF0. The deleting destructor at
// 0x7F2D20 returns the instance to the pool.
StateNodeSubGraph::Instance::~Instance() {}

// Reconstructed from eboot.elf at 0x700520. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeSubGraph::_Create() {
    return new StateNodeSubGraph();
}

// Reconstructed from eboot.elf at 0x6FA3B0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeSubGraph::Init() {
    InitComponentClass<StateNodeSubGraph>();
}
