#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/streams/MemStream.h"
#include "utl/text/Symbol.h"

class BinStream;

// Destroys the arrays among a dynamic component's properties, and sets the
// properties to their registry defaults. Emitted in entity/Utl.o. Not
// reconstructed.
void DynamicComDestroyArrays(Component& component);   // 0x114B50
void DynamicComSetDefaultVals(Component& component);  // 0x1149E0

// The registry a resource builds for the dynamic components of one class:
// the component class's base registry extended with the resource's own
// properties (a shader graph's exposed properties). The resource holds it as
// mDynamicPropRegistry (+208 in RndShaderGraphResource); the builder reaches
// the resource from it. Field names are not in the reference map; the type
// name is inferred from its use.
template <class T>
class DynamicPropRegistry {
public:
    // The registry of the class, rebuilt in the "metadata" heap when it is
    // stale (0x4F60C0 for RndShaderGraphResource). Not reconstructed.
    PropRegistry& Get(Symbol className);
    // Builds the registry: the class's base registry (found through
    // DynamicComBase::RegisterBasePropRegistryInit) and the resource's
    // properties, and hashes the property layout into mCrc (0x4F6190 for
    // RndShaderGraphResource). Not reconstructed.
    void _Build(DynamicPropRegistry& registry);

    // The component class the registry was built for.
    Symbol mClassName;
    unsigned char mReserved8[8];
    PropRegistry mRegistry;
    // The FNV-1a hash of the property layout, which the components store
    // as their "prop_crc".
    unsigned int mCrc;
    unsigned char mReserved180[12];
    // Set while the registry must be rebuilt.
    bool mDirty;
};

// The base of the components whose properties are extended by a resource's
// (DynamicCom<T>). The class is not in the reference map: the map's build
// keeps these methods in the DynamicCom template; this build compiles them
// once in entity/InstanceCom.o (0x1D0B30 to 0x1D17BF). Its vtable at
// 0x18EA760 has 47 slots: Component's 41 and six pure slots for the
// template. The dynamic properties live in mPropStorage; while the resource
// reloads they are kept in mSavedProps. Name not in the reference map.
class DynamicComBase : public Component {
public:
    // Inlined into the subclasses' constructors, for example RndMaterialCom's
    // at 0x4F3290. mSavedPropsRev is not initialized.
    DynamicComBase() : mPropStorage(), mPropCrc(0), mPropStorageLoaded(false), mSavedProps(false) {}
    // Slots 0-1: 0x1D1650, 0x1D1700.
    ~DynamicComBase() override;

    // Slot 11: imprints the base registry's properties, and the dynamic
    // ones when the resource is loaded. Not reconstructed.
    char* _ImprintProps(char* buffer, Component* imprint) override;  // 0x1D1540
    // Slots 12-13: the prop storage forms that include the dynamic
    // properties. Not reconstructed.
    void _SaveStorage(PropStorageArgs& args) override;  // 0x107290
    void _LoadStorage(PropStorageArgs& args) override;  // 0x107350
    // Slots 15-16: read and write the saved dynamic properties with their
    // revision. Not reconstructed.
    void _PostLoad(BinStream& stream) override;  // 0x1D0DD0
    void _Save(BinStream& stream) override;      // 0x1D0C20
    // Slot 20 at 0x1D1470: frees the dynamic properties unless they came
    // from the prop storage.
    void _PreDestroy(DestroyType type) override;
    // Slot 21 at 0x1D17B0: true.
    bool _LoadResources() override;
    // Slot 22 at 0x1D14E0: the resource's registry while it is loaded,
    // else the class's base registry.
    PropRegistry& _GetPropRegistry() override;
    // Slot 24 at 0x1D1530: the dynamic properties came from the prop storage
    // when it holds any.
    void _PostLoadStorage() override;
    // Slot 29 at 0x1D1630: keeps the dynamic properties and rebuilds them
    // for the loaded resource.
    bool _OnResourcesLoaded() override;

    // Slot 41: the resource registry's layout hash, which "prop_crc" keeps;
    // 0 while it must be rebuilt. Name not in the reference map.
    virtual unsigned int _GetDynamicPropCrc() = 0;
    // Slot 42: whether a resource is loaded and did not fail. Name not in
    // the reference map.
    virtual bool _HasDynamicProps() = 0;
    // Slot 43: shares the other component's resource; _ImprintProps calls it
    // on the imprint. Name not in the reference map.
    virtual void _CopyDynamicResource(DynamicComBase& other) = 0;
    // Slot 44: the class's own registry.
    virtual PropRegistry& _GetBasePropRegistry() = 0;
    // Slot 45: the resource's registry, or null while it is not built.
    // Name not in the reference map.
    virtual PropRegistry* _GetDynamicPropRegistry() = 0;
    // Slot 46: builds the resource's registry when needed and returns it.
    // Name not in the reference map.
    virtual PropRegistry& _BuildDynamicPropRegistry() = 0;

