#include "entity/props/PropMetadata.h"

#include <cstring>
#include <limits>

#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/ResourceMetaData.h"
#include "utl/containers/Map.h"
#include "utl/data/DataUtl.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"

namespace {

// Whether the tag functions that need a component may be set. Name not in
// the reference map.
bool gNonStaticTagFunctionsAllowed = true;  // 0x19B0308

// The id of no object, which the object's static initializer (0x17E790)
// sets; each object that uses it has its own copy. Name not in the
// reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};  // 0x19E3CF0

// Converts the stored radians of the angle types for display. Name not in
// the reference map.
constexpr float kRadiansToDegrees = 57.29578F;
// Converts the Euler members' degrees back to radians. Name not in the
// reference map.
constexpr float kDegreesToRadians = 0.017453292F;

// The default range of a number property: the type's limits, or +-1e30 for
// float. Name not in the reference map.
template <typename T>
T NumberRangeMin() {
    return std::numeric_limits<T>::lowest();
}
template <typename T>
T NumberRangeMax() {
    return std::numeric_limits<T>::max();
}
template <>
float NumberRangeMin<float>() {
    return -1e30F;
}
template <>
float NumberRangeMax<float>() {
    return 1e30F;
}

// The name of the path's last node, or its index. Shared by the display
// name functions. Name not in the reference map.
const PropPath::Node& LastNode(const PropPath& path) {
    return path[-1];
}

// Appends (key value) to a Dump array. Inlined into every Dump. Name not
// in the reference map.
void DumpValue(DataArray* dump, const char* key, const DataNode& value) {
    dump->Insert(dump->Size(), DataNode(MakeDataArray(DataNode(key), value), kDataArray));
}

// Appends (key value "bool"), with the value as an integer. Inlined into
// every Dump. Name not in the reference map.
void DumpFlag(DataArray* dump, const char* key, bool value) {
    dump->Insert(
        dump->Size(),
        DataNode(MakeDataArray(DataNode(key), DataNode(static_cast<int>(value)), DataNode("bool")), kDataArray));
}

// The printf conversion IntMetadata<T>::FormatValue formats a T with. Name
// not in the reference map.
template <typename T>
const char* IntFormat();
template <>
const char* IntFormat<signed char>() {
    return "%d";
}
template <>
const char* IntFormat<short>() {
    return "%d";
}
template <>
const char* IntFormat<int>() {
    return "%d";
}
template <>
const char* IntFormat<long>() {
    return "%lld";
}
template <>
const char* IntFormat<unsigned char>() {
    return "%u";
}
template <>
const char* IntFormat<unsigned short>() {
    return "%u";
}
template <>
const char* IntFormat<unsigned int>() {
    return "%u";
}
template <>
const char* IntFormat<unsigned long>() {
    return "%llu";
}

// The characters _GetAllowedValuesHelp escapes with a backslash in the
// labels (0x1250E2B), the backslash first. Name not in the reference map.
const char kHelpSpecialChars[] = "\\!\"#$%&'()*+,-./:;<=>?@[]^_`{|}~";

// The blend modes' names (0x18E8790), the inlined
// WaveformResource<T>::BlendModeToString of the waveform Dumps; an unknown
// mode has an empty name. Name not in the reference map.
const char* const kWaveformBlendModeNames[] = {
    "Even blend",
    "Sloped blend",
    "Noise",
    "Even blend (HSV)",
    "Sloped blend (HSV)",
    "Noise (HSV)",
};

const char* WaveformBlendModeName(int mode) {
    if (static_cast<unsigned long>(static_cast<long>(mode)) < 6) {
        return kWaveformBlendModeNames[mode];
    }
    return "";
}

// Whether a waveform of the type may use the blend mode: the HSV modes
// are for colors only. Name not in the reference map.
template <class T>
bool WaveformAllowsBlendMode(int mode);
template <>
bool WaveformAllowsBlendMode<float>(int mode) {
    return !IsHSVWaveformBlendMode(mode);
}
template <>
bool WaveformAllowsBlendMode<Hmx::Color>(int mode) {
    static_cast<void>(mode);
    return true;
}

}  // namespace

// The label of the first allowed value equal to `value`, in the MakeString
// buffer, or null when there is none or it has no label. The binary emits
// a copy for each metadata class, listed with the explicit instantiations.
// Name not in the reference map.
template <class M, class T>
const char* GetAllowedValueName(const M& metadata, const Component& component, const PropPath& path, const T& value) {
    eastl::vector<AllowedValue<T>> values;
    if (!metadata.mAllowedValues(component, path, values)) {
        return nullptr;
    }
    for (const AllowedValue<T>& allowed : values) {
        if (TagValueEquals(allowed.mValue, value)) {
            if (allowed.mName.c_str()[0] == '\0') {
                return nullptr;
            }
            FormatString format(allowed.mName.c_str());
            return format.Str();
        }
    }
    return nullptr;
}

// The editor's help for the allowed values, when they are shown: a line
// "- label" for each, with ": description" when it has one. A value
// without a label is named by the first component's ValueToString; the
// label's punctuation is escaped with backslashes.
template <class M>
String _GetAllowedValuesHelp(
    const M& metadata,
    const eastl::vector<Component*>& components,
    const PropPath& path) {
    String help;
    decltype(metadata.mAllowedValues.mValue) values;
    if (metadata.mShowAllowedValues && metadata.mAllowedValues(components, path, values, nullptr)) {
        for (const auto& allowed : values) {
            const char* label = allowed.mName.c_str();
            if (label[0] == '\0') {
                label = metadata.ValueToString(*components[0], path, &allowed.mValue, false);
            }
            String text(label);
            if (text.c_str()[0] == '\0') {
                continue;
            }
            for (int i = 0; i < 32; ++i) {
                const char escaped[] = {'\\', kHelpSpecialChars[i], '\0'};
                text.replace(kHelpSpecialChars[i], escaped);
            }
            FormatString item("- %s");
            item << text;
            help += item.Str();
            if (allowed.mDescription.c_str()[0] != '\0') {
                FormatString description(": %s");
                description << allowed.mDescription.c_str();
                help += description.Str();
            }
            help += '\n';
        }
    }
    return help;
}

// The allowed values' help as ("allowed_values_help" help), when the
// values are constant and the help is not empty.
template <class M>
DataArrayPtr _DumpAllowedValuesHelp(const M& metadata, const Component& component, const PropPath& path) {
    DataArrayPtr dump;
    if (metadata.mAllowedValues.mMode == TagBase<decltype(metadata.mAllowedValues.mValue)>::kModeConstant) {
        eastl::vector<Component*> components;
        components.push_back(const_cast<Component*>(&component));
        const String help = metadata.GetAllowedValuesHelp(components, path);
        if (help.c_str()[0] != '\0') {
            dump = MakeDataArray(DataNode("allowed_values_help"), DataNode(help));
        }
    }
    return dump;
}

// The object's statics, in the order of its static initializer
// (0x17E790): the null object id, the display name functions and the
// members' registries of the vector, angle and transform types.

// The property's name made readable: an index as a number, a name with
// spaces for its underscores and lowercase, and the words of a struct or
// array capitalized; a bool ends in "?". Names are cached by symbol. The
// lambda's call operator is at 0x14D1A0.
std::function<String(const TagArgs&)> DefaultDisplayName = [](const TagArgs& args) -> String {
    const PropPath::Node& node = LastNode(args.mPath);
    if (node.mType == PropPath::Node::kNodeIndex) {
        FormatString format("%d");
        format << node.mValue;
        return String(format.Str());
    }
    static eastl::map<const char*, String> sNames;
    const char* const key = node.NameStr();
    const auto cached = sNames.find(key);
    if (cached != sNames.end()) {
        return String(cached->second.c_str());
    }
    String name(Symbol(node.NameStr()));
    for (char* c = const_cast<char*>(name.c_str()); *c != '\0'; ++c) {
        if (*c == '_') {
            *c = ' ';
        }
    }
    name.ToLower();
    Component& component = const_cast<Component&>(args.mComponent);
    const PropInfo* const info = component._GetPropRegistry().FindProp(args.mPath, nullptr);
    if (info->mType == kPropertyStruct || (info->mType & kPropertyArray) != 0) {
        String upper(name);
        upper.ToUpper();
        name[0] = upper[0];
        for (unsigned long i = 1; i < strlen(name.c_str()); ++i) {
            if (name[i - 1] == ' ') {
                name[i] = upper[i];
            }
        }
    }
    if (info->mType == kPropertyBool) {
        name += "?";
    }
    sNames[key] = name.c_str();
    return String(static_cast<String&&>(name));
};

// Without a component the name is the last node as it is. The lambda's
// body is at 0x1437A0.
String (*StaticDefaultDisplayName)(const StaticTagArgs& args) = [](const StaticTagArgs& args) {
    const PropPath::Node& node = LastNode(args.mPath);
    if (node.mType == PropPath::Node::kNodeIndex) {
        FormatString format("%d");
        format << node.mValue;
        return String(format.Str());
    }
    return String(node.NameStr());
};
PropRegistry Vector3Metadata::sRegistry;
PropRegistry EulerMetadata::sRegistry;
PropRegistry XfmMetadata::sRegistry;
Vector3 EulerMetadata::sZeroAngles;
// Filled by the static initializer at 0x2195C0, among the math objects'.
PropXfm PropXfm::sIdentity = {{1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}};

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

