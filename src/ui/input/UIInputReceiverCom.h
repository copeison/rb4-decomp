#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;
class UIButtonDownMsg;
class UIButtonUpMsg;
class UIKeyboardKeyMsg;
class User;

// The state of a UI input receiver. The map names the type; the enumerator
// names follow the properties' allowed values.
enum UIState : int {
    kUIStateInvalid = -1,
    kUIStateNormal = 0,
    kUIStateFocused = 1,
    kUIStateDisabled = 2,
    kUIStateSelected = 3,
    kUIStateSelecting = 4,
};

// The symbol of a state: "normal", "focused", "disabled", "selected" or
// "selecting", and "invalid" for any other. The symbols are a guarded
// static array (0x1AC8CB0). The map places it in ui/UIInputReceiverCom.o;
// this build emits it after UIInputCom's static initializer. Not
// reconstructed.
Symbol UIStateToSym(UIState state);  // 0x8A48F0

// A selectable object the UI input component focuses and sends input to
// ("Com that receives all input from the UI Manager";
// ui/UIInputReceiverCom.o, 0x8A4A60-0x8A93FF with the static initializer at
// 0x8A9340). A queued state applies on the next poll, which exports
// "ui_state_change"; a selection shows the selected state for
// "num_selecting_frames" frames, and with "select_to_focus_params" returns
// to the focused state after some frames. The vtable at 0x195CDA8 has 44
// slots. The object is 88 bytes.
class UIInputReceiverCom : public Component {
public:
    // The run-time state, reset for an imprint. Name and field names not in
    // the reference map.
    struct RuntimeData {
        RuntimeData()
            : mState(kUIStateInvalid),
              mPreviousState(kUIStateInvalid),
              mQueuedState(kUIStateInvalid),
              mQueuedUserInitiated(false),
              mSelectUser(nullptr),
              mSelectPadNum(-1),
              mSelectingFrames(0),
              mSelectedFrames(0),
              mSelectUserInitiated(false),
              mSelectExport(true) {}

        // The current, previous and queued states ("selection_state" and
        // "previous_state" read the first two; setting the first queues).
        UIState mState;
        UIState mPreviousState;
        UIState mQueuedState;
        // Whether the user's input queued the state ("user_initiated").
        bool mQueuedUserInitiated;
        // A selection waiting for the selecting frames to end: the user,
        // the pad and the flags _SelectImpl takes; the second flag is
        // passed on to _ExportMsg and its meaning is weak.
        User* mSelectUser;
        int mSelectPadNum;
        // The frames left in the selecting state, and in the selected
        // state before the focus returns.
        int mSelectingFrames;
        int mSelectedFrames;
        bool mSelectUserInitiated;
        bool mSelectExport;
    };

    // "select_to_focus_params". The map names neither the type nor the
    // fields.
    struct SelectToFocusParams {
        // "enable": return from selected to focused.
        bool mEnable;
        // "frames": the frames to stay selected first.
        int mFrames;
    };

    UIInputReceiverCom();  // 0x8A4A60
    // The imprint's copy keeps the properties and starts a fresh state.
    // Inlined into _Imprint. Not in the reference map.
    UIInputReceiverCom(const UIInputReceiverCom& other)
        : Component(other),
          mRuntime(),
          mSelectToFocusParams(other.mSelectToFocusParams),
          mNumSelectingFrames(other.mNumSelectingFrames),
          mCanSelect(other.mCanSelect),
          mCanNavigateTo(other.mCanNavigateTo),
          mLockedToPadNum(other.mLockedToPadNum),
          mAlternateNavigationTrans(other.mAlternateNavigationTrans) {}
    ~UIInputReceiverCom() override;  // slots 0-1: 0x8A4AF0, 0x8A4B00

