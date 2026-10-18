// entity/InstanceCom.o (0x118DC0 to 0x122DA0). The interface property
// copies (0x11A1F0, 0x11A360, 0x11C220 and 0x11C420) are not
// reconstructed.
#include "entity/core/InstanceCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"
#include "utl/streams/BinStream.h"
#include "utl/streams/MemStream.h"
#include "utl/threading/PollDep.h"
#include "utl/threading/PollMgr.h"

// The object's statics, in the order of its static initializer (0x122D30).
// The int it first sets (0x19E3270) comes from a shared header and is not
// modelled.
Symbol InstanceCom::sId("Instance");
Symbol InstanceCom::sClassName("Instance");
PropRegistry InstanceCom::sPropRegistry;
ComMetaData InstanceCom::sMetaData;

namespace {

// The EntityHeaderCom of the entity's root, which must exist: the search
// is unbounded. Inlined throughout. Name not in the reference map.
EntityHeaderCom* RootHeader(const Entity* entity) {
    return entity->GetRoot()->GetExistingCom<EntityHeaderCom>();
}

// Gives the object's TransCom the local transform of the child's root,
// which drives it, and marks it dirty. Inlined into _InstanceChild, _Enter,
// _EditEnter and _PollChild. Name not in the reference map.
void TakeChildTransform(TransCom* trans, const Entity* child) {
    const TransCom* const source = child->GetRoot()->GetExistingCom<TransCom>();
    trans->mLocalXfm = source->mLocalXfm;
    trans->mDirtyFlags |= 1;
}

// Whether the thread enters and polls entities at once (ThreadPollContext).
// Inlined. Name not in the reference map.
bool EntersImmediately() {
    const ThreadPollContext& context = gEntityThreadState;
    return context.mPollMgr != nullptr && context.mEnterImmediately;
}

// The property handlers _Init installs (0x11B8A0, 0x11B8C0). Names not in
// the reference map.
void OnActiveChanged(const PropAccessorArgs& args);
void OnPollEnabledChanged(const PropAccessorArgs& args);

// Whether the instanced entity's root does not copy on instance, which
// makes "active" and "poll_enabled" read-only; false while the resource is
// missing or failed. The body of the tag functions at 0x11BC10 and
// 0x11BD90. Name not in the reference map.
bool IsSharedInstance(const TagArgs& args) {
    const InstanceCom& instance = static_cast<const InstanceCom&>(args.mComponent);
    const TransEntityResource* const resource = instance.mResource.Get();
    if (resource == nullptr || resource->Fail()) {
        return false;
    }
    return !RootHeader(resource->mEntity)->CopyOnInstance();
}

}  // namespace

// Reconstructed from eboot.elf at 0x118DC0. The heap index is looked up
// once.
void InstanceCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata, const char* fileProp) {
    metadata.InitSuperclass("InstanceCom", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata, fileProp);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x118E60. The file property's metadata
// is looked up by its path, once to no effect and once to set its resource
// class. Each registration builds a "prop" symbol it does not use.
void InstanceCom::_Init(PropRegistry& registry, ComMetaData& metadata, const char* fileProp) {
    metadata.mDescription = "Used to load an entity resource.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mAllowedResources.push_back(Symbol("RndDrawableEntityResource"));
    metadata.mAliases.push_back(Symbol("InstanceCom"));
    metadata.mInterface = sClassName;
    metadata.mLightweight = true;
    _InitPropRegistry(registry, fileProp);

    InstanceCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    } else {
        static_cast<void>(sPropRegistry.FindProp(PropPath(Symbol("file")), nullptr));
        const ResourcePath fileType;
        ResourcePathMetadata& file = TypeSpecificMetadata(
            *const_cast<PropInfo*>(sPropRegistry.FindProp(PropPath(Symbol("file")), nullptr)), fileType);
        static_cast<void>(Symbol("prop"));
        file.mResourceId = Symbol("RndEntityResource");

        BoolMetadata& active = TypeSpecificMetadata(
            sPropRegistry.RegisterProp("active", offsetof(InstanceCom, mActive), kPropertyBool, 1),
            prototype.mActive);
        active.mDefault = prototype.mActive;
        static_cast<void>(Symbol("prop"));
        active.mHelp = String("If unchecked, my child entity will stop polling and stop drawing");
        active.mOnChanged = OnActiveChanged;
        active.mReadOnly._Set([](const TagArgs& args) { return IsSharedInstance(args); });

        BoolMetadata& pollEnabled = TypeSpecificMetadata(
            sPropRegistry.RegisterProp("poll_enabled", offsetof(InstanceCom, mPollEnabled), kPropertyBool, 1),
            prototype.mPollEnabled);
        pollEnabled.mDefault = prototype.mPollEnabled;
        static_cast<void>(Symbol("prop"));
        pollEnabled.mHelp = String("If unchecked, my child entity will stop polling (but keep drawing)");
        pollEnabled.mOnChanged = OnPollEnabledChanged;
        pollEnabled.mReadOnly._Set([](const TagArgs& args) { return IsSharedInstance(args); });
    }
    // At 0x11BF10: records the revision the properties were saved under.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            static_cast<InstanceCom*>(args.mComponent)->mRuntime.mLoadedRev = args.mSavedRev;
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    }
}

