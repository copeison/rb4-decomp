// ui/UIListCom.o (0x8B2530 to 0x8C364F). The registration and the factory
// are emitted in ui/UI.o. The handler, the game-mode poll and the scrolling,
// focus and instance management are not reconstructed.
#include "ui/lists/UIListCom.h"

#include <cmath>
#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/scene/RndEntityResource.h"
#include "ui/input/UIInputReceiverCom.h"
#include "ui/widgets/UIScrollbarCom.h"
#include "utl/time/TimeMgr.h"

namespace {

// The list a tag function or a property accessor is given. Names not in
// the reference map.
const UIListCom& ListOf(const TagArgs& args) {
    return static_cast<const UIListCom&>(args.mComponent);
}

UIListCom& ListOf(const PropAccessorArgs& args) {
    return *static_cast<UIListCom*>(args.mComponent);
}

// The entity of the loaded focus file, or null. Inlined into the tag
// functions of the focus bindings and the clip planes (0x8C2D30, 0x8C2F80,
// 0x8C30B0). Name not in the reference map.
Entity* FocusEntity(const TagArgs& args) {
    RndEntityResource* const resource = ListOf(args).mRuntime.mFocusResource.Get();
    if (resource == nullptr || resource->Fail()) {
        return nullptr;
    }
    return resource->mEntity;
}

using ChangeCallback = void (*)(const PropAccessorArgs& args);

// Registers the float members "x", "y" and "z" of a 3D vector struct with
// zero defaults and their change callbacks. Inlined into every spacing
// struct of the registry. Name not in the reference map.
void RegisterVector3Members(
    PropRegistry& registry,
    ChangeCallback onXChanged,
    ChangeCallback onYChanged,
    ChangeCallback onZChanged) {
    const Vector3 prototype = {0.0F, 0.0F, 0.0F};
    FloatMetadata& x = TypeSpecificMetadata(
        registry.RegisterProp("x", offsetof(Vector3, x), kPropertyFloat, sizeof(float)), prototype.x);
    x.mDefault = prototype.x;
    static_cast<void>(Symbol("prop"));
    x.mOnChanged = onXChanged;
    FloatMetadata& y = TypeSpecificMetadata(
        registry.RegisterProp("y", offsetof(Vector3, y), kPropertyFloat, sizeof(float)), prototype.y);
    y.mDefault = prototype.y;
    static_cast<void>(Symbol("prop"));
    y.mOnChanged = onYChanged;
    FloatMetadata& z = TypeSpecificMetadata(
        registry.RegisterProp("z", offsetof(Vector3, z), kPropertyFloat, sizeof(float)), prototype.z);
    z.mDefault = prototype.z;
    static_cast<void>(Symbol("prop"));
    z.mOnChanged = onZChanged;
}

// Registers the members "property" and "data_name" of a binding array's
// struct. The property names into the entity `entity` gives; `setter`
// assigns the reference to the binding. Inlined for "property_bindings"
// and "focus_property_bindings". Name not in the reference map.
void RegisterBindingMembers(
    PropRegistry& registry,
    std::function<Entity*(const TagArgs&)>&& entity,
    PropRefMetadata::RefSetter setter,
    ChangeCallback onPropertyChanged,
    ChangeCallback onDataNameChanged) {
    const UIListCom::PropertyBinding prototype;
    PropRefMetadata& property = TypeSpecificMetadata(
        registry.RegisterProp(
            "property",
            offsetof(UIListCom::PropertyBinding, mProperty),
            kPropertyPropRef,
            sizeof(PropRef)),
        prototype.mProperty);
    property.mRefDefault = prototype.mProperty;
    static_cast<void>(Symbol("prop"));
    property.mEntity._Set(static_cast<std::function<Entity*(const TagArgs&)>&&>(entity));
    property.mRefSetter = setter;
    property.mHelp = String("The property to apply the column bindings to.");
    property.mRefOnChanged = onPropertyChanged;

    SymbolMetadata& dataName = TypeSpecificMetadata(
        registry.RegisterProp(
            "data_name", offsetof(UIListCom::PropertyBinding, mDataName), kPropertySymbol, sizeof(Symbol)),
        prototype.mDataName);
    dataName.mDefault = prototype.mDataName;
    static_cast<void>(Symbol("prop"));
    dataName.mHelp = String("The name of the data.");
    dataName.mOnChanged = onDataNameChanged;
}

// Marks a debug view: read-only, unsaved, shown among the debug
// properties. Inlined into each view. Name not in the reference map.
void MarkDebugView(PropMetadata& metadata) {
    metadata.mDebug = true;
    metadata.mReadOnly = true;
    metadata.mSaves = false;
}

// Registers a read-only debug view without storage, read through the
// getter. Inlined into each view. Name not in the reference map.
template <typename T>
void RegisterDebugView(
    PropRegistry& registry,
    const char* name,
    PropertyType type,
    typename PropAccessors<T>::Getter getter) {
    const T value = T();
    auto& metadata = TypeSpecificMetadata(registry.RegisterProp(name, -1, type, 0), value);
    static_cast<void>(Symbol("prop"));
    MarkDebugView(metadata);
    metadata.mGetter = getter;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8C3590).
// The int it first sets to -1 (0x1AC9030) comes from a shared header and is
// not modelled.
Symbol UIListCom::sId("UIListCom");
Symbol UIListCom::sClassName("UIListCom");
PropRegistry UIListCom::sPropRegistry;
ComMetaData UIListCom::sMetaData;

// Reconstructed from eboot.elf at 0x8B2100.
void UIListCom::RuntimeData::Destroy() {
    while (mGridItems.mSize != 0) {
        delete mGridItems.PopFront();
    }
    while (mUnusedItems.mSize != 0) {
        delete mUnusedItems.PopFront();
    }
    while (mFocusItems.mSize != 0) {
        delete mFocusItems.PopFront();
    }
}

// Reconstructed from eboot.elf at 0x8B21E0. A linked subscription leaves
// its source before its link unlinks itself.
UIListCom::RuntimeData::~RuntimeData() {
    for (MsgSource::EventSinkElem* sink :
         {&mButtonDownSink, &mKeySink, &mRowsClearedSink, &mEntryChangedSink, &mRowRemovedSink, &mRowAddedSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x8B2530. "num_columns" is left unset.
UIListCom::UIListCom()
    : mRuntime(),
      mRows(1),
      mMaxItemsDisplayed(10),
      mMaxItemsAllocated(10),
      mFilePath(),
      mPropertyBindings(),
      mGridLayout(kGridLayoutNone),
      mNavigationDirection(kNavigationVertical),
      mScrollTime(1.0F),
      mAllowEdgeScroll(false),
      mRequiresPropRefFixup(true),
      mTopScrollBuffer(0),
      mBottomScrollBuffer(0),
      mSpacing{0.0F, 0.0F, 10.0F},
      mHorizontalSpacing(0.0F),
      mVerticalSpacing(0.0F),
      mInstancePadding(2),
      mScrollbar(GameObjectId{0xFFFFFFFFU}),
      mWrapNavigation(false),
      mRadius(0.0F),
      mStaticFocus() {}

// Reconstructed from eboot.elf at 0x8B28E0. The deleting destructor is at
// 0x8B2B50.
UIListCom::~UIListCom() {
    _DeleteGrid();
    mRuntime.Destroy();
}

// Reconstructed from eboot.elf at 0x8B2B70. Revision 1 gives older saves
// as many allocated items as displayed ones; the handler's call operator is
// at 0x8C32E0. The tag functions' call operators are at 0x8C14D0 to
// 0x8C31E0 and the accessors at 0x8BFCE0 to 0x8C0770.
void UIListCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Lists of selectable entities.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mRequiredComponents.push_back(RndDrawNodeCom::sClassName);
    metadata.mRequiredComponents.push_back(TransCom::sClassName);
    RegisterPropStruct<PropertyBinding>(Symbol("UIListCom::PropertyBinding"));
    UIListCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    ResourcePathMetadata& filePath = TypeSpecificMetadata(
        registry.RegisterProp(
            "file_path", offsetof(UIListCom, mFilePath), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mFilePath);
    filePath.mDefault = prototype.mFilePath;
    static_cast<void>(Symbol("prop"));
    filePath.mHelp = String("Entity file to instance.");
    filePath.mOnChanged = [](const PropAccessorArgs&) {};
    filePath.mResourceId = TransEntityResource::Id();

    IntMetadata<int>& rows = TypeSpecificMetadata(
        registry.RegisterProp("rows", offsetof(UIListCom, mRows), kPropertyInt, sizeof(int)), prototype.mRows);
    rows.mDefault = prototype.mRows;
    static_cast<void>(Symbol("prop"));
    rows.mHelp = String("Number of rows if this is a grid.");
    rows.mOnChanged = [](const PropAccessorArgs&) {};
    rows.mMin = 0;
    rows.mMax = 10;
    rows.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout == kGridLayoutNone; });

    // Outside the "Set Number of Columns" layout a change recomputes the
    // columns from the items showing and the rows.
    IntMetadata<int>& numColumns = TypeSpecificMetadata(
        registry.RegisterProp("num_columns", offsetof(UIListCom, mNumColumns), kPropertyInt, sizeof(int)),
        prototype.mNumColumns);
    numColumns.mDefault = prototype.mNumColumns;
    static_cast<void>(Symbol("prop"));
    numColumns.mHelp = String("Number of columns (of data) in each list/grid entity.");
    numColumns.mOnChanged = [](const PropAccessorArgs& args) {
        UIListCom& list = ListOf(args);
        if (list.mGridLayout == kGridLayoutSetColumns) {
            return;
        }
        if (list.mRows == 0) {
            list.mNumColumns = 1;
            return;
        }
        const float columns = std::ceil(
            static_cast<float>(list.mRuntime.mGridItems.mSize) / static_cast<float>(list.mRows));
        list.mNumColumns = static_cast<int>(static_cast<unsigned long>(columns));
    };
    numColumns.mReadOnly._Set(
        [](const TagArgs& args) { return ListOf(args).mGridLayout != kGridLayoutSetColumns; });

    const bool scrollingType = false;
    BoolMetadata& isScrolling =
        TypeSpecificMetadata(registry.RegisterProp("is_scrolling", -1, kPropertyBool, 0), scrollingType);
    static_cast<void>(Symbol("prop"));
    isScrolling.mReadOnly = true;
    isScrolling.mHelp = String("A flag to indicate when the list is currently scrolling.");
    isScrolling.mGetter = [](const PropAccessorArgs& args) { return ListOf(args).IsScrolling(); };

    BoolMetadata& onlyAppScrolling = TypeSpecificMetadata(
        registry.RegisterProp(
            "only_app_scrolling",
            offsetof(UIListCom, mRuntime) + offsetof(RuntimeData, mOnlyAppScrolling),
            kPropertyBool,
            sizeof(bool)),
        prototype.mRuntime.mOnlyAppScrolling);
    onlyAppScrolling.mDefault = prototype.mRuntime.mOnlyAppScrolling;
    static_cast<void>(Symbol("prop"));
    onlyAppScrolling.mHelp =
        String("When true, only code/stategraph can scroll the list, and user input is ignored.");

    // The override when one is set, else the input receiver's.
    const bool canSelectType = false;
    BoolMetadata& canSelect =
        TypeSpecificMetadata(registry.RegisterProp("can_select", -1, kPropertyBool, 0), canSelectType);
    static_cast<void>(Symbol("prop"));
    canSelect.mHelp = String("Can the list and its focused element be selected?");
    canSelect.mReadOnly = true;
    canSelect.mGetter = [](const PropAccessorArgs& args) {
        const UIListCom& list = ListOf(args);
        if (list.mRuntime.mCanSelectOverridden) {
            return list.mRuntime.mCanSelect;
        }
        if (list.mRuntime.mReceiver != nullptr) {
            return list.mRuntime.mReceiver->mCanSelect;
        }
        return false;
    };

    // The displayed items lie between the scroll buffers with the padding
    // and the allocated items.
    IntMetadata<int>& maxDisplayed = TypeSpecificMetadata(
        registry.RegisterProp(
            "max_items_displayed", offsetof(UIListCom, mMaxItemsDisplayed), kPropertyInt, sizeof(int)),
        prototype.mMaxItemsDisplayed);
    maxDisplayed.mDefault = prototype.mMaxItemsDisplayed;
    static_cast<void>(Symbol("prop"));
    maxDisplayed.mHelp = String(
        "Max number of items displayed. You must have max items allocated be at least the value you want to "
        "set here.");
    maxDisplayed.mOnChanged = [](const PropAccessorArgs&) {};
    maxDisplayed.mMin._Set([](const TagArgs& args) {
        const UIListCom& list = ListOf(args);
        return static_cast<int>(list.mBottomScrollBuffer + list.mTopScrollBuffer) + list.mInstancePadding;
    });
    maxDisplayed.mMax._Set([](const TagArgs& args) { return ListOf(args).mMaxItemsAllocated; });

    IntMetadata<int>& maxAllocated = TypeSpecificMetadata(
        registry.RegisterProp(
            "max_items_allocated", offsetof(UIListCom, mMaxItemsAllocated), kPropertyInt, sizeof(int)),
        prototype.mMaxItemsAllocated);
    maxAllocated.mDefault = prototype.mMaxItemsAllocated;
    static_cast<void>(Symbol("prop"));
    maxAllocated.mHelp = String("Max number of items we will allocate.");
    maxAllocated.mOnChanged = [](const PropAccessorArgs&) {};
    maxAllocated.mMin._Set([](const TagArgs& args) {
        const UIListCom& list = ListOf(args);
        return static_cast<int>(list.mBottomScrollBuffer + list.mTopScrollBuffer) + list.mInstancePadding;
    });
    maxAllocated.mMax = 1000;

    GroupMetadata& spacing = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "spacing", offsetof(UIListCom, mSpacing), kPropertyStruct, sizeof(Vector3)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    spacing.mKind = 1;
    RegisterVector3Members(
        spacing.mRegistry,
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); });
    spacing.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout != kGridLayoutNone; });
    spacing.mHelp = String("XYZ spacing between entities' centers in a non-grid view.");

    FloatMetadata& horizontal = TypeSpecificMetadata(
        registry.RegisterProp(
            "horizontal_spacing", offsetof(UIListCom, mHorizontalSpacing), kPropertyFloat, sizeof(float)),
        prototype.mHorizontalSpacing);
    horizontal.mDefault = prototype.mHorizontalSpacing;
    static_cast<void>(Symbol("prop"));
    horizontal.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout == kGridLayoutNone; });
    horizontal.mHelp = String("Horizontal spacing between elements on a grid.");
    horizontal.mOnChanged = [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); };

    FloatMetadata& vertical = TypeSpecificMetadata(
        registry.RegisterProp(
            "vertical_spacing", offsetof(UIListCom, mVerticalSpacing), kPropertyFloat, sizeof(float)),
        prototype.mVerticalSpacing);
    vertical.mDefault = prototype.mVerticalSpacing;
    static_cast<void>(Symbol("prop"));
    vertical.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout == kGridLayoutNone; });
    vertical.mHelp = String("Vertical spacing between elements on a grid.");
    vertical.mOnChanged = [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); };

    FloatMetadata& radius = TypeSpecificMetadata(
        registry.RegisterProp("radius", offsetof(UIListCom, mRadius), kPropertyFloat, sizeof(float)),
        prototype.mRadius);
    radius.mDefault = prototype.mRadius;
    static_cast<void>(Symbol("prop"));
    radius.mHelp = String("Bend the grid around a radius - will not work with scrolling or non-grid.");
    radius.mIsHidden._Set([](const TagArgs& args) { return ListOf(args).mGridLayout == kGridLayoutNone; });
    radius.mOnChanged = [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); };

    FloatMetadata& scrollTime = TypeSpecificMetadata(
        registry.RegisterProp("scroll_time", offsetof(UIListCom, mScrollTime), kPropertyFloat, sizeof(float)),
        prototype.mScrollTime);
    scrollTime.mDefault = prototype.mScrollTime;
    static_cast<void>(Symbol("prop"));
    scrollTime.mHelp = String("How long it takes to scroll.");

    // Each buffer leaves room for the other and the focus among the
    // displayed items; the static focus replaces both.
    IntMetadata<unsigned int>& topBuffer = TypeSpecificMetadata(
        registry.RegisterProp(
            "top_scroll_buffer", offsetof(UIListCom, mTopScrollBuffer), kPropertyUInt, sizeof(unsigned int)),
        prototype.mTopScrollBuffer);
    topBuffer.mDefault = prototype.mTopScrollBuffer;
    static_cast<void>(Symbol("prop"));
    topBuffer.mHelp = String(
        "Minimum number of visible elements above the current focus.  Changing focus beyond the buffer will "
        "scroll the list if the list has room to scroll.");
    topBuffer.mMin = 0U;
    topBuffer.mMax._Set([](const TagArgs& args) {
        const UIListCom& list = ListOf(args);
        return static_cast<unsigned int>(list.mMaxItemsDisplayed - 1) - list.mBottomScrollBuffer;
    });
    topBuffer.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mStaticFocus.mEnable; });

    IntMetadata<unsigned int>& bottomBuffer = TypeSpecificMetadata(
        registry.RegisterProp(
            "bottom_scroll_buffer",
            offsetof(UIListCom, mBottomScrollBuffer),
            kPropertyUInt,
            sizeof(unsigned int)),
        prototype.mBottomScrollBuffer);
    bottomBuffer.mDefault = prototype.mBottomScrollBuffer;
    static_cast<void>(Symbol("prop"));
    bottomBuffer.mHelp = String(
        "Minimum number of elements below the current focus.  Changing focus beyond the buffer will scroll the "
        "list if the list has room to scroll.");
    bottomBuffer.mMin = 0U;
    bottomBuffer.mMax._Set([](const TagArgs& args) {
        const UIListCom& list = ListOf(args);
        return static_cast<unsigned int>(list.mMaxItemsDisplayed - 1) - list.mTopScrollBuffer;
    });
    bottomBuffer.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mStaticFocus.mEnable; });

    IntMetadata<int>& padding = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_padding", offsetof(UIListCom, mInstancePadding), kPropertyInt, sizeof(int)),
        prototype.mInstancePadding);
    padding.mDefault = prototype.mInstancePadding;
    static_cast<void>(Symbol("prop"));
    padding.mHelp = String(
        "Adds more instances into the pool to make sure that we always have one to add to the bottom/top to "
        "keep scrolling smooth.  Note:  This number allocates more entity instances so keep it as low as you "
        "need to make scrolling feel good.");
    padding.mMin = 1;
    padding.mMax = 10;
    padding.mOnChanged = [](const PropAccessorArgs&) {};

    BoolMetadata& edgeScroll = TypeSpecificMetadata(
        registry.RegisterProp(
            "allow_edge_scroll", offsetof(UIListCom, mAllowEdgeScroll), kPropertyBool, sizeof(bool)),
        prototype.mAllowEdgeScroll);
    edgeScroll.mDefault = prototype.mAllowEdgeScroll;
    static_cast<void>(Symbol("prop"));
    edgeScroll.mHelp = String("Allow the edges to scroll. Ha...i have no idea.");
    edgeScroll.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mStaticFocus.mEnable; });

    IntMetadata<int>& gridLayout = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "grid_layout", offsetof(UIListCom, mGridLayout), kPropertyInt, sizeof(int)).mMetadata.Get());
    gridLayout.mDefault = prototype.mGridLayout;
    static_cast<void>(Symbol("prop"));
    gridLayout.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kGridLayoutFillColumnFirst, String("Fill Column First"), String("")},
        AllowedValue<int>{kGridLayoutFillRowFirst, String("Fill Row First"), String("")},
        AllowedValue<int>{kGridLayoutNone, String("No Grid"), String("")},
        AllowedValue<int>{
            kGridLayoutSetColumns, String("Set Number of Columns and Fill Rows First"), String("")}};
    gridLayout.mOnChanged = [](const PropAccessorArgs& args) {
        ListOf(args)._UpdateGridTransforms();
        ListOf(args)._UpdateInstanceProperties();
    };
    gridLayout.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mStaticFocus.mEnable; });

    IntMetadata<int>& navigation = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "navigation_direction", offsetof(UIListCom, mNavigationDirection), kPropertyInt, sizeof(int)).mMetadata.Get());
    navigation.mDefault = prototype.mNavigationDirection;
    static_cast<void>(Symbol("prop"));
    navigation.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kNavigationHorizontal, String("Horizontal Navigation"), String("")},
        AllowedValue<int>{kNavigationVertical, String("Vertical Navigation"), String("")}};
    navigation.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout != kGridLayoutNone; });

    // The bound properties name members of the spawner's prototype entity.
    PropInfo& bindings = registry.RegisterPropArray(
        "property_bindings", offsetof(UIListCom, mPropertyBindings), sizeof(PropArray<PropertyBinding>));
    ArrayMetadata& bindingsMetadata = static_cast<ArrayMetadata&>(*bindings.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(bindings, kPropertyStruct, sizeof(PropertyBinding));
    PropRegistry& bindingMembers =
        static_cast<GroupMetadata&>(*bindingsMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    RegisterBindingMembers(
        bindingMembers,
        [](const TagArgs& args) { return ListOf(args).mRuntime.mPrototype; },
        [](const PropAccessorArgs& args) {
            const PropPath& path = *static_cast<const PropPath*>(args.mPath);
            ListOf(args).mPropertyBindings[path[-2].mValue].SetProperty(*static_cast<const PropRef*>(args.mValue));
        },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateInstanceProperties(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateInstanceProperties(); });

    GameObjectIdMetadata& scrollbar = TypeSpecificMetadata(
        registry.RegisterProp(
            "scrollbar", offsetof(UIListCom, mScrollbar), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mScrollbar);
    scrollbar.mDefault = prototype.mScrollbar;
    static_cast<void>(Symbol("prop"));
    scrollbar.mHelp =
        String("The scrollbar object to use. Must be an entity instance with a UIScrollbarCom on root.");
    scrollbar.mFilterClass = UIScrollbarCom::sId;

    BoolMetadata& wrap = TypeSpecificMetadata(
        registry.RegisterProp(
            "wrap_navigation", offsetof(UIListCom, mWrapNavigation), kPropertyBool, sizeof(bool)),
        prototype.mWrapNavigation);
    wrap.mDefault = prototype.mWrapNavigation;
    static_cast<void>(Symbol("prop"));
    wrap.mHelp = String(
        "When we get to the last item in the list, focus the first one again.  NOTE:  This isn't handling "
        "scrolling, so only use it when the whole list is visible at once.");
    wrap.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mStaticFocus.mEnable; });

    // Only lists without a grid have a static focus; its members are
    // read-only until it is enabled, and the focus file's members until a
    // file is given.
    GroupMetadata& staticFocus = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "static_focus", offsetof(UIListCom, mStaticFocus), kPropertyStruct, sizeof(StaticFocus)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const StaticFocus focusPrototype;
    PropRegistry& focusMembers = staticFocus.mRegistry;
    staticFocus.mHelp = String(
        "Optionally override which visible index will always show the focused item. This implies that the "
        "list will be scroll while the focused item will always be in a static spot.");
    staticFocus.mReadOnly._Set([](const TagArgs& args) { return ListOf(args).mGridLayout != kGridLayoutNone; });

    BoolMetadata& enable = TypeSpecificMetadata(
        focusMembers.RegisterProp("enable", offsetof(StaticFocus, mEnable), kPropertyBool, sizeof(bool)),
        focusPrototype.mEnable);
    enable.mDefault = focusPrototype.mEnable;
    static_cast<void>(Symbol("prop"));
    enable.mHelp = String("Enable this mode.");

    IntMetadata<unsigned long>& visibleIndex = TypeSpecificMetadata(
        focusMembers.RegisterProp(
            "visible_index", offsetof(StaticFocus, mVisibleIndex), kPropertyUInt64, sizeof(unsigned long)),
        focusPrototype.mVisibleIndex);
    visibleIndex.mDefault = focusPrototype.mVisibleIndex;
    static_cast<void>(Symbol("prop"));
    visibleIndex.mHelp = String(
        "The visible index that we should set the focus to. This must be within the max displayed limits.");
    visibleIndex.mDefault = 0UL;
    visibleIndex.mMin = 0UL;
    visibleIndex.mMax._Set([](const TagArgs& args) {
        return static_cast<unsigned long>(static_cast<long>(ListOf(args).mMaxItemsDisplayed) - 1);
    });
    visibleIndex.mReadOnly._Set([](const TagArgs& args) { return !ListOf(args).mStaticFocus.mEnable; });
    visibleIndex.mOnChanged = [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); };

    GroupMetadata& focusSpacing = static_cast<GroupMetadata&>(*focusMembers.RegisterProp(
        "spacing", offsetof(StaticFocus, mSpacing), kPropertyStruct, sizeof(Vector3)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    focusSpacing.mKind = 1;
    RegisterVector3Members(
        focusSpacing.mRegistry,
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); });
    focusSpacing.mReadOnly._Set([](const TagArgs& args) { return !ListOf(args).mStaticFocus.mEnable; });
    focusSpacing.mHelp = String("XYZ spacing before and after the static focus entity's center in non-grid view.");

    ResourcePathMetadata& focusFile = TypeSpecificMetadata(
        focusMembers.RegisterProp(
            "focus_file_path", offsetof(StaticFocus, mFocusFilePath), kPropertyResourcePath, sizeof(ResourcePath)),
        focusPrototype.mFocusFilePath);
    focusFile.mDefault = focusPrototype.mFocusFilePath;
    static_cast<void>(Symbol("prop"));
    focusFile.mHelp = String("The entity file to instance for the focused entity.");
    focusFile.mResourceId = TransEntityResource::Id();
    focusFile.mReadOnly._Set([](const TagArgs& args) { return !ListOf(args).mStaticFocus.mEnable; });

    PropInfo& focusBindings = focusMembers.RegisterPropArray(
        "focus_property_bindings",
        offsetof(StaticFocus, mFocusPropertyBindings),
        sizeof(PropArray<PropertyBinding>));
    ArrayMetadata& focusBindingsMetadata = static_cast<ArrayMetadata&>(*focusBindings.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    focusBindingsMetadata.mReadOnly._Set([](const TagArgs& args) {
        const StaticFocus& focus = ListOf(args).mStaticFocus;
        return !focus.mEnable || focus.mFocusFilePath.mPath == Symbol();
    });
    PropRegistry::RegisterPropArrayItem(focusBindings, kPropertyStruct, sizeof(PropertyBinding));
    PropRegistry& focusBindingMembers =
        static_cast<GroupMetadata&>(*focusBindingsMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    RegisterBindingMembers(
        focusBindingMembers,
        [](const TagArgs& args) { return FocusEntity(args); },
        [](const PropAccessorArgs& args) {
            const PropPath& path = *static_cast<const PropPath*>(args.mPath);
            ListOf(args).mStaticFocus.mFocusPropertyBindings[path[-2].mValue].SetProperty(
                *static_cast<const PropRef*>(args.mValue));
        },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateInstanceProperties(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateInstanceProperties(); });

    GroupMetadata& instanceSpacing = static_cast<GroupMetadata&>(*focusMembers.RegisterProp(
        "focus_instance_spacing",
        offsetof(StaticFocus, mFocusInstanceSpacing),
        kPropertyStruct,
        sizeof(Vector3)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    RegisterVector3Members(
        instanceSpacing.mRegistry,
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); },
        [](const PropAccessorArgs& args) { ListOf(args)._UpdateGridTransforms(); });
    instanceSpacing.mReadOnly._Set([](const TagArgs& args) {
        const StaticFocus& focus = ListOf(args).mStaticFocus;
        return !focus.mEnable || focus.mFocusFilePath.mPath == Symbol();
    });
    instanceSpacing.mHelp = String("XYZ spacing between the center of the focus instances.");
    instanceSpacing.mKind = 1;

    // The clip planes are transforms of the focus file's entity.
    GameObjectIdMetadata& clipUp = TypeSpecificMetadata(
        focusMembers.RegisterProp(
            "clip_plane_up", offsetof(StaticFocus, mClipPlaneUp), kPropertyGameObjectId, sizeof(GameObjectId)),
        focusPrototype.mClipPlaneUp);
    clipUp.mDefault = focusPrototype.mClipPlaneUp;
    static_cast<void>(Symbol("prop"));
    clipUp.mEntity._Set([](const TagArgs& args) { return FocusEntity(args); });
    clipUp.mRequiredComs = {TransCom::sId};

    GameObjectIdMetadata& clipDown = TypeSpecificMetadata(
        focusMembers.RegisterProp(
            "clip_plane_down", offsetof(StaticFocus, mClipPlaneDown), kPropertyGameObjectId, sizeof(GameObjectId)),
        focusPrototype.mClipPlaneDown);
    clipDown.mDefault = focusPrototype.mClipPlaneDown;
    static_cast<void>(Symbol("prop"));
    clipDown.mEntity._Set([](const TagArgs& args) { return FocusEntity(args); });
    clipDown.mRequiredComs = {TransCom::sId};

    BoolMetadata& fixup = TypeSpecificMetadata(
        registry.RegisterProp(
            "requires_propref_fixup", offsetof(UIListCom, mRequiresPropRefFixup), kPropertyBool, sizeof(bool)),
        prototype.mRequiresPropRefFixup);
    fixup.mDefault = prototype.mRequiresPropRefFixup;
    static_cast<void>(Symbol("prop"));
    fixup.mIsHidden = true;

    IntMetadata<unsigned int>& editIndex = TypeSpecificMetadata(
        registry.RegisterProp(
            "edit_in_place_index",
            offsetof(UIListCom, mRuntime) + offsetof(RuntimeData, mEditInPlaceIndex),
            kPropertyUInt,
            sizeof(unsigned int)),
        prototype.mRuntime.mEditInPlaceIndex);
    editIndex.mDefault = prototype.mRuntime.mEditInPlaceIndex;
    static_cast<void>(Symbol("prop"));
    editIndex.mSaves = false;
    editIndex.mHistory = false;
    editIndex.mDebug = true;
    editIndex.mMin = 0U;

    // The action does nothing in this build.
    ActionMetadata& editInPlace = static_cast<ActionMetadata&>(
        *registry.RegisterAction("hacky_edit_in_place_set_breakpoints_before_using").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    editInPlace.mDebug = true;
    editInPlace.mHistory = false;
    editInPlace.mAction = [](const TagArgs&) {};

    // The debug views.
    RegisterDebugView<unsigned int>(registry, "num_data_rows", kPropertyUInt, [](const PropAccessorArgs& args) {
        const UIListCom& list = ListOf(args);
        return list.mRuntime.mData != nullptr ? static_cast<unsigned int>(list.mRuntime.mData->mRows.size()) : 0U;
    });
    RegisterDebugView<unsigned int>(registry, "num_showing", kPropertyUInt, [](const PropAccessorArgs& args) {
        return static_cast<unsigned int>(ListOf(args).GetNumShowing());
    });
    RegisterDebugView<unsigned int>(
        registry, "num_total_elements", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).GetTotalNumElements());
        });
    RegisterDebugView<unsigned int>(
        registry, "get_max_items_displayed", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).GetMaxItemsDisplayed());
        });
    RegisterDebugView<unsigned int>(
        registry, "data_focus_index_no_scroll", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).GetDataFocusIndex(false));
        });
    RegisterDebugView<unsigned int>(
        registry, "data_focus_index_scroll", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).GetDataFocusIndex(true));
        });
    RegisterDebugView<unsigned int>(
        registry, "visible_focus_index", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).GetVisibleFocusIndex());
        });
    RegisterDebugView<bool>(registry, "can_scroll_up", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args).CanScroll(kListDirectionUp);
    });
    RegisterDebugView<bool>(registry, "can_scroll_down", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args).CanScroll(kListDirectionDown);
    });
    RegisterDebugView<bool>(registry, "scrolled_to_top", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args).mRuntime.mTopDataIndex == 0;
    });
    RegisterDebugView<bool>(registry, "scrolled_to_bottom", kPropertyBool, [](const PropAccessorArgs& args) {
        const UIListCom& list = ListOf(args);
        return static_cast<long>(list.GetNumShowing()) + list.mRuntime.mTopDataIndex - 1 ==
            list.GetTotalNumElements() - 1;
    });
    RegisterDebugView<bool>(registry, "is_at_top_list_edge", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args)._IsAtListEdge(kListPositionTop);
    });
    RegisterDebugView<bool>(registry, "is_at_bottom_list_edge", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args)._IsAtListEdge(kListPositionBottom);
    });
    RegisterDebugView<unsigned int>(
        registry, "data_index_at_top_edge", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args)._GetDataIndexAtListEdge(kListPositionTop));
        });
    RegisterDebugView<unsigned int>(
        registry, "data_index_at_bottom_edge", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args)._GetDataIndexAtListEdge(kListPositionBottom));
        });
    RegisterDebugView<int>(registry, "update_properties_start_index", kPropertyInt, [](const PropAccessorArgs& args) {
        return ListOf(args)._GetUpdatePropertiesStartIndex();
    });
    RegisterDebugView<int>(registry, "top_effective_scroll_buffer", kPropertyInt, [](const PropAccessorArgs& args) {
        return static_cast<int>(ListOf(args)._GetEffectiveScrollBuffer(kListPositionTop));
    });
    RegisterDebugView<int>(
        registry, "bottom_effective_scroll_buffer", kPropertyInt, [](const PropAccessorArgs& args) {
            return static_cast<int>(ListOf(args)._GetEffectiveScrollBuffer(kListPositionBottom));
        });
    RegisterDebugView<unsigned int>(registry, "data_rows", kPropertyUInt, [](const PropAccessorArgs& args) {
        const UIListCom& list = ListOf(args);
        return list.mRuntime.mData != nullptr ? static_cast<unsigned int>(list.mRuntime.mData->mRows.size()) : 0U;
    });
    // The columns are the default row's entries (UIListData::GetNumColumns,
    // 0x8C42D0).
    RegisterDebugView<unsigned int>(registry, "data_columns", kPropertyUInt, [](const PropAccessorArgs& args) {
        const UIListCom& list = ListOf(args);
        return list.mRuntime.mData != nullptr
            ? static_cast<unsigned int>(list.mRuntime.mData->mDefaultRow.mEntries.size())
            : 0U;
    });
    RegisterDebugView<unsigned int>(registry, "instances_count", kPropertyUInt, [](const PropAccessorArgs& args) {
        return static_cast<unsigned int>(ListOf(args).mRuntime.mGridItems.mSize);
    });
    RegisterDebugView<unsigned int>(
        registry, "unused_instances_count", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).mRuntime.mUnusedItems.mSize);
        });
    RegisterDebugView<int>(registry, "first_item_offset", kPropertyInt, [](const PropAccessorArgs& args) {
        return ListOf(args).mRuntime.mFirstItemOffset;
    });
    RegisterDebugView<int>(registry, "top_data_index", kPropertyInt, [](const PropAccessorArgs& args) {
        return ListOf(args).mRuntime.mTopDataIndex;
    });
    RegisterDebugView<unsigned int>(
        registry, "last_focused_data_index", kPropertyUInt, [](const PropAccessorArgs& args) {
            return static_cast<unsigned int>(ListOf(args).mRuntime.mLastFocusedDataIndex);
        });
    RegisterDebugView<bool>(registry, "needs_update_instances", kPropertyBool, [](const PropAccessorArgs& args) {
        return ListOf(args).mRuntime.mNeedsUpdateInstances;
    });

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            UIListCom* const list = static_cast<UIListCom*>(args.mComponent);
            list->mMaxItemsAllocated = list->mMaxItemsDisplayed;
        };
    if (&registry == &own) {
        registry.SetCurrentRev(1);
        registry.mRevisionFunc = onRevision;
    }
}

