// anim/PropKeysBoolCom.o (0x2FFD60-0x302A91). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysBoolCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x3029E0).
Symbol PropKeysBoolCom::sId("PropKeysBoolCom");
Symbol PropKeysBoolCom::sClassName("PropKeysBoolCom");
PropRegistry PropKeysBoolCom::sPropRegistry;
ComMetaData PropKeysBoolCom::sMetaData;

// Reconstructed from eboot.elf at 0x2FFD60.
PropDriver* PropKeysBoolCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysBoolCom>(prop);
}

// Reconstructed from eboot.elf at 0x2FFE30.
void PropKeysBoolCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x2FFF10.
PropDriver* PropKeysBoolCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysBoolCom>(mProp);
}

// Reconstructed from eboot.elf at 0x2FFFE0.
const char* PropKeysBoolCom::StaticKeyTypeName() {
    return "TKey<kBool>";
}

// Reconstructed from eboot.elf at 0x2FFFF0. The struct registration is
// RegisterPropStruct<TKey<bool>> (0x300420) and the base properties
// _InitPropKeysBase<PropKeysBoolCom> (0x3005D0).
void PropKeysBoolCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<bool>>(Symbol("TKey<bool>"));
    PropKeysBoolCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysBoolCom, mKeys), sizeof(PropArray<TKey<bool>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<bool>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<bool> keyPrototype = {};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<bool>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    BoolMetadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<bool>, mValue), kPropertyBool, 1),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    key.mHelp = String("A Bool Key in the PropAnim!");
    _InitPropKeysBase<PropKeysBoolCom>(registry);
    metadata.mDescription = "Com that contains the Bool PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x300C00.
Symbol PropKeysBoolCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x300C10.
Symbol PropKeysBoolCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x300C20.
int PropKeysBoolCom::CurrentRev() const {
    return const_cast<PropKeysBoolCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x300C40.
bool PropKeysBoolCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x300C70.
Component* PropKeysBoolCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x300C80. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysBoolCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysBoolCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysBoolCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x300EB0.
PropRegistry& PropKeysBoolCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x300EC0.
ComMetaData& PropKeysBoolCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x3010F0.
PropertyType PropKeysBoolCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x301100.
Symbol PropKeysBoolCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x3019E0.
DataNode PropKeysBoolCom::_ValueToDataNode(const bool& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(static_cast<int>(value));
}

// Reconstructed from eboot.elf at 0x301A00.
bool PropKeysBoolCom::_ValueFromDataNode(const DataNode& value) const {
    return value.Int(nullptr) != 0;
}

// Reconstructed from eboot.elf at 0x301A20.
bool PropKeysBoolCom::_DefaultValue() const {
    return false;
}

// Reconstructed from eboot.elf at 0x2B1E60. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysBoolCom::_Create() {
    return new PropKeysBoolCom();
}

// Reconstructed from eboot.elf at 0x2A8030. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysBoolCom::Init() {
    InitComponentClass<PropKeysBoolCom>();
}