// Reconstructed from eboot.elf at 0x133860.
DataArrayPtr PropMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump(new DataArray(0));
    if (mDisplayName.mMode == TagBase<String>::kModeConstant) {
        DumpValue(dump, "display_name", DataNode(mDisplayName.mValue));
    }
    if (mHelp.mMode == TagBase<String>::kModeConstant && mHelp.mValue.c_str()[0] != '\0') {
        DumpValue(dump, "help", DataNode(mHelp.mValue));
    }
    if (mGroup.mMode == TagBase<Symbol>::kModeConstant && mGroup.mValue != Symbol()) {
        DumpValue(dump, "group", DataNode(mGroup.mValue));
    }
    if (mIsHidden.mMode == TagBase<bool>::kModeConstant && mIsHidden.mValue) {
        DumpFlag(dump, "is_hidden", mIsHidden.mValue);
    }
    if (mReadOnly.mMode == TagBase<bool>::kModeConstant && mReadOnly.mValue) {
        DumpFlag(dump, "read_only", mReadOnly.mValue);
    }
    if (mDebug.mMode == TagBase<bool>::kModeConstant && mDebug.mValue) {
        DumpFlag(dump, "debug", mDebug.mValue);
    }
    if (mAnimatable.mMode == TagBase<bool>::kModeConstant && !mAnimatable.mValue) {
        DumpFlag(dump, "animatable", mAnimatable.mValue);
    }
    if (mNoTest) {
        DumpFlag(dump, "no_test", mNoTest);
    }
    if (!mSaves) {
        DumpFlag(dump, "saves", mSaves);
    }
    if (mInstanceAccess == kInstanceAccessView) {
        DumpFlag(dump, "view_on_instance", true);
    } else if (mInstanceAccess == kInstanceAccessSet) {
        DumpFlag(dump, "set_on_instance", true);
    }
    const DataArrayPtr help = DumpAllowedValuesHelp(component, path);
    if (help != nullptr) {
        dump->Insert(dump->Size(), DataNode(help));
    }
    return dump;
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

// Reconstructed from eboot.elf at 0x143C10.
const char* GroupMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    static_cast<void>(useNames);
    return "struct";
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

// Reconstructed from eboot.elf at 0x12ED70. Number, bool, symbol, struct,
// vector and array metadata are constructed inline; the other classes call
// their constructors. Types 14 and 17 have none here: an action's metadata
// is made by RegisterAction.
PropMetadata* PropMetadata::Create(PropertyType type) {
    if ((type & kPropertyArray) != 0) {
        return new ArrayMetadata();
    }
    switch (type) {
    case kPropertyFloat:
        return new FloatMetadata();
    case kPropertyInt8:
        return new IntMetadata<signed char>();
    case kPropertyInt16:
        return new IntMetadata<short>();
    case kPropertyInt:
        return new IntMetadata<int>();
    case kPropertyInt64:
        return new IntMetadata<long>();
    case kPropertyUInt8:
        return new IntMetadata<unsigned char>();
    case kPropertyUInt16:
        return new IntMetadata<unsigned short>();
    case kPropertyUInt:
        return new IntMetadata<unsigned int>();
    case kPropertyUInt64:
        return new IntMetadata<unsigned long>();
    case kPropertyBool:
        return new BoolMetadata();
    case kPropertyGameObjectId:
        return new GameObjectIdMetadata();
    case kPropertySymbol:
        return new SymbolMetadata();
    case kPropertyResourcePath:
        return new ResourcePathMetadata();
    case kPropertyColor:
        return new ColorMetadata();
    case kPropertyStruct:
        return new GroupMetadata();
    case kPropertyPropRef:
        return new PropRefMetadata();
    case kPropertyWaveformFloat:
        return new FloatWaveformMetadata();
    case kPropertyWaveformColor:
        return new ColorWaveformMetadata();
    case kPropertyVector3:
        return new Vector3Metadata();
    case kPropertyEuler:
        return new EulerMetadata();
    case kPropertyXfm:
        return new XfmMetadata();
    default:
        return nullptr;
    }
}

// Reconstructed from eboot.elf at 0x136E20.
template <>
bool AggUseNullIfConflicted<Symbol>(const Symbol& value, unsigned long index, Symbol& result) {
    if (index == 0) {
        result = value;
    } else if (value != result) {
        result = Symbol(nullptr);
    }
    return result != Symbol(nullptr);
}

// Reconstructed from eboot.elf at 0x135EF0.
template <>
bool AggUseNullIfConflicted<Entity*>(Entity* const& value, unsigned long index, Entity*& result) {
    if (index == 0) {
        result = value;
    } else if (value != result) {
        result = nullptr;
    }
    return result != nullptr;
}

// Reconstructed from eboot.elf at 0x139220.
template <>
bool AggUseNullIfConflicted<GameObjectId>(
    const GameObjectId& value,
    unsigned long index,
    GameObjectId& result) {
    if (index == 0) {
        result = value;
    } else if (value.mId != result.mId) {
        result = gNullObjectId;
    }
    return result.mId != gNullObjectId.mId;
}

// Reconstructed from eboot.elf at 0x13CD80. Of two resource classes the one
// deriving from the other is kept; unrelated classes give "_conflicted_",
// which stops the aggregation. An empty id takes the other.
bool AggUseMostSpecificResourceId(const Symbol& value, unsigned long index, Symbol& result) {
    static_cast<void>(index);
    static Symbol sConflicted;
    if (sConflicted == Symbol()) {
        sConflicted = Symbol("_conflicted_");
    }
    if (value == Symbol()) {
        return result != sConflicted;
    }
    if (value != result) {
        if (result == Symbol()) {
            result = value;
        } else {
            const ResourceMetaData* valueClass = ResourceMetaData::GetMetaData(value, false);
            const ResourceMetaData* resultClass = ResourceMetaData::GetMetaData(result, false);
            for (; valueClass != nullptr; valueClass = valueClass->mParent) {
                if (valueClass->mId == result) {
                    result = value;
                    return result != sConflicted;
                }
            }
            for (; resultClass != nullptr; resultClass = resultClass->mParent) {
                if (resultClass->mId == value) {
                    return result != sConflicted;
                }
            }
            result = sConflicted;
        }
    }
    return result != sConflicted;
}

// The number metadata's constructor; the binary emits a copy for each type.
template <typename T>
NumberMetadata<T>::NumberMetadata()
    : mAllowedValues(AggVectorIntersection<AllowedValue<T>>),
      mShowAllowedValues(true),
      mDefault(T(), AggUseFirstResult<T>),
      mMin(NumberRangeMin<T>(), AggUseMax<T>),
      mMax(NumberRangeMax<T>(), AggUseMin<T>),
      mSlider(false, AggTrueIfAllTrue),
      mSliderLogResolution(AggUseAverage),
      mReserved(AggUseFirstResult<long>),
      mPrefix(Symbol(""), AggUseNullIfConflicted<Symbol>),
      mUnits(Symbol(""), AggUseNullIfConflicted<Symbol>),
      mIncrement(AggUseAverage),
      mEnabled(true, AggTrueIfAllTrue) {}

template <typename T>
NumberMetadata<T>::~NumberMetadata() {}

template <typename T>
IntMetadata<T>::~IntMetadata() {}

template <typename T>
bool NumberMetadata<T>::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    const T& number = *static_cast<const T*>(value);
    eastl::vector<AllowedValue<T>> values;
    if (mAllowedValues(component, path, values)) {
        for (const AllowedValue<T>& allowed : values) {
            if (allowed.mValue == number) {
                return true;
            }
        }
        return false;
    }
    if (number < mMin.Get(component, path)) {
        return false;
    }
    return number <= mMax.Get(component, path);
}

template <typename T>
DataArrayPtr NumberMetadata<T>::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mMin.mMode == TagBase<T>::kModeConstant && mMin.mValue != NumberRangeMin<T>()) {
        DumpValue(dump, "min", DataNode(static_cast<float>(mMin.mValue)));
    }
    if (mMax.mMode == TagBase<T>::kModeConstant && mMax.mValue != NumberRangeMax<T>()) {
        DumpValue(dump, "max", DataNode(static_cast<float>(mMax.mValue)));
    }
    if (mPrefix.mMode == TagBase<Symbol>::kModeConstant && mPrefix.mValue != Symbol()) {
        DumpValue(dump, "prefix", DataNode(mPrefix.mValue));
    }
    if (mUnits.mMode == TagBase<Symbol>::kModeConstant && mUnits.mValue != Symbol()) {
        DumpValue(dump, "units", DataNode(mUnits.mValue));
    }
    return dump;
}

template <typename T>
String NumberMetadata<T>::GetAllowedValuesHelp(
    const eastl::vector<Component*>& components,
    const PropPath& path) const {
    return _GetAllowedValuesHelp(*this, components, path);
}

template <typename T>
DataArrayPtr NumberMetadata<T>::DumpAllowedValuesHelp(const Component& component, const PropPath& path) const {
    return _DumpAllowedValuesHelp(*this, component, path);
}

template <typename T>
const char* IntMetadata<T>::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    return ValueToString(component, path, *static_cast<const T*>(value), useNames);
}

template <typename T>
bool IntMetadata<T>::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    return NumberMetadata<T>::IsValueAllowed(component, path, value);
}

template <typename T>
DataArrayPtr IntMetadata<T>::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = NumberMetadata<T>::Dump(component, path);
    if (mBitfield.mMode == TagBase<bool>::kModeConstant && mBitfield.mValue) {
        DumpFlag(dump, "prefix", mBitfield.mValue);
    }
    return dump;
}

