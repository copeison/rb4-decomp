#pragma once

#include <cstddef>

#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/PropAnimDriver.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/waveform/Key.h"
#include "math/color/Color.h"
#include "os/memory/PoolAlloc.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class Entity;

// The base of the property-animation key components (anim/PropKeysCom.o,
// 0x305350-0x305C8F; interface "PropKeys"): the keys of one property,
// "driven_prop", of an object of the entity the animation plays on, which a
// PropAnimResource holds one object per keyed property for. The key track
// itself is the second base of PropKeysTypeCom<T>, at offset 264, and most
// slots forward to it. The vtable at 0x18F7BB8 has 73 slots; slots 41-72
// are the class's own. Classes register their key type with
// RegisterFactory, which NewKeysType uses to create the keys of a property.
// The object is 264 bytes. Field names are not in the reference map.
class PropKeysBaseCom : public Component {
public:
    // Makes the driver of a property for the class (the classes'
    // StaticCreatePropDriver). Name not in the reference map.
    using PropDriverFactory = PropDriver* (*)(const PropRef& prop);

    PropKeysBaseCom() : mProp(), mHiddenInEditor(false), mColorInEditor(0.0f, 0.0f, 0.0f, 1.0f) {}
    // Slots 0-1: 0x305750 (Component's destructor) and 0x305760.
    ~PropKeysBaseCom() override;

    // Slots 31, 33 and 38: the keys do nothing on their own; edit mode polls
    // as the game mode does. The map's _Enter, _Poll and _EditPoll take the
    // object (ObjPtr const&).
    void _Enter() override {}  // 0x2B1FD0
    void _Poll() override {}   // 0x2B1FE0
    void _EditPoll() override {
        _Poll();
    }  // 0x2B1FF0

