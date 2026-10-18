#pragma once

#include <cstddef>
#include <cstring>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "render/scene/RndEntityResource.h"
#include "ui/lists/UIListData.h"
#include "utl/containers/Vector.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class Entity;
class RndDrawNodeCom;
class TransCom;
class UIInputReceiverCom;
class UIListItemCom;

// A scrolling list or grid of instanced entities, one per row of its
// UIListData ("Lists of selectable entities."; ui/UIListCom.o,
// 0x8B2530-0x8C364F with the static initializer at 0x8C3590). Each
// instance's properties are bound to the row's columns, and an optional
// static focus instance shows the focused row. The vtable at 0x195D3F8 has
// 41 slots. The object is 784 bytes.
//
// The structure, the run-time state, the constructors, the registration
// and the small slots are reconstructed. The scrolling, focus and instance
// management the handler and the game-mode poll drive is declared only.
class UIListCom : public Component {
public:
    // The ends of the visible window. The map names the type
    // (UIListCom::ListPosition); the enumerator names follow
    // "top_effective_scroll_buffer" and "bottom_effective_scroll_buffer".
    enum ListPosition : int {
        kListPositionTop = 0,
        kListPositionBottom = 1,
    };

    // The scroll directions. The map names the type
    // (UIListCom::ListDirection); the enumerator names follow
    // "can_scroll_up" and "can_scroll_down".
    enum ListDirection : int {
        kListDirectionUp = 0,
        kListDirectionDown = 1,
    };

    // The values of "grid_layout", named after their labels. Name not in
    // the reference map.
    enum GridLayout : int {
        kGridLayoutFillColumnFirst = 0,
        kGridLayoutFillRowFirst = 1,
        kGridLayoutNone = 2,
        kGridLayoutSetColumns = 3,
    };

    // The values of "navigation_direction". Name not in the reference map.
    enum NavigationDirection : int {
        kNavigationHorizontal = 0,
        kNavigationVertical = 1,
    };

    // One binding of an instance's property to a data column
    // ("UIListCom::PropertyBinding", registered as a struct type by _Init).
    // Field names are not in the reference map.
    struct PropertyBinding {
        // Takes the reference and, while the binding has no column, names
        // the column after the property's last member. Inlined into the
        // "property" setters (0x8BFE80, 0x8C0080). The binary checks the
        // path with PropPath::IsValid (0x17EA40), which PropPath.h does not
        // declare: the path starts with a member and has no unused node.
        void SetProperty(const PropRef& property) {
            mProperty = property;
            if (std::strcmp(mDataName.Str(), "") != 0) {
                return;
            }
            const PropPath path = mProperty.mPath;
            if (path.mSize == 0 || path.mNodes[0].mType != PropPath::Node::kNodeSymbol) {
                return;
            }
            for (unsigned long node = 1; node < path.mSize; ++node) {
                if (path.mNodes[node].mType == PropPath::Node::kNodeNone) {
                    return;
                }
            }
            mDataName = Symbol(path[-1].NameStr());
        }

        Symbol mDataName;  // "data_name"
        PropRef mProperty;  // "property"
    };

    // One instance of the list: its entity, the entity's UIListItemCom and
    // the data row it shows (the map's UIListCom::GridItem, whose
    // constructor is GridItem(UIListCom*, EntityPtr const&)). The list news
    // the items and links them through mNode. Field names are not in the
    // reference map.
    struct GridItem {
        UIListCom* mList;
        // The root's UIListItemCom, or null.
        UIListItemCom* mItem;
        Entity* mEntity;
        // The data row, or -1.
        int mDataIndex;
        LinkedListSizeTracked::Node mNode;
    };

    // A list of grid items (the map's LinkedListSizeTracked::List<
    // UIListCom::GridItem, UIListCom::GridItem::ListNode>). Name not in the
    // reference map.
    using GridItemList = LinkedListSizeTracked::List<GridItem, &GridItem::mNode>;