template <typename T>
const char* IntMetadata<T>::ValueToString(
    const Component& component,
    const PropPath& path,
    const T& value,
    bool useNames) const {
    if (mBitfield.Get(component, path)) {
        eastl::vector<AllowedValue<T>> values;
        if (!this->mAllowedValues(component, path, values)) {
            return FormatValue(component, path, value);
        }
        FixedString text(MakeStringBuf(), 4096);
        const char* result = "<None>";
        bool all = !values.empty();
        bool any = false;
        for (const AllowedValue<T>& allowed : values) {
            if ((allowed.mValue & value) != 0) {
                if (text.c_str()[0] != '\0') {
                    text += "|";
                }
                const char* label = allowed.mName.c_str();
                if (label[0] == '\0') {
                    label = FormatValue(component, path, allowed.mValue);
                }
                text += label;
                any = true;
            } else {
                all = false;
            }
        }
        if (any) {
            result = all ? "<All>" : text.c_str();
        }
        return result;
    }
    if (useNames) {
        const char* const name = GetAllowedValueName<NumberMetadata<T>, T>(*this, component, path, value);
        if (name != nullptr) {
            return name;
        }
    }
    return FormatValue(component, path, value);
}

template <typename T>
const char* IntMetadata<T>::FormatValue(const Component& component, const PropPath& path, T value) const {
    Symbol units = this->mUnits.Get(component, path);
    if (units != Symbol()) {
        FormatString spaced(" %s");
        spaced << units;
        units = Symbol(spaced.Str());
    }
    FormatString numberFormat("%%s%s%%s");
    numberFormat << IntFormat<T>();
    const char* const format = numberFormat.Str();
    const Symbol prefix = this->mPrefix.Get(component, path);
    FormatString text(format);
    text << prefix << value << units;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x134E50.
template NumberMetadata<float>::NumberMetadata();
// Reconstructed from eboot.elf at 0x14DA30.
template NumberMetadata<signed char>::NumberMetadata();
// Reconstructed from eboot.elf at 0x1512F0.
template NumberMetadata<short>::NumberMetadata();
// Reconstructed from eboot.elf at 0x155310.
template NumberMetadata<int>::NumberMetadata();
// Reconstructed from eboot.elf at 0x158CB0.
template NumberMetadata<long>::NumberMetadata();
// Reconstructed from eboot.elf at 0x15CC80.
template NumberMetadata<unsigned char>::NumberMetadata();
// Reconstructed from eboot.elf at 0x160510.
template NumberMetadata<unsigned short>::NumberMetadata();
// Reconstructed from eboot.elf at 0x164520.
template NumberMetadata<unsigned int>::NumberMetadata();
// Reconstructed from eboot.elf at 0x167DB0.
template NumberMetadata<unsigned long>::NumberMetadata();

// Reconstructed from eboot.elf at 0x171C30.
template NumberMetadata<float>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x14E860.
template NumberMetadata<signed char>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x152130.
template NumberMetadata<short>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x156110.
template NumberMetadata<int>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x159AE0.
template NumberMetadata<long>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x15DAA0.
template NumberMetadata<unsigned char>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x161350.
template NumberMetadata<unsigned short>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x165340.
template NumberMetadata<unsigned int>::~NumberMetadata();
// Reconstructed from eboot.elf at 0x168BD0.
template NumberMetadata<unsigned long>::~NumberMetadata();

// Reconstructed from eboot.elf at 0x14DE10.
template IntMetadata<signed char>::~IntMetadata();
// Reconstructed from eboot.elf at 0x1516D0.
template IntMetadata<short>::~IntMetadata();
// Reconstructed from eboot.elf at 0x1556F0.
template IntMetadata<int>::~IntMetadata();
// Reconstructed from eboot.elf at 0x1590A0.
template IntMetadata<long>::~IntMetadata();
// Reconstructed from eboot.elf at 0x15D060.
template IntMetadata<unsigned char>::~IntMetadata();
// Reconstructed from eboot.elf at 0x1608F0.
template IntMetadata<unsigned short>::~IntMetadata();
// Reconstructed from eboot.elf at 0x164900.
template IntMetadata<unsigned int>::~IntMetadata();
// Reconstructed from eboot.elf at 0x168190.
template IntMetadata<unsigned long>::~IntMetadata();

// Reconstructed from eboot.elf at 0x135750.
template bool NumberMetadata<float>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x14E890.
template bool NumberMetadata<signed char>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x152160.
template bool NumberMetadata<short>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x156140.
template bool NumberMetadata<int>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x159B10.
template bool NumberMetadata<long>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x15DAD0.
template bool NumberMetadata<unsigned char>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x161380.
template bool NumberMetadata<unsigned short>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x165370.
template bool NumberMetadata<unsigned int>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x168C00.
template bool NumberMetadata<unsigned long>::IsValueAllowed(const Component&, const PropPath&, const void*) const;

// Reconstructed from eboot.elf at 0x1427B0.
template DataArrayPtr NumberMetadata<float>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x14EAF0.
template DataArrayPtr NumberMetadata<signed char>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1523C0.
template DataArrayPtr NumberMetadata<short>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1563A0.
template DataArrayPtr NumberMetadata<int>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x159D70.
template DataArrayPtr NumberMetadata<long>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x15DD30.
template DataArrayPtr NumberMetadata<unsigned char>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1615E0.
template DataArrayPtr NumberMetadata<unsigned short>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1655D0.
template DataArrayPtr NumberMetadata<unsigned int>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x168E60.
template DataArrayPtr NumberMetadata<unsigned long>::Dump(const Component&, const PropPath&) const;

// Reconstructed from eboot.elf at 0x131FA0.
template String NumberMetadata<float>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x12F9A0.
template String NumberMetadata<signed char>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&)
    const;
// Reconstructed from eboot.elf at 0x12FE60.
template String NumberMetadata<short>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x130320.
template String NumberMetadata<int>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1307E0.
template String NumberMetadata<long>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x130CA0.
template String NumberMetadata<unsigned char>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&)
    const;
// Reconstructed from eboot.elf at 0x131160.
template String NumberMetadata<unsigned short>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&)
    const;
// Reconstructed from eboot.elf at 0x131620.
template String NumberMetadata<unsigned int>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&)
    const;
// Reconstructed from eboot.elf at 0x131AE0.
template String NumberMetadata<unsigned long>::GetAllowedValuesHelp(const eastl::vector<Component*>&, const PropPath&)
    const;

// Reconstructed from eboot.elf at 0x132260.
template DataArrayPtr NumberMetadata<float>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x12FC60.
template DataArrayPtr NumberMetadata<signed char>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x130120.
template DataArrayPtr NumberMetadata<short>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1305E0.
template DataArrayPtr NumberMetadata<int>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x130AA0.
template DataArrayPtr NumberMetadata<long>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x130F60.
template DataArrayPtr NumberMetadata<unsigned char>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x131420.
template DataArrayPtr NumberMetadata<unsigned short>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1318E0.
template DataArrayPtr NumberMetadata<unsigned int>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x131DA0.
template DataArrayPtr NumberMetadata<unsigned long>::DumpAllowedValuesHelp(const Component&, const PropPath&) const;

// Reconstructed from eboot.elf at 0x14DFE0.
template const char* IntMetadata<signed char>::ValueToString(const Component&, const PropPath&, const void*, bool) const;
// Reconstructed from eboot.elf at 0x1518A0.
template const char* IntMetadata<short>::ValueToString(const Component&, const PropPath&, const void*, bool) const;
// Reconstructed from eboot.elf at 0x1558C0.
template const char* IntMetadata<int>::ValueToString(const Component&, const PropPath&, const void*, bool) const;
// Reconstructed from eboot.elf at 0x159270.
template const char* IntMetadata<long>::ValueToString(const Component&, const PropPath&, const void*, bool) const;
// Reconstructed from eboot.elf at 0x15D230.
template const char* IntMetadata<unsigned char>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;
// Reconstructed from eboot.elf at 0x160AC0.
template const char* IntMetadata<unsigned short>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;
// Reconstructed from eboot.elf at 0x164AD0.
template const char* IntMetadata<unsigned int>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;
// Reconstructed from eboot.elf at 0x168360.
template const char* IntMetadata<unsigned long>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;

// Reconstructed from eboot.elf at 0x14DFF0.
template bool IntMetadata<signed char>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x1518B0.
template bool IntMetadata<short>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x1558D0.
template bool IntMetadata<int>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x159280.
template bool IntMetadata<long>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x15D240.
template bool IntMetadata<unsigned char>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x160AD0.
template bool IntMetadata<unsigned short>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x164AE0.
template bool IntMetadata<unsigned int>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x168370.
template bool IntMetadata<unsigned long>::IsValueAllowed(const Component&, const PropPath&, const void*) const;

// Reconstructed from eboot.elf at 0x14E000.
template DataArrayPtr IntMetadata<signed char>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1518C0.
template DataArrayPtr IntMetadata<short>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x1558E0.
template DataArrayPtr IntMetadata<int>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x159290.
template DataArrayPtr IntMetadata<long>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x15D250.
template DataArrayPtr IntMetadata<unsigned char>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x160AE0.
template DataArrayPtr IntMetadata<unsigned short>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x164AF0.
template DataArrayPtr IntMetadata<unsigned int>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x168380.
template DataArrayPtr IntMetadata<unsigned long>::Dump(const Component&, const PropPath&) const;

// Reconstructed from eboot.elf at 0x150BB0.
template const char* IntMetadata<signed char>::ValueToString(
    const Component&,
    const PropPath&,
    const signed char&,
    bool) const;