    // Slot 41: a new driver for the property, of the class's type. The map
    // has it on each class, as PropKeysFloatCom::CreatePropDriver() const.
    virtual PropDriver* CreatePropDriver() const = 0;
    // Slot 42: whether the keys have tangents (the float keys).
    virtual bool HasTangents() const {
        return false;
    }  // 0x2B2000
    // Slots 43-54: the key track's operations (KeyTrackBase); the typed
    // keys forward them to their track.
    virtual void SetInterpolation(KeyTrackBase::Interpolation interpolation) = 0;
    virtual KeyTrackBase::Interpolation GetInterpolation() const = 0;
    virtual void Clear() = 0;
    virtual unsigned long NumKeys() const = 0;
    // Slot 47.
    virtual bool Empty() const {
        return NumKeys() == 0;
    }  // 0x305790
    virtual float GetFirstFrame() const = 0;
    virtual float GetLastFrame() const = 0;
    virtual void RemoveKeyAtIndex(unsigned long index) = 0;
    virtual unsigned long MoveKey(unsigned long index, float frame, bool before) = 0;
    // Name not in the reference map.
    virtual void OffsetFrames(float offset) = 0;
    virtual unsigned long RemoveKeysInFrameRange(float start, float end) = 0;
    virtual float GetKeyFrameAtIndex(unsigned long index) const = 0;
    // Slots 55-56: the property type the keys drive and, for structs, its
    // name ("quaternion", "string"); the empty symbol otherwise.
    virtual PropertyType GetKeysType() const = 0;
    virtual Symbol GetKeysSubType() const = 0;
    // Slot 57: whether the keys can interpolate so; the base allows only
    // None. The map has ValidInterpolations() const, which lists them.
    // Name not in the reference map.
    virtual bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
        return interpolation == KeyTrackBase::kInterpolationNone;
    }  // 0x2B2010
    // Slot 58: true for the colour and colour waveform keys. No caller was
    // found. Name not in the reference map; the evidence is weak.
    virtual bool IsColorKeys() const {
        return false;
    }  // 0x2B2020
    // Slots 59-60: true for the rotation and transform keys, which hold a
    // quaternion and flags beside their editable value. No caller was
    // found. Names not in the reference map; the evidence is weak.
    virtual bool IsRotationKeys() const {
        return false;
    }  // 0x2B2030
    virtual bool HasSerializedValue() const {
        return false;
    }  // 0x2B2040
    // Slots 61-65: the keys' values as DataNodes. GetValueAtIndex's flag
    // goes to the node conversion; SetValueAtFrame adds a key (replacing
    // one at the frame when asked) and returns its index; GetValueAtFrame
    // samples the keys and returns the index of the key at or before the
    // frame, or -1. UpdateValueAtIndex sets the key to the property's
    // current value in the entity.
    virtual DataNode GetValueAtIndex(unsigned long index, bool forEditor) const = 0;
    virtual void SetValueAtIndex(unsigned long index, const DataNode& value) = 0;
    virtual void UpdateValueAtIndex(unsigned long index, Entity* entity) = 0;
    virtual unsigned long SetValueAtFrame(float frame, const DataNode& value, bool replace) = 0;
    virtual unsigned long GetValueAtFrame(float frame, DataNode& value) const = 0;
    // Slots 66-68: no key has extra data.
    virtual bool CanHaveExtra() const {
        return false;
    }  // 0x2B2050
    virtual DataNode GetExtraAtIndex(unsigned long index) const {
        static_cast<void>(index);
        return DataNode();
    }  // 0x2B2060
    virtual void SetExtraAtIndex(unsigned long index, const DataNode& extra) {
        static_cast<void>(index);
        static_cast<void>(extra);
    }  // 0x2B2080
    // Slot 69: adds a key of the property's current value at the frame and
    // returns its index.
    virtual unsigned long AddNewKey(float frame, Entity* entity, bool replace) = 0;
    // Slot 70: sets the property in the entity to the keys' value at the
    // frame; false when the property is missing or refuses the value. The
    // map's ApplyFrame(float, EntityPtr const&) const.
    virtual bool ApplyFrame(float frame, Entity* entity) const = 0;
    // Slot 71: adds a copy of the key after the keys at its frame and
    // returns the copy's index.
    virtual unsigned long CloneKey(unsigned long index) = 0;
    // Slot 72: takes the property the keys drive.
    virtual void _Setup(const PropRef& prop) {
        mProp = prop;
    }  // 0x2B2090

    // The entity of the resource that inlines the entity the keys live in:
    // the entity the animation is authored for. Name not in the reference
    // map's signature list but in its PropKeysCom.o.
    Entity* _GetContextEntityPrototype() const;  // 0x3054E0
    // Whether the driven property exists in the entity. The map's
    // QueryValidPropTarget(EntityPtr) const.
    bool QueryValidPropTarget(Entity* entity) const;  // 0x305780

    // Records the key class of a property type (and struct name) and the
    // factory of its drivers. The map's signature takes the class symbol by
    // value; this build passes both symbols by reference.
    static void RegisterFactory(
        PropertyType type,
        const Symbol& subType,
        const Symbol& classId,
        PropDriverFactory factory);  // 0x305350
    // Creates the key component for a property of the type in a new
    // object of the entity (or the given one) named "Keys type <type>
    // <subtype>", sets it up for the property and returns the object's id;
    // the null id when no class keys the type. The map's
    // NewKeysType(EntityPtr, PropertyType, Symbol const&, PropRef const&,
    // PropKeysBaseCom**); this build takes the subtype by value and the
    // object.
    static GameObjectId NewKeysType(
        Entity* entity,
        PropertyType type,
        Symbol subType,
        const PropRef& prop,
        PropKeysBaseCom** keys,
        GameObject* object);  // 0x305500

    // Registers the properties every key class has: "interpolation",
    // "driven_prop", "hidden_in_editor" and "color_in_editor", and, for the
    // class's own registry, revision 1, whose handler upgrades an older
    // interpolation. Inlined into each class's _Init, as at 0x3005D0 for
    // bool.
    template <class T>
    static void _InitPropKeysBase(PropRegistry& registry);

    // Points the metadata of a key's value at the driven property's: the
    // entity, the referenced component and its property. Name not in the
    // reference map.
    static void _RemapKeyValue(TagRemapArgs& args);  // 0x305800
    // The function the key values' metadata remap with; set by the static
    // initializer (0x305BC0). Name not in the reference map.
    static TagRemapFunc sRemapKeyValue;  // 0x19F9EF8

    // "driven_prop".
    PropRef mProp;
    // "hidden_in_editor" and "color_in_editor": how the editor's timeline
    // shows the keys.
    bool mHiddenInEditor;
    Hmx::Color mColorInEditor;
};

