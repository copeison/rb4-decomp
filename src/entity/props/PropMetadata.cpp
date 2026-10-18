#include "entity/props/PropMetadata.h"

#include "utl/text/MakeString.h"

namespace {

// Whether the tag functions that need a component may be set. Name not in
// the reference map.
bool gNonStaticTagFunctionsAllowed = true;  // 0x19B0308

}  // namespace

// Reconstructed from eboot.elf at 0x12DD40.
void SetNonStaticTagFunctionsAllowed(bool allowed) {
    gNonStaticTagFunctionsAllowed = allowed;
}

// Reconstructed from eboot.elf at 0x12DD50.
bool AreNonStaticTagFunctionsAllowed() {
    return gNonStaticTagFunctionsAllowed;
}

// Reconstructed from eboot.elf at 0x12DD60.
TagArgs::TagArgs(const Component& component, const PropPath& path)
    : mComponent(component),
      mPath(path) {}

// Reconstructed from eboot.elf at 0x12DD70.
StaticTagArgs::StaticTagArgs(Entity* entity, const PropPath& path)
    : mEntity(entity),
      mPath(path) {}

// Reconstructed from eboot.elf at 0x12DDB0. A new metadata names its
// property with the default display name function, or with the static one
// while tag functions that need a component are not allowed.
PropMetadata::PropMetadata()
    : mDisplayName(String(), AggUseFirstResultWithAsteriskIfConflicted),
      mGroup(Symbol(""), AggUseFirstResult<Symbol>),
      mIsHidden(false, AggTrueIfAnyTrue),
      mReadOnly(false, AggTrueIfAnyTrue),
      mDebug(false, AggTrueIfAnyTrue),
      mAnimatable(true, AggTrueIfAllTrue),
      mExtraFlag0(true, AggTrueIfAllTrue),
      mExtraFlag1(true, AggTrueIfAllTrue),
      mExtraFlag2(true, AggTrueIfAllTrue),
      mNoTest(false),
      mSaves(true),
      mHistory(true),
      mReserved(false),
      mInstanceFile(false),
      mHelp(String(""), AggUseFirstResult<String>),
      mInlineAction(nullptr),
      mInstanceAccess(kInstanceAccessNone),
      mRefCount(0) {
    if (gNonStaticTagFunctionsAllowed) {
        mDisplayName.mMode = TagBase<String>::kModeFunction;
        mDisplayName.mFunction = DefaultDisplayName;
    } else {
        mDisplayName.mMode = TagBase<String>::kModeStaticFunction;
        mDisplayName.mStaticFunction = StaticDefaultDisplayName;
    }
}

// Reconstructed from eboot.elf at 0x12E300. The first component's name is
// taken; a later one that differs marks it with an asterisk and stops.
bool AggUseFirstResultWithAsteriskIfConflicted(
    const String& value,
    unsigned long index,
    String& result) {
    if (index == 0) {
        result = value.c_str();
        return true;
    }
    if (value != result) {
        result = MakeString("%s*", result.c_str());
        return false;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x12E450. Stops at the first true value.
bool AggTrueIfAnyTrue(const bool& value, unsigned long index, bool& result) {
    static_cast<void>(index);
    result = value;
    return !value;
}

// Reconstructed from eboot.elf at 0x12E460. Stops at the first false value.
bool AggTrueIfAllTrue(const bool& value, unsigned long index, bool& result) {
    static_cast<void>(index);
    result = value;
    return value;
}

// Reconstructed from eboot.elf at 0x12E610.
PropMetadata::~PropMetadata() {
    delete mInlineAction;
}

// Reconstructed from eboot.elf at 0x1420B0.
const char* PropMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    static_cast<void>(useNames);
    return "PropMetadata::ValueToString not implemented";
}

// Reconstructed from eboot.elf at 0x1420C0.
bool PropMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return false;
}

// Reconstructed from eboot.elf at 0x1420D0.
String PropMetadata::GetAllowedValuesHelp(
    const eastl::vector<Component*>& components,
    const PropPath& path) const {
    static_cast<void>(components);
    static_cast<void>(path);
    return String("");
}

