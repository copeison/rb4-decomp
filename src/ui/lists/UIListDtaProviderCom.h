#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/DataResource.h"
#include "entity/resources/Resource.h"
#include "ui/lists/UIListData.h"
#include "utl/text/Symbol.h"

class UIListCom;

// Feeds the object's UIListCom from a DTA file, for prototyping lists before
// a coded provider exists ("Useful for protoyping UI lists before a coder
// writes a real provider backed by data."; ui/UIListDtaProviderCom.o,
// 0x8C6610-0x8C750F with the static initializer at 0x8C7450). The vtable at
// 0x195E038 has 41 slots. The object is 176 bytes.
class UIListDtaProviderCom : public Component {
public:
    UIListDtaProviderCom();  // 0x8C6610
    // The imprint's copy keeps the file and starts with empty data, no
    // list and no resource. Inlined into _Imprint. Not in the reference
    // map.
    UIListDtaProviderCom(const UIListDtaProviderCom& other)
        : Component(other), mData(), mList(nullptr), mDta(), mDtaPath(other.mDtaPath) {}
    ~UIListDtaProviderCom() override;  // slots 0-1: 0x8C6680, 0x8C6740

    Symbol GetId() const override;         // slot 4: 0x8C7260
    Symbol GetClassName() const override;  // slot 5: 0x8C7270
    int CurrentRev() const override;       // slot 7: 0x8C7280
    bool IsA(Symbol type) const override;  // slot 8: 0x8C72A0
    Component* AsComponent() override;     // slot 9: 0x8C72D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8C72E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8C7430
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8C7440
    // Slot 29: loads the file and parses it. The map's _LoadResources.
    bool _OnResourcesLoaded() override;  // 0x8C6A60
    // Slot 31: nothing.
    void _Enter() override;  // 0x8C7090
    // Slot 33: installs the data on the list.
    void _Poll() override;  // 0x8C7160
    // Slot 38: reloads a changed file, then installs the data.
    void _EditPoll() override;  // 0x8C70A0

    // Finds the object's UIListCom and gives it the data unless it has
    // some (UIListCom::SetData). Inlined into _Poll and _EditPoll; the
    // emitted copy has no caller.
    void _InstallSelf();  // 0x8C71E0
    // Clears the data and refills it from the file: the first array's
    // symbols name the columns and each later array is a row, the first of
    // which also adds the columns with its entries as their defaults. A
    // file that failed to load is dropped.
    // Not reconstructed: it evaluates the file's commands.
    void _ParseDta();  // 0x8C6C50

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x892B20
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895140) is InitComponentClass's.
    static Component* _Create();  // 0x895110
    // Describes the class and registers "dta_path".
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8C6760

    static Symbol sId;                  // 0x1AC9350, "UIListDtaProvider"
    // Also "UIListDtaProvider". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC9358
    static PropRegistry sPropRegistry;  // 0x1AC9360
    static ComMetaData sMetaData;       // 0x1AC9400

    // Field names are not in the reference map.
    // The rows the file fills.
    UIListData mData;
    // The object's list, once found.
    UIListCom* mList;
    // The loaded file.
    ResourcePtr<DataResource> mDta;
    // "dta_path".
    ResourcePath mDtaPath;
};

static_assert(offsetof(UIListDtaProviderCom, mData) == 24);
static_assert(offsetof(UIListDtaProviderCom, mList) == 152);
static_assert(offsetof(UIListDtaProviderCom, mDta) == 160);
static_assert(offsetof(UIListDtaProviderCom, mDtaPath) == 168);
static_assert(sizeof(UIListDtaProviderCom) == 176);