    // The run-time state, which the component's copy starts afresh. The map
    // names the type and its constructors; both are inlined. The destructor
    // is emitted at 0x8B21E0. Field names are not in the reference map.
    struct RuntimeData {
        // The binary builds the identity from constants.
        RuntimeData()
            : mOnlyAppScrolling(false),
              mIgnoreInput(false),
              mGridItems(),
              mUnusedItems(),
              mFirstItemOffset(0),
              mTopDataIndex(0x7FFFFFFF),
              mInstances(),
              mTrans(nullptr),
              mDrawNode(nullptr),
              mReceiver(nullptr),
              mData(nullptr),
              mNeedsRebuild(false),
              mLastWorldXfm{{{1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 1.0F}}, {0.0F, 0.0F, 0.0F}},
              mPrototype(nullptr),
              mScrollState(0),
              mShowHideFlags(0),
              mCanSelectOverridden(false),
              mCanSelect(false),
              mRestoreCanSelect(false),
              mUnusedIndex(-1),
              mSelectionHandlers(),
              mEditInPlaceIndex(0),
              mInitFocusPending(false),
              mFocusInstances(),
              mFocusResource(),
              mFocusItems(),
              mLastFocusedDataIndex(-1),
              mFocusFromInput(false),
              mHidden(false),
              mNeedsUpdateInstances(false),
              mRowAddedSink(),
              mRowRemovedSink(),
              mEntryChangedSink(),
              mRowsClearedSink(),
              mKeySink(),
              mButtonDownSink() {}
        // The map's RuntimeData(RuntimeData const&): a fresh state.
        RuntimeData(const RuntimeData& other) : RuntimeData() {
            static_cast<void>(other);
        }
        // Leaves the sources of the linked subscriptions, the last first,
        // and unlinks the items without freeing them.
        ~RuntimeData();  // 0x8B21E0

        // Frees the items of the three lists. The map's Destroy().
        void Destroy();  // 0x8B2100

        // "only_app_scrolling": only code and state graphs scroll the
        // list.
        bool mOnlyAppScrolling;
        // While set, the joypad messages leave the list alone. The name is
        // weak.
        bool mIgnoreInput;
        // The items showing, top first.
        GridItemList mGridItems;
        // The allocated items not showing.
        GridItemList mUnusedItems;
        // "first_item_offset".
        int mFirstItemOffset;
        // "top_data_index": the row of the top item; 0x7FFFFFFF without
        // data.
        int mTopDataIndex;
        // The resource whose entity spawns the instances of "file_path".
        ResourcePtr<RndEntityResource> mInstances;
        // The object's TransCom, draw node and input receiver, found when
        // the resources load; the draw node is created when it is missing.
        TransCom* mTrans;
        RndDrawNodeCom* mDrawNode;
        UIInputReceiverCom* mReceiver;
        // The rows (SetData), which UIListDtaProviderCom installs.
        UIListData* mData;
        // Set when the resources load; the next _RebuildGrid recreates the
        // instances.
        bool mNeedsRebuild;
        // The world transform the instances were last placed for
        // (_PollTransforms).
        Transform mLastWorldXfm;
        // The spawner's prototype entity, whose properties the bindings
        // name. The name is weak.
        Entity* mPrototype;
        // Non-zero while the list scrolls ("is_scrolling"). The name is
        // weak.
        int mScrollState;
        // The UI clock when the list was entered or last polled.
        float mLastUISeconds;
        // The current scroll's duration, "scroll_time" when it starts.
        float mScrollTime;
        // The draw node's world show/hide flags, last given to the items.
        unsigned int mShowHideFlags;
        // "can_select" when overridden, else the input receiver's. Names
        // weak.
        bool mCanSelectOverridden;
        bool mCanSelect;
        // Gives the items back their "can_select" on the next update. The
        // name is weak.
        bool mRestoreCanSelect;
        // Set to -1 and not read by the object's code.
        int mUnusedIndex;
        // The components given the selection first
        // (RegisterForSelectionHandling).
        eastl::vector<Component*> mSelectionHandlers;
        // "edit_in_place_index".
        unsigned int mEditInPlaceIndex;
        // Set on enter; the next post-poll initializes the focus.
        bool mInitFocusPending;
        // The resource whose entity spawns the static focus instances.
        ResourcePtr<RndEntityResource> mFocusInstances;
        // The loaded "focus_file_path", whose entity the focus bindings
        // and clip planes name.
        ResourcePtr<RndEntityResource> mFocusResource;
        // The static focus items.
        GridItemList mFocusItems;
        // "last_focused_data_index".
        long mLastFocusedDataIndex;
        // Set when the joypad moved the focus; passed with the next focus
        // change. The name is weak.
        bool mFocusFromInput;
        // Deactivates the instances (0x8B97F0).
        bool mHidden;
        // "needs_update_instances": the instances are refreshed once the
        // scrolling stops.
        bool mNeedsUpdateInstances;
        // The data's "ui_list_data_row_added", "ui_list_data_row_removed",
        // "ui_list_data_entry_changed" and "ui_list_data_rows_cleared_msg"
        // subscriptions (SetData), and the entity's "ui_key" and
        // "ui_button_down" ones (_RebuildGrid).
        MsgSource::EventSinkElem mRowAddedSink;
        MsgSource::EventSinkElem mRowRemovedSink;
        MsgSource::EventSinkElem mEntryChangedSink;
        MsgSource::EventSinkElem mRowsClearedSink;
        MsgSource::EventSinkElem mKeySink;
        MsgSource::EventSinkElem mButtonDownSink;
    };

