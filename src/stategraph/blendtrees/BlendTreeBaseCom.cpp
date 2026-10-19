// stategraph/BlendTreeBaseCom.o (0x7FB540 to 0x7FC51F). The class's
// registration and factory are emitted in stategraph/StateGraph.o, and the
// superclass helper in stategraph/ClipBlendTreeCom.o.
#include "stategraph/blendtrees/BlendTreeBaseCom.h"

#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "os/memory/MemMgr.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/resources/StateGraphResource.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"

namespace {

// The id of no object, at 0x1ABE348, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

}  // namespace

// The object's statics, in the order of its static initializer (0x7FC460).
Symbol BlendTreeBaseCom::sId("BlendTreeBase");
Symbol BlendTreeBaseCom::sClassName("BlendTreeBase");
PropRegistry BlendTreeBaseCom::sPropRegistry;
ComMetaData BlendTreeBaseCom::sMetaData;

// Reconstructed from eboot.elf at 0x7FB540. The trees may only be created in
// a state graph.
void BlendTreeBaseCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    static_cast<void>(registry);
    metadata.mDescription =
        "Base class for 'BlendTree-like' objects that manage a set of animations or other data that can be "
        "blended between or crossfaded based on some criteria.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(StateGraphResource::Id());
    metadata.mInterface = sClassName;
}

// Reconstructed from eboot.elf at 0x7FB6A0. Destroying the whole entity
// leaves the subtrees to it.
void BlendTreeBaseCom::_PreDestroy(DestroyType type) {
    if (type == kDestroyRoot) {
        return;
    }
    eastl::vector<GameObjectId> children;
    GetChildObjects(children, false);
    Entity* const entity = mObject->mEntity;
    for (const GameObjectId& child : children) {
        GameObjectId id = child;
        SafeDestroyObject(entity, id);
    }
}

// Reconstructed from eboot.elf at 0x7FB780.
void BlendTreeBaseCom::GetChildObjects(eastl::vector<GameObjectId>& children, bool recursive) const {
    const unsigned long count = GetNumChildren();
    for (unsigned long index = 0; index < count; ++index) {
        if (!IsChildBlendTree(index)) {
            continue;
        }
        const BlendTreeBaseCom* const child = GetChildBlendTree(index);
        if (child == nullptr) {
            continue;
        }
        children.push_back(child->mObject->mId);
        if (recursive) {
            child->GetChildObjects(children, true);
        }
    }
}

// Reconstructed from eboot.elf at 0x7FB9E0.
Entity* BlendTreeBaseCom::_GetContextEntityPrototype() const {
    EntityResource* const resource = mObject->mEntity->mResource;
    StateGraphResource* graph = nullptr;
    if (resource != nullptr && resource->IsA(StateGraphResource::Id())) {
        graph = static_cast<StateGraphResource*>(resource);
    }
    return graph->GetContextEntityPrototype();
}

// Reconstructed from eboot.elf at 0x7FBAD0.
int BlendTreeBaseCom::GetChildType(unsigned long index) const {
    static_cast<void>(GetNumChildren());
    return _GetChildType(index);
}

// Reconstructed from eboot.elf at 0x7FBB00.
bool BlendTreeBaseCom::IsChildBlendTree(unsigned long index) const {
    return GetChildType(index) == kChildBlendTree;
}

// Reconstructed from eboot.elf at 0x7FBB30. No caller in this build.
bool BlendTreeBaseCom::IsChildClip(unsigned long index) const {
    return GetChildType(index) == kChildClip;
}

// Reconstructed from eboot.elf at 0x7FBB60.
GameObjectId BlendTreeBaseCom::GetChildObject(unsigned long index) const {
    return _GetChildObject(index);
}

// Reconstructed from eboot.elf at 0x7FBB70.
int BlendTreeBaseCom::GetTimeUnits() const {
    return _GetTimeUnits();
}

// Reconstructed from eboot.elf at 0x7FBB80.
void BlendTreeBaseCom::Refresh() {
    _Refresh();
}

