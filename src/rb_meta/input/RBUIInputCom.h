#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Joypad.h"
#include "ui/input/UIInputCom.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"
#include "utl/time/Timer.h"

class BandUser;
class ButtonDownMsg;
class ButtonUpMsg;
class DataArray;

// The game's UI input component (rb_meta/RBUIInputCom.o, 0xCB1A00-0xCB3CDF
// with the static initializer at 0xCB3B90): UIInputCom with the band's
// controllers. Button presses are filtered before UIInputCom sees them:
// with drum navigation the pad shift button turns Option and Shell Option
// into Up and Down, without it a held shift button swallows the action,
// lefty players get their directions flipped, and a second strum within
// 50 ms is dropped. A scene also remembers its focus: a transition started
// by Confirm saves the focused receiver's object path (and its list row),
// any other transition forgets it, and the scene's next enter focuses it
// again. The vtable at 0x1977FD0 has 43 slots. The object is 536 bytes.
class RBUIInputCom : public UIInputCom {
public:
    RBUIInputCom();  // 0xCB1A00
    // Copies the properties for an imprint and starts afresh. Inlined into
    // _Imprint (0xCB33B0). Not in the reference map.
    RBUIInputCom(const RBUIInputCom& other);
    // Slots 0-1: 0xCB1F70, 0xCB32B0. Leaves the UI manager's
    // "transition_started" event.
    ~RBUIInputCom() override;

    // Slot 2: "on_transition_started" goes to _OnTransitionStarted, the
    // other messages to UIInputCom.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xCB3170
    Symbol GetId() const override;          // slot 4: 0xCB3330
    Symbol GetClassName() const override;   // slot 5: 0xCB3340
    int CurrentRev() const override;        // slot 7: 0xCB3350
    bool IsA(Symbol type) const override;   // slot 8: 0xCB3370
    Component* AsComponent() override;      // slot 9: 0xCB33A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xCB33B0
    // Slot 20: empty in this build. The map's _PreDestroy(ObjPtr const&,
    // DestroyType).
    void _PreDestroy(DestroyType type) override;  // 0xCB2340
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xCB3720
    ComMetaData& _GetMetaData() override;       // slot 23: 0xCB3730
    // Slot 31: UIInputCom's enter, then the scene's saved focus is
    // restored on the first poll, and the component subscribes to theUI's
    // "transition_started" as "on_transition_started". The map's
    // _Enter(ObjPtr const&).
    void _Enter() override;  // 0xCB2020
    // Slot 33. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xCB2150
    // Slots 41-42: filter the press or release (_FilterAction) and drop a
    // double strum before UIInputCom handles it.
    DataNode OnMsg(const ButtonDownMsg& msg) override;  // 0xCB2DE0
    DataNode OnMsg(const ButtonUpMsg& msg) override;    // 0xCB2FF0

    // Whether the scene's receivers strum lock when focused: "auto_strum_lock"
    // unless "disable_strum_lock". Called by RBUIInputReceiverCom. Name not
    // in the reference map.
    bool ShouldAutoStrumLock() const;  // 0xCB1FF0
    // Whether the scene uses strum lock at all. Called by
    // RBUIInputReceiverCom. Name not in the reference map.
    bool IsStrumLockEnabled() const;  // 0xCB2010

    // The action UIInputCom gets for the user's press: drum navigation's
    // shift remaps, the shift buttons swallowing the action, and the lefty
    // flip of the directions.
    JoypadAction _FilterAction(BandUser* user, JoypadAction action);  // 0xCB2350
    // Whether a strum button press comes within 50 ms of the pad's last
    // one. Inlined into OnMsg(ButtonDownMsg const&); the binary keeps an
    // uncalled copy.
    bool _IsDoubleStrum(BandUser* user, JoypadButton button);  // 0xCB2550
    // Saves the focus after a Confirm, forgets it otherwise. Inlined into
    // Handle; the binary keeps an uncalled copy.
    DataNode _OnTransitionStarted(const Symbol& from, const Symbol& to);  // 0xCB3130
    // Remembers the focused receiver's object path from the scene's
    // entity, and its list's focused row.
    void _SaveSceneFocusState();  // 0xCB25E0
    // Forgets the scene's focus.
    void _ClearSceneFocusState();  // 0xCB2CB0
    // Focuses the scene's saved receiver, and its list's row.
    void _RestoreSceneFocusState();  // 0xCB2180

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB92B20.
    static void Init();
    // The class factory, emitted at 0xBACAF0; the placement factory beside
    // it (0xBACB20) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xCB1AC0

    // "RBUIInputCom".
    static Symbol sId;  // 0x1B0F6B0
    // Also "RBUIInputCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B0F6B8
    static PropRegistry sPropRegistry;  // 0x1B0F6C0
    static ComMetaData sMetaData;       // 0x1B0F760

    // Field names are not in the reference map. The run-time fields start
    // afresh in an imprint.
    // Set on enter; the next poll restores the scene's focus.
    bool mRestoreFocusPending;
    // The action of the last button press, which decides what a transition
    // does with the focus; 15, past the last action, before any.
    JoypadAction mLastAction;
    // Cleared while the filtered press or release goes on to UIInputCom.
    bool mFilterButtonDown;
    bool mFilterButtonUp;
    // Measures the time between strums.
    Hmx::Timer mStrumTimer;
    // Each pad's last strum, in milliseconds of mStrumTimer.
    float mLastStrumMs[8];
    // Keeps drum navigation's shift remaps from applying. Nothing in this
    // build sets it; the name is weak.
    bool mNoDrumNavigationRemap;
    // The subscription to theUI's "transition_started", received as
    // "on_transition_started".
    MsgSource::EventSinkElem mTransitionStartedSink;
    // "ignore_sticky_focus": "Turn this on if you want your scene to deal
    // with focus itself, rather than having RBUIInputCom attempt to handle
    // it generically."
    bool mIgnoreStickyFocus;
    // "auto_strum_lock": the receivers strum lock when focused; hidden while
    // "disable_strum_lock" is set.
    bool mAutoStrumLock;
    // "disable_strum_lock": no strum lock in the scene (true by default).
    bool mDisableStrumLock;
};

static_assert(offsetof(RBUIInputCom, mRestoreFocusPending) == 408);
static_assert(offsetof(RBUIInputCom, mLastAction) == 412);
static_assert(offsetof(RBUIInputCom, mFilterButtonDown) == 416);
static_assert(offsetof(RBUIInputCom, mFilterButtonUp) == 417);
static_assert(offsetof(RBUIInputCom, mStrumTimer) == 424);
static_assert(offsetof(RBUIInputCom, mLastStrumMs) == 448);
static_assert(offsetof(RBUIInputCom, mNoDrumNavigationRemap) == 480);
static_assert(offsetof(RBUIInputCom, mTransitionStartedSink) == 488);
static_assert(offsetof(RBUIInputCom, mIgnoreStickyFocus) == 528);
static_assert(offsetof(RBUIInputCom, mAutoStrumLock) == 529);
static_assert(offsetof(RBUIInputCom, mDisableStrumLock) == 530);
static_assert(sizeof(RBUIInputCom) == 536);