// Reconstructed from eboot.elf at 0x154BE0.
template const char* IntMetadata<short>::ValueToString(const Component&, const PropPath&, const short&, bool) const;
// Reconstructed from eboot.elf at 0x158590.
template const char* IntMetadata<int>::ValueToString(const Component&, const PropPath&, const int&, bool) const;
// Reconstructed from eboot.elf at 0x15C560.
template const char* IntMetadata<long>::ValueToString(const Component&, const PropPath&, const long&, bool) const;
// Reconstructed from eboot.elf at 0x15FDF0.
template const char* IntMetadata<unsigned char>::ValueToString(
    const Component&,
    const PropPath&,
    const unsigned char&,
    bool) const;
// Reconstructed from eboot.elf at 0x163DF0.
template const char* IntMetadata<unsigned short>::ValueToString(
    const Component&,
    const PropPath&,
    const unsigned short&,
    bool) const;
// Reconstructed from eboot.elf at 0x167690.
template const char* IntMetadata<unsigned int>::ValueToString(
    const Component&,
    const PropPath&,
    const unsigned int&,
    bool) const;
// Reconstructed from eboot.elf at 0x16B680.
template const char* IntMetadata<unsigned long>::ValueToString(
    const Component&,
    const PropPath&,
    const unsigned long&,
    bool) const;

// Reconstructed from eboot.elf at 0x150EE0.
template const char* IntMetadata<signed char>::FormatValue(const Component&, const PropPath&, signed char) const;
// Reconstructed from eboot.elf at 0x154F10.
template const char* IntMetadata<short>::FormatValue(const Component&, const PropPath&, short) const;
// Reconstructed from eboot.elf at 0x1588C0.
template const char* IntMetadata<int>::FormatValue(const Component&, const PropPath&, int) const;
// Reconstructed from eboot.elf at 0x15C890.
template const char* IntMetadata<long>::FormatValue(const Component&, const PropPath&, long) const;
// Reconstructed from eboot.elf at 0x160120.
template const char* IntMetadata<unsigned char>::FormatValue(const Component&, const PropPath&, unsigned char) const;
// Reconstructed from eboot.elf at 0x164120.
template const char* IntMetadata<unsigned short>::FormatValue(const Component&, const PropPath&, unsigned short) const;
// Reconstructed from eboot.elf at 0x1679C0.
template const char* IntMetadata<unsigned int>::FormatValue(const Component&, const PropPath&, unsigned int) const;
// Reconstructed from eboot.elf at 0x16B9B0.
template const char* IntMetadata<unsigned long>::FormatValue(const Component&, const PropPath&, unsigned long) const;

// Reconstructed from eboot.elf at 0x1355E0.
template const char* GetAllowedValueName(
    const NumberMetadata<float>&,
    const Component&,
    const PropPath&,
    const float&);
// Reconstructed from eboot.elf at 0x151190.
template const char* GetAllowedValueName(
    const NumberMetadata<signed char>&,
    const Component&,
    const PropPath&,
    const signed char&);
// Reconstructed from eboot.elf at 0x1551B0.
template const char* GetAllowedValueName(const NumberMetadata<short>&, const Component&, const PropPath&, const short&);
// Reconstructed from eboot.elf at 0x158B60.
template const char* GetAllowedValueName(const NumberMetadata<int>&, const Component&, const PropPath&, const int&);
// Reconstructed from eboot.elf at 0x15CB30.
template const char* GetAllowedValueName(const NumberMetadata<long>&, const Component&, const PropPath&, const long&);
// Reconstructed from eboot.elf at 0x1603C0.
template const char* GetAllowedValueName(
    const NumberMetadata<unsigned char>&,
    const Component&,
    const PropPath&,
    const unsigned char&);
// Reconstructed from eboot.elf at 0x1643C0.
template const char* GetAllowedValueName(
    const NumberMetadata<unsigned short>&,
    const Component&,
    const PropPath&,
    const unsigned short&);
// Reconstructed from eboot.elf at 0x167C60.
template const char* GetAllowedValueName(
    const NumberMetadata<unsigned int>&,
    const Component&,
    const PropPath&,
    const unsigned int&);
// Reconstructed from eboot.elf at 0x16BC50.
template const char* GetAllowedValueName(
    const NumberMetadata<unsigned long>&,
    const Component&,
    const PropPath&,
    const unsigned long&);
// Reconstructed from eboot.elf at 0x13BEF0.
template const char* GetAllowedValueName(const SymbolMetadata&, const Component&, const PropPath&, const Symbol&);
// Reconstructed from eboot.elf at 0x13D5A0.
template const char* GetAllowedValueName(
    const ResourcePathMetadata&,
    const Component&,
    const PropPath&,
    const ResourcePath&);
// Reconstructed from eboot.elf at 0x140100.
template const char* GetAllowedValueName(const ColorMetadata&, const Component&, const PropPath&, const Hmx::Color&);

