#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Joypad.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class Entity;
class UIButtonDownMsg;
class UIKeyboardKeyMsg;

// "helpbar_changed_msg", which the helpbar exports through its entity when
// the prompts change. The complete destructor is Message<0>'s (0x8DBA0);
// the vtable is at 0x198D290. Name not in the reference map.
class HelpbarChangedMsg : public Message<0> {
public:
    HelpbarChangedMsg() : Message<0>(Event()) {}
    ~HelpbarChangedMsg() override {}

    // The message type, built on first use (0x1B407E8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("helpbar_changed_msg");
        }
        return t;
    }
};

// The component of the helpbar widget ("Com that handles everything in
// helpbar widget"; rb_meta/RBHelpbarCom.o, 0xED3250-0xED7EAF with the
// static initializer at 0xED7DA0). It reads the prompts of each layout and
// of each named override from ui/shared/scripts/helpbar_config.dta, shows
// the current layout's (or theRBMetaState's override's) prompts with the
// button overrides applied through "prompts" and "prompt_count", and turns
// cancel and confirm into "helpbar_back" and "helpbar_accept" for the
// scene, going to "previous_layout" or "next_layout". The vtable at
// 0x198D590 has 41 slots. The object is 480 bytes.
class RBHelpbarCom : public Component {
public:
    // One prompt: a joypad action and the tokens shown for it. The map
    // names the type; the field names are not in the reference map.
    struct PromptInfo {
        PromptInfo() : mAction(static_cast<JoypadAction>(0)), mToken(), mSecondaryToken() {}

        // "action".
        JoypadAction mAction;
        // "token" and "secondary_token".
        Symbol mToken;
        Symbol mSecondaryToken;
    };

    // The tokens of an action's prompt, which _GetToken returns in
    // registers. Name and field names not in the reference map.
    struct PromptTokens {
        Symbol mToken;
        Symbol mSecondaryToken;
    };

    // The run-time state, which the imprint's copy starts afresh; it holds
    // the two read-only properties. The constructor and destructor are out
    // of line. Name and field names not in the reference map.
    struct RuntimeData {
        RuntimeData();   // 0xED3340
        ~RuntimeData();  // 0xED6CE0

        // The prompts of each layout and of each override configuration,
        // seven per entry.
        eastl::map<RBUILayout, eastl::vector<PromptInfo>> mLayoutPromptInfo;
        eastl::map<Symbol, eastl::vector<PromptInfo>> mOverridePromptInfo;
        // The prompts shown, with the button overrides applied.
        eastl::vector<PromptInfo> mCurrentPromptInfo;
        // "prompts" (read-only, seven) and "prompt_count" (unsaved): the
        // shown prompts and one past the last with a token.
        PropArray<PromptInfo> mPrompts;
        // The scene entity, from _Enter.
        Entity* mSceneEntity;
        // The layout the prompts were last updated for.
        RBUILayout mLastLayout;
        unsigned long mPromptCount;
        // Set by "helpbar_override_msg" and by "helpbar_button_override_msg"
        // for its action; the poll updates the prompts.
        bool mConfigChanged;
        bool mButtonOverrideChanged[kNumHelpbarButtonOverrides];
        // RBMetaState::GetNoButtonOverride: the override label that hides
        // a prompt.
        Symbol mNoButtonOverride;
        // The subscriptions _Enter makes: the scene entity's
        // "transition_done", theRBMetaState's two override messages, and
        // theBandUserMgr's "user_slot_cleared" and "user_slot_set".
        MsgSource::EventSinkElem mTransitionDoneSink;
        MsgSource::EventSinkElem mHelpbarOverrideSink;
        MsgSource::EventSinkElem mHelpbarButtonOverrideSink;
        MsgSource::EventSinkElem mUserSlotClearedSink;
        MsgSource::EventSinkElem mUserSlotSetSink;
    };

