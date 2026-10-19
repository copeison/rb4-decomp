#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <strings.h>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "math/vector/Vector3.h"
#include "utl/containers/Sort.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class Component;
class Entity;
class PropPath;

// What a property accessor receives, built on the stack by the property
// setters such as the bool one at 0x1A6AA0. A getter receives the first four
// fields and a setter six; a change callback receives the old value, the
// value read back after the set (in mValue) and the requested value. Names
// not in the reference map.
struct PropAccessorArgs {
    // The component whose property is accessed; the accessors of
    // AudioEmitterCom (0x37D70) read it.
    Component* mComponent;
    // Passed through by the setters; the map's setters take a PropPath
    // after the component (weak).
    const void* mPath;
    // The property's PropInfo.
    const PropInfo* mInfo;
    // The property's storage, or null for an accessor-only property.
    void* mStorage;
    // The value before the change, or null when the set is forced.
    const void* mOldValue;
    // The new value, for a setter.
    const void* mValue;
    // The requested value, for a change callback.
    const void* mRequestedValue;
};

static_assert(offsetof(PropAccessorArgs, mValue) == 40);

// What a tag function receives (entity/PropMetadata.o). The map's
// constructor is TagArgs(ObjPtr const&, Component const&, PropPath const&);
// this build passes the component and the path. Field names are not in the
// reference map.
struct TagArgs {
    TagArgs(const Component& component, const PropPath& path);  // 0x12DD60

    const Component& mComponent;
    const PropPath& mPath;
};

static_assert(sizeof(TagArgs) == 16);

// What a static tag function receives: the component's entity and the
// path. ValueToString builds it from the component's object. The map's
// constructor is StaticTagArgs(EntityPtr const&, PropPath const&); this
// build takes the entity by value. Field names are not in the reference
// map.
struct StaticTagArgs {
    StaticTagArgs(Entity* entity, const PropPath& path);  // 0x12DD70

    Entity* mEntity;
    const PropPath& mPath;
};

static_assert(sizeof(StaticTagArgs) == 16);

// What an optional tag function receives: the component, the path and the
// value to fill. The function returns whether it gave a value (0x16E2F0).
// Field names are not in the reference map.
template <typename T>
struct OptionalTagArgs : TagArgs {
    OptionalTagArgs(const Component& component, const PropPath& path, T& result)
        : TagArgs(component, path),
          mResult(result) {}

    T& mResult;
};

// One value a property editor offers, with its label and help text. The
// map instantiates it for the number, symbol, resource path and color
// types. Field names are not in the reference map.
template <class T>
struct AllowedValue {
    // Copies the texts into the strings' own buffers, as the vector
    // assignments that copy allowed values do (0x199C0).
    AllowedValue& operator=(const AllowedValue& other) {
        mValue = other.mValue;
        mName = other.mName.c_str();
        mDescription = other.mDescription.c_str();
        return *this;
    }

    T mValue;
    String mName;
    String mDescription;
};

// One attribute of a property's metadata, such as its help text or its
// minimum: a constant, or a function, script or static function that
// computes it for a component. When the editor shows several components at
// once, the aggregator combines their values; it returns false to stop.
// The map's constructor takes the aggregator as a std::function; this build
// stores a function pointer. Field names are not in the reference map.
template <typename T>
class TagBase {
public:
    using Aggregator = bool (*)(const T& value, unsigned long index, T& result);

    // How the attribute is given. Names not in the reference map.
    enum Mode : int {
        kModeUnset = 0,
        kModeConstant = 1,
        kModeFunction = 2,
        // A script DataNode; the map's _Set(DataArray const&).
        kModeScript = 3,
        kModeOptionalFunction = 4,
        kModeStaticFunction = 5,
    };

    explicit TagBase(Aggregator aggregator)
        : mMode(kModeUnset),
          mValue(),
          mFunction(),
          mOptionalFunction(),
          mStaticFunction(nullptr),
          mScript(),
          mAggregator(aggregator) {}

    // The map's _Set(std::function<T (TagArgs const&)> const&), inlined by
    // every registry builder.
    void _Set(std::function<T(const TagArgs&)>&& function) {
        mMode = kModeFunction;
        mFunction = static_cast<std::function<T(const TagArgs&)>&&>(function);
    }
    void _Set(T (*function)(const StaticTagArgs&)) {
        mMode = kModeStaticFunction;
        mStaticFunction = function;
    }

    // The attribute's value for the component's property: the constant, or
    // what the function, the script or the static function gives; the
    // type's default when unset. The binary emits a copy for each type, such
    // as 0x171380 for String. Name not in the reference map.
    T Get(const Component& component, const PropPath& path) const;
    // Evaluates the script for the property into `result`. Not
    // reconstructed (0x171470 for String). Name not in the reference map.
    void _EvalScript(const Component& component, const PropPath& path, T& result) const;

    Mode mMode;
    T mValue;
    std::function<T(const TagArgs&)> mFunction;
    std::function<bool(OptionalTagArgs<T>&)> mOptionalFunction;
    T (*mStaticFunction)(const StaticTagArgs& args);
    DataNode mScript;
    Aggregator mAggregator;
};

template <typename T>
T TagBase<T>::Get(const Component& component, const PropPath& path) const {
    switch (mMode) {
    case kModeConstant:
        return T(mValue);
    case kModeFunction:
        return mFunction(TagArgs(component, path));
    case kModeScript: {
        T result = T();
        _EvalScript(component, path, result);
        return result;
    }
    case kModeStaticFunction:
        return mStaticFunction(StaticTagArgs(component.mObject->mEntity, path));
    default:
        return T();
    }
}

static_assert(offsetof(TagBase<bool>, mFunction) == 16);
static_assert(offsetof(TagBase<bool>, mStaticFunction) == 112);
static_assert(offsetof(TagBase<bool>, mScript) == 120);
static_assert(sizeof(TagBase<bool>) == 144);
static_assert(offsetof(TagBase<String>, mFunction) == 32);
static_assert(sizeof(TagBase<String>) == 160);

// Copies an attribute's constant. A String copies the text into its own
// buffer, as the inline FixedString assignment does. Name not in the
// reference map.
template <typename T>
inline void AssignTagValue(T& value, const T& from) {
    value = from;
}
inline void AssignTagValue(String& value, const String& from) {
    value = from.c_str();
}

// An attribute with a value: a constant until a function or a script
// replaces it.
template <typename T>
class Tag : public TagBase<T> {
public:
    Tag(const T& value, typename TagBase<T>::Aggregator aggregator)
        : TagBase<T>(aggregator) {
        this->mMode = TagBase<T>::kModeConstant;
        AssignTagValue(this->mValue, value);
    }

    // The constant forms of the map's Tag<T>::operator=.
    Tag& operator=(const T& value) {
        this->mMode = TagBase<T>::kModeConstant;
        AssignTagValue(this->mValue, value);
        return *this;
    }
    Tag& operator=(T&& value) {
        this->mMode = TagBase<T>::kModeConstant;
        this->mValue = static_cast<T&&>(value);
        return *this;
    }
    Tag& operator=(std::function<T(const TagArgs&)>&& function) {
        this->_Set(static_cast<std::function<T(const TagArgs&)>&&>(function));
        return *this;
    }
};

// Whether two attribute values agree, for the conflict check of the
// multi-component OptionalTag::operator(). Allowed values compare by value
// alone. Name not in the reference map.
template <typename T>
inline bool TagValueEquals(const T& a, const T& b) {
    return a == b;
}
inline bool TagValueEquals(const GameObjectId& a, const GameObjectId& b) {
    return a.mId == b.mId;
}
inline bool TagValueEquals(const Hmx::Color& a, const Hmx::Color& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}
template <typename T>
inline bool TagValueEquals(const eastl::vector<AllowedValue<T>>& a, const eastl::vector<AllowedValue<T>>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (unsigned long i = 0; i < a.size(); ++i) {
        if (!TagValueEquals(a[i].mValue, b[i].mValue)) {
            return false;
        }
    }
    return true;
}

// An attribute without a value until one is set.
template <typename T>
class OptionalTag : public TagBase<T> {
public:
    explicit OptionalTag(typename TagBase<T>::Aggregator aggregator)
        : TagBase<T>(aggregator) {}

