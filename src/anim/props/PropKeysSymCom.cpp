// anim/PropKeysSymCom.o (0x31B470-0x31E1F0). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysSymCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x31E170).
Symbol PropKeysSymCom::sId("PropKeysSymCom");
Symbol PropKeysSymCom::sClassName("PropKeysSymCom");
PropRegistry PropKeysSymCom::sPropRegistry;
ComMetaData PropKeysSymCom::sMetaData;

// Reconstructed from eboot.elf at 0x31B470.
PropDriver* PropKeysSymCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysSymCom>(prop);
}

// Reconstructed from eboot.elf at 0x31B540.
void PropKeysSymCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x31B620.
PropDriver* PropKeysSymCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysSymCom>(mProp);
}

// Reconstructed from eboot.elf at 0x31B700. The struct registration is
// RegisterPropStruct<TKey<Symbol>> (0x31BB40) and the base properties
// _InitPropKeysBase<PropKeysSymCom> (0x31BCF0).
void PropKeysSymCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<Symbol>>(Symbol("TKey<Symbol>"));
    PropKeysSymCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysSymCom, mKeys), sizeof(PropArray<TKey<Symbol>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<Symbol>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<Symbol> keyPrototype = {0.0f, Symbol()};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<Symbol>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    SymbolMetadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<Symbol>, mValue), kPropertySymbol, 8),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    key.mHelp = String("A Symbol Key in the PropAnim!");
    _InitPropKeysBase<PropKeysSymCom>(registry);
    metadata.mDescription = "Com that contains the Symbol PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x31C320.
Symbol PropKeysSymCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x31C330.
Symbol PropKeysSymCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x31C340.
int PropKeysSymCom::CurrentRev() const {
    return const_cast<PropKeysSymCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x31C360.
bool PropKeysSymCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x31C390.
Component* PropKeysSymCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x31C3A0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysSymCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysSymCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysSymCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x31C5D0.
PropRegistry& PropKeysSymCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x31C5E0.
ComMetaData& PropKeysSymCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x31C830.
PropertyType PropKeysSymCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x31C840.
Symbol PropKeysSymCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x31D150.
DataNode PropKeysSymCom::_ValueToDataNode(const Symbol& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(value);
}

// Reconstructed from eboot.elf at 0x31D170.
Symbol PropKeysSymCom::_ValueFromDataNode(const DataNode& value) const {
    return value.Sym(nullptr);
}

// Reconstructed from eboot.elf at 0x31D180.
Symbol PropKeysSymCom::_DefaultValue() const {
    return Symbol();
}

// Reconstructed from eboot.elf at 0x31B6F0.
const char* PropKeysSymCom::StaticKeyTypeName() {
    return "TKey<kSymbol>";
}

// Reconstructed from eboot.elf at 0x2B4860. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysSymCom::_Create() {
    return new PropKeysSymCom();
}

// Reconstructed from eboot.elf at 0x2AC5A0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysSymCom::Init() {
    InitComponentClass<PropKeysSymCom>();
}
