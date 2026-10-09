// DynamicComBase, compiled in entity/InstanceCom.o (0x1D0B30 to 0x1D17BF).
// The template members of DynamicCom<T> are in DynamicCom.h.
#include "entity/core/DynamicCom.h"

#include "entity/core/EntityConstants.h"
#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x1D0DB0.
PropRegistry& DynamicComBase::GetBasePropRegistry() {
    return _GetBasePropRegistry();
}

// Reconstructed from eboot.elf at 0x1D0DC0.
PropRegistry* DynamicComBase::GetDynamicPropRegistry() {
    return _GetDynamicPropRegistry();
}

// Reconstructed from eboot.elf at 0x1D1050.
void DynamicComBase::_DestroyDynamicProps() {
    if (!_HasDynamicProps()) {
        return;
    }
    DynamicComDestroyArrays(*this);
    mPropCrc = 0;
    mPropStorage.Resize(0);
    if (mPropStorage.mStatic == 0) {
        MemFree(mPropStorage.mData);
    }
    mPropStorage.mData = nullptr;
    mPropStorage.mCapacity = 0;
    mPropStorageLoaded = false;
}

// Reconstructed from eboot.elf at 0x1D10B0.
bool DynamicComBase::HasDynamicProps() {
    return _HasDynamicProps();
}

// Reconstructed from eboot.elf at 0x1D13D0. The properties are written with
// Component's _Save, and their revision with them.
void DynamicComBase::_StoreDynamicProps() {
    if (!_HasDynamicProps()) {
        return;
    }
    mSavedProps.Seek(0, kSeekBegin);
    Component::_Save(mSavedProps);
    mSavedPropsRev = gEntityRev;
    _DestroyDynamicProps();
}

// Reconstructed from eboot.elf at 0x1D1470.
void DynamicComBase::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    if (!mPropStorageLoaded) {
        _DestroyDynamicProps();
    }
}

// Reconstructed from eboot.elf at 0x1D14E0.
PropRegistry& DynamicComBase::_GetPropRegistry() {
    if (_HasDynamicProps()) {
        return *_GetDynamicPropRegistry();
    }
    return _GetBasePropRegistry();
}

// Reconstructed from eboot.elf at 0x1D1530.
void DynamicComBase::_PostLoadStorage() {
    mPropStorageLoaded = mPropStorage.mSize != 0;
}

// Reconstructed from eboot.elf at 0x1D1540. The imprint's _HasDynamicProps
// is called and ignored first.
char* DynamicComBase::_ImprintProps(char* buffer, Component* imprint) {
    auto* other = static_cast<DynamicComBase*>(imprint);
    if (other != nullptr) {
        static_cast<void>(other->_HasDynamicProps());
    }
    PropRegistry& base = _GetBasePropRegistry();
    char* end = _ImprintRegistry(
        buffer, imprint, base, other != nullptr ? &other->_GetBasePropRegistry() : nullptr);
    if (!_HasDynamicProps()) {
        return end;
    }
    if (other != nullptr) {
        other->_CopyDynamicResource(*this);
    }
    PropRegistry* dynamic = _GetDynamicPropRegistry();
    return _ImprintRegistry(
        end, imprint, *dynamic, other != nullptr ? other->_GetDynamicPropRegistry() : nullptr);
}

// Reconstructed from eboot.elf at 0x1D1630.
bool DynamicComBase::_OnResourcesLoaded() {
    _StoreDynamicProps();
    return _InitializeDynamicProps();
}

// Reconstructed from eboot.elf at 0x1D1650. The members free the properties
// and the saved stream.
DynamicComBase::~DynamicComBase() {}

// Reconstructed from eboot.elf at 0x1D17B0.
bool DynamicComBase::_LoadResources() {
    return true;
}
