// anim/PropKeysObjCom.o (0x30E2A0-0x310FAF). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysObjCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x310F10).
Symbol PropKeysObjCom::sId("PropKeysObjCom");
Symbol PropKeysObjCom::sClassName("PropKeysObjCom");
PropRegistry PropKeysObjCom::sPropRegistry;
ComMetaData PropKeysObjCom::sMetaData;

// Reconstructed from eboot.elf at 0x30E2A0.
PropDriver* PropKeysObjCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysObjCom>(prop);
}

// Reconstructed from eboot.elf at 0x30E370.
void PropKeysObjCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x30E450.
PropDriver* PropKeysObjCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysObjCom>(mProp);
}

// Reconstructed from eboot.elf at 0x30E530. The struct registration is
// RegisterPropStruct<TKey<GameObjectId>> (0x30E960) and the base
// properties _InitPropKeysBase<PropKeysObjCom> (0x30EB10). The struct's help
// is the bool keys' text.
void PropKeysObjCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<GameObjectId>>(Symbol("TKey<GameObjectId>"));
    PropKeysObjCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysObjCom, mKeys), sizeof(PropArray<TKey<GameObjectId>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<GameObjectId>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<GameObjectId> keyPrototype = {0.0f, GameObjectId{0xFFFFFFFFU}};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<GameObjectId>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    GameObjectIdMetadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<GameObjectId>, mValue), kPropertyGameObjectId, 4),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    key.mHelp = String("A Bool Key in the PropAnim!");
    _InitPropKeysBase<PropKeysObjCom>(registry);
    metadata.mDescription = "Com that contains the Object PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x30F140.
Symbol PropKeysObjCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x30F150.
Symbol PropKeysObjCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x30F160.
int PropKeysObjCom::CurrentRev() const {
    return const_cast<PropKeysObjCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x30F180.
bool PropKeysObjCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x30F1B0.
Component* PropKeysObjCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x30F1C0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysObjCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysObjCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysObjCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x30F3F0.
PropRegistry& PropKeysObjCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x30F400.
ComMetaData& PropKeysObjCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x30F630.
PropertyType PropKeysObjCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x30F640.
Symbol PropKeysObjCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x30FF30.
DataNode PropKeysObjCom::_ValueToDataNode(const GameObjectId& value, bool forEditor) const {
    static_cast<void>(forEditor);
    DataNode node;
    node.mValue.integer = static_cast<int>(value.mId);
    node.mType = kDataGameObjectId;
    return node;
}

// Reconstructed from eboot.elf at 0x30FF40. The node is not evaluated.
GameObjectId PropKeysObjCom::_ValueFromDataNode(const DataNode& value) const {
    return GameObjectId{static_cast<unsigned int>(value.mValue.integer)};
}

// Reconstructed from eboot.elf at 0x30FF50. The binary reads the object's
// copy of the header's null id.
GameObjectId PropKeysObjCom::_DefaultValue() const {
    return GameObjectId{0xFFFFFFFFU};
}

// Reconstructed from eboot.elf at 0x30E520.
const char* PropKeysObjCom::StaticKeyTypeName() {
    return "TKey<kGameObjectId>";
}

// Reconstructed from eboot.elf at 0x2B3E30. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysObjCom::_Create() {
    return new PropKeysObjCom();
}

// Reconstructed from eboot.elf at 0x2AC0F0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysObjCom::Init() {
    InitComponentClass<PropKeysObjCom>();
}
