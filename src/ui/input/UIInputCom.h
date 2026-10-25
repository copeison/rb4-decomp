#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

struct PropAccessorArgs;
class ButtonDownMsg;
class ButtonUpMsg;
class DataArray;
class Entity;
class UIInputReceiverCom;

// The component that takes the UI manager's input for a scene and moves
// the focus between its UIInputReceiverComs ("Com that handles all input
// from the UI Manager."; ui/UIInputCom.o, 0x89C8B0-0x8A481F with the static
// initializer at 0x8A4820). Its enter and exit transitions play through the
// scene's StateGraphDriverCom. The vtable at 0x195CBB0 has 43 slots. The
// object is 408 bytes.
class UIInputCom : public Component {
public:
    // A receiver and the entity it lives in. Name and field names not in
    // the reference map.
    struct ReceiverRef {
        UIInputReceiverCom* mReceiver;
        Entity* mEntity;
    };

    // The run-time state, which the component's copy starts afresh. Its
    // constructor is inlined; the destructor is emitted at 0x8A31E0. Name
    // and field names not in the reference map.
    struct RuntimeData {
        // The binary reads the null id from the shared header's copy
        // (0x1AC8748).
        RuntimeData()
            : mFocus(),
              mReceivers(),
              mRegisteredComs(),
              mPendingAnimation(),
              mFocusFirstPending(false),
              mPendingFocus(GameObjectId{0xFFFFFFFFU}),
              mFocusFrames(0),
              mInputBlocked(false),
              mKeySink(),
              mKeyReleaseSink(),
              mButtonDownSink(),
              mButtonUpSink(),
              mJoypadConnectSink() {}
        // Leaves the sources of the linked subscriptions, the last first.
        ~RuntimeData();  // 0x8A31E0

        // The focused receiver of each pad (pad 0 alone unless
        // "multi_focus").
        eastl::map<int, ReceiverRef> mFocus;
        // The receivers found on enter (HarvestInputReceivers).
        eastl::vector<ReceiverRef> mReceivers;
        // The components given the input first (RegisterCom).
        eastl::vector<Component*> mRegisteredComs;
        // The state graph to run on the next post-poll (_UpdateAnimation);
        // the enter animation on enter.
        ResourcePath mPendingAnimation;
        // Set on enter with "focus_first_on_enter".
        bool mFocusFirstPending;
        // A focus "current_focus" requested, tried for mFocusFrames
        // post-polls.
        GameObjectId mPendingFocus;
        int mFocusFrames;
        // While set, the input messages are left unhandled. The name is
        // weak.
        bool mInputBlocked;
        // The entity's "key", "key_release", "button_down", "button_up"
        // and "joypad_connect" subscriptions, made on enter.
        MsgSource::EventSinkElem mKeySink;
        MsgSource::EventSinkElem mKeyReleaseSink;
        MsgSource::EventSinkElem mButtonDownSink;
        MsgSource::EventSinkElem mButtonUpSink;
        MsgSource::EventSinkElem mJoypadConnectSink;
    };

    // The navigation directions "navigation_direction" combines. The map
    // names the type (UIInputCom::NavDirection); only the combinations the
    // property offers are known, so the enumerator names are inferred.
    enum NavDirection : int {
        kNavHorizontal = 48,
        kNavVertical = 192,
        kNavAll = 240,
        kNavGrid = 496,
    };

    UIInputCom();  // 0x89C8B0
    // The imprint's copy keeps the properties and starts a fresh state.
    // Inlined into _Imprint. Not in the reference map.
    UIInputCom(const UIInputCom& other)
        : Component(other),
          mRuntime(),
          mWrapNavigation(other.mWrapNavigation),
          mNavigationDirection(other.mNavigationDirection),
          mMultiFocus(other.mMultiFocus),
          mLockedToPadNum(other.mLockedToPadNum),
          mFocusFirstOnEnter(other.mFocusFirstOnEnter),
          mIsModal(other.mIsModal),
          mEnterAnimation(other.mEnterAnimation),
          mExitAnimation(other.mExitAnimation) {}
    ~UIInputCom() override;  // slots 0-1: 0x89CAD0, 0x89CB00

    // Slot 2: the input messages, "set_focus" and "register_com". Not
    // reconstructed: the input messages are not modelled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x8A28F0
    Symbol GetId() const override;         // slot 4: 0x8A2E60
    Symbol GetClassName() const override;  // slot 5: 0x8A2E70
    int CurrentRev() const override;       // slot 7: 0x8A2E80
    bool IsA(Symbol type) const override;  // slot 8: 0x8A2EA0
    Component* AsComponent() override;     // slot 9: 0x8A2ED0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8A2EE0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8A31B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8A31C0
    // Slot 31: subscribes to the entity's input events, harvests the
    // receivers and queues the enter animation and the first focus. Not
    // reconstructed.
    void _Enter() override;  // 0x89CB40
    // Slot 33: asks for a post-poll while a focus or an animation is
    // pending. Not reconstructed.
    void _Poll() override;  // 0x89E870
    // Slot 39: true.
    bool _HasPostPoll() const override;  // 0x8A31D0
    // Slot 40: applies the pending focus and animation. Not reconstructed.
    void _PostPoll() override;  // 0x89E8B0
    // Slots 41-42: the joypad's button messages. Not reconstructed.
    virtual DataNode OnMsg(const ButtonDownMsg& msg);  // 0x8A1020
    virtual DataNode OnMsg(const ButtonUpMsg& msg);    // 0x8A1E00

