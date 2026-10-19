// anim/PropKeysWaveformCom.o (0x325320-0x326BDF) and the waveform key
// classes' template instances, which the binary emits in anim/AnimInit.o.
#include "anim/props/PropKeysWaveformCom.h"

#include <new>
#include <stdio.h>

#include "anim/props/PropAnimResource.h"
#include "entity/props/PropUtl.h"

namespace {

// The waveform node's value. Inlined into _ValueFromDataNode. Name not in
// the reference map.
template <class T>
Waveform<T> LiteralWaveform(const DataNode& node);
template <>
Waveform<float> LiteralWaveform<float>(const DataNode& node) {
    return node.LiteralWaveformFloat(nullptr);
}
template <>
Waveform<Hmx::Color> LiteralWaveform<Hmx::Color>(const DataNode& node) {
    return node.LiteralWaveformColor(nullptr);
}

// The waveform of the last playback's keys at its frame: the key's
// waveform, blending into the next key's through its second resource with
// the eased position and the interpolation's 0x20000000 flag. False when
// the playback is paused or the keys are empty. 0x325AD0 for float. Name
// not in the reference map.
template <class T>
bool SampleLastPlayback(const TPropDriver<PropKeysWaveformCom<T>>& driver, Waveform<T>& value) {
    const typename TPropDriver<PropKeysWaveformCom<T>>::Entry& entry = driver.mEntries.mpEnd[-1];
    if (entry.mInfo->mPaused || entry.mKeys->Empty()) {
        return false;
    }
    const TKey<Waveform<T>>* prev = nullptr;
    const TKey<Waveform<T>>* next = nullptr;
    float t = 0.0f;
    entry.mKeys->FindKeysAtFrame(entry.mInfo->GetCurrentFrame(), prev, next, t);
    Waveform<T> sample(prev->mValue.mResource.Get());
    sample.mOverride = prev->mValue.mOverride.Get();
    sample.mOverrideData[0] = prev->mValue.mOverrideData[0];
    sample.mOverrideData[1] = prev->mValue.mOverrideData[1];
    if (t > 0.0f && prev != next) {
        const unsigned int interpolation = static_cast<unsigned int>(entry.mKeys->mInterpolation);
        const float blend =
            InterpolationEase(static_cast<KeyTrackBase::Interpolation>(interpolation & 0xFFFF), t);
        sample.mOverride = next->mValue.mResource.Get();
        __builtin_memcpy(&sample.mOverrideData[0], &blend, sizeof(blend));
        sample.mOverrideData[1] = static_cast<int>((interpolation >> 29) & 1);
    }
    value = sample;
    return true;
}

// The driver's poll for both waveform types: sets the property of the first
// playback's component. Name not in the reference map.
template <class T>
void PollWaveformKeys(TPropDriver<PropKeysWaveformCom<T>>& driver) {
    Waveform<T> value;
    if (SampleLastPlayback(driver, value) && driver.mEntries.mpBegin->mCom != nullptr) {
        driver._SetDrivenProperty(
            driver.mEntries.mpBegin->mCom, driver.mEntries.mpBegin->mKeys->mProp.mPath, &value);
    }
}

}  // namespace

template <class T>
const char* PropKeysWaveformCom<T>::_MakeClassName() {
    static char name[80];
    sprintf(name, "PropKeysWaveformCom_%s", TypeName(StaticKeysType(), true));
    return name;
}

template <class T>
Symbol PropKeysWaveformCom<T>::sId(PropKeysWaveformCom<T>::_MakeClassName());
template <class T>
Symbol PropKeysWaveformCom<T>::sClassName(PropKeysWaveformCom<T>::_MakeClassName());
template <class T>
PropRegistry PropKeysWaveformCom<T>::sPropRegistry;
template <class T>
ComMetaData PropKeysWaveformCom<T>::sMetaData;

// Reconstructed from eboot.elf at 0x325320.
template <class T>
PropertyType PropKeysWaveformCom<T>::StaticKeysType() {
    return PropValueType<Waveform<T>>::kType;
}

// Reconstructed from eboot.elf at 0x325330.
template <class T>
const Symbol& PropKeysWaveformCom<T>::StaticKeysSubType() {
    static Symbol subType;
    if (subType == Symbol()) {
        subType = Symbol("");
    }
    return subType;
}

// Reconstructed from eboot.elf at 0x3253D0.
template <class T>
PropDriver* PropKeysWaveformCom<T>::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysWaveformCom<T>>(prop);
}

