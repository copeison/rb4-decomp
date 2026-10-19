// anim/PropKeysFloatCom.o (0x305D40-0x308AAF). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysFloatCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x3089F0).
Symbol PropKeysFloatCom::sId("PropKeysFloatCom");
Symbol PropKeysFloatCom::sClassName("PropKeysFloatCom");
PropRegistry PropKeysFloatCom::sPropRegistry;
ComMetaData PropKeysFloatCom::sMetaData;

// Reconstructed from eboot.elf at 0x305D40.
PropDriver* PropKeysFloatCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysFloatCom>(prop);
}

// Reconstructed from eboot.elf at 0x305E10.
void PropKeysFloatCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x305EF0.
PropDriver* PropKeysFloatCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysFloatCom>(mProp);
}

// Reconstructed from eboot.elf at 0x305FC0.
const char* PropKeysFloatCom::StaticKeyTypeName() {
    return "TKey<kFloat>";
}

// Reconstructed from eboot.elf at 0x305FD0. The struct registrations are
// RegisterPropStruct<TKey<float>> (0x3064E0) and
// RegisterPropStruct<TKey<ValueTangent<float>>> (0x1C9310, emitted with the
// waveforms) and the base properties _InitPropKeysBase<PropKeysFloatCom>
// (0x306690).
void PropKeysFloatCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<float>>(Symbol("TKey<float>"));
    RegisterPropStruct<TKey<ValueTangent<float>>>(Symbol("TKey<ValueTangentFloat>"));
    PropKeysFloatCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysFloatCom, mKeys), sizeof(PropArray<TKey<ValueTangent<float>>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<ValueTangent<float>>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<ValueTangent<float>> keyPrototype = {};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<ValueTangent<float>>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    FloatMetadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<ValueTangent<float>>, mValue.mValue), kPropertyFloat, 4),
        keyPrototype.mValue.mValue);
    value.mDefault = keyPrototype.mValue.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    FloatMetadata& inTangent = TypeSpecificMetadata(
        key.mRegistry.RegisterProp(
            "float_tangent1", offsetof(TKey<ValueTangent<float>>, mValue.mInTangent), kPropertyFloat, 4),
        keyPrototype.mValue.mInTangent);
    inTangent.mDefault = keyPrototype.mValue.mInTangent;
    FloatMetadata& outTangent = TypeSpecificMetadata(
        key.mRegistry.RegisterProp(
            "float_tangent2", offsetof(TKey<ValueTangent<float>>, mValue.mOutTangent), kPropertyFloat, 4),
        keyPrototype.mValue.mOutTangent);
    outTangent.mDefault = keyPrototype.mValue.mOutTangent;
    BoolMetadata& autoTangents = TypeSpecificMetadata(
        key.mRegistry.RegisterProp(
            "auto", offsetof(TKey<ValueTangent<float>>, mValue.mAutoTangents), kPropertyBool, 1),
        keyPrototype.mValue.mAutoTangents);
    autoTangents.mDefault = keyPrototype.mValue.mAutoTangents;
    key.mHelp = String("A Float Key in the PropAnim!");
    _InitPropKeysBase<PropKeysFloatCom>(registry);
    metadata.mDescription = "Com that contains the Float PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x307000.
Symbol PropKeysFloatCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x307010.
Symbol PropKeysFloatCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x307020.
int PropKeysFloatCom::CurrentRev() const {
    return const_cast<PropKeysFloatCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x307040.
bool PropKeysFloatCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x307070.
Component* PropKeysFloatCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x307080. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysFloatCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysFloatCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysFloatCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x3072B0.
PropRegistry& PropKeysFloatCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x3072C0.
ComMetaData& PropKeysFloatCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x307520.
PropertyType PropKeysFloatCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x307530.
Symbol PropKeysFloatCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x3072D0.
bool PropKeysFloatCom::HasTangents() const {
    return true;
}

// Reconstructed from eboot.elf at 0x3075D0.
bool PropKeysFloatCom::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    static_cast<void>(interpolation);
    return true;
}

// Reconstructed from eboot.elf at 0x307E40.
DataNode PropKeysFloatCom::_ValueToDataNode(const ValueTangent<float>& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(value.mValue);
}

// Reconstructed from eboot.elf at 0x307E60.
ValueTangent<float> PropKeysFloatCom::_ValueFromDataNode(const DataNode& value) const {
    return {value.Float(nullptr), 0.0f, 0.0f, true};
}

// Reconstructed from eboot.elf at 0x307E90.
ValueTangent<float> PropKeysFloatCom::_DefaultValue() const {
    return ValueTangent<float>();
}

// Reconstructed from eboot.elf at 0x306C30. The sample keeps the previous
// playback's value across a paused one.
template <>
void TPropDriver<PropKeysFloatCom>::_Poll(GameObject* root) {
    static_cast<void>(root);
    ValueTangent<float> sample;
    float sum = 0.0f;
    float weights = 0.0f;
    bool first = true;
    for (const Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
        if (entry->mInfo->mPaused) {
            continue;
        }
        entry->mKeys->SampleValueAtFrame(entry->mInfo->GetCurrentFrame(), sample);
        const float weight = entry->mInfo->mWeight;
        if (first) {
            sum = weight * sample.mValue;
            weights = weight;
            first = false;
        } else {
            sum += weight * sample.mValue;
            weights += weight;
        }
    }
    if (weights > 0.0f) {
        const ValueTangent<float> value = {sum / weights, 0.0f, 0.0f, true};
        if (mEntries.mpBegin->mCom != nullptr) {
            _SetDrivenProperty(mEntries.mpBegin->mCom, mEntries.mpBegin->mKeys->mProp.mPath, &value);
        }
    }
}

// Reconstructed from eboot.elf at 0x2B2990. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysFloatCom::_Create() {
    return new PropKeysFloatCom();
}

// Reconstructed from eboot.elf at 0x2AAA60. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysFloatCom::Init() {
    InitComponentClass<PropKeysFloatCom>();
}
