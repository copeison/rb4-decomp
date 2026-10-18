// stategraph/StateBase.o (0x807E80 to 0x808AD5). The class's registration
// and factory are emitted in stategraph/StateGraph.o, and the superclass
// helper in stategraph/StateNode.o.
#include "stategraph/base/StateBase.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "os/memory/MemMgr.h"
#include "stategraph/resources/StateGraphResource.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"

namespace {

// The "name" property reads and writes the object's name and redraws the
// node when it changes. Names not in the reference map.
// Reconstructed from eboot.elf at 0x8087D0.
Symbol GetObjectName(const PropAccessorArgs& args) {
    return args.mComponent->mObject->mName;
}

// Reconstructed from eboot.elf at 0x8087E0.
void SetObjectName(const PropAccessorArgs& args) {
    args.mComponent->mObject->SetName(*static_cast<const Symbol*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x808800.
void OnNameChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x808810.
void OnCommentChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

// "object_id" reads the serial of the object's id. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x808820.
unsigned int GetObjectSerial(const PropAccessorArgs& args) {
    return args.mComponent->mObject->mId.mId >> 16;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x808A00).
// The id of no object it first sets (0x1ABED98) comes from a shared header
// and is not used here.
Symbol StateBase::sId("StateBase");
Symbol StateBase::sClassName("StateBase");
PropRegistry StateBase::sPropRegistry;
ComMetaData StateBase::sMetaData;
std::function<Entity*(const TagArgs&)> StateBase::sContextEntityTagFunc;

// Reconstructed from eboot.elf at 0x807EB0. "name" and "object_id" have no
// storage: they go through the object. Every property registration builds
// a "prop" symbol it does not use.
void StateBase::_Init(PropRegistry& registry, ComMetaData& metadata) {
    if (!sContextEntityTagFunc) {
        // At 0x8088B0.
        sContextEntityTagFunc = [](const TagArgs& args) {
            return static_cast<const StateBase&>(args.mComponent)._GetContextEntityPrototype();
        };
    }
    StateBase prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    const Symbol nameType;
    SymbolMetadata& name = TypeSpecificMetadata(registry.RegisterProp("name", -1, kPropertySymbol, 0), nameType);
    static_cast<void>(Symbol("prop"));
    name.mGetter = GetObjectName;
    name.mSetter = SetObjectName;
    name.mOnChanged = OnNameChanged;
    name.mDebug = true;

    SymbolMetadata& comment = TypeSpecificMetadata(
        registry.RegisterProp("comment", offsetof(StateBase, mComment), kPropertySymbol, sizeof(Symbol)),
        prototype.mComment);
    comment.mDefault = prototype.mComment;
    static_cast<void>(Symbol("prop"));
    comment.mHelp = String("A comment that will be displayed in this node's tool tip.");
    comment.mOnChanged = OnCommentChanged;

    const unsigned int serialType = 0;
    IntMetadata<unsigned int>& objectId =
        TypeSpecificMetadata(registry.RegisterProp("object_id", -1, kPropertyUInt, 0), serialType);
    static_cast<void>(Symbol("prop"));
    objectId.mGetter = GetObjectSerial;
    objectId.mReadOnly = true;

    metadata.mDescription = "Base component for all StateGraph nodes and transitions.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mAllowedResources.push_back(StateGraphResource::Id());
    metadata.mEditorRestrictions = 0;
}

// Reconstructed from eboot.elf at 0x808220.
bool StateBase::_OnResourcesLoaded() {
    _Fixup();
    return true;
}

// Reconstructed from eboot.elf at 0x808240.
void StateBase::Fixup() {
    _Fixup();
}

// Reconstructed from eboot.elf at 0x808250.
Entity* StateBase::_GetContextEntityPrototype() const {
    return _GetStateGraphResource()->GetContextEntityPrototype();
}

// Reconstructed from eboot.elf at 0x808330.
StateGraphResource* StateBase::_GetStateGraphResource() const {
    EntityResource* const resource = mObject->mEntity->mResource;
    if (resource == nullptr || !resource->IsA(StateGraphResource::Id())) {
        return nullptr;
    }
    return static_cast<StateGraphResource*>(resource);
}

// Reconstructed from eboot.elf at 0x808410.
std::function<Entity*(const TagArgs&)>& StateBase::_GetContextEntityTagFunc() {
    return sContextEntityTagFunc;
}

// Reconstructed from eboot.elf at 0x808420.
Symbol StateBase::GetName() const {
    return mObject->mName;
}

// Reconstructed from eboot.elf at 0x808430.
void StateBase::_SendDisplayTextUpdated() const {
    static Symbol appearanceUpdated;
    if (appearanceUpdated == Symbol()) {
        appearanceUpdated = Symbol("appearance_updated");
    }
    Message<1> msg(appearanceUpdated);
    DataNode object;
    object.mType = kDataGameObjectId;
    object.mValue.var = mObject->mId.mId;
    msg.mData->Node(2) = object;
    mObject->mEntity->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0x8085F0.
Symbol StateBase::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x808600.
Symbol StateBase::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x808610.
int StateBase::CurrentRev() const {
    return const_cast<StateBase*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x808630.
bool StateBase::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x808660.
Component* StateBase::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x808670. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateBase::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateBase*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateBase(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8087A0.
PropRegistry& StateBase::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8087B0.
ComMetaData& StateBase::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7092A0. The binary emits it in
// stategraph/StateNode.o.
void StateBase::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateBase", sMetaData);
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

// Reconstructed from eboot.elf at 0x6FEE00. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateBase::_Create() {
    return new StateBase();
}

// Reconstructed from eboot.elf at 0x6F6730. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateBase::Init() {
    InitComponentClass<StateBase>();
}