static_assert(offsetof(PropKeysBaseCom, mProp) == 24);
static_assert(offsetof(PropKeysBaseCom, mHiddenInEditor) == 240);
static_assert(offsetof(PropKeysBaseCom, mColorInEditor) == 244);
static_assert(sizeof(PropKeysBaseCom) == 264);

// The key component of a value type (the map's PropKeysTypeCom<T>): the key
// track is its second base, at offset 264, and the slots of both bases
// forward to it, so the second vtable holds inlined thunks. Slots 73-75 are
// the value's conversions to and from DataNodes and its default, which each
// class defines. The template's methods are emitted with each class; the
// addresses given are PropKeysTypeCom<bool>'s (vtable 0x18F73A8).
template <class T>
class PropKeysTypeCom : public PropKeysBaseCom, public TKeyTrack<T> {
public:
    // The value the keys hold. Name not in the reference map.
    using ValueType = T;

    PropKeysTypeCom() {}
    ~PropKeysTypeCom() override {}

    // Slots 43-54, overriding both bases.
    void SetInterpolation(KeyTrackBase::Interpolation interpolation) override {
        TKeyTrack<T>::SetInterpolation(interpolation);
    }  // 0x300ED0
    KeyTrackBase::Interpolation GetInterpolation() const override {
        return TKeyTrack<T>::GetInterpolation();
    }  // 0x300EE0
    void Clear() override {
        TKeyTrack<T>::Clear();
    }  // 0x300EF0
    unsigned long NumKeys() const override {
        return TKeyTrack<T>::NumKeys();
    }  // 0x300F00
    bool Empty() const override {
        return TKeyTrack<T>::NumKeys() == 0;
    }  // 0x300F10
    float GetFirstFrame() const override {
        return TKeyTrack<T>::GetFirstFrame();
    }  // 0x300F20
    float GetLastFrame() const override {
        return TKeyTrack<T>::GetLastFrame();
    }  // 0x300F40
    void RemoveKeyAtIndex(unsigned long index) override {
        TKeyTrack<T>::RemoveKeyAtIndex(index);
    }  // 0x300F60
    unsigned long MoveKey(unsigned long index, float frame, bool before) override {
        return TKeyTrack<T>::MoveKey(index, frame, before);
    }  // 0x301000
    void OffsetFrames(float offset) override {
        TKeyTrack<T>::OffsetFrames(offset);
    }  // 0x301090
    unsigned long RemoveKeysInFrameRange(float start, float end) override {
        return TKeyTrack<T>::RemoveKeysInFrameRange(start, end);
    }  // 0x3010D0
    float GetKeyFrameAtIndex(unsigned long index) const override {
        return TKeyTrack<T>::GetKeyFrameAtIndex(index);
    }  // 0x3010E0

    // Slots 61-65.
    DataNode GetValueAtIndex(unsigned long index, bool forEditor) const override {
        return _ValueToDataNode(KeyAt(index).mValue, forEditor);
    }  // 0x3011A0
    void SetValueAtIndex(unsigned long index, const DataNode& value) override {
        KeyAt(index).mValue = _ValueFromDataNode(value);
    }  // 0x3011D0
    void UpdateValueAtIndex(unsigned long index, Entity* entity) override {
        KeyAt(index).mValue = mProp.template Get<T>(entity);
    }  // 0x301200
    unsigned long SetValueAtFrame(float frame, const DataNode& value, bool replace) override {
        return this->AddKey(_ValueFromDataNode(value), frame, replace, false);
    }  // 0x301230
    unsigned long GetValueAtFrame(float frame, DataNode& value) const override {
        T sample;
        const unsigned long index = this->SampleValueAtFrame(frame, sample);
        value = _ValueToDataNode(sample, false);
        return index;
    }  // 0x3012B0