    // The attribute's value for the component's property in `result`, or
    // false when it has none. The binary emits a copy for each type, such
    // as 0x16E2F0 for float allowed values and 0xFA6E0 for the entity.
    bool operator()(const Component& component, const PropPath& path, T& result) const;
    // The value for the components the editor shows: the constant, the
    // single component's value, or the aggregate of the values of those
    // that give one. `conflicted`, when given, tells whether the values
    // differ or only some components give one; without it the aggregation
    // stops at the first value the aggregator refuses. False when no
    // component gives a value. The map's signature starts with an
    // EntityPtr const& and the objects' ids; 0x16DEC0 for float allowed
    // values.
    bool operator()(
        const eastl::vector<Component*>& components,
        const PropPath& path,
        T& result,
        bool* conflicted) const;

    OptionalTag& operator=(const T& value) {
        this->mMode = TagBase<T>::kModeConstant;
        AssignTagValue(this->mValue, value);
        return *this;
    }
    OptionalTag& operator=(T&& value) {
        this->mMode = TagBase<T>::kModeConstant;
        this->mValue = static_cast<T&&>(value);
        return *this;
    }
    // A constant list: the binary emits one copy per element type, such as
    // 0x410650 for Symbol lists.
    template <typename U>
    OptionalTag& operator=(std::initializer_list<U> values) {
        T list;
        for (const U& value : values) {
            list.push_back(value);
        }
        this->mMode = TagBase<T>::kModeConstant;
        AssignTagValue(this->mValue, list);
        return *this;
    }
    OptionalTag& operator=(std::function<T(const TagArgs&)>&& function) {
        this->_Set(static_cast<std::function<T(const TagArgs&)>&&>(function));
        return *this;
    }
};

template <typename T>
bool OptionalTag<T>::operator()(const Component& component, const PropPath& path, T& result) const {
    switch (this->mMode) {
    case TagBase<T>::kModeConstant:
        AssignTagValue(result, this->mValue);
        return true;
    case TagBase<T>::kModeFunction:
        result = this->mFunction(TagArgs(component, path));
        return true;
    case TagBase<T>::kModeScript:
        this->_EvalScript(component, path, result);
        return true;
    case TagBase<T>::kModeOptionalFunction: {
        OptionalTagArgs<T> args(component, path, result);
        return this->mOptionalFunction(args);
    }
    case TagBase<T>::kModeStaticFunction:
        result = this->mStaticFunction(StaticTagArgs(component.mObject->mEntity, path));
        return true;
    default:
        return false;
    }
}

template <typename T>
bool OptionalTag<T>::operator()(
    const eastl::vector<Component*>& components,
    const PropPath& path,
    T& result,
    bool* conflicted) const {
    if (conflicted != nullptr) {
        *conflicted = false;
    }
    if (this->mMode == TagBase<T>::kModeConstant) {
        AssignTagValue(result, this->mValue);
        return true;
    }
    const unsigned long count = components.size();
    if (count == 1) {
        return (*this)(*components[0], path, result);
    }
    T first = T();
    unsigned long found = 0;
    bool stopped = false;
    for (unsigned long i = 0; i < count; ++i) {
        T value = T();
        if (!(*this)(*components[i], path, value)) {
            continue;
        }
        if (conflicted != nullptr) {
            if (i == 0) {
                first = value;
            } else if (!*conflicted) {
                *conflicted = !TagValueEquals(value, first);
            }
        }
        if (!stopped) {
            stopped = !this->mAggregator(value, found, result);
        }
        ++found;
        if (stopped && conflicted == nullptr) {
            break;
        }
    }
    if (conflicted != nullptr) {
        *conflicted = *conflicted || (found != count && found != 0);
    }
    return found != 0;
}

// The aggregators of the base attributes.
// The display name: the first value, marked with an asterisk when the
// others differ.
bool AggUseFirstResultWithAsteriskIfConflicted(
    const String& value,
    unsigned long index,
    String& result);  // 0x12E300
bool AggTrueIfAnyTrue(const bool& value, unsigned long index, bool& result);  // 0x12E450
bool AggTrueIfAllTrue(const bool& value, unsigned long index, bool& result);  // 0x12E460
bool AggUseAverage(const float& value, unsigned long index, float& result);   // 0x141E70
// Takes the first component's value. The binary emits a copy for each type
// (0x12E440 for Symbol, 0x12E470 for String). Name not in the reference
// map.
template <typename T>
bool AggUseFirstResult(const T& value, unsigned long index, T& result) {
    static_cast<void>(index);
    AssignTagValue(result, value);
    return false;
}
// The most restrictive bound: the largest minimum (AggUseMax, 0x144440 for
// float) and the smallest maximum (AggUseMin, 0x144460). The first value is
// copied.
template <typename T>
bool AggUseMax(const T& value, unsigned long index, T& result) {
    if (index == 0) {
        result = value;
    } else if (result < value) {
        result = value;
    }
    return true;
}
template <typename T>
bool AggUseMin(const T& value, unsigned long index, T& result) {
    if (index == 0) {
        result = value;
    } else if (value < result) {
        result = value;
    }
    return true;
}
// The shared value, or the empty value (the empty symbol, the null entity,
// the invalid object) once two differ; stops at the empty value
// (0x136E20 for Symbol, 0x135EF0 for the entity, 0x139220 for objects).
template <typename T>
bool AggUseNullIfConflicted(const T& value, unsigned long index, T& result);
template <>
bool AggUseNullIfConflicted<Symbol>(const Symbol& value, unsigned long index, Symbol& result);  // 0x136E20
template <>
bool AggUseNullIfConflicted<Entity*>(Entity* const& value, unsigned long index, Entity*& result);  // 0x135EF0
template <>
bool AggUseNullIfConflicted<GameObjectId>(
    const GameObjectId& value,
    unsigned long index,
    GameObjectId& result);  // 0x139220

// The order AggVectorIntersection and AggVectorUnion sort by: symbols
// without regard to case, allowed values by value and then label.
inline bool AggCompare(const Symbol& a, const Symbol& b) {
    return a != b && strcasecmp(a.Str(), b.Str()) < 0;
}
inline bool AggCompare(const GameObjectId& a, const GameObjectId& b) {
    return a.mId < b.mId;
}
inline bool AggCompare(const ResourcePath& a, const ResourcePath& b) {
    return a < b;
}
inline bool AggCompare(const Hmx::Color& a, const Hmx::Color& b) {
    if (a.red != b.red) {
        return a.red < b.red;
    }
    if (a.green != b.green) {
        return a.green < b.green;
    }
    if (a.blue != b.blue) {
        return a.blue < b.blue;
    }
    return a.alpha < b.alpha;
}
template <typename T>
bool AggCompare(const T& a, const T& b) {
    return a < b;
}
template <typename T>
bool AggCompare(const AllowedValue<T>& a, const AllowedValue<T>& b) {
    if (AggCompare(a.mValue, b.mValue)) {
        return true;
    }
    if (AggCompare(b.mValue, a.mValue)) {
        return false;
    }
    return a.mName < b.mName;
}
// The values every component allows: the first list sorted, then its
// intersection with each further list. Stops when nothing is left. The
// binary emits a copy for each type, such as 0x13B840 for allowed symbols.
template <typename T>
bool AggVectorIntersection(
    const eastl::vector<T>& value,
    unsigned long index,
    eastl::vector<T>& result) {
    eastl::vector<T> sorted(value);
    if (!sorted.empty()) {
        eastl::sort(sorted.begin(), sorted.end(), [](const T& a, const T& b) {
            return AggCompare(a, b);
        });
    }
    if (index == 0) {
        result = sorted;
    } else {
        eastl::vector<T> both;
        const T* first = sorted.begin();
        const T* second = result.begin();
        while (first != sorted.end() && second != result.end()) {
            if (AggCompare(*first, *second)) {
                ++first;
            } else if (AggCompare(*second, *first)) {
                ++second;
            } else {
                both.push_back(*first);
                ++first;
                ++second;
            }
        }
        result = both;
    }
    return !result.empty();
}
// The values any component lists: each list is appended, sorted and made
// unique (0x1364C0 for objects, 0x1367A0 for symbols; 0x136B20 is a copy
// that orders symbols by address).
template <typename T>
bool AggVectorUnion(const eastl::vector<T>& value, unsigned long index, eastl::vector<T>& result) {
    static_cast<void>(index);
    for (const T& element : value) {
        result.push_back(element);
    }
    if (!result.empty()) {
        eastl::sort(result.begin(), result.end(), [](const T& a, const T& b) {
            return AggCompare(a, b);
        });
        T* last = result.begin();
        for (T* element = result.begin() + 1; element < result.end(); ++element) {
            if (AggCompare(*last, *element)) {
                *++last = *element;
            }
        }
        result.erase(last + 1, result.end());
    }
    return true;
}
// The resource class every component's class derives from, through
// ResourceMetaData::GetMetaData.
bool AggUseMostSpecificResourceId(const Symbol& value, unsigned long index, Symbol& result);  // 0x13CD80