    // Gives the component the input before the receivers, once. Not
    // reconstructed.
    void RegisterCom(Component* com);  // 0x89EC30
    // Removes a component RegisterCom gave the input. Not reconstructed.
    void UnregisterCom(Component* com);  // 0x89ED30
    // Sets "is_modal". Not reconstructed.
    void SetIsModal(bool modal);  // 0x89EF10
    // Moves the pad's focus to the receiver; the flag goes on to the
    // receiver's focused state. Not reconstructed. The parameter name is
    // weak.
    void SetFocusedObject(UIInputReceiverCom* receiver, int padNum, bool immediate);  // 0x89D9F0
    // Takes the focus away from the pad's receiver. Name not in the
    // reference map. Not reconstructed.
    void ClearFocus(int padNum);  // 0x89D750
    // Sets "focus_first_on_enter" and the pending first focus. Not
    // reconstructed.
    void SetFocusFirstOnEnter(bool focusFirst);  // 0x89EF20
    // The focus of the pad: pad 0's entry with "multi_focus", otherwise
    // the only one; null when there is none. Inlined into the property
    // accessors.
    const ReceiverRef* FindFocus(int padNum) const {
        auto& focus = const_cast<eastl::map<int, ReceiverRef>&>(mRuntime.mFocus);
        if (mMultiFocus) {
            const auto found = focus.find(padNum);
            return found != focus.end() ? &found->second : nullptr;
        }
        return focus.size() == 0 ? nullptr : &focus.begin()->second;
    }

    // Starts the exit animation on the scene root's state graph driver.
    void Exit();  // 0x89ED90
    // Whether the driver still runs the enter or the exit animation.
    bool IsEntering() const;  // 0x89EE10
    bool IsExiting() const;   // 0x89EE90

    // Registers the class (InitComponentClass, as a superclass root).
    // Inline in the map; emitted in ui/UI.o.
    static void Init();  // 0x891BA0
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895000) is InitComponentClass's.
    static Component* _Create();  // 0x894FD0
    // Describes the class, exports "ui_select" and "transition_done", and
    // registers its properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x89F430
    // Registers the class's properties for a subclass, under the
    // "metadata" heap, and inherits its editor restrictions. Inline; the
    // subclass's object emits it (RBUIInputCom.o at 0xCB1EC0).
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
        metadata.InitSuperclass("UIInputCom", sMetaData);
        const ComMetaData::EditorRestriction restrictions =
            static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
        static const long heap = MemFindHeap("metadata");
        MemPushHeap(heap);
        _Init(registry, metadata);
        MemPopHeap();
        metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
            static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
            restrictions);
    }

    static Symbol sId;                  // 0x1AC8758, "UIInputCom"
    // The class symbol GameObject::CreateComponent takes; also
    // "UIInputCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC8760
    static PropRegistry sPropRegistry;  // 0x1AC8770
    static ComMetaData sMetaData;       // 0x1AC8810

    // Field names are not in the reference map.
    RuntimeData mRuntime;
    bool mWrapNavigation;               // "wrap_navigation"
    NavDirection mNavigationDirection;  // "navigation_direction"
    bool mMultiFocus;                   // "multi_focus"
    // "locked_to_pad_num": the only pad the scene takes, or -1.
    int mLockedToPadNum;
    bool mFocusFirstOnEnter;  // "focus_first_on_enter"
    bool mIsModal;            // "is_modal"
    // "enter_animation" and "exit_animation": the state graphs run when the
    // scene enters and exits.
    ResourcePath mEnterAnimation;
    ResourcePath mExitAnimation;
};

static_assert(sizeof(UIInputCom::ReceiverRef) == 16);
static_assert(offsetof(UIInputCom::RuntimeData, mReceivers) == 56);
static_assert(offsetof(UIInputCom::RuntimeData, mRegisteredComs) == 88);
static_assert(offsetof(UIInputCom::RuntimeData, mPendingAnimation) == 120);
static_assert(offsetof(UIInputCom::RuntimeData, mPendingFocus) == 132);
static_assert(offsetof(UIInputCom::RuntimeData, mInputBlocked) == 140);
static_assert(offsetof(UIInputCom::RuntimeData, mKeySink) == 144);
static_assert(sizeof(UIInputCom::RuntimeData) == 344);
static_assert(offsetof(UIInputCom, mWrapNavigation) == 368);
static_assert(offsetof(UIInputCom, mNavigationDirection) == 372);
static_assert(offsetof(UIInputCom, mMultiFocus) == 376);
static_assert(offsetof(UIInputCom, mLockedToPadNum) == 380);
static_assert(offsetof(UIInputCom, mFocusFirstOnEnter) == 384);
static_assert(offsetof(UIInputCom, mIsModal) == 385);
static_assert(offsetof(UIInputCom, mEnterAnimation) == 392);
static_assert(offsetof(UIInputCom, mExitAnimation) == 400);
static_assert(sizeof(UIInputCom) == 408);

// The accessor of "current_focus" (0x8A35E0): the id of the focused object,
// or of the object in this entity that instances it, or the null id. Not
// reconstructed: the object lookup (0x114170) is not modelled. Name not in
// the reference map.
GameObjectId GetUIInputCurrentFocus(const PropAccessorArgs& args);
