#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "render/fonts/RndTextCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;
class UIButtonDownMsg;
class UIKeyboardKeyMsg;

// The property type of a combo box's value type: the entries and
// "current_value" are registered with it. Name not in the reference map.
template <class T>
struct UIComboBoxValueType;
template <>
struct UIComboBoxValueType<int> {
    static constexpr PropertyType kType = kPropertyInt;
};
template <>
struct UIComboBoxValueType<Symbol> {
    static constexpr PropertyType kType = kPropertySymbol;
};
template <>
struct UIComboBoxValueType<ResourcePath> {
    static constexpr PropertyType kType = kPropertyResourcePath;
};

// The base of the combo boxes (the map's UIComboBoxBaseCom<T>, instantiated
// for int, ResourcePath and Symbol): a list of entries the pad and the
// keyboard step through, shown in a text object. Its members are emitted
// with each instantiation in ui/UI.o (the int vtable is at 0x195C2A0, the
// ResourcePath one at 0x195C458 and the Symbol one at 0x195C5D0); slots
// 43 and 44 are pure. Only the layout, the property registration and the
// small slots are reconstructed. Field names are not in the reference map.
template <class T>
class UIComboBoxBaseCom : public Component {
public:
    // The index of no entry.
    static constexpr unsigned long kNoIndex = static_cast<unsigned long>(-1);

    // Inlined into the subclasses' constructors.
    UIComboBoxBaseCom()
        : mEntries(),
          mInitiallyDisabled(false),
          mDisabled(false),
          mTextObject(GameObjectId{0xFFFFFFFFU}),
          mDisableWrapping(false),
          mSwitchSound(""),
          mCurrentIndex(kNoIndex),
          mEntered(false),
          mTextDirty(false) {}
    // The imprint's copy keeps the properties and starts without a
    // selection. Inlined into the subclasses' _Imprint.
    UIComboBoxBaseCom(const UIComboBoxBaseCom& other)
        : Component(other),
          mEntries(),
          mInitiallyDisabled(other.mInitiallyDisabled),
          mDisabled(other.mDisabled),
          mTextObject(other.mTextObject),
          mDisableWrapping(other.mDisableWrapping),
          mSwitchSound(other.mSwitchSound),
          mCurrentIndex(kNoIndex),
          mEntered(false),
          mTextDirty(false) {
        mEntries._Copy(other.mEntries);
    }
    // Slots 0-1, such as 0x895420 and 0x895490 for int.
    ~UIComboBoxBaseCom() override {}

    // Slot 2: the "add_entry", "remove_entry" and UI messages, such as
    // 0x895510 for int. Not reconstructed: the UI messages (ui/UIMsg.o) are
    // not modelled.
    DataNode Handle(DataArray* msg, bool warn) override;
    // Slot 31: selects the first entry and shows it, and disables the box
    // when "initially_disabled" asks (0x8959B0 for int). Not
    // reconstructed.
    void _Enter() override;
    // Slot 33: shows the current entry when the text object changed
    // (0x895C00 for int). Not reconstructed.
    void _Poll() override;

    // Slots 41-42: add and remove an entry for the "add_entry" and
    // "remove_entry" messages, returning the entry count; the entries
    // alone change here.
    virtual unsigned int _AddEntryHelper(DataArray* msg) {
        static_cast<void>(msg);
        return mEntries.mSize;
    }
    virtual unsigned int _RemoveEntryHelper(unsigned long index) {
        static_cast<void>(index);
        return mEntries.mSize;
    }
    // Slot 43: shows the current entry in the text object. The map's
    // _UpdateCurrent(ObjPtr const&).
    virtual void _UpdateCurrent() = 0;
    // Slot 44: the value a message node gives.
    virtual T _DataNodeToValue(const DataNode& node) = 0;

    // Map names; not reconstructed.
    void _Fixup(bool notify);
    void _SetCurrentIndex(unsigned long index);
    DataNode OnMsg(const UIButtonDownMsg& msg);
    DataNode OnMsg(const UIKeyboardKeyMsg& msg);
    DataNode _OnAddEntry(DataArray* msg);
    DataNode _OnRemoveEntry(DataArray* msg);