// Whether tag functions that need a component may be set
// (SetNonStaticTagFunctionsAllowed); new metadata gives its display name
// the static function otherwise.
void SetNonStaticTagFunctionsAllowed(bool allowed);  // 0x12DD40
bool AreNonStaticTagFunctionsAllowed();              // 0x12DD50
// The display name functions every new metadata starts with: the
// property's name made readable.
extern std::function<String(const TagArgs&)> DefaultDisplayName;  // 0x19E3D00
extern String (*StaticDefaultDisplayName)(const StaticTagArgs& args);  // 0x19E3D30

// A property's editing metadata (entity/PropMetadata.o), created for its
// type by Create. PropInfo::PropMetadataPtr counts its references. The
// vtable at 0x18E7960 has eight slots. Field names are not in the reference
// map; the attribute names are the keys Dump (0x133860) writes.
class PropMetadata {
public:
    // Which of the instancing object's editors shows the property: Dump
    // writes "set_on_instance" for 1 and "view_on_instance" for 2. Names
    // not in the reference map.
    enum InstanceAccess : int {
        kInstanceAccessNone = 0,
        kInstanceAccessSet = 1,
        kInstanceAccessView = 2,
    };

    // A button the editor shows beside the property, set by a data-driven
    // registry's "inline_action" (0x202760): its label, the script command
    // it runs and the flag that follows the label. Name and field names not
    // in the reference map.
    struct InlineAction {
        // Inlined into DrivenProp<T>::Register (0x4107C0).
        InlineAction() : mLabel(), mAction(), mEnabled(true) {}

        String mLabel;
        std::function<void(const TagArgs&)> mAction;
        // The name is weak.
        bool mEnabled;
    };

    PropMetadata();           // 0x12DDB0
    virtual ~PropMetadata();  // slots 0-1: 0x12E610, 0x12ED50

    // Slot 2. The value as text. The map's ValueToString(ObjPtr const&,
    // Component const&, PropPath const&, void const*, bool); this build
    // drops the object. The base has no text for any value.
    virtual const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const;  // 0x1420B0
    // Slot 3. False for every value in the base.
    virtual bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const;  // 0x1420C0
    // Slot 4. The attributes with constant values that differ from the
    // defaults, as (key value) arrays, (key value "bool") for flags,
    // followed by DumpAllowedValuesHelp's array.
    virtual DataArrayPtr Dump(const Component& component, const PropPath& path) const;  // 0x133860
    // Slot 5. The editor's help for the allowed values of the components;
    // empty in the base.
    virtual String GetAllowedValuesHelp(
        const eastl::vector<Component*>& components,
        const PropPath& path) const;  // 0x1420D0
    // Slot 6. The allowed values' help as a script array; null in the base.
    virtual DataArrayPtr DumpAllowedValuesHelp(
        const Component& component,
        const PropPath& path) const;  // 0x142100
    // Slot 7. The registry of a struct property's members, or null.
    // PropInfo::GetStructRegistry calls it. Name not in the reference map.
    virtual PropRegistry* GetStructRegistry() const;  // 0x142110

    // A new metadata of the class for the type, such as BoolMetadata. Not
    // reconstructed: most of the classes' constructors are not.
    static PropMetadata* Create(PropertyType type);  // 0x12ED70

    // The name the editor shows, by default made from the property's name.
    Tag<String> mDisplayName;
    // The editor group the property is listed under.
    Tag<Symbol> mGroup;
    Tag<bool> mIsHidden;
    Tag<bool> mReadOnly;
    Tag<bool> mDebug;
    Tag<bool> mAnimatable;
    // Three attributes set by default that Dump does not write, the
    // data-driven registries do not parse and nothing in this build reads.
    // Their names are not recovered; these describe their position after
    // mAnimatable.
    Tag<bool> mExtraFlag0;
    Tag<bool> mExtraFlag1;
    Tag<bool> mExtraFlag2;
    // Excludes the property from IsPropTestable (0x189380), Dump's
    // "no_test".
    bool mNoTest;
    // Whether the property is saved (Dump's "saves"); PropInfo::IsSaved
    // reads it.
    bool mSaves;
    // Whether edits are recorded in the editor's history: a data-driven
    // registry's "history". IsPropTestable also requires it.
    bool mHistory;
    // No writer in this build.
    bool mReserved;
    // Set on InstanceCom's file property (0x11B8E0, 0x127BF0). The name is
    // weak.
    bool mInstanceFile;
    Tag<String> mHelp;
    // Owned; null unless a registry sets one.
    InlineAction* mInlineAction;
    // ComMetaData::Init records a class with a property that sets it.
    InstanceAccess mInstanceAccess;
    // The references PropInfo::PropMetadataPtr holds. Name not in the
    // reference map.
    long mRefCount;
};

static_assert(offsetof(PropMetadata, mDisplayName) == 16);
static_assert(offsetof(PropMetadata, mGroup) == 176);
static_assert(offsetof(PropMetadata, mIsHidden) == 320);
static_assert(offsetof(PropMetadata, mReadOnly) == 464);
static_assert(offsetof(PropMetadata, mDebug) == 608);
static_assert(offsetof(PropMetadata, mAnimatable) == 752);
static_assert(offsetof(PropMetadata, mExtraFlag2) == 1184);
static_assert(offsetof(PropMetadata, mNoTest) == 1328);
static_assert(offsetof(PropMetadata, mSaves) == 1329);
static_assert(offsetof(PropMetadata, mHelp) == 1344);
static_assert(offsetof(PropMetadata, mHistory) == 1330);
static_assert(offsetof(PropMetadata, mInstanceFile) == 1332);
static_assert(offsetof(PropMetadata, mInlineAction) == 1504);
static_assert(offsetof(PropMetadata, mInstanceAccess) == 1512);
static_assert(offsetof(PropMetadata, mRefCount) == 1520);
// The binary's object is 1528 bytes; the attributes' std::functions align
// the class to 16, and subclasses place their members in the tail padding.
static_assert(sizeof(PropMetadata) == 1536);
static_assert(sizeof(PropMetadata::InlineAction) == 80);

// The accessors a typed metadata holds, used by the property setters and
// getters when the property has no storage or needs to react to a change.
// Names not in the reference map.
template <typename T>
struct PropAccessors {
    using Setter = void (*)(const PropAccessorArgs& args);
    using Getter = T (*)(const PropAccessorArgs& args);

    PropAccessors()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr) {}

    Setter mSetter;
    // Called after a set that changed the value, with the old value, the
    // value read back and the requested value (0x1A6AA0).
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    // Zeroed by PropMetadata::Create (0x12ED70); no reader was found.
    void* mReserved;
    void* mReserved2;
};

static_assert(sizeof(PropAccessors<bool>) == 40);

// The metadata of a bool property (1712 bytes, vtable 0x18E7A00). A
// property registered without an offset is read and written through its
// accessors. Field names are not in the reference map.
class BoolMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<bool>::Setter;
    using Getter = PropAccessors<bool>::Getter;

    // Inlined into Create.
    BoolMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDefault(false, AggUseFirstResult<bool>) {}
    ~BoolMetadata() override;  // slots 0-1: 0x142DF0, 0x142ED0

    // Slot 2: "true" or "false".
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142FC0
    // Slot 3: every value is allowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x142FE0

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<bool> mDefault;
};