// Reconstructed from eboot.elf at 0x131FC0.
template String _GetAllowedValuesHelp(
    const NumberMetadata<float>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x12F9C0.
template String _GetAllowedValuesHelp(
    const NumberMetadata<signed char>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x12FE80.
template String _GetAllowedValuesHelp(
    const NumberMetadata<short>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x130340.
template String _GetAllowedValuesHelp(const NumberMetadata<int>&, const eastl::vector<Component*>&, const PropPath&);
// Reconstructed from eboot.elf at 0x130800.
template String _GetAllowedValuesHelp(const NumberMetadata<long>&, const eastl::vector<Component*>&, const PropPath&);
// Reconstructed from eboot.elf at 0x130CC0.
template String _GetAllowedValuesHelp(
    const NumberMetadata<unsigned char>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x131180.
template String _GetAllowedValuesHelp(
    const NumberMetadata<unsigned short>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x131640.
template String _GetAllowedValuesHelp(
    const NumberMetadata<unsigned int>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x131B00.
template String _GetAllowedValuesHelp(
    const NumberMetadata<unsigned long>&,
    const eastl::vector<Component*>&,
    const PropPath&);
// Reconstructed from eboot.elf at 0x132480.
template String _GetAllowedValuesHelp(const SymbolMetadata&, const eastl::vector<Component*>&, const PropPath&);
// Reconstructed from eboot.elf at 0x132940.
template String _GetAllowedValuesHelp(const ResourcePathMetadata&, const eastl::vector<Component*>&, const PropPath&);
// Reconstructed from eboot.elf at 0x132E00.
template String _GetAllowedValuesHelp(const ColorMetadata&, const eastl::vector<Component*>&, const PropPath&);

// Reconstructed from eboot.elf at 0x132280.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<float>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x12FC80.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<signed char>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x130140.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<short>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x130600.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<int>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x130AC0.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<long>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x130F80.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<unsigned char>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x131440.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<unsigned short>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x131900.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<unsigned int>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x131DC0.
template DataArrayPtr _DumpAllowedValuesHelp(const NumberMetadata<unsigned long>&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x132740.
template DataArrayPtr _DumpAllowedValuesHelp(const SymbolMetadata&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x132C00.
template DataArrayPtr _DumpAllowedValuesHelp(const ResourcePathMetadata&, const Component&, const PropPath&);
// Reconstructed from eboot.elf at 0x1330C0.
template DataArrayPtr _DumpAllowedValuesHelp(const ColorMetadata&, const Component&, const PropPath&);

// Reconstructed from eboot.elf at 0x142790.
const char* FloatMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    return ValueToString(component, path, *static_cast<const float*>(value), useNames);
}

// Reconstructed from eboot.elf at 0x1427A0.
bool FloatMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return NumberMetadata<float>::IsValueAllowed(component, path, value);
}

// Reconstructed from eboot.elf at 0x135250.
const char* FloatMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const float& value,
    bool useNames) const {
    if (useNames) {
        const char* const name = GetAllowedValueName<NumberMetadata<float>, float>(*this, component, path, value);
        if (name != nullptr) {
            return name;
        }
    }
    Symbol units = mUnits.Get(component, path);
    if (units != Symbol()) {
        FormatString spaced(" %s");
        spaced << units;
        units = Symbol(spaced.Str());
    }
    FormatString numberFormat("%%s%%.%df%%s");
    numberFormat << mDecimalPlaces.Get(component, path);
    const char* const format = numberFormat.Str();
    const Symbol prefix = mPrefix.Get(component, path);
    FormatString text(format);
    text << prefix << value << units;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x1425C0.
FloatMetadata::~FloatMetadata() {}

// Reconstructed from eboot.elf at 0x142120.
SymbolMetadata::~SymbolMetadata() {}

// Reconstructed from eboot.elf at 0x142300.
const char* SymbolMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    const Symbol& symbol = *static_cast<const Symbol*>(value);
    if (useNames) {
        const char* const name = GetAllowedValueName(*this, component, path, symbol);
        if (name != nullptr) {
            return name;
        }
    }
    return symbol.Str();
}

// Reconstructed from eboot.elf at 0x142330.
bool SymbolMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return IsValueAllowed(component, path, *static_cast<const Symbol*>(value));
}

// Reconstructed from eboot.elf at 0x132460.
String SymbolMetadata::GetAllowedValuesHelp(
    const eastl::vector<Component*>& components,
    const PropPath& path) const {
    return _GetAllowedValuesHelp(*this, components, path);
}

// Reconstructed from eboot.elf at 0x132720.
DataArrayPtr SymbolMetadata::DumpAllowedValuesHelp(const Component& component, const PropPath& path) const {
    return _DumpAllowedValuesHelp(*this, component, path);
}

// Reconstructed from eboot.elf at 0x13C040.
bool SymbolMetadata::IsValueAllowed(const Component& component, const PropPath& path, const Symbol& value) const {
    eastl::vector<AllowedValue<Symbol>> values;
    if (!mAllowedValues(component, path, values)) {
        return true;
    }
    for (const AllowedValue<Symbol>& allowed : values) {
        if (allowed.mValue == value) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x135AC0. The object may be null, the
// component's own object or a repeat by default.
GameObjectIdMetadata::GameObjectIdMetadata()
    : mSetter(nullptr),
      mOnChanged(nullptr),
      mGetter(nullptr),
      mReserved(nullptr),
      mReserved2(nullptr),
      mDefault(gNullObjectId, AggUseFirstResult<GameObjectId>),
      mEntity(AggUseNullIfConflicted<Entity*>),
      mAllowedObjects(AggVectorIntersection<GameObjectId>),
      mAllowsNull(true, AggTrueIfAllTrue),
      mAllowsThis(true, AggTrueIfAllTrue),
      mExcludedObjects(AggVectorUnion<GameObjectId>),
      mAllowsDuplicates(true, AggTrueIfAllTrue),
      mRequiredComs(AggVectorUnion<Symbol>),
      mRequiredInterfaces(AggVectorUnion<Symbol>),
      mFilterClass(AggUseNullIfConflicted<Symbol>) {
    mFilterClass.mValue = Symbol("");
}

// Reconstructed from eboot.elf at 0x142400.
GameObjectIdMetadata::~GameObjectIdMetadata() {}

// Reconstructed from eboot.elf at 0x142430.
const char* GameObjectIdMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(useNames);
    Entity* entity = component.mObject->mEntity;
    mEntity(component, path, entity);
    if (entity == nullptr) {
        return "<INVALID>";
    }
    const GameObjectId& id = *static_cast<const GameObjectId*>(value);
    if (id.mId == gNullObjectId.mId) {
        return "<None>";
    }
    const GameObject* const object = entity->GetObject(id);
    if (object == nullptr) {
        return "<None>";
    }
    return object->GetSafeName();
}

// Reconstructed from eboot.elf at 0x1424E0.
bool GameObjectIdMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return IsValueAllowed(component, path, *static_cast<const GameObjectId*>(value));
}

// Reconstructed from eboot.elf at 0x1384C0.
DataArrayPtr GameObjectIdMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mAllowsNull.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_null", mAllowsNull.mValue);
    }
    if (mAllowsThis.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_this", mAllowsThis.mValue);
    }
    if (mAllowsDuplicates.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_duplicates", mAllowsDuplicates.mValue);
    }
    if (mRequiredComs.mMode == TagBase<eastl::vector<Symbol>>::kModeConstant) {
        const DataArrayPtr coms(new DataArray(0));
        const eastl::vector<Symbol> required(mRequiredComs.mValue);
        for (const Symbol& com : required) {
            coms->Insert(coms->Size(), DataNode(com));
        }
        DumpValue(dump, "required_coms", DataNode(coms));
    }
    return dump;
}

// Reconstructed from eboot.elf at 0x136F40.
bool GameObjectIdMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const GameObjectId& value) const {
    eastl::vector<Component*> components;
    components.push_back(const_cast<Component*>(&component));
    eastl::vector<GameObjectId> objects;
    if (!GetAllowedObjects(components, path, objects)) {
        return false;
    }
    for (const GameObjectId& object : objects) {
        if (object.mId == value.mId) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x13C370.
ResourcePathMetadata::ResourcePathMetadata()
    : mSetter(nullptr),
      mOnChanged(nullptr),
      mGetter(nullptr),
      mReserved(nullptr),
      mReserved2(nullptr),
      mDefault(ResourcePath(), AggUseFirstResult<ResourcePath>),
      mAllowedValues(AggVectorIntersection<AllowedValue<ResourcePath>>),
      mShowAllowedValues(true),
      mResourceId(Symbol(""), AggUseMostSpecificResourceId),
      mAllowsInlined(true, AggTrueIfAllTrue),
      mCustomLabel(AggUseNullIfConflicted<Symbol>),
      mCustomExtensions(AggVectorIntersection<Symbol>),
      mDefaultExtension(AggUseFirstResult<Symbol>),
      mEntity(AggUseFirstResult<Entity*>),
      mAllowsEmpty(true, AggTrueIfAnyTrue),
      mAllowsUntyped(false),
      mFlag1(false),
      mIsRelativeToEntity(false) {
    mCustomLabel.mValue = Symbol("");
    mDefaultExtension.mValue = Symbol("");
}

// Reconstructed from eboot.elf at 0x142340.
ResourcePathMetadata::~ResourcePathMetadata() {}

// Reconstructed from eboot.elf at 0x142370.
const char* ResourcePathMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    const ResourcePath& file = *static_cast<const ResourcePath*>(value);
    if (useNames) {
        const char* const name = GetAllowedValueName(*this, component, path, file);
        if (name != nullptr) {
            return name;
        }
    }
    return file.Str();
}

// Reconstructed from eboot.elf at 0x1423A0.
bool ResourcePathMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return IsValueAllowed(component, path, *static_cast<const ResourcePath*>(value));
}

// Reconstructed from eboot.elf at 0x13EE30.
DataArrayPtr ResourcePathMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mResourceId.mMode == TagBase<Symbol>::kModeConstant) {
        DumpValue(dump, "resource_id", DataNode(mResourceId.mValue));
    }
    if (mIsRelativeToEntity) {
        DumpFlag(dump, "is_relative_to_entity", mIsRelativeToEntity);
    }
    if (mCustomLabel.mMode == TagBase<Symbol>::kModeConstant) {
        DumpValue(dump, "custom_label", DataNode(mCustomLabel.mValue));
    }
    if (mCustomExtensions.mMode == TagBase<eastl::vector<Symbol>>::kModeConstant) {
        const DataArrayPtr extensions(new DataArray(0));
        const eastl::vector<Symbol> custom(mCustomExtensions.mValue);
        for (const Symbol& extension : custom) {
            extensions->Insert(extensions->Size(), DataNode(extension));
        }
        DumpValue(dump, "custom_extensions", DataNode(extensions));
    }
    return dump;
}

// Reconstructed from eboot.elf at 0x132920.
String ResourcePathMetadata::GetAllowedValuesHelp(
    const eastl::vector<Component*>& components,
    const PropPath& path) const {
    return _GetAllowedValuesHelp(*this, components, path);
}

// Reconstructed from eboot.elf at 0x132BE0.
DataArrayPtr ResourcePathMetadata::DumpAllowedValuesHelp(const Component& component, const PropPath& path) const {
    return _DumpAllowedValuesHelp(*this, component, path);
}

// Reconstructed from eboot.elf at 0x13E790. A path with a sub-object is
// not checked against the extensions.
bool ResourcePathMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const ResourcePath& value) const {
    if (value.mPath == Symbol()) {
        return true;
    }
    eastl::vector<AllowedValue<ResourcePath>> values;
    if (mAllowedValues(component, path, values)) {
        for (const AllowedValue<ResourcePath>& allowed : values) {
            if (allowed.mValue == value) {
                return true;
            }
        }
        return false;
    }
    ResourcePath file = value;
    Entity* entity = component.mObject->mEntity;
    mEntity(component, path, entity);
    bool hasSubObject = false;
    if (IsResourcePathInlined(value, entity)) {
        if (!mAllowsInlined.Get(component, path)) {
            return false;
        }
    } else {
        StackString<512> filePart;
        hasSubObject = value.GetFilepathAndSubObject(filePart, nullptr) != nullptr;
        if (hasSubObject) {
            file = filePart.c_str();
        }
    }
    eastl::vector<Component*> components;
    components.push_back(const_cast<Component*>(&component));
    eastl::vector<Symbol> extensions;
    GetValidExtensions(components, path, extensions, nullptr);
    if (!extensions.empty() && !hasSubObject) {
        const char* const extension = FileGetExt(file.Str(), false);
        for (const Symbol& valid : extensions) {
            if (strcasecmp(valid.Str(), extension) == 0) {
                return true;
            }
        }
        return false;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x13F620. White by default, without
// alpha.
ColorMetadata::ColorMetadata()
    : mSetter(nullptr),
      mOnChanged(nullptr),
      mGetter(nullptr),
      mReserved(nullptr),
      mReserved2(nullptr),
      mDefault(Hmx::Color::GetWhite(), AggUseFirstResult<Hmx::Color>),
      mAllowedValues(AggVectorIntersection<AllowedValue<Hmx::Color>>),
      mShowAllowedValues(true),
      mHasAlpha(false, AggTrueIfAllTrue) {}

// Reconstructed from eboot.elf at 0x1423B0.
ColorMetadata::~ColorMetadata() {}

// Reconstructed from eboot.elf at 0x1423E0.
const char* ColorMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    return ValueToString(component, path, *static_cast<const Hmx::Color*>(value), useNames);
}

// Reconstructed from eboot.elf at 0x1423F0.
bool ColorMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return IsValueAllowed(component, path, *static_cast<const Hmx::Color*>(value));
}

// Reconstructed from eboot.elf at 0x132DE0.
String ColorMetadata::GetAllowedValuesHelp(
    const eastl::vector<Component*>& components,
    const PropPath& path) const {
    return _GetAllowedValuesHelp(*this, components, path);
}