    RBHelpbarCom();  // 0xED3270
    // Copies the properties for an imprint and starts afresh. Inlined into
    // _Imprint (0xED6B20). Not in the reference map.
    RBHelpbarCom(const RBHelpbarCom& other)
        : Component(other),
          mPreviousLayout(other.mPreviousLayout),
          mNextLayout(other.mNextLayout),
          mBackSound(other.mBackSound),
          mAcceptSound(other.mAcceptSound),
          mRuntime() {}
    // Slots 0-1: 0xED32D0, 0xED3300.
    ~RBHelpbarCom() override;

    // Slot 2: the user slot and transition events, "ui_button_down",
    // "ui_key" and the two override messages; other messages are
    // unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xED65A0
    Symbol GetId() const override;          // slot 4: 0xED6AA0
    Symbol GetClassName() const override;   // slot 5: 0xED6AB0
    int CurrentRev() const override;        // slot 7: 0xED6AC0
    bool IsA(Symbol type) const override;   // slot 8: 0xED6AE0
    Component* AsComponent() override;      // slot 9: 0xED6B10
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xED6B20
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xED6C60
    ComMetaData& _GetMetaData() override;       // slot 23: 0xED6C70
    // Slot 29: reads the configuration file and shows the prompts. The
    // map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0xED3FF0
    // Slot 31: finds the scene entity, gives its RBUIInputCom the input and
    // subscribes to the events. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xED4E90
    // Slot 33: updates the prompts after a layout change or an override,
    // and exports "helpbar_config_changed" and, outside a UI transition,
    // "helpbar_changed_msg". The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xED51D0

    // Reads (helpbar_config (layout_configs (layout (action token
    // [secondary_token])...)...)) into mLayoutPromptInfo. The map's
    // _LoadLayoutPromptInfo(DataArrayPtr).
    void _LoadLayoutPromptInfo(const DataArrayPtr& config);  // 0xED44A0
    // The same for (override_configs (name ...)...) into
    // mOverridePromptInfo. The map's _LoadOverridePromptInfo(DataArrayPtr).
    void _LoadOverridePromptInfo(const DataArrayPtr& config);  // 0xED4800
    // Walks both prompt tables and does nothing else; the release build
    // keeps only the walks. Uncalled. Name not in the reference map.
    void _DumpPromptInfo();  // 0xED4B80
    // Shows the current prompts with the button overrides applied, sets
    // the prompt count and clears the changed flags. The map's
    // _UpdatePromptInfo().
    void _UpdatePromptInfo();  // 0xED4BE0
    // Whether a button override changed since the last update. Inlined
    // into _Poll; the binary keeps an uncalled copy. Name not in the
    // reference map.
    bool _AnyButtonOverrideChanged() const;  // 0xED5430
    // Sends "helpbar_back" (pad) to the scene's other components, plays
    // "back_sound" when one handled it or a layout is gone to, and goes
    // to "previous_layout". True when handled. The map's _GoBack(int).
    bool _GoBack(int padNum);  // 0xED54C0
    // The same with "helpbar_accept", "accept_sound" and "next_layout".
    // The map's _Accept(int).
    bool _Accept(int padNum);  // 0xED5700
    // Seven prompts from a configuration entry's arrays, unset after the
    // last. Name not in the reference map.
    eastl::vector<PromptInfo> _ParsePromptInfo(const DataArrayPtr& entry);  // 0xED5930
    // One prompt from (action token [secondary_token]). Inlined into
    // _ParsePromptInfo; the binary keeps an uncalled copy. Name not in the
    // reference map.
    PromptInfo _ParsePrompt(const DataArrayPtr& prompt);  // 0xED5AC0
    // The prompts of theRBMetaState's override configuration, else those
    // of the current layout, else none. Name not in the reference map.
    const eastl::vector<PromptInfo>& _GetCurrentPromptInfo();  // 0xED5B30
    // Applies theRBMetaState's button overrides to the prompts of their
    // actions; the "no button" override clears the tokens. True when one
    // applied. The map's _UpdateButtonOverride() takes no argument.
    bool _UpdateButtonOverride(eastl::vector<PromptInfo>& prompts);  // 0xED5C70
    // Clears the changed flags of the button overrides. Inlined; the
    // binary keeps an uncalled copy. Name not in the reference map.
    void _ClearButtonOverrideChanges();  // 0xED5DB0
    // Unsets "prompts" and copies the current prompts into it. Inlined
    // into _UpdatePromptInfo; the binary keeps an uncalled copy. Name not in
    // the reference map.
    void _UpdatePromptProps();  // 0xED5DD0
    // The tokens of the action's first prompt, or empty ones. Uncalled.
    // The map's _GetToken returns one symbol.
    PromptTokens _GetToken(const eastl::vector<PromptInfo>& prompts, JoypadAction action) const;  // 0xED5EA0
    // A user joined or left a slot: tells the entity the helpbar changed.
    // Returns unhandled. Name not in the reference map.
    DataNode _OnUserSlotChanged();  // 0xED5EE0
    // The scene's transition finished: the same. Name not in the reference
    // map.
    DataNode _OnTransitionDone();  // 0xED5FE0
    // Backspace (8) goes back and enter (10) accepts, unless the key's
    // fifth flag is set.
    DataNode OnMsg(const UIKeyboardKeyMsg& msg);  // 0xED60E0
    // Cancel goes back and confirm accepts, for the message's pad.
    DataNode OnMsg(const UIButtonDownMsg& msg);  // 0xED6320
    // Marks the action's override changed. Inlined into Handle; the binary
    // keeps an uncalled copy.
    DataNode OnMsg(const HelpbarButtonOverrideMsg& msg);  // 0xED6540
    // Marks the configuration changed. The map's
    // OnMsg(HelpbarPromptOverrideMsg const&); inlined into Handle, the
    // binary keeps an uncalled copy.
    DataNode OnMsg(const HelpbarOverrideMsg& msg);  // 0xED6580

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB81280.
    static void Init();
    // The class factory, emitted at 0xBAA450; the placement factory beside
    // it (0xBAA480) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xED35B0