// Reconstructed from eboot.elf at 0x1196D0.
void InstanceCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    _Init(registry, metadata, "file");
}

// Reconstructed from eboot.elf at 0x1196E0.
InstanceCom::RuntimeData::RuntimeData()
    : mEntity(nullptr),
      mCopyBuffer(nullptr),
      mStoredProps(nullptr),
      mStoredPropsRev(0),
      mLoadedRev(1),
      mCanBeDrivenByChild(true),
      mParentPollDepAdded(false),
      mNeedsInstance(false) {}

// Reconstructed from eboot.elf at 0x119710. The deleting destructor is at
// 0x119790.
InstanceCom::~InstanceCom() {
    delete mRuntime.mStoredProps;
}

// Reconstructed from eboot.elf at 0x119810.
void InstanceCom::_Save(BinStream& stream) {
    Component::_Save(stream);
    const int rev = 2;
    stream.WriteEndian(&rev, sizeof(rev));
}

// Reconstructed from eboot.elf at 0x119870. Revision 1 keeps the bytes of
// the properties in the stream's byte order, with the entity revision they
// were read under. A revision of 0 or less only reads the class id.
void InstanceCom::_PostLoad(BinStream& stream) {
    delete mRuntime.mStoredProps;
    mRuntime.mStoredProps = nullptr;
    int rev = 0;
    stream.ReadEndian(&rev, sizeof(rev));
    if (rev <= 0) {
        static_cast<void>(GetId());
        return;
    }
    if (rev != 1) {
        return;
    }
    unsigned int size = 0;
    stream.ReadEndian(&size, sizeof(size));
    mRuntime.mStoredProps = new MemStream(false);
    mRuntime.mStoredProps->SetLittleEndian(stream.mLittleEndian);
    mRuntime.mStoredProps->Resize(size);
    if (size != 0) {
        stream.Read(mRuntime.mStoredProps->mBuffer.begin(), size);
    }
    mRuntime.mStoredPropsRev = gEntityRev;
}

// Reconstructed from eboot.elf at 0x119990.
void InstanceCom::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    if (mRuntime.mEntity != nullptr && RootHeader(mRuntime.mEntity)->mDrivesParent) {
        precedes.push_back(TransCom::sClassName);
    } else {
        follows.push_back(TransCom::sClassName);
    }
}

// Reconstructed from eboot.elf at 0x119B40.
bool InstanceCom::IsDrivenByChild() const {
    return mRuntime.mEntity != nullptr && RootHeader(mRuntime.mEntity)->mDrivesParent;
}