static_assert(offsetof(BoolMetadata, mSetter) == 1528);
static_assert(offsetof(BoolMetadata, mOnChanged) == 1536);
static_assert(offsetof(BoolMetadata, mGetter) == 1544);
static_assert(offsetof(BoolMetadata, mDefault) == 1568);
static_assert(sizeof(BoolMetadata) == 1712);

// The attributes of a number property (3168 bytes). The map instantiates
// it for float and the eight integer types; the integer vtables start at
// 0x18E7FA0 and float's is at 0x18E8480. The attribute names are those the
// data-driven registries parse (0x202F80). Field names are not in the
// reference map.
template <typename T>
class NumberMetadata : public PropMetadata {
public:
    // The value's full range: the type's limits, or +-1e30 for float.
    NumberMetadata();            // 0x14DA30 for signed char, 0x134E50 for float
    ~NumberMetadata() override;  // 0x14E860 for signed char

    // Slot 3: one of the allowed values when they are given, else a value
    // within the range. 0x14E890 for signed char, 0x135750 for float.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;
    // Slot 4: adds the range when it is narrower than the type's, the prefix
    // and the units. 0x14EAF0 for signed char, 0x1427B0 for float.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;
    // Slot 5: _GetAllowedValuesHelp. 0x12F9A0 for signed char, 0x131FA0 for
    // float.
    String GetAllowedValuesHelp(
        const eastl::vector<Component*>& components,
        const PropPath& path) const override;
    // Slot 6: _DumpAllowedValuesHelp. 0x12FC60 for signed char, 0x132260 for
    // float.
    DataArrayPtr DumpAllowedValuesHelp(const Component& component, const PropPath& path) const override;

    // Not set by the constructor; no reader was found.
    void* mUnused;
    OptionalTag<eastl::vector<AllowedValue<T>>> mAllowedValues;
    // Lists the allowed values in the editor's help (GetAllowedValuesHelp,
    // 0x12F9C0). The name is weak.
    bool mShowAllowedValues;
    Tag<T> mDefault;
    Tag<T> mMin;
    Tag<T> mMax;
    Tag<bool> mSlider;
    OptionalTag<float> mSliderLogResolution;
    // No writer or reader identified; the aggregator copies 8 bytes.
    OptionalTag<long> mReserved;
    Tag<Symbol> mPrefix;
    Tag<Symbol> mUnits;
    OptionalTag<float> mIncrement;
    // Set by default; no reader was identified.
    Tag<bool> mEnabled;
};

static_assert(offsetof(NumberMetadata<int>, mAllowedValues) == 1536);
static_assert(offsetof(NumberMetadata<int>, mShowAllowedValues) == 1712);
static_assert(offsetof(NumberMetadata<int>, mDefault) == 1728);
static_assert(offsetof(NumberMetadata<int>, mMin) == 1872);
static_assert(offsetof(NumberMetadata<int>, mMax) == 2016);
static_assert(offsetof(NumberMetadata<int>, mSlider) == 2160);
static_assert(offsetof(NumberMetadata<int>, mSliderLogResolution) == 2304);
static_assert(offsetof(NumberMetadata<int>, mPrefix) == 2592);
static_assert(offsetof(NumberMetadata<int>, mUnits) == 2736);
static_assert(offsetof(NumberMetadata<int>, mIncrement) == 2880);
static_assert(offsetof(NumberMetadata<int>, mEnabled) == 3024);
static_assert(sizeof(NumberMetadata<int>) == 3168);
static_assert(sizeof(NumberMetadata<float>) == 3168);

// The metadata of an integer property (3360 bytes; types 1 to 8, the
// vtables from 0x18E7F50). Field names are not in the reference map.
template <typename T>
class IntMetadata : public NumberMetadata<T> {
public:
    using Setter = typename PropAccessors<T>::Setter;
    using Getter = typename PropAccessors<T>::Getter;

    // Inlined into Create.
    IntMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mBitfield(false, AggTrueIfAllTrue) {}
    ~IntMetadata() override;

    // Slot 2: the typed ValueToString. 0x14DFE0 for signed char.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;
    // Slot 3: NumberMetadata's. 0x14DFF0 for signed char.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;
    // Slot 4: adds the bitfield flag, under the key "prefix". 0x14E000 for
    // signed char.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;

    // A bitfield joins the labels of the allowed values whose bits are set
    // ("<None>" or "<All>" for none or every one); otherwise the value's
    // label when `useNames` asks for it, else the number. 0x150BB0 for
    // signed char.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const T& value,
        bool useNames) const;
    // The number with the prefix before it and the units after a space.
    // 0x150EE0 for signed char. Name not in the reference map.
    const char* FormatValue(const Component& component, const PropPath& path, T value) const;

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    // Joins the names of the set bits with "|" in ValueToString (0x150BB0)
    // instead of naming one value. Dump writes it as "prefix". The name is
    // inferred.
    Tag<bool> mBitfield;
};

static_assert(offsetof(IntMetadata<int>, mSetter) == 3168);
static_assert(offsetof(IntMetadata<int>, mGetter) == 3184);
static_assert(offsetof(IntMetadata<int>, mBitfield) == 3216);
static_assert(sizeof(IntMetadata<int>) == 3360);

// The metadata of a float property (type 0; 3360 bytes, vtable 0x18E79B0).
// Field names are not in the reference map.
class FloatMetadata : public NumberMetadata<float> {
public:
    using Setter = PropAccessors<float>::Setter;
    using Getter = PropAccessors<float>::Getter;

    // Inlined into Create.
    FloatMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDecimalPlaces(4, AggUseMax<int>) {}
    ~FloatMetadata() override;  // slots 0-1: 0x1425C0, 0x1426A0

    // Slot 2: the typed ValueToString.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142790
    // Slot 3: NumberMetadata's.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x1427A0

    // The value's label when `useNames` asks for it, else the number with
    // mDecimalPlaces digits, the prefix and the units.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const float& value,
        bool useNames) const;  // 0x135250

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    // The digits after the point in ValueToString (0x135250); 4 by
    // default. The name is inferred.
    Tag<int> mDecimalPlaces;
};

static_assert(offsetof(FloatMetadata, mSetter) == 3168);
static_assert(offsetof(FloatMetadata, mGetter) == 3184);
static_assert(offsetof(FloatMetadata, mDecimalPlaces) == 3216);
static_assert(sizeof(FloatMetadata) == 3360);

// The metadata of a symbol property (type 11; 1904 bytes, vtable
// 0x18E7AF0). Field names are not in the reference map.
class SymbolMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<Symbol>::Setter;
    using Getter = PropAccessors<Symbol>::Getter;

    // Inlined into Create.
    SymbolMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDefault(Symbol(""), AggUseFirstResult<Symbol>),
          mAllowedValues(AggVectorIntersection<AllowedValue<Symbol>>),
          mShowAllowedValues(true),
          mUnused(nullptr) {}
    ~SymbolMetadata() override;  // slots 0-1: 0x142120, 0x142210

    // Slot 2: the value's label when `useNames` asks for it, else the
    // symbol.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142300
    // Slot 3: the typed IsValueAllowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x142330
    // Slots 5 and 6: _GetAllowedValuesHelp and _DumpAllowedValuesHelp.
    String GetAllowedValuesHelp(
        const eastl::vector<Component*>& components,
        const PropPath& path) const override;  // 0x132460
    DataArrayPtr DumpAllowedValuesHelp(const Component& component, const PropPath& path) const override;  // 0x132720

    // One of the allowed values when they are given; any value otherwise.
    bool IsValueAllowed(const Component& component, const PropPath& path, const Symbol& value) const;  // 0x13C040

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<Symbol> mDefault;
    OptionalTag<eastl::vector<AllowedValue<Symbol>>> mAllowedValues;
    // As NumberMetadata::mShowAllowedValues. The name is weak.
    bool mShowAllowedValues;
    // Zeroed by Create; no reader was found.
    void* mUnused;
};

static_assert(offsetof(SymbolMetadata, mDefault) == 1568);
static_assert(offsetof(SymbolMetadata, mAllowedValues) == 1712);
static_assert(offsetof(SymbolMetadata, mShowAllowedValues) == 1888);
static_assert(sizeof(SymbolMetadata) == 1904);