    // "RBHelpbar".
    static Symbol sId;  // 0x1B40848
    // Also "RBHelpbar". Name not in the reference map.
    static Symbol sClassName;           // 0x1B40850
    static PropRegistry sPropRegistry;  // 0x1B40860
    static ComMetaData sMetaData;       // 0x1B40900

    // Field names are not in the reference map.
    // "previous_layout" and "next_layout": where back and accept go, or
    // kLayoutInvalid.
    RBUILayout mPreviousLayout;
    RBUILayout mNextLayout;
    // "back_sound" and "accept_sound".
    Symbol mBackSound;
    Symbol mAcceptSound;
    RuntimeData mRuntime;
};

static_assert(sizeof(RBHelpbarCom::PromptInfo) == 24);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mOverridePromptInfo) == 56);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mCurrentPromptInfo) == 112);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mPrompts) == 144);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mSceneEntity) == 184);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mLastLayout) == 192);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mPromptCount) == 200);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mConfigChanged) == 208);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mButtonOverrideChanged) == 209);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mNoButtonOverride) == 224);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mTransitionDoneSink) == 232);
static_assert(offsetof(RBHelpbarCom::RuntimeData, mUserSlotSetSink) == 392);
static_assert(sizeof(RBHelpbarCom::RuntimeData) == 432);
static_assert(offsetof(RBHelpbarCom, mPreviousLayout) == 24);
static_assert(offsetof(RBHelpbarCom, mNextLayout) == 28);
static_assert(offsetof(RBHelpbarCom, mBackSound) == 32);
static_assert(offsetof(RBHelpbarCom, mAcceptSound) == 40);
static_assert(offsetof(RBHelpbarCom, mRuntime) == 48);
static_assert(sizeof(RBHelpbarCom) == 480);