// Reconstructed from eboot.elf at 0x119B90. The shared child's
// _ReenterSharedChildren is asked and the answer ignored.
void InstanceCom::_PreDestroy(DestroyType type) {
    _PreDestroyInterfaces(this, type);
    if (_GetCopyChild()) {
        if (mRuntime.mEntity != nullptr) {
            mRuntime.mEntity->_Destroy();
            mRuntime.mEntity = nullptr;
        }
        return;
    }
    if (mRuntime.mEntity == nullptr) {
        return;
    }
    static_cast<void>(_ReenterSharedChildren());
    if (mRuntime.mEntity->mParentObject == mObject) {
        mRuntime.mEntity->ClearParent();
    }
    mRuntime.mEntity = nullptr;
    mRuntime.mNeedsInstance = true;
}

// Reconstructed from eboot.elf at 0x119C90.
bool InstanceCom::_GetCopyChild() const {
    const TransEntityResource* const resource = mResource;
    if (resource == nullptr || resource->Fail()) {
        return true;
    }
    return RootHeader(resource->mEntity)->CopyOnInstance();
}

// Reconstructed from eboot.elf at 0x119D20.
bool InstanceCom::_OnResourcesLoaded() {
    const bool reentered = _LoadChildResource();
    _InstanceChild();
    return reentered;
}

// Reconstructed from eboot.elf at 0x119D50. A resource loaded from a file
// that is not transient is reported and its entity destroyed. The CRC of
// the component order is recorded the first time; afterwards a change
// keeps the component from re-entering.
bool InstanceCom::_LoadChildResource() {
    mRuntime.mCanBeDrivenByChild = _CanBeDrivenByChild();
    if (mRuntime.mEntity != nullptr) {
        if (_GetCopyChild()) {
            mRuntime.mEntity->_Destroy();
        }
        mRuntime.mEntity = nullptr;
        mRuntime.mNeedsInstance = true;
    }
    if (!_LoadResource()) {
        return false;
    }
    TransEntityResource* const resource = mResource;
    if (resource != nullptr && !resource->Fail() && !resource->IsTransient()) {
        resource->mEntity->MakeErrorName();
        resource->DestroyEntity();
    }
    const int previous = mComOrderCrc;
    const unsigned int crc = _GetOrderDepsCRC();
    if (previous == -1) {
        mComOrderCrc = static_cast<int>(crc);
        return true;
    }
    return crc == static_cast<unsigned int>(mComOrderCrc);
}

// Reconstructed from eboot.elf at 0x119E90. A shared child already entered
// in the game mode by an editor resource keeps its parent.
void InstanceCom::_InstanceChild() {
    void* const buffer = mRuntime.mCopyBuffer;
    mRuntime.mCopyBuffer = nullptr;
    TransEntityResource* const resource = mResource;
    if (resource != nullptr && !resource->Fail()) {
        if (_GetCopyChild()) {
            mRuntime.mEntity = resource->mEntity->Copy(buffer, true, mObject, GetId());
            mRuntime.mEntity->SetActive(mActive);
            mRuntime.mEntity->SetPollEnabled(mPollEnabled);
            mRuntime.mNeedsInstance = true;
        } else {
            Entity* const entity = resource->mEntity;
            if (entity != mRuntime.mEntity) {
                mRuntime.mEntity = entity;
                mRuntime.mNeedsInstance = true;
            }
            if (!entity->mResource->_IsEditorEntity() || (entity->mFlags & 0x16) != 0x12) {
                entity->SetParent(mObject, GetId());
            }
        }
    }
    Entity* const child = mRuntime.mEntity;
    if (child != nullptr) {
        TransCom* const trans = mObject->GetCom<TransCom>();
        if (trans != nullptr && RootHeader(child)->mDrivesParent) {
            TakeChildTransform(trans, child);
        }
    }
    _SyncInterfaces(this);
}

// Reconstructed from eboot.elf at 0x11A0D0. The copy's size is truncated to
// an int before it is rounded.
unsigned long InstanceCom::_ChildSize() const {
    if (!_GetCopyChild()) {
        return 0;
    }
    const TransEntityResource* const resource = mResource;
    if (resource == nullptr || resource->Fail()) {
        return 0;
    }
    const long size = static_cast<int>(reinterpret_cast<unsigned long>(resource->mEntity->_Imprint(nullptr)));
    return static_cast<unsigned long>(size + 7) & ~7UL;
}