// The metadata of an object reference (type 10; 3136 bytes, vtable
// 0x18E7A50, constructor 0x135AC0). Field names are not in the reference
// map; those Dump (0x1384C0) does not write are inferred from their types.
class GameObjectIdMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<GameObjectId>::Setter;
    using Getter = PropAccessors<GameObjectId>::Getter;

    GameObjectIdMetadata();            // 0x135AC0
    ~GameObjectIdMetadata() override;  // slots 0-1: 0x142400, 0x142410

    // Slot 2: the object's name in the property's entity, "<None>" for no
    // object or "<INVALID>" without an entity.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142430
    // Slot 3: the typed IsValueAllowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x1424E0
    // Slot 4: adds the object flags and the required components.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;  // 0x1384C0

    // Whether the object is one GetAllowedObjects lists.
    bool IsValueAllowed(const Component& component, const PropPath& path, const GameObjectId& value) const;  // 0x136F40
    // The objects the components' property may name: the allowed objects,
    // or every object of the entity, filtered by the required components,
    // the filter class and the duplicates, with the null object when it is
    // allowed. False when none is left. The map's signature starts with an
    // EntityPtr const& and the objects' ids. Not reconstructed.
    bool GetAllowedObjects(
        const eastl::vector<Component*>& components,
        const PropPath& path,
        eastl::vector<GameObjectId>& objects) const;  // 0x137250

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    // The invalid id by default (0x19E3CF0).
    Tag<GameObjectId> mDefault;
    // The entity the id refers into when it is not the component's own,
    // usually given as a function (the map has OptionalTag<EntityPtr>);
    // GetAllowedObjects (0x137250) reads it. The name is weak.
    OptionalTag<Entity*> mEntity;
    OptionalTag<eastl::vector<GameObjectId>> mAllowedObjects;
    Tag<bool> mAllowsNull;
    Tag<bool> mAllowsThis;
    // Objects gathered from every selected component (a union, 0x1364C0);
    // no reader was identified. The name is weak.
    OptionalTag<eastl::vector<GameObjectId>> mExcludedObjects;
    Tag<bool> mAllowsDuplicates;
    // The component classes the object must hold.
    OptionalTag<eastl::vector<Symbol>> mRequiredComs;
    // RndPostProcCom's stages set it to the stage interface,
    // "PostProcStage"; no reader was found. The name is weak.
    OptionalTag<eastl::vector<Symbol>> mRequiredInterfaces;
    // Read with the filter by GetAllowedObjects. The name is weak.
    OptionalTag<Symbol> mFilterClass;
};

static_assert(offsetof(GameObjectIdMetadata, mDefault) == 1568);
static_assert(offsetof(GameObjectIdMetadata, mEntity) == 1712);
static_assert(offsetof(GameObjectIdMetadata, mAllowedObjects) == 1856);
static_assert(offsetof(GameObjectIdMetadata, mAllowsNull) == 2032);
static_assert(offsetof(GameObjectIdMetadata, mAllowsThis) == 2176);
static_assert(offsetof(GameObjectIdMetadata, mExcludedObjects) == 2320);
static_assert(offsetof(GameObjectIdMetadata, mAllowsDuplicates) == 2496);
static_assert(offsetof(GameObjectIdMetadata, mRequiredComs) == 2640);
static_assert(offsetof(GameObjectIdMetadata, mRequiredInterfaces) == 2816);
static_assert(offsetof(GameObjectIdMetadata, mFilterClass) == 2992);
static_assert(sizeof(GameObjectIdMetadata) == 3136);

// The metadata of a resource path (type 12; 2960 bytes, vtable 0x18E7B40,
// constructor 0x13C370). Field names are not in the reference map; those
// Dump (0x13EE30) does not write are inferred from their types.
class ResourcePathMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<ResourcePath>::Setter;
    using Getter = PropAccessors<ResourcePath>::Getter;

    ResourcePathMetadata();            // 0x13C370
    ~ResourcePathMetadata() override;  // slots 0-1: 0x142340, 0x142350

    // Slot 2: the value's label when `useNames` asks for it, else the
    // path.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142370
    // Slot 3: the typed IsValueAllowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x1423A0
    // Slot 4: adds the resource class, the entity flag, the custom label
    // and the custom extensions.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;  // 0x13EE30
    // Slots 5 and 6: _GetAllowedValuesHelp and _DumpAllowedValuesHelp.
    String GetAllowedValuesHelp(
        const eastl::vector<Component*>& components,
        const PropPath& path) const override;  // 0x132920
    DataArrayPtr DumpAllowedValuesHelp(const Component& component, const PropPath& path) const override;  // 0x132BE0

    // An empty path, one of the allowed values when they are given, or a
    // path whose extension is valid; a resource inlined in the entity only
    // when mAllowsInlined.
    bool IsValueAllowed(const Component& component, const PropPath& path, const ResourcePath& value) const;  // 0x13E790
    // The file extensions of the resource class and its subclasses,
    // intersected with the custom extensions; `defaultExtension`, when
    // given, gets the custom label or the class's name. False when there
    // is none. The map's signature starts with an EntityPtr const& and the
    // objects' ids. Not reconstructed.
    bool GetValidExtensions(
        const eastl::vector<Component*>& components,
        const PropPath& path,
        eastl::vector<Symbol>& extensions,
        Symbol* defaultExtension) const;  // 0x13D7D0

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<ResourcePath> mDefault;
    OptionalTag<eastl::vector<AllowedValue<ResourcePath>>> mAllowedValues;
    bool mShowAllowedValues;
    // The resource class the path loads.
    Tag<Symbol> mResourceId;
    // Whether the path may name a resource inlined in the entity
    // (IsValueAllowed). The name is weak.
    Tag<bool> mAllowsInlined;
    OptionalTag<Symbol> mCustomLabel;
    OptionalTag<eastl::vector<Symbol>> mCustomExtensions;
    // Not written by Dump. The name is weak.
    OptionalTag<Symbol> mDefaultExtension;
    // The entity IsValueAllowed looks for inlined resources in, the
    // component's own by default. The name is weak.
    OptionalTag<Entity*> mEntity;
    Tag<bool> mAllowsEmpty;
    // ComMetaData::Init skips the check of a path without a resource class
    // or extensions when it is set. The name is weak.
    bool mAllowsUntyped;
    // No reader was identified.
    bool mFlag1;
    // Dump's "is_relative_to_entity".
    bool mIsRelativeToEntity;
};

static_assert(offsetof(ResourcePathMetadata, mDefault) == 1568);
static_assert(offsetof(ResourcePathMetadata, mAllowedValues) == 1712);
static_assert(offsetof(ResourcePathMetadata, mResourceId) == 1904);
static_assert(offsetof(ResourcePathMetadata, mCustomLabel) == 2192);
static_assert(offsetof(ResourcePathMetadata, mCustomExtensions) == 2336);
static_assert(offsetof(ResourcePathMetadata, mAllowsEmpty) == 2800);
static_assert(offsetof(ResourcePathMetadata, mEntity) == 2656);
static_assert(offsetof(ResourcePathMetadata, mAllowsUntyped) == 2944);
static_assert(offsetof(ResourcePathMetadata, mIsRelativeToEntity) == 2946);
static_assert(sizeof(ResourcePathMetadata) == 2960);

// The metadata of a color (type 13; 2064 bytes, vtable 0x18E7B90,
// constructor 0x13F620). Field names are not in the reference map.
class ColorMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<Hmx::Color>::Setter;
    using Getter = PropAccessors<Hmx::Color>::Getter;

    ColorMetadata();            // 0x13F620
    ~ColorMetadata() override;  // slots 0-1: 0x1423B0, 0x1423C0

    // Slots 2 and 3: the typed functions.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x1423E0
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x1423F0
    // Slots 5 and 6: _GetAllowedValuesHelp and _DumpAllowedValuesHelp.
    String GetAllowedValuesHelp(
        const eastl::vector<Component*>& components,
        const PropPath& path) const override;  // 0x132DE0
    DataArrayPtr DumpAllowedValuesHelp(const Component& component, const PropPath& path) const override;  // 0x1330A0

    // The value's label when `useNames` asks for it, else the sRGB
    // channels as "(R:%i G:%i B:%i)".
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const Hmx::Color& value,
        bool useNames) const;  // 0x140000
    // One of the allowed values when they are given; any value otherwise.
    bool IsValueAllowed(const Component& component, const PropPath& path, const Hmx::Color& value) const;  // 0x140280

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<Hmx::Color> mDefault;
    OptionalTag<eastl::vector<AllowedValue<Hmx::Color>>> mAllowedValues;
    bool mShowAllowedValues;
    // Clear by default; the name is weak.
    Tag<bool> mHasAlpha;
};