// Reconstructed from eboot.elf at 0x1330A0.
DataArrayPtr ColorMetadata::DumpAllowedValuesHelp(const Component& component, const PropPath& path) const {
    return _DumpAllowedValuesHelp(*this, component, path);
}

// Reconstructed from eboot.elf at 0x140000. The channels are rounded to
// 0-255.
const char* ColorMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const Hmx::Color& value,
    bool useNames) const {
    if (useNames) {
        const char* const name = GetAllowedValueName(*this, component, path, value);
        if (name != nullptr) {
            return name;
        }
    }
    Hmx::Color gamma(0.0F, 0.0F, 0.0F, 1.0F);
    LinearToGamma_sRGB(value, gamma);
    const int red = static_cast<int>(gamma.red * 255.0F + 0.5F);
    const int green = static_cast<int>(gamma.green * 255.0F + 0.5F);
    const int blue = static_cast<int>(gamma.blue * 255.0F + 0.5F);
    FormatString format("(R:%i G:%i B:%i)");
    format << red << green << blue;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x140280.
bool ColorMetadata::IsValueAllowed(const Component& component, const PropPath& path, const Hmx::Color& value) const {
    eastl::vector<AllowedValue<Hmx::Color>> values;
    if (!mAllowedValues(component, path, values)) {
        return true;
    }
    for (const AllowedValue<Hmx::Color>& allowed : values) {
        if (TagValueEquals(allowed.mValue, value)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x138D80. Read-only, non-animatable and
// empty references are allowed by default; arrays are not.
PropRefMetadata::PropRefMetadata()
    : mRefSetter(nullptr),
      mRefOnChanged(nullptr),
      mRefGetter(nullptr),
      mRefReserved(nullptr),
      mRefReserved2(nullptr),
      mRefDefault(PropRef(), AggUseFirstResult<PropRef>),
      mObject(AggUseNullIfConflicted<GameObjectId>),
      mAllowedTypes(AggVectorIntersection<PropertyType>),
      mExcludedTypes(AggVectorUnion<PropertyType>),
      mAllowedComs(AggUseFirstResult<eastl::vector<Symbol>>),
      mAllowsReadOnly(true, AggTrueIfAllTrue),
      mAllowsNonAnimatable(true, AggTrueIfAllTrue),
      mAllowsEmpty(true, AggTrueIfAllTrue),
      mAllowsDebug(false, AggTrueIfAllTrue) {
    mObject.mValue = gNullObjectId;
}

// Reconstructed from eboot.elf at 0x1424F0.
PropRefMetadata::~PropRefMetadata() {}

// Reconstructed from eboot.elf at 0x142520.
const char* PropRefMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(useNames);
    Entity* entity = component.mObject->mEntity;
    mEntity(component, path, entity);
    return static_cast<const PropRef*>(value)->ValueToString(entity);
}

// Reconstructed from eboot.elf at 0x142580.
bool PropRefMetadata::IsValueAllowed(const Component& component, const PropPath& path, const void* value) const {
    return IsValueAllowed(component, path, *static_cast<const PropRef*>(value));
}

// Reconstructed from eboot.elf at 0x13AE40.
DataArrayPtr PropRefMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = GameObjectIdMetadata::Dump(component, path);
    if (mAllowedTypes.mMode == TagBase<eastl::vector<PropertyType>>::kModeConstant) {
        const DataArrayPtr types(new DataArray(0));
        const eastl::vector<PropertyType> allowed(mAllowedTypes.mValue);
        for (const PropertyType type : allowed) {
            types->Insert(types->Size(), DataNode(TypeName(type, true)));
        }
        DumpValue(dump, "allowed_types", DataNode(types));
    }
    if (mAllowsReadOnly.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_read_only", mAllowsReadOnly.mValue);
    }
    if (mAllowsNonAnimatable.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_non_animatable", mAllowsNonAnimatable.mValue);
    }
    if (mAllowsEmpty.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "allows_empty", mAllowsEmpty.mValue);
    }
    return dump;
}

// Reconstructed from eboot.elf at 0x139C20. The flags are checked on the
// path's every level, from the property up.
bool PropRefMetadata::IsValueAllowed(const Component& component, const PropPath& path, const PropRef& value) const {
    if (mAllowsEmpty.Get(component, path) && value == PropRef()) {
        return true;
    }
    eastl::vector<PropertyType> types;
    if (mAllowedTypes(component, path, types)) {
        Entity* entity = component.mObject->mEntity;
        mEntity(component, path, entity);
        const PropInfo* const info = value.Find(entity, nullptr);
        if (info == nullptr) {
            return false;
        }
        const PropertyType* type = types.begin();
        while (type != types.end() && *type != info->mType) {
            ++type;
        }
        if (type == types.end()) {
            return false;
        }
    }
    GameObjectId object = gNullObjectId;
    if (mObject(component, path, object)) {
        return value.mObject.mId == object.mId;
    }
    const bool allowsReadOnly = mAllowsReadOnly.Get(component, path);
    const bool allowsNonAnimatable = mAllowsNonAnimatable.Get(component, path);
    const bool allowsDebug = mAllowsDebug.Get(component, path);
    eastl::vector<Symbol> coms;
    if (mAllowedComs(component, path, coms) || !(allowsReadOnly && allowsNonAnimatable && allowsDebug)) {
        Entity* entity = component.mObject->mEntity;
        mEntity(component, path, entity);
        Component* const com = value.Com(entity);
        if (com == nullptr) {
            return false;
        }
        const Symbol className = com->GetClassName();
        if (!coms.empty()) {
            const Symbol* allowed = coms.begin();
            while (allowed != coms.end() && *allowed != className) {
                ++allowed;
            }
            if (allowed == coms.end()) {
                return false;
            }
        }
        PropPath member = value.mPath;
        const PropRegistry& registry = com->_GetPropRegistry();
        while (member.mSize != 0) {
            const PropInfo* const info = registry.FindProp(member, nullptr);
            if (info == nullptr) {
                return false;
            }
            const PropMetadata* const metadata = info->mMetadata.Get();
            if (!allowsReadOnly && metadata->mReadOnly.Get(*com, value.mPath)) {
                return false;
            }
            if (!allowsNonAnimatable && !metadata->mAnimatable.Get(*com, value.mPath)) {
                return false;
            }
            if (!allowsDebug && metadata->mDebug.Get(*com, value.mPath)) {
                return false;
            }
            member.PopNode();
        }
    }
    return GameObjectIdMetadata::IsValueAllowed(component, path, value.mObject);
}

// Reconstructed from eboot.elf at 0x141ED0.
StructMetadata::~StructMetadata() {}

// Reconstructed from eboot.elf at 0x1419D0.
DataArrayPtr StructMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mSummary.mMode == TagBase<String>::kModeConstant) {
        DumpValue(dump, "summary", DataNode(mSummary.mValue));
    }
    return dump;
}

// Reconstructed from eboot.elf at 0x143BE0.
GroupMetadata::~GroupMetadata() {}

// Reconstructed from eboot.elf at 0x1415F0.
ArrayMetadata::ArrayMetadata()
    : mItemInfo(),
      mInsert(nullptr),
      mOnInsert(nullptr),
      mHook2(nullptr),
      mHook3(nullptr),
      mFixedSize(false, AggTrueIfAnyTrue),
      mSummary(AggUseFirstResult<String>) {}

// Reconstructed from eboot.elf at 0x142590.
ArrayMetadata::~ArrayMetadata() {}

// Reconstructed from eboot.elf at 0x1416E0.
const char* ArrayMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    FormatString format("array of size %u");
    format << static_cast<const PropArrayBase*>(value)->mSize;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x141750.
DataArrayPtr ArrayMetadata::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mFixedSize.mMode == TagBase<bool>::kModeConstant) {
        DumpFlag(dump, "fixed_size", mFixedSize.mValue);
    }
    return dump;
}

template <class T>
WaveformMetadata<T>::WaveformMetadata()
    : mSetter(nullptr),
      mOnChanged(nullptr),
      mGetter(nullptr),
      mReserved(nullptr),
      mReserved2(nullptr),
      mDefault(Waveform<T>(), AggUseFirstResult<Waveform<T>>),
      mAllowedTypes(eastl::vector<int>(), AggVectorIntersection<int>),
      mAllowedBlendModes(eastl::vector<int>(), AggVectorIntersection<int>) {
    eastl::vector<int> types;
    types.reserve(4);
    for (int type = 0; type != 4; ++type) {
        types.push_back(type);
    }
    mAllowedTypes = types;
    eastl::vector<int> modes;
    modes.reserve(6);
    for (int mode = 0; mode != 6; ++mode) {
        if (WaveformAllowsBlendMode<T>(mode)) {
            modes.push_back(mode);
        }
    }
    mAllowedBlendModes = modes;
}

template <class T>
WaveformMetadata<T>::~WaveformMetadata() {}

template <class T>
const char* WaveformMetadata<T>::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    return static_cast<const Waveform<T>*>(value)->mResource.Get()->Description();
}

template <class T>
bool WaveformMetadata<T>::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return true;
}