// Reconstructed from eboot.elf at 0x11A190.
TransEntityResource* InstanceCom::GetInterfaceResource() {
    TransEntityResource* const resource = mResource;
    if (resource == nullptr || resource->Fail()) {
        return nullptr;
    }
    return resource;
}

// Reconstructed from eboot.elf at 0x11A620.
void InstanceCom::_PreLoadResources() {
    if (mRuntime.mEntity == nullptr || !EntersImmediately()) {
        return;
    }
    mRuntime.mParentPollDepAdded = false;
    mRuntime.mEntity->_PreLoadResources();
}

// Reconstructed from eboot.elf at 0x11A6E0. A child that drives the object
// makes the object's entity poll after it and itself poll after the
// topmost entity; one whose parent is the topmost entity is reported, and
// polls after the dependency instead.
void InstanceCom::_PostLoadResources(void* context, bool loaded) {
    if (mRuntime.mEntity == nullptr || !EntersImmediately()) {
        return;
    }
    mRuntime.mParentPollDepAdded = true;
    Entity* const child = mRuntime.mEntity;
    Entity* const parent = mObject->mEntity;
    bool pollAfterDep = true;
    if (child != nullptr && RootHeader(child)->mDrivesParent) {
        Entity* const top = parent->GetTopEntity();
        if (top != parent) {
            parent->AddPollDep(child);
            child->AddPollDep(top);
            pollAfterDep = false;
        } else {
            child->MakeErrorName();
            parent->MakeErrorName();
        }
    } else {
        pollAfterDep = _HasParentPollDep();
    }
    if (pollAfterDep) {
        PollDepBase* const dep = static_cast<PollDepBase*>(context);
        child->AddPollDep(dep != nullptr ? dep : parent);
    }
    if (loaded) {
        child->mResource->_PollImmediately(child);
    }
}

// Reconstructed from eboot.elf at 0x11A8B0.
bool InstanceCom::_AreResourcesReady(int pass) {
    return mRuntime.mEntity == nullptr || mRuntime.mEntity->AreResourcesReady(pass);
}

// Reconstructed from eboot.elf at 0x11A8D0. A shared child that does not
// re-enter and is already entered stays as it is.
void InstanceCom::_Enter() {
    Entity* const child = mRuntime.mEntity;
    if (child == nullptr) {
        return;
    }
    if (!_GetCopyChild()) {
        child->SetParent(mObject, GetId());
        if (!_ReenterSharedChildren() && (child->mFlags & 0x10) != 0 && (child->mFlags & 6) != 0) {
            return;
        }
    }
    if (EntersImmediately()) {
        child->ClearAllDeps();
    }
    if (mRuntime.mCanBeDrivenByChild) {
        TransCom* const trans = mObject->GetCom<TransCom>();
        if (trans != nullptr && mRuntime.mEntity != nullptr && RootHeader(mRuntime.mEntity)->mDrivesParent) {
            TakeChildTransform(trans, mRuntime.mEntity);
        }
    }
    mRuntime.mEntity->mResource->EnterEntity(mRuntime.mEntity, 0);
}

// Reconstructed from eboot.elf at 0x11ABE0. The topmost entity is taken as
// the object entity's root's own entity, which is that entity itself, so
// the binary always takes the second removal for a driving child.
void InstanceCom::_Exit(DestroyType type) {
    Entity* const child = mRuntime.mEntity;
    if (child == nullptr) {
        return;
    }
    if ((type == kDestroyObject || type == kDestroyComponent) && (child->mFlags & 0x16) == 0x12 &&
        mRuntime.mParentPollDepAdded) {
        Entity* const parent = mObject->mEntity;
        if (RootHeader(child)->mDrivesParent) {
            Entity* const top = parent->GetRoot()->mEntity;
            if (top != parent) {
                parent->RemovePollDep(child);
                child->RemovePollDep(top);
            } else {
                child->TryRemovePollDep(parent);
            }
        } else if (_HasParentPollDep()) {
            child->TryRemovePollDep(parent);
        }
        mRuntime.mParentPollDepAdded = false;
    }
    if (_GetCopyChild() || _ReenterSharedChildren()) {
        mRuntime.mEntity->mResource->ExitEntity(mRuntime.mEntity, 0);
    }
}