    // The current entry, or the type's empty value. The accessor of
    // "current_value". Name not in the reference map.
    T GetCurrentValue() const {
        T value = T();
        if (mCurrentIndex != kNoIndex && mCurrentIndex < mEntries.mSize) {
            value = mEntries[mCurrentIndex];
        }
        return value;
    }

    // Registers the base class's properties for the subclass and describes
    // the class (interface "UIComboBox"), such as 0x899490 for
    // UIComboBoxEnumCom. Name not in the reference map.
    template <class Derived>
    static void _InitComboBox(PropRegistry& registry, ComMetaData& metadata);

    // The property accessors _InitComboBox installs, emitted with each
    // subclass (0x89A2E0 to 0x89A310 for int). Names not in the reference
    // map.
    static void OnTextObjectChanged(const PropAccessorArgs& args) {
        static_cast<UIComboBoxBaseCom*>(args.mComponent)->mTextDirty = true;
    }
    static unsigned long GetEntryCount(const PropAccessorArgs& args) {
        return static_cast<const UIComboBoxBaseCom*>(args.mComponent)->mEntries.mSize;
    }
    static unsigned long GetCurrentIndex(const PropAccessorArgs& args) {
        return static_cast<const UIComboBoxBaseCom*>(args.mComponent)->mCurrentIndex;
    }
    static T GetCurrentValueProp(const PropAccessorArgs& args) {
        return static_cast<const UIComboBoxBaseCom*>(args.mComponent)->GetCurrentValue();
    }

    // "entry_list": the entries, in the order the box steps through them.
    PropArray<T> mEntries;
    bool mInitiallyDisabled;  // "initially_disabled"
    // Set while the box is disabled. The name is weak.
    bool mDisabled;
    // "text_object": the RndTextCom object that shows the entry.
    GameObjectId mTextObject;
    bool mDisableWrapping;  // "disable_wrapping"
    Symbol mSwitchSound;    // "switch_sound"
    // The selected entry, or kNoIndex.
    unsigned long mCurrentIndex;
    // The names of these two are weak; the second is set when the text
    // object changes.
    bool mEntered;
    bool mTextDirty;
};

static_assert(offsetof(UIComboBoxBaseCom<int>, mEntries) == 24);
static_assert(offsetof(UIComboBoxBaseCom<int>, mInitiallyDisabled) == 64);
static_assert(offsetof(UIComboBoxBaseCom<int>, mTextObject) == 68);
static_assert(offsetof(UIComboBoxBaseCom<int>, mDisableWrapping) == 72);
static_assert(offsetof(UIComboBoxBaseCom<int>, mSwitchSound) == 80);
static_assert(offsetof(UIComboBoxBaseCom<int>, mCurrentIndex) == 88);
static_assert(offsetof(UIComboBoxBaseCom<int>, mTextDirty) == 97);
static_assert(sizeof(UIComboBoxBaseCom<int>) == 104);
static_assert(sizeof(UIComboBoxBaseCom<Symbol>) == 104);
static_assert(sizeof(UIComboBoxBaseCom<ResourcePath>) == 104);