    // "static_focus": an instance kept in place for the focused row.
    // Field names are not in the reference map.
    struct StaticFocus {
        bool mEnable = false;                         // "enable"
        unsigned long mVisibleIndex = 0;              // "visible_index"
        Vector3 mSpacing = {0.0F, 0.0F, 10.0F};       // "spacing"
        ResourcePath mFocusFilePath;                  // "focus_file_path"
        PropArray<PropertyBinding> mFocusPropertyBindings;  // "focus_property_bindings"
        Vector3 mFocusInstanceSpacing = {0.0F, 0.0F, 10.0F};  // "focus_instance_spacing"
        // The binary reads the null id from the shared header's copy
        // (0x1AC9030).
        GameObjectId mClipPlaneUp = GameObjectId{0xFFFFFFFFU};    // "clip_plane_up"
        GameObjectId mClipPlaneDown = GameObjectId{0xFFFFFFFFU};  // "clip_plane_down"
    };

    UIListCom();  // 0x8B2530
    // The imprint's copy keeps the properties and starts a fresh state.
    UIListCom(const UIListCom& other);  // 0x8C0DF0
    // Slots 0-1: 0x8B28E0, 0x8B2B50. Frees the instances first.
    ~UIListCom() override;

    // Slot 2. Not reconstructed.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x8BF2D0
    Symbol GetId() const override;         // slot 4: 0x8BFAF0
    Symbol GetClassName() const override;  // slot 5: 0x8BFB00
    int CurrentRev() const override;       // slot 7: 0x8BFB10
    bool IsA(Symbol type) const override;  // slot 8: 0x8BFB30
    Component* AsComponent() override;     // slot 9: 0x8BFB60
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8BFB70
    // Slot 18: the list follows the draw node in the component order,
    // without the base class's TransCom dependency.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x8BBA10
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8BFC80
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8BFC90
    // Slot 29: finds the object's components, creating the draw node, and
    // loads the focus file. The map's _LoadResources.
    bool _OnResourcesLoaded() override;  // 0x8B9A60
    // Slot 31: rebuilds in the game mode and records the time.
    void _Enter() override;  // 0x8B9DA0
    // Slot 33: polls the transforms, the scrolling and the instances. Not
    // reconstructed: it queues the post-poll through
    // Entity::QueueForPostPoll (0xF5F40), which Entity.h does not declare.
    void _Poll() override;  // 0x8BAA80
    // Slot 36: rebuilds in the edit mode.
    void _EditEnter() override;  // 0x8BA7C0
    // Slot 38: rebuilds, follows the transform and the show/hide flags and
    // polls the instances.
    void _EditPoll() override;  // 0x8BA7D0
    // Slot 39: true.
    bool _HasPostPoll() const override;  // 0x8BFCA0
    // Slot 40: initializes the focus after an enter.
    void _PostPoll() override;  // 0x8BB9E0