// Reconstructed from eboot.elf at 0x11AD80.
void InstanceCom::_EditEnter() {
    Entity* const child = mRuntime.mEntity;
    if (child == nullptr) {
        return;
    }
    if (mRuntime.mCanBeDrivenByChild) {
        TransCom* const trans = mObject->GetCom<TransCom>();
        if (trans != nullptr && RootHeader(child)->mDrivesParent) {
            TakeChildTransform(trans, child);
        }
    }
    child->mResource->EnterEntity(child, 1);
}

// Reconstructed from eboot.elf at 0x11AE60.
void InstanceCom::_EditExit(DestroyType type) {
    static_cast<void>(type);
    Entity* const child = mRuntime.mEntity;
    if (child != nullptr) {
        child->mResource->ExitEntity(child, 0);
    }
}

// Reconstructed from eboot.elf at 0x11AE80.
void InstanceCom::_Poll() {
    _PollChild();
}

// Reconstructed from eboot.elf at 0x11AE90.
void InstanceCom::_PollChild() {
    Entity* const child = mRuntime.mEntity;
    if (child != nullptr && (child->mFlags & 0x100) != 0) {
        TransCom* const trans = mRuntime.mCanBeDrivenByChild ? mObject->GetCom<TransCom>() : nullptr;
        const bool drives = RootHeader(child)->mDrivesParent;
        const bool immediately = EntersImmediately();
        if (!immediately) {
            PollChildEntity(mObject->mEntity, child);
        }
        if (trans != nullptr && drives) {
            TakeChildTransform(trans, mRuntime.mEntity);
        }
        if (!drives && immediately && (mRuntime.mEntity->mFlags & 0x80) != 0 && mObject->mAllComsFlagged) {
            mObject->mEarlyFreeDep = mRuntime.mEntity;
        }
    }
    mRuntime.mNeedsInstance = false;
}

// Reconstructed from eboot.elf at 0x11B090.
void InstanceCom::_OnDeactivate() {
    if (mRuntime.mEntity != nullptr) {
        mRuntime.mEntity->SetActive(false);
    }
}

// Reconstructed from eboot.elf at 0x11B0B0.
void InstanceCom::_OnActivate() {
    if (mRuntime.mEntity != nullptr) {
        mRuntime.mEntity->SetActive(mActive);
    }
}

// Reconstructed from eboot.elf at 0x11B0D0. Where DynamicCom<T>'s edit poll
// reloads a changed resource, the binary has a trap instruction.
void InstanceCom::_EditPoll() {
    TransEntityResource* const resource = mResource;
    if (mReloadsSelf && resource != nullptr && (resource->mFileChangedOnDisk || resource->NeedsReload())) {
        __builtin_trap();
    }
    _PollChild();
}

// Reconstructed from eboot.elf at 0x11B110.
DataNode InstanceCom::_OnUsingChild(DataArray* msg) {
    Entity* const defaultEntity = gDataThread.mDefaultEntity;
    DataSetDefaultEntity(mRuntime.mEntity);
    GameObject* const thisObject = gDataThread.mThisObject;
    gDataThread.mThisObject = mRuntime.mEntity->GetRoot();
    DataNode result = DataExecuteBlock(msg, 2);
    gDataThread.mThisObject = thisObject;
    DataSetDefaultEntity(defaultEntity);
    return result;
}

