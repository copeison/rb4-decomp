#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>

#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "math/vector/Vector3.h"
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

// What an optional tag function receives; the layout is not recovered.
template <typename T>
struct OptionalTagArgs;

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

    Mode mMode;
    T mValue;
    std::function<T(const TagArgs&)> mFunction;
    std::function<bool(OptionalTagArgs<T>&)> mOptionalFunction;
    T (*mStaticFunction)(const StaticTagArgs& args);
    DataNode mScript;
    Aggregator mAggregator;
};

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

// An attribute without a value until one is set.
template <typename T>
class OptionalTag : public TagBase<T> {
public:
    explicit OptionalTag(typename TagBase<T>::Aggregator aggregator)
        : TagBase<T>(aggregator) {}

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
    // Slot 4. The attributes as a script array. Not reconstructed.
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
          mDefault(false, AggTrueIfAllTrue) {}
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
    NumberMetadata();            // 0x14DA30 for signed char
    ~NumberMetadata() override;  // 0x14E860 for signed char

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

    ~IntMetadata() override;

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

    ~FloatMetadata() override;  // slots 0-1: 0x1425C0, 0x1426A0

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

    ~SymbolMetadata() override;  // slots 0-1: 0x142120, 0x142210

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
    OptionalTag<eastl::vector<Symbol>> mAllowedEntities;
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
static_assert(offsetof(GameObjectIdMetadata, mAllowedEntities) == 2320);
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
    // Set by default; no reader was identified.
    Tag<bool> mEnabled;
    Tag<Symbol> mCustomLabel;
    OptionalTag<eastl::vector<Symbol>> mCustomExtensions;
    // Not written by Dump. The names are weak.
    OptionalTag<Symbol> mDefaultExtension;
    OptionalTag<long> mReserved3;
    Tag<bool> mAllowsEmpty;
    bool mIsRelativeToEntity;
    bool mFlag1;
    bool mFlag2;
};

static_assert(offsetof(ResourcePathMetadata, mDefault) == 1568);
static_assert(offsetof(ResourcePathMetadata, mAllowedValues) == 1712);
static_assert(offsetof(ResourcePathMetadata, mResourceId) == 1904);
static_assert(offsetof(ResourcePathMetadata, mCustomLabel) == 2192);
static_assert(offsetof(ResourcePathMetadata, mCustomExtensions) == 2336);
static_assert(offsetof(ResourcePathMetadata, mAllowsEmpty) == 2800);
static_assert(offsetof(ResourcePathMetadata, mIsRelativeToEntity) == 2944);
static_assert(sizeof(ResourcePathMetadata) == 2960);

// The metadata of a color (type 13; 2064 bytes, vtable 0x18E7B90,
// constructor 0x13F620). Field names are not in the reference map.
class ColorMetadata : public PropMetadata {
public:
    using Setter = PropAccessors<Hmx::Color>::Setter;
    using Getter = PropAccessors<Hmx::Color>::Getter;

    ColorMetadata();            // 0x13F620
    ~ColorMetadata() override;  // slots 0-1: 0x1423B0, 0x1423C0

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

    RefSetter mRefSetter;
    void (*mRefOnChanged)(const PropAccessorArgs& args);
    RefGetter mRefGetter;
    void* mRefReserved;
    void* mRefReserved2;
    Tag<PropRef> mRefDefault;
    // -1 by default. The name is weak.
    Tag<int> mArrayIndex;
    OptionalTag<eastl::vector<PropertyType>> mAllowedTypes;
    OptionalTag<eastl::vector<Symbol>> mAllowedComs;
    OptionalTag<eastl::vector<Symbol>> mAllowedProps;
    Tag<bool> mAllowsReadOnly;
    Tag<bool> mAllowsNonAnimatable;
    Tag<bool> mAllowsEmpty;
    Tag<bool> mAllowsArrays;
};

static_assert(offsetof(PropRefMetadata, mRefSetter) == 3136);
static_assert(offsetof(PropRefMetadata, mRefDefault) == 3184);
static_assert(offsetof(PropRefMetadata, mArrayIndex) == 3536);
static_assert(offsetof(PropRefMetadata, mAllowedTypes) == 3680);
static_assert(offsetof(PropRefMetadata, mAllowsReadOnly) == 4208);
static_assert(offsetof(PropRefMetadata, mAllowsEmpty) == 4496);
static_assert(sizeof(PropRefMetadata) == 4784);

// The metadata of a struct property (vtable 0x18E7D20): a summary of its
// value. Field names are not in the reference map.
class StructMetadata : public PropMetadata {
public:
    ~StructMetadata() override;  // slots 0-1: 0x141ED0, 0x141FC0

    // Not written by any constructor; no reader was found.
    int mUnused;
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
    ~GroupMetadata() override;  // slots 0-1: 0x143BE0, 0x143BF0
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

    PropInfo mItemInfo;
    // Inserts an element in place of the default insertion (0x197A60);
    // mOnInsert runs first. The other two are not identified. Names not in
    // the reference map.
    void (*mInsert)(const PropAccessorArgs& args);
    void (*mOnInsert)(const PropAccessorArgs& args);
    void* mHook2;
    void* mHook3;
    Tag<bool> mFixedSize;
    // Dump's "summary".
    OptionalTag<String> mSummary;
};

static_assert(offsetof(ArrayMetadata, mItemInfo) == 1528);
static_assert(offsetof(ArrayMetadata, mInsert) == 1560);
static_assert(offsetof(ArrayMetadata, mFixedSize) == 1600);
static_assert(offsetof(ArrayMetadata, mSummary) == 1744);
static_assert(sizeof(ArrayMetadata) == 1904);

// The metadata of a waveform (types 18 and 19; 3104 and 2096 bytes, vtables
// 0x18E7DC0 and 0x18E7E60). Only the attributes Dump writes are named; the
// float type adds its range after them. Field names are not in the
// reference map.
template <class T>
class WaveformMetadata : public PropMetadata {
public:
    using Setter = typename PropAccessors<Waveform<T>>::Setter;
    using Getter = typename PropAccessors<Waveform<T>>::Getter;

    WaveformMetadata();            // 0x143D30 (float), 0x147860 (color)
    ~WaveformMetadata() override;  // 0x145720 (float), 0x148690 (color)

    Setter mSetter;
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    void* mReserved;
    void* mReserved2;
    Tag<Waveform<T>> mDefault;
    OptionalTag<eastl::vector<int>> mAllowedTypes;
    OptionalTag<eastl::vector<int>> mAllowedBlendModes;
};

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