    // Installs the rows, leaving the old data's events. Not reconstructed.
    void SetData(UIListData* data);  // 0x8B7450
    // Whether the list can scroll in the direction. Not reconstructed.
    bool CanScroll(ListDirection direction) const;  // 0x8B7A90
    // Hides or shows the list, deactivating its instances. Name not in
    // the reference map. Not reconstructed.
    void SetHidden(bool hidden);  // 0x8B97F0

    // Inlined into "is_scrolling".
    bool IsScrolling() const {
        return mRuntime.mScrollState != 0;
    }
    // Inlined into "get_max_items_displayed".
    int GetMaxItemsDisplayed() const {
        return mMaxItemsDisplayed;
    }
    // The items showing. Inlined into "num_showing".
    unsigned long GetNumShowing() const {
        return mRuntime.mGridItems.mSize;
    }
    // The rows, or the displayed maximum without data. Inlined into
    // "num_total_elements". The binary reads the rows through
    // UIListData::GetNumRows (0x8C3BE0), which UIListData.h does not
    // declare.
    long GetTotalNumElements() const {
        return mRuntime.mData != nullptr ? static_cast<long>(mRuntime.mData->mRows.size())
                                         : mMaxItemsDisplayed;
    }
    // The focused row, or -1 when it is not a row of the data. Inlined
    // into "data_focus_index_no_scroll" and "data_focus_index_scroll".
    int GetDataFocusIndex(bool scroll) const {
        int index = -1;
        _GetFocusItem(&index, scroll);
        if (index >= 0 && mRuntime.mData != nullptr &&
            index < static_cast<long>(mRuntime.mData->mRows.size())) {
            return index;
        }
        return -1;
    }
    // The focused item's place among the items showing, or -1. Inlined
    // into "visible_focus_index".
    int GetVisibleFocusIndex() const {
        int index = -1;
        _GetFocusItem(&index, true);
        const int visible = index - mRuntime.mTopDataIndex;
        return visible < 0 ? -1 : visible;
    }

    // Whether the window reaches the data's end. Inlined into
    // "is_at_top_list_edge" and "is_at_bottom_list_edge".
    bool _IsAtListEdge(ListPosition position) const {
        if (position == kListPositionTop) {
            return mRuntime.mTopDataIndex <= 0;
        }
        return static_cast<long>(GetNumShowing()) + mRuntime.mTopDataIndex >= GetTotalNumElements();
    }
    // The row at the window's end, or -1 above the data. Inlined into
    // "data_index_at_top_edge" and "data_index_at_bottom_edge".
    int _GetDataIndexAtListEdge(ListPosition position) const {
        if (position == kListPositionTop) {
            return mRuntime.mTopDataIndex >= 0 ? mRuntime.mTopDataIndex : -1;
        }
        return mRuntime.mTopDataIndex + static_cast<int>(GetNumShowing()) - 1;
    }
    // The row the top item shows: the top data index when it is a row,
    // else 0 (a top index past the rows is reported, with the message
    // compiled out). -1 without data. Inlined into _UpdateInstanceProperties
    // and "update_properties_start_index". Name not in the reference map.
    int _GetUpdatePropertiesStartIndex() const {
        if (mRuntime.mData == nullptr) {
            return -1;
        }
        const long rows = static_cast<long>(mRuntime.mData->mRows.size());
        if (rows != 0 && mRuntime.mTopDataIndex != 0x7FFFFFFF) {
            if (mRuntime.mTopDataIndex < rows) {
                return mRuntime.mTopDataIndex;
            }
            static_cast<void>(MakeErrorName());
        }
        return 0;
    }

