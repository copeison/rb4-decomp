#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The interface between a UIListCom and one of its items ("Com that
// handles interface between UIList and its list items"; ui/UIListItemCom.o,
// 0x8C7B60-0x8C843F with the static initializer at 0x8C8380). The list
// tells each item where it lies relative to the focus, whether the list
// scrolls and which data row it shows; the item's properties only read and
// write that state through accessors. The vtable at 0x195E378 has 41
// slots. The object is 40 bytes.
class UIListItemCom : public Component {
public:
    // Where the item lies relative to the list's focus. The map names the
    // type; the enumerator names follow the property's allowed values.
    enum RelativeToFocus : int {
        kRelativeToFocusAt = 0,
        kRelativeToFocusAbove = 1,
        kRelativeToFocusBelow = 2,
    };

    UIListItemCom();  // 0x8C7B60
    // The imprint's copy: the list state starts afresh. Inlined into
    // _Imprint. Not in the reference map.
    UIListItemCom(const UIListItemCom& other) : Component(other) {}
    ~UIListItemCom() override;  // slots 0-1: 0x8C7BA0, 0x8C7BB0

    Symbol GetId() const override;         // slot 4: 0x8C8170
    Symbol GetClassName() const override;  // slot 5: 0x8C8180
    int CurrentRev() const override;       // slot 7: 0x8C8190
    bool IsA(Symbol type) const override;  // slot 8: 0x8C81B0
    Component* AsComponent() override;     // slot 9: 0x8C81E0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8C81F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8C8320
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8C8330

    // The map's SetRelativeToFocus(UIListItemCom::RelativeToFocus const&)
    // and SetParentListIsScrolling(bool).
    void SetRelativeToFocus(const RelativeToFocus& relative);  // 0x8C7BD0
    void SetParentListIsScrolling(bool scrolling);              // 0x8C7BE0
    // Sets the data row the item shows; UIListCom passes it when it fills
    // the item (0x8BD020). Name not in the reference map.
    void SetDataIndex(long index);  // 0x8C7BF0

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x892F00
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895190) is InitComponentClass's.
    static Component* _Create();  // 0x895160
    // Describes the class and registers the accessor properties
    // "relative_to_focus" and "parent_list_scrolling".
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8C7C00

    static Symbol sId;                  // 0x1AC9810, "UIListItem"
    // Also "UIListItem". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC9818
    static PropRegistry sPropRegistry;  // 0x1AC9820
    static ComMetaData sMetaData;       // 0x1AC98C0

    // Field names are not in the reference map.
    RelativeToFocus mRelativeToFocus = kRelativeToFocusAbove;
    bool mParentListScrolling = false;
    // The data row the item shows, or -1.
    long mDataIndex = -1;
};

static_assert(offsetof(UIListItemCom, mRelativeToFocus) == 24);
static_assert(offsetof(UIListItemCom, mParentListScrolling) == 28);
static_assert(offsetof(UIListItemCom, mDataIndex) == 32);
static_assert(sizeof(UIListItemCom) == 40);
