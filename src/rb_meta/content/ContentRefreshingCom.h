#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The component that brings up a blocking dialog while the song content is
// refreshing ("Brings up a blocking dialog whenever song content is
// refreshing."; rb_meta/ContentRefreshingCom.o, 0xDB1A30-0xDB23A7 with the
// static initializer at 0xDB22F0). Entering and every poll read whether the
// content manager is refreshing, make the root's RBUIInputCom modal while it
// is, and keep the progress text. The vtable at 0x19814C8 has 41 slots. The
// object is 40 bytes.
class ContentRefreshingCom : public Component {
public:
    // "state". Names are the allowed values' texts; the enum's name is not
    // in the reference map.
    enum State {
        kStateInactive = 0,
        kStateRefreshing = 1,
    };

    ContentRefreshingCom();  // 0xDB1A30
    // Copies nothing; inlined into _Imprint (0xDB2170). Not in the
    // reference map.
    ContentRefreshingCom(const ContentRefreshingCom& other)
        : Component(other), mState(kStateInactive), mCompletedStr() {}
    // Slots 0-1: 0xDB1A70, 0xDB1A80.
    ~ContentRefreshingCom() override;

    Symbol GetId() const override;          // slot 4: 0xDB20F0
    Symbol GetClassName() const override;   // slot 5: 0xDB2100
    int CurrentRev() const override;        // slot 7: 0xDB2110
    bool IsA(Symbol type) const override;   // slot 8: 0xDB2130
    Component* AsComponent() override;      // slot 9: 0xDB2160
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xDB2170
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xDB22A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0xDB22B0
    // Slot 31. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xDB1F10
    // Slot 33. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xDB20E0

    // Reads the refresh, and while it runs the localized progress
    // "loading_additional_progress" with the mounted and the discovered
    // content counts (or the bare token before any is discovered), then
    // makes the root's RBUIInputCom modal while refreshing. The map's
    // _UpdateState(ObjPtr const&).
    void _UpdateState();  // 0xDB1F20

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB63C50.
    static void Init();
    // The class factory, emitted at 0xBA0B30; the placement factory beside
    // it (0xBA0B60) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xDB1AA0

    // "ContentRefreshingCom".
    static Symbol sId;  // 0x1B25C10
    // Also "ContentRefreshingCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B25C18
    static PropRegistry sPropRegistry;  // 0x1B25C20
    static ComMetaData sMetaData;       // 0x1B25CC0

    // Field names are not in the reference map.
    // "state", read and set through accessors.
    State mState;
    // "completed_str": the progress text while refreshing; read-only.
    Symbol mCompletedStr;
};

static_assert(offsetof(ContentRefreshingCom, mState) == 24);
static_assert(offsetof(ContentRefreshingCom, mCompletedStr) == 32);
static_assert(sizeof(ContentRefreshingCom) == 40);