    // Out-of-line callers of slots 44, 45 and 42. Names not in the reference
    // map.
    PropRegistry& GetBasePropRegistry();     // 0x1D0DB0
    PropRegistry* GetDynamicPropRegistry();  // 0x1D0DC0
    bool HasDynamicProps();                  // 0x1D10B0

    // Frees the dynamic properties. Name not in the reference map.
    void _DestroyDynamicProps();  // 0x1D1050
    // Writes the dynamic properties to mSavedProps and frees them. Name not
    // in the reference map.
    void _StoreDynamicProps();  // 0x1D13D0
    // Sizes the dynamic properties for the resource's registry and fills
    // them from mSavedProps or the defaults; false when the stored layout no
    // longer matches. Not reconstructed.
    bool _InitializeDynamicProps();  // 0x1D10C0
    // Records the function that builds a class's base registry, by class
    // symbol, for DynamicPropRegistry::_Build. Not reconstructed. Name not
    // in the reference map.
    static void RegisterBasePropRegistryInit(Symbol className, void (*init)(PropRegistry&));  // 0x1D0B30

    // Field names are not in the reference map.
    // "prop_storage": the dynamic properties, laid out by the resource's
    // registry.
    PropArray<unsigned char> mPropStorage;
    // "prop_crc": the registry layout hash the properties were built for.
    unsigned int mPropCrc;
    // Set when the dynamic properties were read from the prop storage; they
    // are then not freed by _PreDestroy.
    bool mPropStorageLoaded;
    // The dynamic properties written by _StoreDynamicProps or read by
    // _PostLoad, and the property revision they were written with.
    MemStream mSavedProps;
    int mSavedPropsRev;
};

static_assert(offsetof(DynamicComBase, mPropStorage) == 24);
static_assert(offsetof(DynamicComBase, mPropCrc) == 64);
static_assert(offsetof(DynamicComBase, mPropStorageLoaded) == 68);
static_assert(offsetof(DynamicComBase, mSavedProps) == 72);
static_assert(offsetof(DynamicComBase, mSavedPropsRev) == 152);
static_assert(sizeof(DynamicComBase) == 160);

// The resource half of DynamicCom<T>, a polymorphic base at +160 with one
// slot. Its methods reach the component through a downcast. The map has
// _ShouldLoadUniqueResource on DynamicCom<T> and a DynamicCom<T>::RuntimeData
// copy constructor; neither fits a nested type here. Name not in the
// reference map.
template <class T>
class DynamicComResource {
public:
    // The only slot, 0x4F4EA0 for RndShaderGraphResource: false loads the
    // resource shared (GetOrLoad), true a private copy (LoadUnique).
    virtual bool _ShouldLoadUniqueResource() const {
        return false;
    }

    // The loaded resource.
    ResourcePtr<T> mResource;
    // A resource assigned in place of the file; it is used as it is.
    ResourcePtr<T> mInlineResource;
};

// A component whose properties are extended by those of a T resource it
// loads from mFile (the map's DynamicCom<T>, instantiated for
// RndShaderGraphResource in render/RndMaterialCom.o). The vtable for
// RndShaderGraphResource is at 0x1910CC8 (47 slots), with the
// DynamicComResource vtable at 0x1910E50.
template <class T>
class DynamicCom : public DynamicComBase, public DynamicComResource<T> {
public:
    // Inlined into the subclasses' constructors.
    DynamicCom() : mFile(), mComOrderCrc(-1), mReloadsSelf(true) {}
    // Slots 0-1: 0x4F4EB0 and 0x4F4EC0, around the body at 0x4F3610.
    ~DynamicCom() override;

    // Slot 14 at 0x4F4D30.
    void _ResetEntered() override;
    // Slot 29 at 0x4F3DD0.
    bool _OnResourcesLoaded() override;
    // Slot 33 at 0x4F4390: empty.
    void _Poll() override {}
    // Slot 38 at 0x4F4620: reloads a changed resource when "reloads_self"
    // is set.
    void _EditPoll() override;
    // Slots 39-40 at 0x4F4D50 and 0x4F4D60: post-polls, with an empty
    // post-poll.
    bool _HasPostPoll() const override {
        return true;
    }
    void _PostPoll() override {}
    // Slots 41-43, 45 and 46 at 0x4F4D70, 0x4F4DA0, 0x4F4DC0, 0x4F4E20 and
    // 0x4F4E70.
    unsigned int _GetDynamicPropCrc() override;
    bool _HasDynamicProps() override;
    void _CopyDynamicResource(DynamicComBase& other) override;
    PropRegistry* _GetDynamicPropRegistry() override;
    PropRegistry& _BuildDynamicPropRegistry() override;