template <class T>
DataArrayPtr WaveformMetadata<T>::Dump(const Component& component, const PropPath& path) const {
    DataArrayPtr dump = PropMetadata::Dump(component, path);
    if (mAllowedTypes.mMode == TagBase<eastl::vector<int>>::kModeConstant) {
        const DataArrayPtr types(new DataArray(0));
        const eastl::vector<int> allowed(mAllowedTypes.mValue);
        for (const int type : allowed) {
            types->Insert(types->Size(), DataNode(WaveformResource<T>::TypeToString(type)));
        }
        DumpValue(dump, "allowed_types", DataNode(types));
    }
    if (mAllowedBlendModes.mMode == TagBase<eastl::vector<int>>::kModeConstant) {
        const DataArrayPtr modes(new DataArray(0));
        const eastl::vector<int> allowed(mAllowedBlendModes.mValue);
        for (const int mode : allowed) {
            modes->Insert(modes->Size(), DataNode(WaveformBlendModeName(mode)));
        }
        DumpValue(dump, "allowed_blend_modes", DataNode(modes));
    }
    return dump;
}

// Reconstructed from eboot.elf at 0x143D30.
template WaveformMetadata<float>::WaveformMetadata();
// Reconstructed from eboot.elf at 0x147860.
template WaveformMetadata<Hmx::Color>::WaveformMetadata();
// Reconstructed from eboot.elf at 0x145720.
template WaveformMetadata<float>::~WaveformMetadata();
// Reconstructed from eboot.elf at 0x148690.
template WaveformMetadata<Hmx::Color>::~WaveformMetadata();
// Reconstructed from eboot.elf at 0x1444B0.
template const char* WaveformMetadata<float>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;
// Reconstructed from eboot.elf at 0x147F80.
template const char* WaveformMetadata<Hmx::Color>::ValueToString(const Component&, const PropPath&, const void*, bool)
    const;
// Reconstructed from eboot.elf at 0x1444C0.
template bool WaveformMetadata<float>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x147F90.
template bool WaveformMetadata<Hmx::Color>::IsValueAllowed(const Component&, const PropPath&, const void*) const;
// Reconstructed from eboot.elf at 0x1444D0.
template DataArrayPtr WaveformMetadata<float>::Dump(const Component&, const PropPath&) const;
// Reconstructed from eboot.elf at 0x147FA0.
template DataArrayPtr WaveformMetadata<Hmx::Color>::Dump(const Component&, const PropPath&) const;

// Reconstructed from eboot.elf at 0x144480.
FloatWaveformMetadata::~FloatWaveformMetadata() {}

// Reconstructed from eboot.elf at 0x147F50.
ColorWaveformMetadata::~ColorWaveformMetadata() {}

// Reconstructed from eboot.elf at 0x142FF0.
Vector3Metadata::~Vector3Metadata() {}

// Reconstructed from eboot.elf at 0x1431C0.
const char* Vector3Metadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    const Vector3& v = *static_cast<const Vector3*>(value);
    FormatString format("(%f, %f, %f)");
    format << v.x << v.y << v.z;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x143250.
bool Vector3Metadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return true;
}

// Reconstructed from eboot.elf at 0x143260.
PropRegistry* Vector3Metadata::GetStructRegistry() const {
    return &sRegistry;
}

// Reconstructed from eboot.elf at 0x143270.
EulerMetadata::~EulerMetadata() {}

// Reconstructed from eboot.elf at 0x143440. The angles are shown in
// degrees.
const char* EulerMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    const Vector3& v = *static_cast<const Vector3*>(value);
    const float x = kRadiansToDegrees * v.x;
    const float y = kRadiansToDegrees * v.y;
    const float z = kRadiansToDegrees * v.z;
    FormatString format("(%f, %f, %f)");
    format << x << y << z;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1434F0.
bool EulerMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return true;
}

// Reconstructed from eboot.elf at 0x143500.
PropRegistry* EulerMetadata::GetStructRegistry() const {
    return &sRegistry;
}

// Reconstructed from eboot.elf at 0x143510.
XfmMetadata::~XfmMetadata() {}

// Reconstructed from eboot.elf at 0x1436E0. The angles are shown in
// degrees; the formatting is an out-of-line MakeString of nine floats
// (0x1414D0).
const char* XfmMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(useNames);
    const PropXfm& xfm = *static_cast<const PropXfm*>(value);
    FormatString format("s = (%.2f, %.2f, %.2f), r = (%.2f, %.2f, %.2f), t = (%.2f, %.2f, %.2f)");
    format << xfm.mScale.x << xfm.mScale.y << xfm.mScale.z;
    format << kRadiansToDegrees * xfm.mRotation.x << kRadiansToDegrees * xfm.mRotation.y
           << kRadiansToDegrees * xfm.mRotation.z;
    format << xfm.mTranslation.x << xfm.mTranslation.y << xfm.mTranslation.z;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x143780.
bool XfmMetadata::IsValueAllowed(
    const Component& component,
    const PropPath& path,
    const void* value) const {
    static_cast<void>(component);
    static_cast<void>(path);
    static_cast<void>(value);
    return true;
}

// Reconstructed from eboot.elf at 0x143790.
PropRegistry* XfmMetadata::GetStructRegistry() const {
    return &sRegistry;
}