static_assert(offsetof(ColorMetadata, mDefault) == 1568);
static_assert(offsetof(ColorMetadata, mAllowedValues) == 1728);
static_assert(offsetof(ColorMetadata, mShowAllowedValues) == 1904);
static_assert(offsetof(ColorMetadata, mHasAlpha) == 1920);
static_assert(sizeof(ColorMetadata) == 2064);

// The metadata of a property reference (type 16; 4784 bytes, vtable
// 0x18E7AA0, constructor 0x138D80): an object reference with the property
// it names. Field names are not in the reference map; those Dump
// (0x13AE40) does not write are inferred from their types.
class PropRefMetadata : public GameObjectIdMetadata {
public:
    using RefSetter = PropAccessors<PropRef>::Setter;
    using RefGetter = PropAccessors<PropRef>::Getter;

    PropRefMetadata();            // 0x138D80
    ~PropRefMetadata() override;  // slots 0-1: 0x1424F0, 0x142500

    // Slot 2: PropRef::ValueToString in the property's entity.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x142520
    // Slot 3: the typed IsValueAllowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x142580
    // Slot 4: adds the allowed types and the reference flags.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;  // 0x13AE40

    // An empty reference when it is allowed; otherwise a property of an
    // allowed type, the given object, or a property of an allowed
    // component whose path passes the read-only, animatable and debug
    // restrictions, on an allowed object.
    bool IsValueAllowed(const Component& component, const PropPath& path, const PropRef& value) const;  // 0x139C20

    RefSetter mRefSetter;
    void (*mRefOnChanged)(const PropAccessorArgs& args);
    RefGetter mRefGetter;
    void* mRefReserved;
    void* mRefReserved2;
    Tag<PropRef> mRefDefault;
    // An object for the reference, the invalid id by default; no reader
    // was identified. The name is weak.
    OptionalTag<GameObjectId> mObject;
    OptionalTag<eastl::vector<PropertyType>> mAllowedTypes;
    // Gathered from every selected component (a union, 0x1397F0). The
    // name is weak.
    OptionalTag<eastl::vector<PropertyType>> mExcludedTypes;
    // The classes the referenced component may have (IsValueAllowed).
    OptionalTag<eastl::vector<Symbol>> mAllowedComs;
    Tag<bool> mAllowsReadOnly;
    Tag<bool> mAllowsNonAnimatable;
    Tag<bool> mAllowsEmpty;
    // Whether the path may pass through debug properties (IsValueAllowed).
    // Dump does not write it; the name is weak.
    Tag<bool> mAllowsDebug;
};

static_assert(offsetof(PropRefMetadata, mRefSetter) == 3136);
static_assert(offsetof(PropRefMetadata, mRefDefault) == 3184);
static_assert(offsetof(PropRefMetadata, mObject) == 3536);
static_assert(offsetof(PropRefMetadata, mAllowedTypes) == 3680);
static_assert(offsetof(PropRefMetadata, mAllowsReadOnly) == 4208);
static_assert(offsetof(PropRefMetadata, mAllowsEmpty) == 4496);
static_assert(sizeof(PropRefMetadata) == 4784);

// The metadata of a struct property (vtable 0x18E7D20): a summary of its
// value. Field names are not in the reference map.
class StructMetadata : public PropMetadata {
public:
    // Inlined into Create.
    StructMetadata()
        : mKind(0),
          mSummary(AggUseFirstResultWithAsteriskIfConflicted) {}
    ~StructMetadata() override;  // slots 0-1: 0x141ED0, 0x141FC0

    // Slot 4: adds the summary. The map has this function on GroupMetadata
    // and a ValueToString on StructMetadata, the reverse of the slots here,
    // which suggests the two names are swapped.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;  // 0x1419D0

    // What the struct is, as the class registries set it: 0 by default,
    // 1 for a compound value such as a vector, plane or extents, 2 for a
    // driven property. No reader was identified, so the name is weak.
    int mKind;
    // Dump's "summary".
    OptionalTag<String> mSummary;
};

static_assert(offsetof(StructMetadata, mSummary) == 1536);
static_assert(sizeof(StructMetadata) == 1696);

// The metadata of a struct property (type 15; 1856 bytes, vtable
// 0x18E7D70): the registry of its members, whose dynamic size and alignment
// PropRegistry::AmendDynamicStructSize reads.
class GroupMetadata : public StructMetadata {
public:
    // Inlined into Create.
    GroupMetadata() : mRegistry() {}
    ~GroupMetadata() override;  // slots 0-1: 0x143BE0, 0x143BF0
    // Slot 2: "struct".
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x143C10
    PropRegistry* GetStructRegistry() const override;  // slot 7: 0x143C20

    PropRegistry mRegistry;
};

static_assert(offsetof(GroupMetadata, mRegistry) == 1696);
static_assert(sizeof(GroupMetadata) == 1856);

// The metadata of an array property (1904 bytes, vtable 0x18E7CD0): the
// PropInfo of its elements, which PropRegistry::RegisterPropArrayItem
// fills, the hooks the element editing calls, and the array's own
// attributes. Field names are not in the reference map.
class ArrayMetadata : public PropMetadata {
public:
    ArrayMetadata();            // 0x1415F0
    ~ArrayMetadata() override;  // slots 0-1: 0x142590, 0x1425A0

    // Slot 2: "array of size %u". The value is the property's PropArray,
    // whose element count follows its first 16 bytes.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x1416E0
    // Slot 4: adds the fixed size flag.
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;  // 0x141750

    PropInfo mItemInfo;
    // Inserts an element in place of the default insertion (0x197A60);
    // mOnInsert runs first. mHook2, a void (*)(const PropAccessorArgs&)
    // stored untyped, removes the element the path's last index names in
    // place of the default removal (_RemoveProp); the last hook is not
    // identified. Names not in the reference map.
    void (*mInsert)(const PropAccessorArgs& args);
    void (*mOnInsert)(const PropAccessorArgs& args);
    void* mHook2;
    void* mHook3;
    // Dump's "fixed_size".
    Tag<bool> mFixedSize;
    // The array's summary; Dump does not write it.
    OptionalTag<String> mSummary;
};

static_assert(offsetof(ArrayMetadata, mItemInfo) == 1528);
static_assert(offsetof(ArrayMetadata, mInsert) == 1560);
static_assert(offsetof(ArrayMetadata, mFixedSize) == 1600);
static_assert(offsetof(ArrayMetadata, mSummary) == 1744);
static_assert(sizeof(ArrayMetadata) == 1904);

// The metadata of a waveform (types 18 and 19; 3104 and 2096 bytes; vtables
// 0x18E7E10 and 0x18E7EB0, and 0x18E7DC0 and 0x18E7E60 for the subclasses
// Create makes). Only the attributes Dump writes are named; the float type
// adds its range after them. Field names are not in the reference map.
template <class T>
class WaveformMetadata : public PropMetadata {
public:
    using Setter = typename PropAccessors<Waveform<T>>::Setter;
    using Getter = typename PropAccessors<Waveform<T>>::Getter;

    // Every waveform type and every blend mode is allowed, apart from the
    // HSV modes for float.
    WaveformMetadata();            // 0x143D30 (float), 0x147860 (color)
    ~WaveformMetadata() override;  // 0x145720 (float), 0x148690 (color)