    // Creates the instances when the resources changed, entering them in
    // the mode (1 the game mode, 2 the edit mode), and subscribes to the
    // entity's key and button events. The map's _RebuildGrid(ObjPtr
    // const&); this build passes the mode. Not reconstructed.
    void _RebuildGrid(int mode);  // 0x8B9DE0
    // Creates the static focus instances. Not reconstructed.
    void _InitFocusInstances();  // 0x8BA350
    // Focuses the first selectable row when the focused list has no focus
    // item. Not reconstructed. The name is weak.
    void _InitFocus();  // 0x8BA6F0
    // Moves the instances when the world transform changed. Not
    // reconstructed.
    void _PollTransforms();  // 0x8BA8A0
    // Adds or removes instances to match the rows, then refreshes them.
    // Not reconstructed.
    void _UpdateInstances();  // 0x8B7920
    // The focused item and its row in `index`, or null. Not reconstructed.
    GridItem* _GetFocusItem(int* index, bool scroll) const;  // 0x8B7C00
    // The scroll buffer at the end, narrowed near the data's ends. Not
    // reconstructed.
    unsigned int _GetEffectiveScrollBuffer(ListPosition position) const;  // 0x8B8BB0
    // Gives each item its row's properties. Not reconstructed.
    void _UpdateInstanceProperties();  // 0x8B8CF0
    // Places the items for the layout and the spacing. Not reconstructed.
    void _UpdateGridTransforms();  // 0x8B8FD0
    // Frees the showing and unused items. Inlined into the destructor and
    // _RebuildGrid.
    void _DeleteGrid() {
        while (mRuntime.mGridItems.mSize != 0) {
            delete mRuntime.mGridItems.PopFront();
        }
        while (mRuntime.mUnusedItems.mSize != 0) {
            delete mRuntime.mUnusedItems.PopFront();
        }
    }

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x892740
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x8950F0) is InitComponentClass's.
    static Component* _Create();  // 0x8950C0
    // Describes the class (requiring a draw node and a TransCom), registers
    // the PropertyBinding struct type, the properties and the debug views.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8B2B70

    static Symbol sId;                  // 0x1AC9038, "UIListCom"
    // Also "UIListCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC9040
    static PropRegistry sPropRegistry;  // 0x1AC9050
    static ComMetaData sMetaData;       // 0x1AC90F0

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    RuntimeData mRuntime;
    // "num_columns": the constructor leaves it unset.
    int mNumColumns;
    int mRows;                                      // "rows"
    int mMaxItemsDisplayed;                         // "max_items_displayed"
    int mMaxItemsAllocated;                         // "max_items_allocated"
    ResourcePath mFilePath;                         // "file_path"
    PropArray<PropertyBinding> mPropertyBindings;   // "property_bindings"
    int mGridLayout;                                // "grid_layout"
    int mNavigationDirection;                       // "navigation_direction"
    float mScrollTime;                              // "scroll_time"
    bool mAllowEdgeScroll;                          // "allow_edge_scroll"
    bool mRequiresPropRefFixup;                     // "requires_propref_fixup"
    unsigned int mTopScrollBuffer;                  // "top_scroll_buffer"
    unsigned int mBottomScrollBuffer;               // "bottom_scroll_buffer"
    Vector3 mSpacing;                               // "spacing"
    float mHorizontalSpacing;                       // "horizontal_spacing"
    float mVerticalSpacing;                         // "vertical_spacing"
    int mInstancePadding;                           // "instance_padding"
    GameObjectId mScrollbar;                        // "scrollbar"
    bool mWrapNavigation;                           // "wrap_navigation"
    float mRadius;                                  // "radius"
    StaticFocus mStaticFocus;                       // "static_focus"
};