// Reconstructed from eboot.elf at 0x142100.
DataArrayPtr PropMetadata::DumpAllowedValuesHelp(
    const Component& component,
    const PropPath& path) const {
    static_cast<void>(component);
    static_cast<void>(path);
    return DataArrayPtr();
}

// Reconstructed from eboot.elf at 0x142110.
PropRegistry* PropMetadata::GetStructRegistry() const {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x142DF0.
BoolMetadata::~BoolMetadata() {}

// Reconstructed from eboot.elf at 0x142FC0.
const char* BoolMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    return *static_cast<const bool*>(value) ? "true" : "false";
}

// Reconstructed from eboot.elf at 0x142FE0.
bool BoolMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return true;
}

// Reconstructed from eboot.elf at 0x143C20.
PropRegistry* GroupMetadata::GetStructRegistry() const {
    return const_cast<PropRegistry*>(&mRegistry);
}

// Reconstructed from eboot.elf at 0x141E70. Keeps the running average of
// the components' values.
bool AggUseAverage(const float& value, unsigned long index, float& result) {
    result = (static_cast<float>(index) * result + value) / static_cast<float>(index + 1);
    return true;
}

// Reconstructed from eboot.elf at 0x140A20.
FloatMetadata& TypeSpecificMetadata(PropInfo& info, const float& type) {
    static_cast<void>(type);
    return *static_cast<FloatMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141B90.
IntMetadata<signed char>& TypeSpecificMetadata(PropInfo& info, const signed char& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<signed char>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141BB0.
IntMetadata<short>& TypeSpecificMetadata(PropInfo& info, const short& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<short>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141BD0.
IntMetadata<int>& TypeSpecificMetadata(PropInfo& info, const int& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<int>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141BF0.
IntMetadata<long>& TypeSpecificMetadata(PropInfo& info, const long& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<long>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141C10.
IntMetadata<unsigned char>& TypeSpecificMetadata(PropInfo& info, const unsigned char& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<unsigned char>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141C30.
IntMetadata<unsigned short>& TypeSpecificMetadata(PropInfo& info, const unsigned short& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<unsigned short>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141C50.
IntMetadata<unsigned int>& TypeSpecificMetadata(PropInfo& info, const unsigned int& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<unsigned int>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141C70.
IntMetadata<unsigned long>& TypeSpecificMetadata(PropInfo& info, const unsigned long& type) {
    static_cast<void>(type);
    return *static_cast<IntMetadata<unsigned long>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141C90.
BoolMetadata& TypeSpecificMetadata(PropInfo& info, const bool& type) {
    static_cast<void>(type);
    return *static_cast<BoolMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141CB0.
GameObjectIdMetadata& TypeSpecificMetadata(PropInfo& info, const GameObjectId& type) {
    static_cast<void>(type);
    return *static_cast<GameObjectIdMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141CD0.
SymbolMetadata& TypeSpecificMetadata(PropInfo& info, const Symbol& type) {
    static_cast<void>(type);
    return *static_cast<SymbolMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141CF0.
ResourcePathMetadata& TypeSpecificMetadata(PropInfo& info, const ResourcePath& type) {
    static_cast<void>(type);
    return *static_cast<ResourcePathMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141D10.
ColorMetadata& TypeSpecificMetadata(PropInfo& info, const Hmx::Color& type) {
    static_cast<void>(type);
    return *static_cast<ColorMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141D30.
PropRefMetadata& TypeSpecificMetadata(PropInfo& info, const PropRef& type) {
    static_cast<void>(type);
    return *static_cast<PropRefMetadata*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141D50.
WaveformMetadata<float>& TypeSpecificMetadata(PropInfo& info, const Waveform<float>& type) {
    static_cast<void>(type);
    return *static_cast<WaveformMetadata<float>*>(info.mMetadata.Get());
}

// Reconstructed from eboot.elf at 0x141D70.
WaveformMetadata<Hmx::Color>& TypeSpecificMetadata(
    PropInfo& info,
    const Waveform<Hmx::Color>& type) {
    static_cast<void>(type);
    return *static_cast<WaveformMetadata<Hmx::Color>*>(info.mMetadata.Get());
}