    // Slot 2: the resource's description. 0x1444B0 (float), 0x147F80
    // (color).
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;
    // Slot 3: every value is allowed. 0x1444C0 (float), 0x147F90 (color).
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;
    // Slot 4: adds the allowed types and blend modes by name. 0x1444D0
    // (float), 0x147FA0 (color).
    DataArrayPtr Dump(const Component& component, const PropPath& path) const override;

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<Waveform<T>> mDefault;
    // The map's Tag<vector<WaveformType>> and Tag<vector<WaveformBlendMode>>.
    Tag<eastl::vector<int>> mAllowedTypes;
    Tag<eastl::vector<int>> mAllowedBlendModes;
};

static_assert(offsetof(WaveformMetadata<float>, mDefault) == 1568);
static_assert(offsetof(WaveformMetadata<float>, mAllowedTypes) == 1744);
static_assert(offsetof(WaveformMetadata<float>, mAllowedBlendModes) == 1920);
static_assert(sizeof(WaveformMetadata<Hmx::Color>) == 2096);

// The metadata Create makes for a float waveform (vtable 0x18E7DC0): the
// waveform's attributes and the range of its values, as a number's. Only
// the destructors are its own. Name and field names not in the reference
// map.
class FloatWaveformMetadata : public WaveformMetadata<float> {
public:
    // Inlined into Create.
    FloatWaveformMetadata()
        : mMin(-1e30F, AggUseMax<float>),
          mMax(1e30F, AggUseMin<float>),
          mSlider(false, AggTrueIfAllTrue),
          mSliderLogResolution(AggUseAverage),
          mIncrement(AggUseAverage),
          mPrefix(Symbol(""), AggUseFirstResult<Symbol>),
          mUnits(Symbol(""), AggUseFirstResult<Symbol>) {}
    ~FloatWaveformMetadata() override;  // slots 0-1: 0x144480, 0x144490

    Tag<float> mMin;
    Tag<float> mMax;
    Tag<bool> mSlider;
    OptionalTag<float> mSliderLogResolution;
    OptionalTag<float> mIncrement;
    Tag<Symbol> mPrefix;
    Tag<Symbol> mUnits;
};

static_assert(offsetof(FloatWaveformMetadata, mMin) == 2096);
static_assert(offsetof(FloatWaveformMetadata, mSlider) == 2384);
static_assert(offsetof(FloatWaveformMetadata, mPrefix) == 2816);
static_assert(sizeof(FloatWaveformMetadata) == 3104);

// The metadata Create makes for a color waveform (vtable 0x18E7E60); it adds
// nothing to its base. Name not in the reference map.
class ColorWaveformMetadata : public WaveformMetadata<Hmx::Color> {
public:
    ~ColorWaveformMetadata() override;  // slots 0-1: 0x147F50, 0x147F60
};

static_assert(sizeof(ColorWaveformMetadata) == 2096);

// The metadata of a 3D vector property (type 20; 1712 bytes, vtable
// 0x18E7BE0): its members are the float properties "x", "y" and "z" of a
// shared registry. The class is newer than the reference map; its name and
// field names are not in it.
class Vector3Metadata : public PropMetadata {
public:
    using Setter = PropAccessors<Vector3>::Setter;
    using Getter = PropAccessors<Vector3>::Getter;

    // Inlined into Create.
    Vector3Metadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDefault(Vector3::sZero, AggUseFirstResult<Vector3>) {}
    ~Vector3Metadata() override;  // slots 0-1: 0x142FF0, 0x1430D0

    // Slot 2: "(x, y, z)".
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x1431C0
    // Slot 3: every value is allowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x143250
    // Slot 7: sRegistry.
    PropRegistry* GetStructRegistry() const override;  // 0x143260

    // Registers the float members "x", "y" and "z" in sRegistry: each
    // reads and writes the struct through its parent's path and defaults
    // to the parent's default. Called from a static initializer
    // (0x1129560). Name not in the reference map.
    static void InitRegistry();  // 0x1406B0

    // The members' registry, constructed by the object's static
    // initializer (0x17E790) and filled by InitRegistry.
    static PropRegistry sRegistry;  // 0x19E3D50

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    // Zero by default.
    Tag<Vector3> mDefault;
};

static_assert(offsetof(Vector3Metadata, mDefault) == 1568);
static_assert(sizeof(Vector3Metadata) == 1712);

// The metadata of an Euler angle property (type 21; 1712 bytes, vtable
// 0x18E7C30): angles stored in radians, with the members "pitch", "roll"
// and "yaw" in degrees. Name and field names not in the reference map.
class EulerMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<Vector3>::Setter;
    using Getter = PropAccessors<Vector3>::Getter;

    // Inlined into Create.
    EulerMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDefault(sZeroAngles, AggUseFirstResult<Vector3>) {}
    ~EulerMetadata() override;  // slots 0-1: 0x143270, 0x143350

    // Slot 2: "(x, y, z)" in degrees.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x143440
    // Slot 3: every value is allowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x1434F0
    // Slot 7: sRegistry.
    PropRegistry* GetStructRegistry() const override;  // 0x143500

    // Registers the members "pitch", "roll" and "yaw", in degrees, as
    // Vector3Metadata::InitRegistry does. Name not in the reference map.
    static void InitRegistry();  // 0x140BA0

    // Filled by InitRegistry.
    static PropRegistry sRegistry;  // 0x19E3DF0

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<Vector3> mDefault;

    // The default, no rotation: zero data among the math module's
    // globals, which nothing writes; its owner is not identified. Name not
    // in the reference map.
    static Vector3 sZeroAngles;  // 0x19E6630
};

static_assert(offsetof(EulerMetadata, mDefault) == 1568);
static_assert(sizeof(EulerMetadata) == 1712);

// The value of a rotation property (type 21, "Rotation"): Euler angles in
// radians, stored as a Vector3. The state graph node templates are
// instantiated for it apart from Vector3, so the binary has a type of its
// own. Name not in the reference map.
struct PropRotation : Vector3 {};

static_assert(sizeof(PropRotation) == 12);

// The value of a transform property (type 22): scale, Euler angles and
// translation, the layout of TransCom's local transform. The identity is
// at 0x19E669C. Name not in the reference map.
struct PropXfm {
    Vector3 mScale;
    Vector3 mRotation;
    Vector3 mTranslation;

    // Unit scale, no rotation, no translation; filled by the static
    // initializer at 0x2195C0.
    static PropXfm sIdentity;  // 0x19E669C
};

static_assert(sizeof(PropXfm) == 36);

// Interpolates two transforms: the scale and translation directly, the
// rotation through quaternions (InterpXfm) or angle by angle
// (InterpXfmLinear), as InterpEuler and InterpEulerLinear do. Emitted in
// the math module. Names not in the reference map. Declared for the state
// graph nodes; not reconstructed.
void InterpXfm(const PropXfm& from, const PropXfm& to, float t, PropXfm& result);        // 0x219450
void InterpXfmLinear(const PropXfm& from, const PropXfm& to, float t, PropXfm& result);  // 0x2192E0

// The metadata of a transform property (type 22; 1744 bytes, vtable
// 0x18E7C80), with the members "scale", "rotation" and "translation". Name
// and field names not in the reference map.
class XfmMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<PropXfm>::Setter;
    using Getter = PropAccessors<PropXfm>::Getter;

    // Inlined into Create.
    XfmMetadata()
        : mSetter(nullptr),
          mOnChanged(nullptr),
          mGetter(nullptr),
          mReserved(nullptr),
          mReserved2(nullptr),
          mDefault(PropXfm::sIdentity, AggUseFirstResult<PropXfm>) {}
    ~XfmMetadata() override;  // slots 0-1: 0x143510, 0x1435F0

    // Slot 2: "s = (...), r = (...), t = (...)", the angles in degrees.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x1436E0
    // Slot 3: every value is allowed.
    bool IsValueAllowed(
        const Component& component,
        const PropPath& path,
        const void* value) const override;  // 0x143780
    // Slot 7: sRegistry.
    PropRegistry* GetStructRegistry() const override;  // 0x143790

    // Registers the members "scale", "rotation" and "translation", as
    // Vector3Metadata::InitRegistry does. Name not in the reference map.
    static void InitRegistry();  // 0x1410A0

    // Filled by InitRegistry.
    static PropRegistry sRegistry;  // 0x19E3E90

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<PropXfm> mDefault;
};