// Reconstructed from eboot.elf at 0x3254A0.
template <class T>
void PropKeysWaveformCom<T>::Register() {
    const Symbol className = sClassName;
    PropKeysBaseCom::RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x325580.
template <class T>
PropDriver* PropKeysWaveformCom<T>::CreatePropDriver() const {
    return new TPropDriver<PropKeysWaveformCom<T>>(this->mProp);
}

// Reconstructed from eboot.elf at 0x325650.
template <class T>
const char* PropKeysWaveformCom<T>::StaticKeyTypeName() {
    return PropValueType<Waveform<T>>::kType == kPropertyWaveformFloat ? "TKey<kWaveformFloat>" : "TKey<kWaveformColor>";
}

// Reconstructed from eboot.elf at 0x2CCD80. The struct registration is
// RegisterPropStruct<TKey<Waveform<T>>> (0x2CD570 for float) and the base
// properties _InitPropKeysBase (0x2CD750). Both types name the struct
// "TKey<Waveform<C>>".
template <class T>
void PropKeysWaveformCom<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<Waveform<T>>>(Symbol("TKey<Waveform<C>>"));
    PropKeysWaveformCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysWaveformCom, mKeys), sizeof(PropArray<TKey<Waveform<T>>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<Waveform<T>>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<Waveform<T>> keyPrototype = {0.0f, Waveform<T>()};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<Waveform<T>>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    WaveformMetadata<T>& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp(
            "value", offsetof(TKey<Waveform<T>>, mValue), StaticKeysType(), sizeof(Waveform<T>)),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, PropKeysBaseCom::sRemapKeyValue);
    key.mHelp = String("A Waveform Key in the PropAnim!");
    PropKeysBaseCom::_InitPropKeysBase<PropKeysWaveformCom>(registry);
    metadata.mDescription = "Com that contains the Waveform PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x2CDF20.
template <class T>
Symbol PropKeysWaveformCom<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2CDF30.
template <class T>
Symbol PropKeysWaveformCom<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2CDF40.
template <class T>
int PropKeysWaveformCom<T>::CurrentRev() const {
    return const_cast<PropKeysWaveformCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2CDF60.
template <class T>
bool PropKeysWaveformCom<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2CDF90.
template <class T>
Component* PropKeysWaveformCom<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2CDFA0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* PropKeysWaveformCom<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysWaveformCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysWaveformCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2CE1D0.
template <class T>
PropRegistry& PropKeysWaveformCom<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2CE1E0.
template <class T>
ComMetaData& PropKeysWaveformCom<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2CE570.
template <class T>
PropertyType PropKeysWaveformCom<T>::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x2CE580.
template <class T>
Symbol PropKeysWaveformCom<T>::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x2CE590.
template <class T>
bool PropKeysWaveformCom<T>::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    return interpolation != KeyTrackBase::kInterpolationSpline;
}

// Reconstructed from eboot.elf at 0x2CE5A0.
template <class T>
bool PropKeysWaveformCom<T>::IsColorKeys() const {
    return PropValueType<Waveform<T>>::kType == kPropertyWaveformColor;
}

// Reconstructed from eboot.elf at 0x2CF040.
template <class T>
DataNode PropKeysWaveformCom<T>::_ValueToDataNode(const Waveform<T>& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(value);
}

// Reconstructed from eboot.elf at 0x2CF060.
template <class T>
Waveform<T> PropKeysWaveformCom<T>::_ValueFromDataNode(const DataNode& value) const {
    return LiteralWaveform<T>(value);
}

// Reconstructed from eboot.elf at 0x2CF080. The local static is at
// 0x19F7FF0 for float.
template <class T>
Waveform<T> PropKeysWaveformCom<T>::_DefaultValue() const {
    static const Waveform<T> sDefaultValue;
    return sDefaultValue;
}

// Reconstructed from eboot.elf at 0x2CD370. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
template <class T>
Component* PropKeysWaveformCom<T>::_Create() {
    return new PropKeysWaveformCom();
}

// Reconstructed from eboot.elf at 0x2AD3B0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
template <class T>
void PropKeysWaveformCom<T>::Init() {
    InitComponentClass<PropKeysWaveformCom<T>>();
}

// Reconstructed from eboot.elf at 0x3259A0.
template <>
void TPropDriver<PropKeysWaveformCom<float>>::_Poll(GameObject* root) {
    static_cast<void>(root);
    PollWaveformKeys(*this);
}

// Reconstructed from eboot.elf at 0x325CE0.
template <>
void TPropDriver<PropKeysWaveformCom<Hmx::Color>>::_Poll(GameObject* root) {
    static_cast<void>(root);
    PollWaveformKeys(*this);
}

template class PropKeysWaveformCom<float>;
template class PropKeysWaveformCom<Hmx::Color>;