// Every property records the prototype's value as its default; the
// read-only ones have no storage and are neither saved nor, apart from the
// entry count, recorded in the editor's history.
template <class T>
template <class Derived>
void UIComboBoxBaseCom<T>::_InitComboBox(PropRegistry& registry, ComMetaData& metadata) {
    Derived prototype;
    if (&registry != &Derived::sPropRegistry) {
        registry.AddInheritedRegistry(Derived::sClassName, Derived::sPropRegistry);
    }

    PropInfo& entries = registry.RegisterPropArray(
        "entry_list", offsetof(UIComboBoxBaseCom, mEntries), sizeof(PropArray<T>));
    ArrayMetadata& entryList = static_cast<ArrayMetadata&>(*entries.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    entryList.mInstanceAccess = PropMetadata::kInstanceAccessSet;
    entryList.mHelp = String(
        "The available list of entries. Order specified is how they will sort in combo box.");
    PropRegistry::RegisterPropArrayItem(entries, UIComboBoxValueType<T>::kType, sizeof(T));

    GameObjectIdMetadata& textObject = TypeSpecificMetadata(
        registry.RegisterProp(
            "text_object", offsetof(UIComboBoxBaseCom, mTextObject), kPropertyGameObjectId, 4),
        prototype.mTextObject);
    textObject.mDefault = prototype.mTextObject;
    static_cast<void>(Symbol("prop"));
    textObject.mRequiredComs = {RndTextCom::sId};
    textObject.mOnChanged = OnTextObjectChanged;
    textObject.mHelp = String("Text object to use to display selected value.");

    SymbolMetadata& switchSound = TypeSpecificMetadata(
        registry.RegisterProp(
            "switch_sound", offsetof(UIComboBoxBaseCom, mSwitchSound), kPropertySymbol, 8),
        prototype.mSwitchSound);
    switchSound.mDefault = prototype.mSwitchSound;
    static_cast<void>(Symbol("prop"));
    switchSound.mHelp = String("Sound to play when switching values.");

    BoolMetadata& initiallyDisabled = TypeSpecificMetadata(
        registry.RegisterProp(
            "initially_disabled",
            offsetof(UIComboBoxBaseCom, mInitiallyDisabled),
            kPropertyBool,
            1),
        prototype.mInitiallyDisabled);
    initiallyDisabled.mDefault = prototype.mInitiallyDisabled;
    static_cast<void>(Symbol("prop"));
    initiallyDisabled.mInstanceAccess = PropMetadata::kInstanceAccessSet;
    initiallyDisabled.mHelp = String("Should this widget be set to disabled state on enter?");

    BoolMetadata& disableWrapping = TypeSpecificMetadata(
        registry.RegisterProp(
            "disable_wrapping", offsetof(UIComboBoxBaseCom, mDisableWrapping), kPropertyBool, 1),
        prototype.mDisableWrapping);
    disableWrapping.mDefault = prototype.mDisableWrapping;
    static_cast<void>(Symbol("prop"));
    disableWrapping.mHelp = String("Should we disable navigation wrapping?");
    disableWrapping.mInstanceAccess = PropMetadata::kInstanceAccessSet;

    const unsigned long countType = 0;
    IntMetadata<unsigned long>& entryCount = TypeSpecificMetadata(
        registry.RegisterProp("entry_count", -1, kPropertyUInt64, 0), countType);
    static_cast<void>(Symbol("prop"));
    entryCount.mHelp = String("Number of entries in the ComboBox");
    entryCount.mReadOnly = true;
    entryCount.mSaves = false;
    entryCount.mGetter = GetEntryCount;

    const unsigned long indexType = 0;
    IntMetadata<unsigned long>& currentIndex = TypeSpecificMetadata(
        registry.RegisterProp("current_index", -1, kPropertyUInt64, 0), indexType);
    static_cast<void>(Symbol("prop"));
    currentIndex.mHelp = String("The current index of the ComboBox (read-only)");
    currentIndex.mReadOnly = true;
    currentIndex.mSaves = false;
    currentIndex.mGetter = GetCurrentIndex;
    currentIndex.mHistory = false;
    currentIndex.mInstanceAccess = PropMetadata::kInstanceAccessView;

    const T valueType = T();
    auto& currentValue = TypeSpecificMetadata(
        registry.RegisterProp("current_value", -1, UIComboBoxValueType<T>::kType, 0), valueType);
    static_cast<void>(Symbol("prop"));
    currentValue.mHelp = String("The current value of the ComboBox (read-only)");
    currentValue.mReadOnly = true;
    currentValue.mSaves = false;
    currentValue.mGetter = GetCurrentValueProp;
    currentValue.mHistory = false;
    currentValue.mInstanceAccess = PropMetadata::kInstanceAccessView;

    metadata.mDescription = "Combo Box";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mInterface = Symbol("UIComboBox");
    metadata.mInstanceCom = true;
}