    // Slots 69-71.
    unsigned long AddNewKey(float frame, Entity* entity, bool replace) override {
        return this->AddKey(mProp.template Get<T>(entity), frame, replace, false);
    }  // 0x3014D0
    bool ApplyFrame(float frame, Entity* entity) const override {
        T value;
        this->SampleValueAtFrame(frame, value);
        GameObject* const object = mProp.Obj(entity);
        if (object == nullptr) {
            return false;
        }
        Component* const component = object->FindCom(mProp.mCom);
        if (component == nullptr) {
            return false;
        }
        void* storage = component;
        const PropInfo* const info = component->_GetPropRegistry().FindProp(mProp.mPath, &storage);
        if (info != nullptr && _SetProp(*component, mProp.mPath, *info, storage, &value, PropValueType<T>::kType, false)) {
            return true;
        }
        static_cast<void>(component->MakeErrorName());
        char path[512] = {};
        mProp.mPath.ToString(path, sizeof(path));
        return false;
    }  // 0x301550
    unsigned long CloneKey(unsigned long index) override {
        const T value = _ValueFromDataNode(GetValueAtIndex(index, false));
        return this->AddKey(value, GetKeyFrameAtIndex(index), false, false);
    }  // 0x3018E0

    // Slots 73-75: the value as a DataNode (for the editor when the flag is
    // set), the value of a DataNode, and the value of a new key.
    virtual DataNode _ValueToDataNode(const T& value, bool forEditor) const = 0;
    virtual T _ValueFromDataNode(const DataNode& value) const = 0;
    virtual T _DefaultValue() const = 0;

private:
    // The key at the index. Name not in the reference map.
    TKey<T>& KeyAt(unsigned long index) const {
        return static_cast<TKey<T>*>(this->mKeys.mData)[index];
    }
};

// The driver of a property animated by key components of class Keys (the
// map's TPropDriver<T>; 256 bytes, from the pool): each playback's keys and
// the component they drive. The classes' _Poll blends or picks the
// playbacks' values and sets the property; the discrete values take the
// value of the heaviest playback that is not paused. Field names are not in
// the reference map.
template <class Keys>
class TPropDriver : public PropDriver {
public:
    // One playback's keys. Name not in the reference map.
    struct Entry {
        const PlaybackInfo* mInfo;
        const Keys* mKeys;
        // The keys' driven component in the entity when the playback
        // started.
        Component* mCom;
    };

    explicit TPropDriver(const PropRef& prop) : mEntries() {
        mProp = prop;
    }
    // Slots 0-1: 0x301D50, 0x301D90 for bool.
    ~TPropDriver() override {}

    // From the pool, labelled "PropDriver".
    static void* operator new(unsigned long size) {
        return PoolAlloc(size, "PropDriver");
    }
    static void operator delete(void* allocation) {
        PoolFree(sizeof(TPropDriver), allocation);
    }

    // Slot 2: 0x301DE0 for bool.
    unsigned long GetNumPropKeysComs() const override {
        return mEntries.size();
    }
    // Slot 3: 0x301E10 for bool.
    void Add(const PlaybackInfo* info, const PropKeysBaseCom* keys, Entity* entity) override {
        Component* const component = keys->mProp.Com(entity);
        mEntries.push_back(Entry{info, static_cast<const Keys*>(keys), component});
    }
    // Slot 4: moves the last entry over the playback's. 0x301F50 for bool.
    void Remove(const PlaybackInfo* info) override {
        for (Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
            if (entry->mInfo == info) {
                if (mEntries.size() >= 2) {
                    *entry = mEntries.mpEnd[-1];
                }
                mEntries.pop_back();
                return;
            }
        }
    }
    // Slot 5: the map's Poll(ObjPtr const&), which calls _Poll when there
    // are playbacks.
    void Poll(GameObject* root) override {
        if (mEntries.mpBegin != mEntries.mpEnd) {
            _Poll(root);
        }
    }

