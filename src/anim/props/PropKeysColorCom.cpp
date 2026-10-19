// anim/PropKeysColorCom.o (0x302B50-0x30534F). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysColorCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"

// The object's statics, in the order of its static initializer (0x305280).
Symbol PropKeysColorCom::sId("PropKeysColorCom");
Symbol PropKeysColorCom::sClassName("PropKeysColorCom");
PropRegistry PropKeysColorCom::sPropRegistry;
ComMetaData PropKeysColorCom::sMetaData;

// Reconstructed from eboot.elf at 0x302B50.
PropDriver* PropKeysColorCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysColorCom>(prop);
}

// Reconstructed from eboot.elf at 0x302C20.
void PropKeysColorCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x302D00.
PropDriver* PropKeysColorCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysColorCom>(mProp);
}

// Reconstructed from eboot.elf at 0x302DD0.
const char* PropKeysColorCom::StaticKeyTypeName() {
    return "TKey<kColor>";
}

// Reconstructed from eboot.elf at 0x302DE0. The struct registration is
// RegisterPropStruct<TKey<Hmx::Color>> (0x1C94C0, emitted with the
// waveforms) and the base properties _InitPropKeysBase<PropKeysColorCom>
// (0x303210). A key's colour defaults to opaque black.
void PropKeysColorCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<Hmx::Color>>(Symbol("TKey<Hmx::Color>"));
    PropKeysColorCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysColorCom, mKeys), sizeof(PropArray<TKey<Hmx::Color>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<Hmx::Color>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<Hmx::Color> keyPrototype = {0.0f, Hmx::Color(0.0f, 0.0f, 0.0f, 1.0f)};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<Hmx::Color>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    ColorMetadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<Hmx::Color>, mValue), kPropertyColor, sizeof(Hmx::Color)),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    key.mHelp = String("A Color Key in the PropAnim");
    _InitPropKeysBase<PropKeysColorCom>(registry);
    metadata.mDescription = "Com that contains the Color PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x303990.
Symbol PropKeysColorCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x3039A0.
Symbol PropKeysColorCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x3039B0.
int PropKeysColorCom::CurrentRev() const {
    return const_cast<PropKeysColorCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x3039D0.
bool PropKeysColorCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x303A00.
Component* PropKeysColorCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x303A10. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysColorCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysColorCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysColorCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x303C40.
PropRegistry& PropKeysColorCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x303C50.
ComMetaData& PropKeysColorCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x303EA0.
PropertyType PropKeysColorCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x303EB0.
Symbol PropKeysColorCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x303F50.
bool PropKeysColorCom::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    return interpolation != KeyTrackBase::kInterpolationSpline;
}

// Reconstructed from eboot.elf at 0x303F60.
bool PropKeysColorCom::IsColorKeys() const {
    return true;
}

// Reconstructed from eboot.elf at 0x304730.
DataNode PropKeysColorCom::_ValueToDataNode(const Hmx::Color& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(value, false);
}

// Reconstructed from eboot.elf at 0x304750.
Hmx::Color PropKeysColorCom::_ValueFromDataNode(const DataNode& value) const {
    return value.Color(false, nullptr);
}

// Reconstructed from eboot.elf at 0x304760.
Hmx::Color PropKeysColorCom::_DefaultValue() const {
    return Hmx::Color(0.0f, 0.0f, 0.0f, 0.0f);
}

// Reconstructed from eboot.elf at 0x3037B0. The sample and the sum start
// as opaque black; each later sample is scaled in place before it is
// added.
template <>
void TPropDriver<PropKeysColorCom>::_Poll(GameObject* root) {
    static_cast<void>(root);
    Hmx::Color sum(0.0f, 0.0f, 0.0f, 1.0f);
    Hmx::Color sample(0.0f, 0.0f, 0.0f, 1.0f);
    float weights = 0.0f;
    bool first = true;
    for (const Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
        if (entry->mInfo->mPaused) {
            continue;
        }
        entry->mKeys->SampleValueAtFrame(entry->mInfo->GetCurrentFrame(), sample);
        const float weight = entry->mInfo->mWeight;
        if (first) {
            sum = Hmx::Color(sample.red * weight, sample.green * weight, sample.blue * weight, sample.alpha * weight);
            weights = weight;
            first = false;
        } else {
            sample = Hmx::Color(sample.red * weight, sample.green * weight, sample.blue * weight, sample.alpha * weight);
            sum = Hmx::Color(
                sample.red + sum.red, sample.green + sum.green, sample.blue + sum.blue, sample.alpha + sum.alpha);
            weights += weight;
        }
    }
    if (weights > 0.0f) {
        const float scale = 1.0f / weights;
        sum = Hmx::Color(sum.red * scale, sum.green * scale, sum.blue * scale, sum.alpha * scale);
        if (mEntries.mpBegin->mCom != nullptr) {
            _SetDrivenProperty(mEntries.mpBegin->mCom, mEntries.mpBegin->mKeys->mProp.mPath, &sum);
        }
    }
}

// Reconstructed from eboot.elf at 0x2B5C90. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysColorCom::_Create() {
    return new PropKeysColorCom();
}

// Reconstructed from eboot.elf at 0x2ACF00. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysColorCom::Init() {
    InitComponentClass<PropKeysColorCom>();
}
