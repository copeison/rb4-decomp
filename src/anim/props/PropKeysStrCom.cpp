// anim/PropKeysStrCom.o (0x318DB0-0x31B3B1). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysStrCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x31B300).
Symbol PropKeysStrCom::sId("PropKeysStrCom");
Symbol PropKeysStrCom::sClassName("PropKeysStrCom");
PropRegistry PropKeysStrCom::sPropRegistry;
ComMetaData PropKeysStrCom::sMetaData;

// Reconstructed from eboot.elf at 0x318DB0.
PropDriver* PropKeysStrCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysStrCom>(prop);
}

// Reconstructed from eboot.elf at 0x318E80.
void PropKeysStrCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x318F60.
PropDriver* PropKeysStrCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysStrCom>(mProp);
}

// Reconstructed from eboot.elf at 0x319040. The struct registration is
// RegisterPropStruct<TKey<String>> (0x319410) and the base properties
// _InitPropKeysBase<PropKeysStrCom> (0x3195C0). Only the keys' frames are
// registered.
void PropKeysStrCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<String>>(Symbol("TKey<String>"));
    PropKeysStrCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysStrCom, mKeys), sizeof(PropArray<TKey<String>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<String>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<String> keyPrototype = {0.0f, String()};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<String>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    key.mHelp = String("A String Key in the PropAnim!");
    _InitPropKeysBase<PropKeysStrCom>(registry);
    metadata.mDescription = "Com that contains the String PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x319C30.
Symbol PropKeysStrCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x319C40.
Symbol PropKeysStrCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x319C50.
int PropKeysStrCom::CurrentRev() const {
    return const_cast<PropKeysStrCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x319C70.
bool PropKeysStrCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x319CA0.
Component* PropKeysStrCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x319CB0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysStrCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysStrCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysStrCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x319EE0.
PropRegistry& PropKeysStrCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x319EF0.
ComMetaData& PropKeysStrCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x31A170.
PropertyType PropKeysStrCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x31A180.
Symbol PropKeysStrCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x31A5E0.
DataNode PropKeysStrCom::_ValueToDataNode(const String& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(value);
}

// Reconstructed from eboot.elf at 0x31A600.
String PropKeysStrCom::_ValueFromDataNode(const DataNode& value) const {
    return String(value.Str(nullptr));
}

// Reconstructed from eboot.elf at 0x31A630.
String PropKeysStrCom::_DefaultValue() const {
    return String();
}

// Reconstructed from eboot.elf at 0x31AD80. The string is sampled into a
// copy and never applied.
template <>
void TPropDriver<PropKeysStrCom>::_Poll(GameObject* root) {
    static_cast<void>(root);
    String chosen;
    String value;
    float heaviest = -100000.0f;
    for (const Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
        if (entry->mInfo->mPaused) {
            continue;
        }
        entry->mKeys->SampleValueAtFrame(entry->mInfo->GetCurrentFrame(), value);
        if (heaviest < entry->mInfo->mWeight) {
            chosen = value;
            heaviest = entry->mInfo->mWeight;
        }
    }
}

// Reconstructed from eboot.elf at 0x319030.
const char* PropKeysStrCom::StaticKeyTypeName() {
    return "TKey<kStruct>";
}

// Reconstructed from eboot.elf at 0x2B5180. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysStrCom::_Create() {
    return new PropKeysStrCom();
}

// Reconstructed from eboot.elf at 0x2ACA50. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysStrCom::Init() {
    InitComponentClass<PropKeysStrCom>();
}