// Reconstructed from eboot.elf at 0x8B9A60. The focus file loads through
// Resource::GetOrLoad<RndEntityResource> (0x8B9C50); its entity stays
// inactive.
bool UIListCom::_OnResourcesLoaded() {
    mRuntime.mNeedsRebuild = true;
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mReceiver = mObject->GetBaseCom<UIInputReceiverCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    if (mRuntime.mDrawNode == nullptr) {
        mRuntime.mDrawNode =
            static_cast<RndDrawNodeCom*>(mObject->CreateComponent(RndDrawNodeCom::sClassName, false));
    }
    if (mStaticFocus.mFocusFilePath.mPath != Symbol()) {
        mRuntime.mFocusResource = Resource::GetOrLoad<RndEntityResource>(mStaticFocus.mFocusFilePath, false);
        RndEntityResource* const resource = mRuntime.mFocusResource.Get();
        if (resource != nullptr && !resource->Fail()) {
            resource->mEntity->SetActive(false);
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x8B9DA0.
void UIListCom::_Enter() {
    _RebuildGrid(1);
    mRuntime.mLastUISeconds = TheTimeMgr->mClock.UISeconds();
    mRuntime.mInitFocusPending = true;
}

// Reconstructed from eboot.elf at 0x8BA7C0.
void UIListCom::_EditEnter() {
    _RebuildGrid(2);
}

// Reconstructed from eboot.elf at 0x8BA7D0. A change of the draw node's
// world show/hide flags is given to each item's draw node.
void UIListCom::_EditPoll() {
    _RebuildGrid(2);
    _PollTransforms();
    const unsigned int flags = mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags;
    if (flags != mRuntime.mShowHideFlags) {
        for (LinkedListSizeTracked::Node* node = mRuntime.mGridItems.mNext;
             node != mRuntime.mGridItems.Sentinel();
             node = node->mNext) {
            Entity* const entity = GridItemList::Owner(node)->mEntity;
            if (entity == nullptr) {
                continue;
            }
            RndDrawNodeCom* const drawNode = entity->GetRoot()->GetCom<RndDrawNodeCom>();
            if (drawNode != nullptr) {
                drawNode->mShowHideFlags = flags;
            }
        }
        mRuntime.mShowHideFlags = flags;
    }
    RndEntityResource* const instances = mRuntime.mInstances.Get();
    instances->PollEntity(instances->mEntity);
}

// Reconstructed from eboot.elf at 0x8BB9E0.
void UIListCom::_PostPoll() {
    if (mRuntime.mInitFocusPending) {
        _InitFocus();
        mRuntime.mInitFocusPending = false;
    }
}

// Reconstructed from eboot.elf at 0x8BBA10.
void UIListCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(RndDrawNodeCom::sClassName);
}

// Reconstructed from eboot.elf at 0x8BFAF0.
Symbol UIListCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8BFB00.
Symbol UIListCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8BFB10.
int UIListCom::CurrentRev() const {
    return const_cast<UIListCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8BFB30.
bool UIListCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8BFB60.
Component* UIListCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8BFB70. The copy goes at the buffer's
// next 8-byte boundary.
char* UIListCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIListCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIListCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8BFC80.
PropRegistry& UIListCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8BFC90.
ComMetaData& UIListCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x8BFCA0.
bool UIListCom::_HasPostPoll() const {
    return true;
}

// Reconstructed from eboot.elf at 0x8C0DF0. The arrays copy their elements
// through PropArrayBase::_Copy, which the imprint skips.
UIListCom::UIListCom(const UIListCom& other)
    : Component(other),
      mRuntime(other.mRuntime),
      mNumColumns(other.mNumColumns),
      mRows(other.mRows),
      mMaxItemsDisplayed(other.mMaxItemsDisplayed),
      mMaxItemsAllocated(other.mMaxItemsAllocated),
      mFilePath(other.mFilePath),
      mPropertyBindings(other.mPropertyBindings),
      mGridLayout(other.mGridLayout),
      mNavigationDirection(other.mNavigationDirection),
      mScrollTime(other.mScrollTime),
      mAllowEdgeScroll(other.mAllowEdgeScroll),
      mRequiresPropRefFixup(other.mRequiresPropRefFixup),
      mTopScrollBuffer(other.mTopScrollBuffer),
      mBottomScrollBuffer(other.mBottomScrollBuffer),
      mSpacing(other.mSpacing),
      mHorizontalSpacing(other.mHorizontalSpacing),
      mVerticalSpacing(other.mVerticalSpacing),
      mInstancePadding(other.mInstancePadding),
      mScrollbar(other.mScrollbar),
      mWrapNavigation(other.mWrapNavigation),
      mRadius(other.mRadius),
      mStaticFocus(other.mStaticFocus) {}

// Reconstructed from eboot.elf at 0x8950C0. The binary emits the factory
// in ui/UI.o.
Component* UIListCom::_Create() {
    return new UIListCom();
}

// Reconstructed from eboot.elf at 0x892740. The binary emits the
// registration in ui/UI.o.
void UIListCom::Init() {
    InitComponentClass<UIListCom>();
}
