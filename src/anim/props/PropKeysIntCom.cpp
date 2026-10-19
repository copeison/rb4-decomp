// anim/PropKeysIntCom.o (0x308AB0-0x30A4AF) and the integer key classes'
// template instances, which the binary emits in anim/AnimInit.o.
#include "anim/props/PropKeysIntCom.h"

#include <new>
#include <stdio.h>

#include "anim/props/PropAnimResource.h"
#include "entity/props/PropUtl.h"

template <class T>
const char* PropKeysIntCom<T>::_MakeClassName() {
    static char name[80];
    sprintf(name, "PropKeysIntCom_%s", TypeName(StaticKeysType(), true));
    return name;
}

template <class T>
Symbol PropKeysIntCom<T>::sId(PropKeysIntCom<T>::_MakeClassName());
template <class T>
Symbol PropKeysIntCom<T>::sClassName(PropKeysIntCom<T>::_MakeClassName());
template <class T>
PropRegistry PropKeysIntCom<T>::sPropRegistry;
template <class T>
ComMetaData PropKeysIntCom<T>::sMetaData;

// Reconstructed from eboot.elf at 0x308AB0.
template <class T>
PropertyType PropKeysIntCom<T>::StaticKeysType() {
    return PropValueType<T>::kType;
}

// Reconstructed from eboot.elf at 0x308AC0.
template <class T>
const Symbol& PropKeysIntCom<T>::StaticKeysSubType() {
    static Symbol subType;
    if (subType == Symbol()) {
        subType = Symbol("");
    }
    return subType;
}

// Reconstructed from eboot.elf at 0x308B60.
template <class T>
PropDriver* PropKeysIntCom<T>::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysIntCom<T>>(prop);
}

// Reconstructed from eboot.elf at 0x308C30.
template <class T>
void PropKeysIntCom<T>::Register() {
    const Symbol className = sClassName;
    PropKeysBaseCom::RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x308D10.
template <class T>
PropDriver* PropKeysIntCom<T>::CreatePropDriver() const {
    return new TPropDriver<PropKeysIntCom<T>>(this->mProp);
}

// Reconstructed from eboot.elf at 0x308DE0.
template <class T>
const char* PropKeysIntCom<T>::StaticKeyTypeName() {
    switch (PropValueType<T>::kType) {
    case kPropertyInt8:
        return "TKey<kInt8>";
    case kPropertyInt16:
        return "TKey<kInt16>";
    case kPropertyInt:
        return "TKey<kInt32>";
    case kPropertyUInt8:
        return "TKey<kUint8>";
    case kPropertyUInt16:
        return "TKey<kUint16>";
    case kPropertyUInt:
        return "TKey<kUint32>";
    case kPropertyInt64:
        return "TKey<kInt64>";
    default:
        return "TKey<kUint64>";
    }
}

// Reconstructed from eboot.elf at 0x2B73F0. The struct registration is
// RegisterPropStruct<TKey<T>> (0x2B7A20 for signed char) and the base
// properties _InitPropKeysBase (0x2B7BD0). The struct's help is the bool
// keys' text.
template <class T>
void PropKeysIntCom<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<T>>(Symbol(StaticKeyTypeName()));
    PropKeysIntCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysIntCom, mKeys), sizeof(PropArray<TKey<T>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<T>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<T> keyPrototype = {};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<T>, mFrame), kPropertyFloat, 4), keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    IntMetadata<T>& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<T>, mValue), StaticKeysType(), sizeof(T)),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, PropKeysBaseCom::sRemapKeyValue);
    key.mHelp = String("A Bool Key in the PropAnim!");
    PropKeysBaseCom::_InitPropKeysBase<PropKeysIntCom>(registry);
    metadata.mDescription = "Com that contains the Int PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x2B83A0.
template <class T>
Symbol PropKeysIntCom<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2B83B0.
template <class T>
Symbol PropKeysIntCom<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2B83C0.
template <class T>
int PropKeysIntCom<T>::CurrentRev() const {
    return const_cast<PropKeysIntCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2B83E0.
template <class T>
bool PropKeysIntCom<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2B8410.
template <class T>
Component* PropKeysIntCom<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2B8420. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* PropKeysIntCom<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysIntCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysIntCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2B8650.
template <class T>
PropRegistry& PropKeysIntCom<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2B8660.
template <class T>
ComMetaData& PropKeysIntCom<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B8890.
template <class T>
PropertyType PropKeysIntCom<T>::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x2B88A0.
template <class T>
Symbol PropKeysIntCom<T>::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x2B90F0.
template <class T>
DataNode PropKeysIntCom<T>::_ValueToDataNode(const T& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(static_cast<int>(value));
}

// Reconstructed from eboot.elf at 0x2B9110.
template <class T>
T PropKeysIntCom<T>::_ValueFromDataNode(const DataNode& value) const {
    return static_cast<T>(value.Int(nullptr));
}

// Reconstructed from eboot.elf at 0x2B9130.
template <class T>
T PropKeysIntCom<T>::_DefaultValue() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x2B7820. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
template <class T>
Component* PropKeysIntCom<T>::_Create() {
    return new PropKeysIntCom();
}

// Reconstructed from eboot.elf at 0x2A84E0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
template <class T>
void PropKeysIntCom<T>::Init() {
    InitComponentClass<PropKeysIntCom<T>>();
}

template class PropKeysIntCom<signed char>;
template class PropKeysIntCom<short>;
template class PropKeysIntCom<int>;
template class PropKeysIntCom<unsigned char>;
template class PropKeysIntCom<unsigned short>;
template class PropKeysIntCom<unsigned int>;
template class PropKeysIntCom<long>;
template class PropKeysIntCom<unsigned long>;