static_assert(sizeof(UIListCom::PropertyBinding) == 224);
static_assert(offsetof(UIListCom::GridItem, mDataIndex) == 24);
static_assert(offsetof(UIListCom::GridItem, mNode) == 32);
static_assert(sizeof(UIListCom::GridItem) == 56);
static_assert(offsetof(UIListCom::RuntimeData, mGridItems) == 8);
static_assert(offsetof(UIListCom::RuntimeData, mUnusedItems) == 32);
static_assert(offsetof(UIListCom::RuntimeData, mFirstItemOffset) == 56);
static_assert(offsetof(UIListCom::RuntimeData, mInstances) == 64);
static_assert(offsetof(UIListCom::RuntimeData, mData) == 96);
static_assert(offsetof(UIListCom::RuntimeData, mNeedsRebuild) == 104);
static_assert(offsetof(UIListCom::RuntimeData, mLastWorldXfm) == 108);
static_assert(offsetof(UIListCom::RuntimeData, mPrototype) == 160);
static_assert(offsetof(UIListCom::RuntimeData, mScrollState) == 168);
static_assert(offsetof(UIListCom::RuntimeData, mShowHideFlags) == 180);
static_assert(offsetof(UIListCom::RuntimeData, mCanSelectOverridden) == 184);
static_assert(offsetof(UIListCom::RuntimeData, mUnusedIndex) == 188);
static_assert(offsetof(UIListCom::RuntimeData, mSelectionHandlers) == 192);
static_assert(offsetof(UIListCom::RuntimeData, mEditInPlaceIndex) == 224);
static_assert(offsetof(UIListCom::RuntimeData, mFocusInstances) == 232);
static_assert(offsetof(UIListCom::RuntimeData, mFocusItems) == 248);
static_assert(offsetof(UIListCom::RuntimeData, mLastFocusedDataIndex) == 272);
static_assert(offsetof(UIListCom::RuntimeData, mFocusFromInput) == 280);
static_assert(offsetof(UIListCom::RuntimeData, mRowAddedSink) == 288);
static_assert(offsetof(UIListCom::RuntimeData, mButtonDownSink) == 488);
static_assert(sizeof(UIListCom::RuntimeData) == 528);
static_assert(offsetof(UIListCom::StaticFocus, mSpacing) == 16);
static_assert(offsetof(UIListCom::StaticFocus, mFocusFilePath) == 32);
static_assert(offsetof(UIListCom::StaticFocus, mFocusPropertyBindings) == 40);
static_assert(offsetof(UIListCom::StaticFocus, mFocusInstanceSpacing) == 80);
static_assert(offsetof(UIListCom::StaticFocus, mClipPlaneUp) == 92);
static_assert(sizeof(UIListCom::StaticFocus) == 104);
static_assert(offsetof(UIListCom, mRuntime) == 24);
static_assert(offsetof(UIListCom, mNumColumns) == 552);
static_assert(offsetof(UIListCom, mFilePath) == 568);
static_assert(offsetof(UIListCom, mPropertyBindings) == 576);
static_assert(offsetof(UIListCom, mGridLayout) == 616);
static_assert(offsetof(UIListCom, mScrollTime) == 624);
static_assert(offsetof(UIListCom, mRequiresPropRefFixup) == 629);
static_assert(offsetof(UIListCom, mTopScrollBuffer) == 632);
static_assert(offsetof(UIListCom, mSpacing) == 640);
static_assert(offsetof(UIListCom, mInstancePadding) == 660);
static_assert(offsetof(UIListCom, mScrollbar) == 664);
static_assert(offsetof(UIListCom, mWrapNavigation) == 668);
static_assert(offsetof(UIListCom, mRadius) == 672);
static_assert(offsetof(UIListCom, mStaticFocus) == 680);
static_assert(sizeof(UIListCom) == 784);