namespace {

// The path of the struct a member accessor's property belongs to. Name not
// in the reference map.
PropPath ParentPath(const PropPath& path) {
    PropPath parent = path;
    parent.PopNode();
    return parent;
}

// Reads the struct property at the path through _GetProp; a missing or
// unconvertible property is reported (the message is compiled out) and
// leaves the value. Inlined into the member accessors. Name not in the
// reference map.
template <class T>
void ReadStructProp(Component* component, const PropPath& path, T& value, PropertyType type) {
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_GetProp(*component, path, *info, storage, &value, type)) {
        component->MakeErrorName();
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// Writes the struct property at the path through _SetProp, reporting a
// failure as ReadStructProp does. Name not in the reference map.
template <class T>
void WriteStructProp(Component* component, const PropPath& path, const T& value, PropertyType type) {
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_SetProp(*component, path, *info, storage, &value, type, false)) {
        component->MakeErrorName();
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// The struct a member's storage belongs to: the member's offset is taken
// from a zeroed object, as the accessors do. Name not in the reference
// map.
template <class S, class M>
const S& StructOfMember(const void* memberStorage, M S::*member, const S& layout) {
    const long offset = reinterpret_cast<const char*>(&(layout.*member)) - reinterpret_cast<const char*>(&layout);
    return *reinterpret_cast<const S*>(static_cast<const char*>(memberStorage) - offset);
}

// Sets a member of a 3D vector property: the vector is read from the
// member's storage or through the parent's path, and written back through
// the parent's path. Inlined into the "x" setter, which the "y" and "z"
// setters enter with their members. Name not in the reference map.
void SetVector3Member(const PropAccessorArgs& args, float Vector3::*member) {
    const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
    Vector3 vector = {0.0F, 0.0F, 0.0F};
    if (args.mStorage != nullptr) {
        static const Vector3 sLayout = {0.0F, 0.0F, 0.0F};
        vector = StructOfMember(args.mStorage, member, sLayout);
    } else {
        ReadStructProp(args.mComponent, parent, vector, kPropertyVector3);
    }
    vector.*member = *static_cast<const float*>(args.mValue);
    WriteStructProp(args.mComponent, parent, vector, kPropertyVector3);
}

// Reconstructed from eboot.elf at 0x148F10.
void SetVector3X(const PropAccessorArgs& args) {
    SetVector3Member(args, &Vector3::x);
}

// Reconstructed from eboot.elf at 0x1496A0.
void SetVector3Y(const PropAccessorArgs& args) {
    SetVector3Member(args, &Vector3::y);
}

// Reconstructed from eboot.elf at 0x149950.
void SetVector3Z(const PropAccessorArgs& args) {
    SetVector3Member(args, &Vector3::z);
}

// Gets a member of a 3D vector property from its storage, or from the
// vector read through the parent's path. Names of the accessors not in the
// reference map.
template <float Vector3::*Member>
float GetVector3Member(const PropAccessorArgs& args) {
    if (args.mStorage != nullptr) {
        return *static_cast<const float*>(args.mStorage);
    }
    const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
    Vector3 vector = {0.0F, 0.0F, 0.0F};
    ReadStructProp(args.mComponent, parent, vector, kPropertyVector3);
    return vector.*Member;
}

// Reconstructed from eboot.elf at 0x149400.
template float GetVector3Member<&Vector3::x>(const PropAccessorArgs& args);
// Reconstructed from eboot.elf at 0x1496B0.
template float GetVector3Member<&Vector3::y>(const PropAccessorArgs& args);
// Reconstructed from eboot.elf at 0x149960.
template float GetVector3Member<&Vector3::z>(const PropAccessorArgs& args);

// Sets an angle of an Euler property from degrees, as SetVector3Member
// does. Inlined into the "pitch" setter. Name not in the reference map.
void SetEulerMember(const PropAccessorArgs& args, float Vector3::*member) {
    const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
    Vector3 angles = {0.0F, 0.0F, 0.0F};
    if (args.mStorage != nullptr) {
        static const Vector3 sLayout = {0.0F, 0.0F, 0.0F};
        angles = StructOfMember(args.mStorage, member, sLayout);
    } else {
        ReadStructProp(args.mComponent, parent, angles, kPropertyEuler);
    }
    angles.*member = *static_cast<const float*>(args.mValue) * kDegreesToRadians;
    WriteStructProp(args.mComponent, parent, angles, kPropertyEuler);
}

// Reconstructed from eboot.elf at 0x149C00.
void SetEulerPitch(const PropAccessorArgs& args) {
    SetEulerMember(args, &Vector3::x);
}

// Reconstructed from eboot.elf at 0x14A3B0.
void SetEulerRoll(const PropAccessorArgs& args) {
    SetEulerMember(args, &Vector3::y);
}

// Reconstructed from eboot.elf at 0x14A3D0.
void SetEulerYaw(const PropAccessorArgs& args) {
    SetEulerMember(args, &Vector3::z);
}

// Gets an angle of an Euler property in degrees. Inlined into the "pitch"
// getter. Name not in the reference map.
float GetEulerMember(const PropAccessorArgs& args, float Vector3::*member) {
    float angle;
    if (args.mStorage != nullptr) {
        angle = *static_cast<const float*>(args.mStorage);
    } else {
        const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
        Vector3 angles = {0.0F, 0.0F, 0.0F};
        ReadStructProp(args.mComponent, parent, angles, kPropertyEuler);
        angle = angles.*member;
    }
    return angle * kRadiansToDegrees;
}

// Reconstructed from eboot.elf at 0x14A0F0.
float GetEulerPitch(const PropAccessorArgs& args) {
    return GetEulerMember(args, &Vector3::x);
}

// Reconstructed from eboot.elf at 0x14A3C0.
float GetEulerRoll(const PropAccessorArgs& args) {
    return GetEulerMember(args, &Vector3::y);
}

// Reconstructed from eboot.elf at 0x14A3E0.
float GetEulerYaw(const PropAccessorArgs& args) {
    return GetEulerMember(args, &Vector3::z);
}

// Sets a member of a transform property, as SetVector3Member does; the
// transform starts as the identity. Inlined into the "scale" setter. Name
// not in the reference map.
void SetXfmMember(const PropAccessorArgs& args, Vector3 PropXfm::*member) {
    const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
    PropXfm xfm = PropXfm::sIdentity;
    if (args.mStorage != nullptr) {
        static const PropXfm sLayout = {{1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}};
        xfm = StructOfMember(args.mStorage, member, sLayout);
    } else {
        ReadStructProp(args.mComponent, parent, xfm, kPropertyXfm);
    }
    xfm.*member = *static_cast<const Vector3*>(args.mValue);
    WriteStructProp(args.mComponent, parent, xfm, kPropertyXfm);
}

// Reconstructed from eboot.elf at 0x14A3F0.
void SetXfmScale(const PropAccessorArgs& args) {
    SetXfmMember(args, &PropXfm::mScale);
}

// Reconstructed from eboot.elf at 0x14ABF0.
void SetXfmRotation(const PropAccessorArgs& args) {
    SetXfmMember(args, &PropXfm::mRotation);
}

// Reconstructed from eboot.elf at 0x14B3D0.
void SetXfmTranslation(const PropAccessorArgs& args) {
    SetXfmMember(args, &PropXfm::mTranslation);
}

// Gets a member of a transform property from its storage, or from the
// transform read through the parent's path.
template <Vector3 PropXfm::*Member>
Vector3 GetXfmMember(const PropAccessorArgs& args) {
    if (args.mStorage != nullptr) {
        return *static_cast<const Vector3*>(args.mStorage);
    }
    const PropPath parent = ParentPath(*static_cast<const PropPath*>(args.mPath));
    PropXfm xfm = PropXfm::sIdentity;
    ReadStructProp(args.mComponent, parent, xfm, kPropertyXfm);
    return xfm.*Member;
}

// Reconstructed from eboot.elf at 0x14A930.
template Vector3 GetXfmMember<&PropXfm::mScale>(const PropAccessorArgs& args);
// Reconstructed from eboot.elf at 0x14B100.
template Vector3 GetXfmMember<&PropXfm::mRotation>(const PropAccessorArgs& args);
// Reconstructed from eboot.elf at 0x14B3E0.
template Vector3 GetXfmMember<&PropXfm::mTranslation>(const PropAccessorArgs& args);

// The default of the struct a member belongs to: the parent property's
// metadata default. Inlined into the members' default functions. Name not
// in the reference map.
template <class M, class T>
T ParentDefault(const TagArgs& args) {
    const PropPath parent = ParentPath(args.mPath);
    Component& component = const_cast<Component&>(args.mComponent);
    const PropInfo* const info = component._GetPropRegistry().FindProp(parent, nullptr);
    return static_cast<const M*>(info->mMetadata.Get())->mDefault.Get(args.mComponent, parent);
}

// The default of a transform's vector member. The scale's and the
// translation's default functions (0x17DF70, 0x17E710) enter it with
// their members. Name not in the reference map.
Vector3 XfmMemberDefault(const TagArgs& args, Vector3 PropXfm::*member) {
    return ParentDefault<XfmMetadata, PropXfm>(args).*member;
}

}  // namespace

// Reconstructed from eboot.elf at 0x1406B0. Each member defaults to its
// own constant first. The default functions' call operators are at
// 0x17CF80, 0x17D3B0 and 0x17D590.
void Vector3Metadata::InitRegistry() {
    const float type = 0.0F;
    FloatMetadata& x = TypeSpecificMetadata(sRegistry.RegisterProp("x", 0, kPropertyFloat, 4), type);
    x.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    x.mSetter = SetVector3X;
    x.mGetter = GetVector3Member<&Vector3::x>;
    x.mDefault = [](const TagArgs& args) {
        return ParentDefault<Vector3Metadata, Vector3>(args).x;
    };
    FloatMetadata& y = TypeSpecificMetadata(sRegistry.RegisterProp("y", 4, kPropertyFloat, 4), type);
    y.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    y.mSetter = SetVector3Y;
    y.mGetter = GetVector3Member<&Vector3::y>;
    y.mDefault = [](const TagArgs& args) {
        return ParentDefault<Vector3Metadata, Vector3>(args).y;
    };
    FloatMetadata& z = TypeSpecificMetadata(sRegistry.RegisterProp("z", 8, kPropertyFloat, 4), type);
    z.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    z.mSetter = SetVector3Z;
    z.mGetter = GetVector3Member<&Vector3::z>;
    z.mDefault = [](const TagArgs& args) {
        return ParentDefault<Vector3Metadata, Vector3>(args).z;
    };
}

// Reconstructed from eboot.elf at 0x140BA0. The default functions' call
// operators are at 0x17D770, 0x17DBA0 and 0x17DD90.
void EulerMetadata::InitRegistry() {
    const float type = 0.0F;
    FloatMetadata& pitch = TypeSpecificMetadata(sRegistry.RegisterProp("pitch", 0, kPropertyFloat, 4), type);
    pitch.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    pitch.mSetter = SetEulerPitch;
    pitch.mGetter = GetEulerPitch;
    pitch.mDefault = [](const TagArgs& args) {
        return ParentDefault<EulerMetadata, Vector3>(args).x * kRadiansToDegrees;
    };
    FloatMetadata& roll = TypeSpecificMetadata(sRegistry.RegisterProp("roll", 4, kPropertyFloat, 4), type);
    roll.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    roll.mSetter = SetEulerRoll;
    roll.mGetter = GetEulerRoll;
    roll.mDefault = [](const TagArgs& args) {
        return ParentDefault<EulerMetadata, Vector3>(args).y * kRadiansToDegrees;
    };
    FloatMetadata& yaw = TypeSpecificMetadata(sRegistry.RegisterProp("yaw", 8, kPropertyFloat, 4), type);
    yaw.mDefault = 0.0F;
    static_cast<void>(Symbol("prop"));
    yaw.mSetter = SetEulerYaw;
    yaw.mGetter = GetEulerYaw;
    yaw.mDefault = [](const TagArgs& args) {
        return ParentDefault<EulerMetadata, Vector3>(args).z * kRadiansToDegrees;
    };
}

// Reconstructed from eboot.elf at 0x1410A0. The rotation is an Euler
// property. The default functions' call operators are at 0x17DF70,
// 0x17E400 and 0x17E710.
void XfmMetadata::InitRegistry() {
    Vector3Metadata& scale = static_cast<Vector3Metadata&>(
        *sRegistry.RegisterProp("scale", offsetof(PropXfm, mScale), kPropertyVector3, sizeof(Vector3)).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    scale.mSetter = SetXfmScale;
    scale.mGetter = GetXfmMember<&PropXfm::mScale>;
    scale.mDefault = [](const TagArgs& args) {
        return XfmMemberDefault(args, &PropXfm::mScale);
    };
    EulerMetadata& rotation = static_cast<EulerMetadata&>(
        *sRegistry.RegisterProp("rotation", offsetof(PropXfm, mRotation), kPropertyEuler, sizeof(Vector3))
             .mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    rotation.mSetter = SetXfmRotation;
    rotation.mGetter = GetXfmMember<&PropXfm::mRotation>;
    rotation.mDefault = [](const TagArgs& args) {
        return ParentDefault<XfmMetadata, PropXfm>(args).mRotation;
    };
    Vector3Metadata& translation = static_cast<Vector3Metadata&>(
        *sRegistry.RegisterProp("translation", offsetof(PropXfm, mTranslation), kPropertyVector3, sizeof(Vector3))
             .mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    translation.mSetter = SetXfmTranslation;
    translation.mGetter = GetXfmMember<&PropXfm::mTranslation>;
    translation.mDefault = [](const TagArgs& args) {
        return XfmMemberDefault(args, &PropXfm::mTranslation);
    };
}

// Reconstructed from eboot.elf at 0x183640.
ActionMetadata::~ActionMetadata() {}

// Reconstructed from eboot.elf at 0x183700. An action shows its display
// name.
const char* ActionMetadata::ValueToString(
    const Component& component,
    const PropPath& path,
    const void* value,
    bool useNames) const {
    static_cast<void>(value);
    static_cast<void>(useNames);
    const String name = mDisplayName.Get(component, path);
    FormatString format("%s");
    format << name.c_str();
    return format.Str();
}