static_assert(offsetof(XfmMetadata, mDefault) == 1568);
static_assert(sizeof(XfmMetadata) == 1744);

// The metadata of an action (type 17; 1584 bytes, vtable 0x18E87D0), which
// PropRegistry::RegisterAction creates: the editor shows a button that runs
// mAction on the component.
class ActionMetadata : public PropMetadata {
public:
    // Inlined into RegisterAction.
    ActionMetadata() : mAction() {}
    ~ActionMetadata() override;  // slots 0-1: 0x183640, 0x1836A0

    // Slot 2.
    const char* ValueToString(
        const Component& component,
        const PropPath& path,
        const void* value,
        bool useNames) const override;  // 0x183700

    // Set by the class registries as a lambda. Name not in the reference
    // map.
    std::function<void(const TagArgs&)> mAction;
};

static_assert(offsetof(ActionMetadata, mAction) == 1536);
static_assert(sizeof(ActionMetadata) == 1584);

// The value of a property with the class of its type; the value selects
// the overload and is not read. Every overload returns the info's metadata.
FloatMetadata& TypeSpecificMetadata(PropInfo& info, const float& type);                         // 0x140A20
IntMetadata<signed char>& TypeSpecificMetadata(PropInfo& info, const signed char& type);        // 0x141B90
IntMetadata<short>& TypeSpecificMetadata(PropInfo& info, const short& type);                    // 0x141BB0
IntMetadata<int>& TypeSpecificMetadata(PropInfo& info, const int& type);                        // 0x141BD0
IntMetadata<long>& TypeSpecificMetadata(PropInfo& info, const long& type);                      // 0x141BF0
IntMetadata<unsigned char>& TypeSpecificMetadata(PropInfo& info, const unsigned char& type);    // 0x141C10
IntMetadata<unsigned short>& TypeSpecificMetadata(PropInfo& info, const unsigned short& type);  // 0x141C30
IntMetadata<unsigned int>& TypeSpecificMetadata(PropInfo& info, const unsigned int& type);      // 0x141C50
IntMetadata<unsigned long>& TypeSpecificMetadata(PropInfo& info, const unsigned long& type);    // 0x141C70
BoolMetadata& TypeSpecificMetadata(PropInfo& info, const bool& type);                           // 0x141C90
GameObjectIdMetadata& TypeSpecificMetadata(PropInfo& info, const GameObjectId& type);           // 0x141CB0
SymbolMetadata& TypeSpecificMetadata(PropInfo& info, const Symbol& type);                       // 0x141CD0
ResourcePathMetadata& TypeSpecificMetadata(PropInfo& info, const ResourcePath& type);           // 0x141CF0
ColorMetadata& TypeSpecificMetadata(PropInfo& info, const Hmx::Color& type);                    // 0x141D10
PropRefMetadata& TypeSpecificMetadata(PropInfo& info, const PropRef& type);                     // 0x141D30
WaveformMetadata<float>& TypeSpecificMetadata(PropInfo& info, const Waveform<float>& type);     // 0x141D50
WaveformMetadata<Hmx::Color>& TypeSpecificMetadata(
    PropInfo& info,
    const Waveform<Hmx::Color>& type);  // 0x141D70
// The 3D vector overload (type 20), newer than the reference map. The Euler
// angle properties (type 21) take a separate overload at 0x141420, whose
// value type, also three floats, is not recovered.
Vector3Metadata& TypeSpecificMetadata(PropInfo& info, const Vector3& type);  // 0x141410

// What a metadata remap function receives (240 bytes): the tag arguments of
// the property being edited, which the remap function points at another
// property, whose metadata then answers the attributes (0x1F3DA0 for a float
// default). The map names the type; the field names are not in the
// reference map.
struct TagRemapArgs {
    // The tag arguments the attribute was asked with.
    const Component& mComponent;
    const PropPath& mPath;
    // Filled by the remap function: the entity, the component and path of
    // the property that answers, and its info.
    Entity* mEntity;
    Component* mTargetCom;
    PropPath mTargetPath;
    const PropInfo* mTargetInfo;
};

static_assert(offsetof(TagRemapArgs, mEntity) == 16);
static_assert(offsetof(TagRemapArgs, mTargetPath) == 32);
static_assert(offsetof(TagRemapArgs, mTargetInfo) == 232);
static_assert(sizeof(TagRemapArgs) == 240);

// The function a remapped attribute calls to find the property that answers
// it. The map's RemapPropMetadata overloads take a std::function<void
// (TagRemapArgs&)> const&; this build passes the function's address.
using TagRemapFunc = void (*)(TagRemapArgs& args);

// Makes the metadata's type-specific attributes (default, range, allowed
// values and the like) answer with those of the property `remap` finds.
// Declared for the state graph comparisons; not reconstructed.
void RemapPropMetadata(FloatMetadata& metadata, const TagRemapFunc& remap);  // 0x1E5FF0
// 0x1E1470 (signed char), 0x1E1DE0 (short), 0x1E2750 (int), 0x1E30C0
// (long), 0x1E3A30 (unsigned char), 0x1E43A0 (unsigned short), 0x1E4D10
// (unsigned int), 0x1E5680 (unsigned long).
template <typename T>
void RemapPropMetadata(IntMetadata<T>& metadata, const TagRemapFunc& remap);
void RemapPropMetadata(BoolMetadata& metadata, const TagRemapFunc& remap);          // 0x1E6950
void RemapPropMetadata(GameObjectIdMetadata& metadata, const TagRemapFunc& remap);  // 0x1E6F30
void RemapPropMetadata(PropRefMetadata& metadata, const TagRemapFunc& remap);       // 0x1E7040
void RemapPropMetadata(SymbolMetadata& metadata, const TagRemapFunc& remap);        // 0x1E7490
void RemapPropMetadata(ResourcePathMetadata& metadata, const TagRemapFunc& remap);  // 0x1E7640
void RemapPropMetadata(ColorMetadata& metadata, const TagRemapFunc& remap);         // 0x1E7DB0
// 0x1E8030 (float), 0x1E8860 (Hmx::Color).
template <class T>
void RemapPropMetadata(WaveformMetadata<T>& metadata, const TagRemapFunc& remap);
// The vector, rotation and transform overloads, which the state graph's
// property nodes call. Declared for the state graph nodes; not
// reconstructed.
void RemapPropMetadata(Vector3Metadata& metadata, const TagRemapFunc& remap);  // 0x1E8AE0
void RemapPropMetadata(EulerMetadata& metadata, const TagRemapFunc& remap);    // 0x1E8BD0
void RemapPropMetadata(XfmMetadata& metadata, const TagRemapFunc& remap);      // 0x1E8CC0

// The rotation and transform overloads of TypeSpecificMetadata, newer than
// the reference map.
EulerMetadata& TypeSpecificMetadata(PropInfo& info, const PropRotation& type);  // 0x141420
XfmMetadata& TypeSpecificMetadata(PropInfo& info, const PropXfm& type);        // 0x141DB0

// The read-only overloads of the numbers (the map's TypeSpecificMetadata(
// PropInfo const&, T const&)), which also return the info's metadata.
const FloatMetadata& TypeSpecificMetadata(const PropInfo& info, const float& type);  // 0x141B80
const IntMetadata<signed char>& TypeSpecificMetadata(const PropInfo& info, const signed char& type);  // 0x141BA0
const IntMetadata<short>& TypeSpecificMetadata(const PropInfo& info, const short& type);  // 0x141BC0
const IntMetadata<int>& TypeSpecificMetadata(const PropInfo& info, const int& type);  // 0x141BE0
const IntMetadata<long>& TypeSpecificMetadata(const PropInfo& info, const long& type);  // 0x141C00
const IntMetadata<unsigned char>& TypeSpecificMetadata(const PropInfo& info, const unsigned char& type);  // 0x141C20
const IntMetadata<unsigned short>& TypeSpecificMetadata(const PropInfo& info, const unsigned short& type);  // 0x141C40
const IntMetadata<unsigned int>& TypeSpecificMetadata(const PropInfo& info, const unsigned int& type);  // 0x141C60
const IntMetadata<unsigned long>& TypeSpecificMetadata(const PropInfo& info, const unsigned long& type);  // 0x141C80