    // Slot 2: "set_state" queues a state by its symbol; "ui_button_down",
    // "ui_button_up" and "ui_key" go to the OnMsg slots. Not
    // reconstructed: the UI messages (ui/UIMsg.o) are not modelled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x8A8060
    Symbol GetId() const override;         // slot 4: 0x8A8440
    Symbol GetClassName() const override;  // slot 5: 0x8A8450
    int CurrentRev() const override;       // slot 7: 0x8A8460
    bool IsA(Symbol type) const override;  // slot 8: 0x8A8480
    Component* AsComponent() override;     // slot 9: 0x8A84B0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8A84C0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8A8630
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8A8640
    // Slot 31: queues the normal state for a receiver nothing set.
    void _Enter() override;  // 0x8A55B0
    // Slot 33: applies the queued state, exporting "ui_state_change", and
    // counts the selected frames down. Not reconstructed: the exported
    // message is not modelled.
    void _Poll() override;  // 0x8A55D0
    // Slot 39: true.
    bool _HasPostPoll() const override;  // 0x8A8650
    // Slot 40: selects once the selecting frames end. Not reconstructed.
    void _PostPoll() override;  // 0x8A57C0
    // Slots 41-43: export the message to the entity and its parents. Not
    // reconstructed.
    virtual DataNode OnMsg(const UIButtonDownMsg& msg);   // 0x8A7F70
    virtual DataNode OnMsg(const UIButtonUpMsg& msg);     // 0x8A8010
    virtual DataNode OnMsg(const UIKeyboardKeyMsg& msg);  // 0x8A7FC0

    // Map names; not reconstructed.
    void _SelectImpl(User* user, int padNum, bool userInitiated, bool exportMsg);  // 0x8A4BE0
    bool _ExportMsg(DataArray* msg, bool exportMsg);  // 0x8A4E70
    // Queues the receiver's state for its next poll. Name not in the
    // reference map. Not reconstructed.
    void QueueState(UIState state, bool immediate);  // 0x8A4B20

    // Registers the class (InitComponentClass, as a superclass root).
    // Inline in the map; emitted in ui/UI.o.
    static void Init();  // 0x891F80
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895050) is InitComponentClass's.
    static Component* _Create();  // 0x895020
    // Describes the class, exports "ui_state_change", "ui_button_down" and
    // "ui_button_down_feedback", and registers its properties. Revision 1
    // clears "num_selecting_frames" saved before it.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8A5830

    static Symbol sId;                  // 0x1AC8A50, "UIInputReceiverCom"
    // Also "UIInputReceiverCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC8A58
    static PropRegistry sPropRegistry;  // 0x1AC8A60
    static ComMetaData sMetaData;       // 0x1AC8B00

    // Field names are not in the reference map.
    RuntimeData mRuntime;
    SelectToFocusParams mSelectToFocusParams;  // "select_to_focus_params"
    int mNumSelectingFrames;                   // "num_selecting_frames"
    bool mCanSelect;                           // "can_select"
    bool mCanNavigateTo;                       // "can_navigate_to"
    // "locked_to_pad_num": the only pad the receiver takes, or -1.
    int mLockedToPadNum;
    // "alternate_navigation_trans": the transform navigation uses instead
    // of the object's.
    GameObjectId mAlternateNavigationTrans;
};

static_assert(offsetof(UIInputReceiverCom::RuntimeData, mSelectUser) == 16);
static_assert(offsetof(UIInputReceiverCom::RuntimeData, mSelectExport) == 37);
static_assert(sizeof(UIInputReceiverCom::SelectToFocusParams) == 8);
static_assert(offsetof(UIInputReceiverCom, mRuntime) == 24);
static_assert(offsetof(UIInputReceiverCom, mSelectToFocusParams) == 64);
static_assert(offsetof(UIInputReceiverCom, mNumSelectingFrames) == 72);
static_assert(offsetof(UIInputReceiverCom, mCanSelect) == 76);
static_assert(offsetof(UIInputReceiverCom, mLockedToPadNum) == 80);
static_assert(offsetof(UIInputReceiverCom, mAlternateNavigationTrans) == 84);
static_assert(sizeof(UIInputReceiverCom) == 88);
