#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class DataArray;
class RBUIInputCom;
class UIButtonDownMsg;

// The component on a UI layout entity's root that names the layout ("Allows
// screens to set state about themselves."; rb_meta/RBUILayoutCom.o,
// 0xCB49F0-0xCB5B0F with the static initializer at 0xCB5A10). Entering
// reports the layout to theRBMetaState, gives the scene's RBUIInputCom the
// input first, and holds the layout's drawing back for some frames, longer
// while a character tracker still loads player characters. Cancel goes back
// to "go_back_to_layout", and a transition saves and drops the input. The
// vtable at 0x1978348 has 41 slots. The object is 96 bytes.
class RBUILayoutCom : public Component {
public:
    // The run-time state, which the imprint's copy starts afresh. Its
    // constructor is inlined. Name and field names not in the reference
    // map.
    struct RuntimeData {
        RuntimeData() : mSkippingDraw(false), mInputCom(nullptr) {
            mTransitionStartedSink.mSink = nullptr;
            mTransitionStartedSink.mSource = nullptr;
        }

        // Set while the layout's drawing is held back; _Poll clears it once
        // no player character is loading.
        bool mSkippingDraw;
        // The component _DoInputRegistration gave the input first.
        RBUIInputCom* mInputCom;
        // The subscription to theUI's "transition_started", received as
        // "on_transition_started".
        MsgSource::EventSinkElem mTransitionStartedSink;
    };

    RBUILayoutCom();  // 0xCB4E80
    // Copies the properties for an imprint and starts afresh. Inlined into
    // _Imprint (0xCB5780). Not in the reference map.
    RBUILayoutCom(const RBUILayoutCom& other)
        : Component(other),
          mRuntime(),
          mLayout(other.mLayout),
          mGoBackToLayout(other.mGoBackToLayout),
          mInitialFramesToSkipDraw(other.mInitialFramesToSkipDraw) {}
    // Slots 0-1: 0xCB4EF0, 0xCB4F50. Leaves the UI manager's
    // "transition_started" event.
    ~RBUILayoutCom() override;

    // Slot 2: "ui_button_down" goes to OnMsg and "on_transition_started" to
    // _OnTransitionStarted; other messages are unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xCB54A0
    Symbol GetId() const override;          // slot 4: 0xCB5700
    Symbol GetClassName() const override;   // slot 5: 0xCB5710
    int CurrentRev() const override;        // slot 7: 0xCB5720
    bool IsA(Symbol type) const override;   // slot 8: 0xCB5740
    Component* AsComponent() override;      // slot 9: 0xCB5770
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xCB5780
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xCB58E0
    ComMetaData& _GetMetaData() override;       // slot 23: 0xCB58F0
    // Slot 31: reports the layout, registers for the input and the
    // transitions, and holds the drawing back for at least ten frames. The
    // map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xCB4FC0
    // Slot 33: while the drawing is held back, waits for the root's
    // RBCharacterLayoutTrackerCom to load its player characters, then holds
    // the drawing back ten more frames and stops waiting. Without a tracker
    // the ten frames are set again on every poll, since setting them marks
    // the drawing as held back again. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xCB52E0

    // Gives the RBUIInputCom of the layout's last instanced scene the input
    // first. The map's _DoInputRegistration(bool); this build takes
    // no argument.
    void _DoInputRegistration();  // 0xCB5140
    // Takes the input back from the RBUIInputCom. Inlined into the
    // transition handlers; the binary keeps an uncalled copy. Name not in
    // the reference map.
    void _UndoInputRegistration();  // 0xCB53A0
    // Sets the layout resource's frames to skip drawing, clamped to 0-300,
    // and marks the component as holding the drawing back when they are not
    // zero. Inlined into _Enter and _Poll; the binary keeps an uncalled
    // copy.
    void _SetFramesToSkipDraw(int frames);  // 0xCB52A0
    // Cancel goes to "go_back_to_layout" when it names a layout. Inlined
    // into Handle; the binary keeps an uncalled copy.
    DataNode OnMsg(const UIButtonDownMsg& msg);  // 0xCB53C0
    // Drops the input and the transition subscription, saves everything
    // and marks the system options clean. Inlined into Handle; the binary
    // keeps an uncalled copy.
    DataNode _OnTransitionStarted(const Symbol& from, const Symbol& to);  // 0xCB5420

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB936F0.
    static void Init();
    // The class factory, emitted at 0xBACBE0; the placement factory beside
    // it (0xBACC10) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xCB49F0

    // "RBUILayoutCom".
    static Symbol sId;  // 0x1B0FC60
    // Also "RBUILayoutCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B0FC68
    static PropRegistry sPropRegistry;  // 0x1B0FC70
    static ComMetaData sMetaData;       // 0x1B0FD10

    // Field names are not in the reference map.
    RuntimeData mRuntime;
    // "layout": the layout this entity is.
    RBUILayout mLayout;
    // "go_back_to_layout": where cancel goes, or kLayoutInvalid.
    RBUILayout mGoBackToLayout;
    // "initial_frames_to_skip_draw", 0 to 300.
    int mInitialFramesToSkipDraw;
};

static_assert(offsetof(RBUILayoutCom::RuntimeData, mInputCom) == 8);
static_assert(offsetof(RBUILayoutCom::RuntimeData, mTransitionStartedSink) == 16);
static_assert(sizeof(RBUILayoutCom::RuntimeData) == 56);
static_assert(offsetof(RBUILayoutCom, mRuntime) == 24);
static_assert(offsetof(RBUILayoutCom, mLayout) == 80);
static_assert(offsetof(RBUILayoutCom, mGoBackToLayout) == 84);
static_assert(offsetof(RBUILayoutCom, mInitialFramesToSkipDraw) == 88);
static_assert(sizeof(RBUILayoutCom) == 96);