// Reconstructed from eboot.elf at 0x11B330. "set_active" expects a child.
DataNode InstanceCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol sSetActive;
    if (sSetActive == Symbol()) {
        sSetActive = Symbol("set_active");
    }
    if (type == sSetActive) {
        mRuntime.mEntity->SetActive(msg->Int(2) != 0);
        return DataNode(0);
    }
    static Symbol sGetActive;
    if (sGetActive == Symbol()) {
        sGetActive = Symbol("get_active");
    }
    if (type == sGetActive) {
        const bool active = mRuntime.mEntity != nullptr && (mRuntime.mEntity->mFlags & 0x100) != 0;
        return DataNode(static_cast<int>(active));
    }
    static Symbol sGetEntity;
    if (sGetEntity == Symbol()) {
        sGetEntity = Symbol("get_entity");
    }
    if (type == sGetEntity) {
        DataNode entity;
        entity.mValue.object = mRuntime.mEntity;
        entity.mType = kDataObject;
        return entity;
    }
    static Symbol sUsingChild;
    if (sUsingChild == Symbol()) {
        sUsingChild = Symbol("using_child");
    }
    if (type == sUsingChild) {
        return _OnUsingChild(msg);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x11B5B0.
Symbol InstanceCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x11B5C0.
Symbol InstanceCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x11B5D0.
int InstanceCom::CurrentRev() const {
    return const_cast<InstanceCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x11B5F0.
bool InstanceCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x11B620.
Component* InstanceCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x11B630. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* InstanceCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<InstanceCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) InstanceCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x11B7D0.
PropRegistry& InstanceCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x11B7E0.
ComMetaData& InstanceCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x11B810.
bool InstanceCom::_ReenterSharedChildren() const {
    return true;
}

// Reconstructed from eboot.elf at 0x11B820.
bool InstanceCom::_HasParentPollDep() const {
    return true;
}

// Reconstructed from eboot.elf at 0x11B830.
bool InstanceCom::_CanBeDrivenByChild() const {
    return true;
}

// Reconstructed from eboot.elf at 0x11B840.
Entity* InstanceCom::GetInterfaceEntity() {
    return mRuntime.mEntity;
}

// Reconstructed from eboot.elf at 0x11B850.
bool InstanceCom::_ForwardsInterfaceEdits() {
    return true;
}

namespace {

// Reconstructed from eboot.elf at 0x11B8A0.
void OnActiveChanged(const PropAccessorArgs& args) {
    InstanceCom* const instance = static_cast<InstanceCom*>(args.mComponent);
    if (instance->mRuntime.mEntity != nullptr) {
        instance->mRuntime.mEntity->SetActive(instance->mActive);
    }
}

// Reconstructed from eboot.elf at 0x11B8C0.
void OnPollEnabledChanged(const PropAccessorArgs& args) {
    InstanceCom* const instance = static_cast<InstanceCom*>(args.mComponent);
    if (instance->mRuntime.mEntity != nullptr) {
        instance->mRuntime.mEntity->SetPollEnabled(instance->mPollEnabled);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x11B8E0. The file property is only
// marked as the instance's file; "reloads_self" and "com_order_crc" are
// read-only debug properties.
void InstanceCom::_InitPropRegistry(PropRegistry& registry, const char* fileProp) {
    InstanceCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    registry.RegisterProp(fileProp, offsetof(InstanceCom, mFile), kPropertyResourcePath, 8)
        .mMetadata->mInstanceFile = true;

    BoolMetadata& reloadsSelf = TypeSpecificMetadata(
        registry.RegisterProp("reloads_self", offsetof(InstanceCom, mReloadsSelf), kPropertyBool, 1),
        prototype.mReloadsSelf);
    reloadsSelf.mDefault = prototype.mReloadsSelf;
    static_cast<void>(Symbol("prop"));
    reloadsSelf.mReadOnly = true;
    reloadsSelf.mDebug = true;

    IntMetadata<int>& comOrderCrc = TypeSpecificMetadata(
        registry.RegisterProp("com_order_crc", offsetof(InstanceCom, mComOrderCrc), kPropertyInt, 4),
        prototype.mComOrderCrc);
    comOrderCrc.mDefault = prototype.mComOrderCrc;
    static_cast<void>(Symbol("prop"));
    comOrderCrc.mDefault = -1;
    comOrderCrc.mDebug = true;
    comOrderCrc.mReadOnly = true;
}

// Reconstructed from eboot.elf at 0x12B470, emitted with InterfaceSource's
// members.
PropPath InterfaceSource::_MapInterfacePath(const PropPath& path) {
    return path;
}