    // Sets the property and reports a failure. Name not in the reference
    // map's signature, which also takes the object; 0x302320 for bool.
    static void _SetDrivenProperty(Component* component, const PropPath& path, const void* value) {
        void* storage = component;
        const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
        if (info != nullptr && _SetProp(*component, path, *info, storage, value, PropValueType<typename Keys::ValueType>::kType, false)) {
            return;
        }
        static_cast<void>(component->MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }

    // Sets the property from the playbacks; the discrete form unless the
    // class specializes it. 0x301FF0 for bool, inlined into Poll.
    void _Poll(GameObject* root) {
        static_cast<void>(root);
        typename Keys::ValueType chosen;
        float heaviest = -100000.0f;
        bool none = true;
        for (const Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
            if (entry->mInfo->mPaused) {
                continue;
            }
            typename Keys::ValueType value;
            entry->mKeys->SampleValueAtFrame(entry->mInfo->GetCurrentFrame(), value);
            none = false;
            if (heaviest < entry->mInfo->mWeight) {
                chosen = value;
                heaviest = entry->mInfo->mWeight;
            }
        }
        if (!none && mEntries.mpBegin->mCom != nullptr) {
            _SetDrivenProperty(mEntries.mpBegin->mCom, mEntries.mpBegin->mKeys->mProp.mPath, &chosen);
        }
    }

    eastl::vector<Entry> mEntries;
};

template <class T>
void PropKeysBaseCom::_InitPropKeysBase(PropRegistry& registry) {
    T prototype;
    if (&registry != &T::sPropRegistry) {
        registry.AddInheritedRegistry(T::sClassName, T::sPropRegistry);
    }
    const int defaultInterpolation = prototype.mInterpolation;
    IntMetadata<int>& interpolation = TypeSpecificMetadata(
        registry.RegisterProp("interpolation", offsetof(T, mInterpolation), kPropertyInt, 4),
        defaultInterpolation);
    interpolation.mDefault = defaultInterpolation;
    static_cast<void>(Symbol("prop"));
    interpolation.mHelp = String("The interpolation type of the keyframes.");

    PropRefMetadata& driven = TypeSpecificMetadata(
        registry.RegisterProp("driven_prop", offsetof(T, mProp), kPropertyPropRef, sizeof(PropRef)),
        prototype.mProp);
    driven.mRefDefault = prototype.mProp;
    static_cast<void>(Symbol("prop"));
    driven.mHelp = String("The animated property");
    driven.mAllowsNonAnimatable = false;
    driven.mEntity.mMode = TagBase<Entity*>::kModeOptionalFunction;
    driven.mEntity.mOptionalFunction = [](OptionalTagArgs<Entity*>& args) {
        args.mResult = static_cast<const PropKeysBaseCom&>(args.mComponent)._GetContextEntityPrototype();
        return true;
    };

    BoolMetadata& hidden = TypeSpecificMetadata(
        registry.RegisterProp("hidden_in_editor", offsetof(T, mHiddenInEditor), kPropertyBool, 1),
        prototype.mHiddenInEditor);
    hidden.mDefault = prototype.mHiddenInEditor;
    ColorMetadata& color = TypeSpecificMetadata(
        registry.RegisterProp("color_in_editor", offsetof(T, mColorInEditor), kPropertyColor, sizeof(Hmx::Color)),
        prototype.mColorInEditor);
    color.mDefault = prototype.mColorInEditor;

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev > 0) {
            return;
        }
        T* const keys = static_cast<T*>(args.mComponent);
        UpgradeInterpolation(keys->mInterpolation, &keys->mInterpolation);
    };
    if (&registry == &T::sPropRegistry) {
        registry.SetCurrentRev(1);
        registry.mRevisionFunc = onRevision;
    }
}