    // Loads the resource from mFile (or takes mInlineResource) and rebuilds
    // the dynamic properties when it changed; false when they could not be
    // rebuilt. The map's _LoadResources(ObjPtr const&); this build compiles
    // it against the DynamicComResource base (0x4F5CB0).
    bool _LoadResource();
    // Registers mFile under the given property name, "reloads_self" and
    // "com_order_crc", and the base's "prop_crc" and "prop_storage". Not
    // reconstructed: the property metadata's attributes are written through
    // helpers that are not modelled.
    static void _Init(PropRegistry& registry, const char* fileProp, Symbol type, bool unique);  // 0x4F4F70

    // Field names are not in the reference map.
    // The resource file; the subclass names the property.
    ResourcePath mFile;
    // "com_order_crc"; -1 until set, and again when the entity resets.
    int mComOrderCrc;
    // "reloads_self": the edit poll reloads a changed resource.
    bool mReloadsSelf;
};

// Reconstructed from eboot.elf at 0x4F3610. An assertion's call remains when
// the dynamic properties were built here.
template <class T>
DynamicCom<T>::~DynamicCom() {
    if (!mPropStorageLoaded && mPropStorage.mSize != 0) {
        static_cast<void>(DynamicCom::_HasDynamicProps());
    }
}

// Reconstructed from eboot.elf at 0x4F4D30.
template <class T>
void DynamicCom<T>::_ResetEntered() {
    mComOrderCrc = -1;
}

// Reconstructed from eboot.elf at 0x4F3DD0.
template <class T>
bool DynamicCom<T>::_OnResourcesLoaded() {
    return _LoadResource();
}

// Reconstructed from eboot.elf at 0x4F4620.
template <class T>
void DynamicCom<T>::_EditPoll() {
    T* resource = this->mResource;
    if (mReloadsSelf && resource != nullptr && (resource->mFileChangedOnDisk || resource->NeedsReload())) {
        LoadResources(false);
    }
}

// Reconstructed from eboot.elf at 0x4F4D70. The class name is read and
// ignored.
template <class T>
unsigned int DynamicCom<T>::_GetDynamicPropCrc() {
    const DynamicPropRegistry<T>& registry = this->mResource->mDynamicPropRegistry;
    static_cast<void>(GetClassName());
    return registry.mDirty ? 0 : registry.mCrc;
}

// Reconstructed from eboot.elf at 0x4F4DA0.
template <class T>
bool DynamicCom<T>::_HasDynamicProps() {
    T* resource = this->mResource;
    return resource != nullptr && !resource->Fail();
}

// Reconstructed from eboot.elf at 0x4F4DC0.
template <class T>
void DynamicCom<T>::_CopyDynamicResource(DynamicComBase& other) {
    this->mResource = static_cast<DynamicCom&>(other).mResource.Get();
}

// Reconstructed from eboot.elf at 0x4F4E20.
template <class T>
PropRegistry* DynamicCom<T>::_GetDynamicPropRegistry() {
    if (!HasDynamicProps()) {
        return nullptr;
    }
    DynamicPropRegistry<T>& registry = this->mResource->mDynamicPropRegistry;
    static_cast<void>(GetClassName());
    return registry.mDirty ? nullptr : &registry.mRegistry;
}

// Reconstructed from eboot.elf at 0x4F4E70.
template <class T>
PropRegistry& DynamicCom<T>::_BuildDynamicPropRegistry() {
    return this->mResource->mDynamicPropRegistry.Get(GetClassName());
}

// Reconstructed from eboot.elf at 0x4F5CB0. A changed resource is reported
// against the component when it is missing or failed, and the dynamic
// properties are stored and rebuilt for it.
template <class T>
bool DynamicCom<T>::_LoadResource() {
    if (mFile == ResourcePath() && this->mInlineResource == nullptr) {
        _DestroyDynamicProps();
        this->mResource = nullptr;
        return true;
    }
    ResourcePtr<T> resource;
    if (this->mInlineResource != nullptr) {
        resource = this->mInlineResource.Get();
    } else if (this->_ShouldLoadUniqueResource()) {
        resource = static_cast<T*>(Resource::LoadUnique(mFile, T::Id(), false).Get());
    } else {
        resource = Resource::GetOrLoad<T>(mFile, false);
    }
    if (!(mFile == ResourcePath()) && (resource == nullptr || resource->Fail())) {
        MakeErrorName();
    }
    if (resource.Get() == this->mResource.Get()) {
        return true;
    }
    _StoreDynamicProps();
    this->mResource = resource.Get();
    return _InitializeDynamicProps();
}