// Reconstructed from eboot.elf at 0x7FBB90. The child's type is read for an
// assertion that is compiled out.
BlendTreeBaseCom* BlendTreeBaseCom::GetChildBlendTree(unsigned long index) const {
    static_cast<void>(GetChildType(index));
    Entity* const entity = mObject->mEntity;
    const GameObjectId id = GetChildObject(index);
    if (id.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* const object = entity->GetObject(id);
    if (object == nullptr) {
        return nullptr;
    }
    return object->GetBaseCom<BlendTreeBaseCom>();
}

// Reconstructed from eboot.elf at 0x7FBC70.
void BlendTreeBaseCom::AddChild(int type) {
    _AddChild(type);
    _SendModified(true);
}

// Reconstructed from eboot.elf at 0x7FBCA0.
void BlendTreeBaseCom::_SendModified(bool added) const {
    static Symbol blendTreeModified;
    if (blendTreeModified == Symbol()) {
        blendTreeModified = Symbol("blend_tree_modified");
    }
    Message<2> msg(blendTreeModified);
    DataNode object;
    object.mType = kDataGameObjectId;
    object.mValue.var = mObject->mId.mId;
    msg.mData->Node(2) = object;
    msg.mData->Node(3) = DataNode(static_cast<int>(added));
    mObject->mEntity->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0x7FBF10.
void BlendTreeBaseCom::RemoveChild(unsigned long index) {
    _RemoveChild(index);
    _SendModified(false);
}

// Reconstructed from eboot.elf at 0x7FBF40. No caller in this build.
const PropPath& BlendTreeBaseCom::GetChildObjectPath() const {
    return _GetChildObjectPath();
}

// Reconstructed from eboot.elf at 0x7FBF50.
bool BlendTreeBaseCom::ContainsObject(const GameObject* object) const {
    if (mObject == object) {
        return true;
    }
    const unsigned long count = GetNumChildren();
    for (unsigned long index = 0; index < count; ++index) {
        if (!IsChildBlendTree(index)) {
            continue;
        }
        const BlendTreeBaseCom* const child = GetChildBlendTree(index);
        if (child != nullptr && child->ContainsObject(object)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7FC0C0.
BlendTreeBaseCom::Instance* BlendTreeBaseCom::CreateInstance(Entity* entity, bool editMode) {
    return _CreateInstance(entity, editMode);
}

// Reconstructed from eboot.elf at 0x7FC0D0.
float BlendTreeBaseCom::GetScale(int units, float rate) const {
    return _GetScale(units, rate);
}

// Reconstructed from eboot.elf at 0x7FC0E0.
bool BlendTreeBaseCom::IsRootBlendTree() const {
    return mObject->FindCom(StateNode::sId) != nullptr;
}

// Reconstructed from eboot.elf at 0x7FC180.
Symbol BlendTreeBaseCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7FC190.
Symbol BlendTreeBaseCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7FC1A0.
int BlendTreeBaseCom::CurrentRev() const {
    return const_cast<BlendTreeBaseCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7FC1C0.
bool BlendTreeBaseCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7FC1F0.
Component* BlendTreeBaseCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7FC200. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* BlendTreeBaseCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<BlendTreeBaseCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) BlendTreeBaseCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7FC320.
PropRegistry& BlendTreeBaseCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7FC330.
ComMetaData& BlendTreeBaseCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7FC340.
unsigned int BlendTreeBaseCom::_GetNumChildren() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x7FC350.
int BlendTreeBaseCom::_GetChildType(unsigned long index) const {
    static_cast<void>(index);
    return -1;
}

// Reconstructed from eboot.elf at 0x7FC360.
GameObjectId BlendTreeBaseCom::_GetChildObject(unsigned long index) const {
    static_cast<void>(index);
    return gNullObjectId;
}

// Reconstructed from eboot.elf at 0x7FC370.
int BlendTreeBaseCom::_GetTimeUnits() const {
    return kTimeUnitsSeconds;
}

// Reconstructed from eboot.elf at 0x7FC3B0. The path is a guarded local
// static (0x1ABE5B8, guard 0x1ABE680).
const PropPath& BlendTreeBaseCom::_GetChildObjectPath() const {
    static PropPath path;
    return path;
}

// Reconstructed from eboot.elf at 0x7FC440.
BlendTreeBaseCom::Instance* BlendTreeBaseCom::_CreateInstance(Entity* entity, bool editMode) {
    static_cast<void>(entity);
    static_cast<void>(editMode);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x7FC450.
float BlendTreeBaseCom::_GetScale(int units, float rate) const {
    static_cast<void>(units);
    static_cast<void>(rate);
    return 0.0f;
}

// Reconstructed from eboot.elf at 0x7FE760. The binary emits the helper in
// stategraph/ClipBlendTreeCom.o, its first user.
void BlendTreeBaseCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("BlendTree", sMetaData);
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

// Reconstructed from eboot.elf at 0x6FEF80. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* BlendTreeBaseCom::_Create() {
    return new BlendTreeBaseCom();
}

// Reconstructed from eboot.elf at 0x6F7860. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void BlendTreeBaseCom::Init() {
    InitComponentClass<BlendTreeBaseCom>(false, true);
}
