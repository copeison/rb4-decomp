// rb_meta/RBHelpbarCom.o (0xED3250 to 0xED7EAF). The EASTL map and vector
// helpers of the prompt tables (0xED7100-0xED7C90) and PropArray<PromptInfo>'s
// members (vtable 0x198D6D8, 0xED6F20-0xED70EF) are the templates'. The
// static initializer is at 0xED7DA0; the shared header statics it sets first
// (0x1B40818-0x1B40820) belong to headers that are not modelled. The class's
// registration and factory are emitted elsewhere (0xB81280, 0xBAA450).
#include "rb_meta/ui/RBHelpbarCom.h"

#include <new>

#include "audio/core/components/AudioEmitterCom.h"
#include "audio/core/components/AudioListenerCom.h"
#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "render/scene/RndEntityResource.h"
#include "ui/input/UIMsg.h"
#include "ui/manager/UIMgr.h"
#include "utl/data/DataFile.h"
#include "utl/text/Str.h"

namespace {

// The configuration file. A pointer in the object's data (0x19BE598). Name
// not in the reference map.
const char* kHelpbarConfigPath = "ui/shared/scripts/helpbar_config.dta";

// The prompts each table entry holds and the layouts the tables start with.
// Names not in the reference map.
constexpr unsigned long kNumPrompts = 7;
constexpr int kNumLayouts = 108;

// The joypad actions and keys the helpbar answers (a JoypadAction and the
// keyboard's backspace and enter). Names not in the reference map.
constexpr int kActionConfirm = 1;
constexpr int kActionCancel = 2;
constexpr int kKeyBackspace = 8;
constexpr int kKeyEnter = 10;

// The pad of a key press. Name not in the reference map.
constexpr int kNoPad = -1;

// The prompts _GetCurrentPromptInfo returns without a configuration
// (0x1B40828, destroyed by 0xED3250). Name not in the reference map.
const eastl::vector<RBHelpbarCom::PromptInfo> sEmptyPromptInfo;

// Leaves the subscription's source if it is linked; the link unlinks itself
// afterwards. Inlined into RuntimeData's destructor. Name not in the
// reference map.
void LeaveSource(MsgSource::EventSinkElem& sink) {
    LinkedList::Node& link = sink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        sink.mSource->RemoveSink(&sink);
    }
}

// Plays the sound on the sound manager's 2D emitter. Inlined into _GoBack
// and _Accept. Name not in the reference map.
void PlayHelpbarSound(Symbol sound) {
    static_cast<AudioEmitterCom*>(theSoundManager.GetDefault2DEmitterComponent())->PlaySound(sound);
}

}  // namespace

// The object's statics, in the order of its static initializer (0xED7DA0).
Symbol RBHelpbarCom::sId("RBHelpbar");
Symbol RBHelpbarCom::sClassName("RBHelpbar");
PropRegistry RBHelpbarCom::sPropRegistry;
ComMetaData RBHelpbarCom::sMetaData;

// Reconstructed from eboot.elf at 0xED3270.
RBHelpbarCom::RBHelpbarCom()
    : mPreviousLayout(kLayoutInvalid),
      mNextLayout(kLayoutInvalid),
      mBackSound("shell/ui/os_back"),
      mAcceptSound("shell/ui/guitar_select"),
      mRuntime() {}

// Reconstructed from eboot.elf at 0xED32D0. The deleting destructor is at
// 0xED3300.
RBHelpbarCom::~RBHelpbarCom() {}

// Reconstructed from eboot.elf at 0xED3340.
RBHelpbarCom::RuntimeData::RuntimeData()
    : mLayoutPromptInfo(),
      mOverridePromptInfo(),
      mCurrentPromptInfo(),
      mPrompts(),
      mSceneEntity(nullptr),
      mLastLayout(kLayoutInvalid),
      mPromptCount(0),
      mConfigChanged(false),
      mButtonOverrideChanged(),
      mNoButtonOverride(RBMetaState::GetNoButtonOverride()) {
    MsgSource::EventSinkElem* const sinks[] = {
        &mTransitionDoneSink,
        &mHelpbarOverrideSink,
        &mHelpbarButtonOverrideSink,
        &mUserSlotClearedSink,
        &mUserSlotSetSink,
    };
    for (MsgSource::EventSinkElem* const sink : sinks) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
}

// Reconstructed from eboot.elf at 0xED6CE0. The subscriptions leave their
// sources, the last one first.
RBHelpbarCom::RuntimeData::~RuntimeData() {
    LeaveSource(mUserSlotSetSink);
    LeaveSource(mUserSlotClearedSink);
    LeaveSource(mHelpbarButtonOverrideSink);
    LeaveSource(mHelpbarOverrideSink);
    LeaveSource(mTransitionDoneSink);
}

// Reconstructed from eboot.elf at 0xED35B0.
void RBHelpbarCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that handles everything in helpbar widget";
    metadata.mAuthor = "Ralph Hebb";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());
    metadata.mRootOnly = true;
    RegisterPropStruct<PromptInfo>(Symbol("PromptInfo"));

    RBHelpbarCom prototype;
    if (&registry != &prototype._GetPropRegistry()) {
        registry.AddInheritedRegistry(sClassName, prototype._GetPropRegistry());
    }

    IntMetadata<int>& previousLayout = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp(
                 "previous_layout", offsetof(RBHelpbarCom, mPreviousLayout), kPropertyInt, sizeof(RBUILayout))
             .mMetadata.Get());
    previousLayout.mDefault = prototype.mPreviousLayout;
    static_cast<void>(Symbol("prop"));
    previousLayout.mHelp = String("Which layout should the back button goto?");
    previousLayout.mAllowedValues = RBMetaState::GetAllowedLayoutValues(true);

    IntMetadata<int>& nextLayout = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("next_layout", offsetof(RBHelpbarCom, mNextLayout), kPropertyInt, sizeof(RBUILayout))
             .mMetadata.Get());
    nextLayout.mDefault = prototype.mNextLayout;
    static_cast<void>(Symbol("prop"));
    nextLayout.mHelp = String("Which layout should the accept button goto?");
    nextLayout.mAllowedValues = RBMetaState::GetAllowedLayoutValues(true);

    SymbolMetadata& backSound = TypeSpecificMetadata(
        registry.RegisterProp("back_sound", offsetof(RBHelpbarCom, mBackSound), kPropertySymbol, sizeof(Symbol)),
        prototype.mBackSound);
    backSound.mDefault = prototype.mBackSound;
    static_cast<void>(Symbol("prop"));
    backSound.mHelp = String("Sound to play when going back.");

    SymbolMetadata& acceptSound = TypeSpecificMetadata(
        registry.RegisterProp("accept_sound", offsetof(RBHelpbarCom, mAcceptSound), kPropertySymbol, sizeof(Symbol)),
        prototype.mAcceptSound);
    acceptSound.mDefault = prototype.mAcceptSound;
    static_cast<void>(Symbol("prop"));
    acceptSound.mHelp = String("Sound to play when accepting");

    IntMetadata<unsigned long>& promptCount = TypeSpecificMetadata(
        registry.RegisterProp(
            "prompt_count",
            offsetof(RBHelpbarCom, mRuntime) + offsetof(RuntimeData, mPromptCount),
            kPropertyUInt64,
            sizeof(unsigned long)),
        prototype.mRuntime.mPromptCount);
    promptCount.mDefault = prototype.mRuntime.mPromptCount;
    static_cast<void>(Symbol("prop"));
    promptCount.mSaves = false;
    promptCount.mHistory = false;

    PropInfo& prompts = registry.RegisterPropArray(
        "prompts", offsetof(RBHelpbarCom, mRuntime) + offsetof(RuntimeData, mPrompts), sizeof(PropArray<PromptInfo>));
    ArrayMetadata& promptsMetadata = static_cast<ArrayMetadata&>(*prompts.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    promptsMetadata.mReadOnly = true;
    promptsMetadata.mHistory = false;
    PropRegistry::RegisterPropArrayItem(prompts, kPropertyStruct, sizeof(PromptInfo));
    GroupMetadata& prompt = static_cast<GroupMetadata&>(*promptsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const PromptInfo promptPrototype;
    PropRegistry& promptMembers = prompt.mRegistry;

    IntMetadata<int>& action = static_cast<IntMetadata<int>&>(
        *promptMembers.RegisterProp("action", offsetof(PromptInfo, mAction), kPropertyInt, sizeof(JoypadAction))
             .mMetadata.Get());
    action.mDefault = promptPrototype.mAction;
    static_cast<void>(Symbol("prop"));
    action.mAllowedValues = GetJoypadActionsAllowedValues();

    SymbolMetadata& token = TypeSpecificMetadata(
        promptMembers.RegisterProp("token", offsetof(PromptInfo, mToken), kPropertySymbol, sizeof(Symbol)),
        promptPrototype.mToken);
    token.mDefault = promptPrototype.mToken;

    SymbolMetadata& secondaryToken = TypeSpecificMetadata(
        promptMembers.RegisterProp(
            "secondary_token", offsetof(PromptInfo, mSecondaryToken), kPropertySymbol, sizeof(Symbol)),
        promptPrototype.mSecondaryToken);
    secondaryToken.mDefault = promptPrototype.mSecondaryToken;

    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);
    metadata.AddExportedEvent(Symbol("helpbar_config_changed"), String("The helpbar config should be changed!"));
    SetNonStaticTagFunctionsAllowed(nonStatic);
}

// Reconstructed from eboot.elf at 0xED3FF0. Every layout starts with seven
// unset prompts.
bool RBHelpbarCom::_OnResourcesLoaded() {
    for (int layout = 0; layout < kNumLayouts; ++layout) {
        const eastl::vector<PromptInfo> prompts;
        mRuntime.mLayoutPromptInfo[static_cast<RBUILayout>(layout)] = prompts;
        mRuntime.mLayoutPromptInfo[static_cast<RBUILayout>(layout)].resize(kNumPrompts);
    }
    mRuntime.mPrompts.Resize(kNumPrompts);
    mRuntime.mCurrentPromptInfo.reserve(kNumPrompts);
    _ClearButtonOverrideChanges();
    const DataArrayPtr config = DataReadFile(kHelpbarConfigPath, true);
    if (config != nullptr) {
        _LoadLayoutPromptInfo(config);
        _LoadOverridePromptInfo(config);
        _DumpPromptInfo();
    }
    _UpdatePromptInfo();
    return true;
}

// Reconstructed from eboot.elf at 0xED44A0.
void RBHelpbarCom::_LoadLayoutPromptInfo(const DataArrayPtr& config) {
    static Symbol layoutConfigs;
    if (layoutConfigs == Symbol()) {
        layoutConfigs = Symbol("layout_configs");
    }
    static Symbol helpbarConfig;
    if (helpbarConfig == Symbol()) {
        helpbarConfig = Symbol("helpbar_config");
    }
    const DataArray* const configs = config->FindArray(helpbarConfig, layoutConfigs);
    for (int index = 1; index < configs->Size(); ++index) {
        const DataArrayPtr entry = configs->Node(index).Array(configs);
        const RBUILayout layout = static_cast<RBUILayout>(entry->Node(0).LiteralInt(entry));
        const eastl::vector<PromptInfo> prompts = _ParsePromptInfo(entry);
        mRuntime.mLayoutPromptInfo[layout] = prompts;
    }
}

// Reconstructed from eboot.elf at 0xED4800.
void RBHelpbarCom::_LoadOverridePromptInfo(const DataArrayPtr& config) {
    static Symbol helpbarConfig;
    if (helpbarConfig == Symbol()) {
        helpbarConfig = Symbol("helpbar_config");
    }
    static Symbol overrideConfigs;
    if (overrideConfigs == Symbol()) {
        overrideConfigs = Symbol("override_configs");
    }
    const DataArray* const configs = config->FindArray(helpbarConfig, overrideConfigs);
    for (int index = 1; index < configs->Size(); ++index) {
        const DataArrayPtr entry = configs->Node(index).Array(configs);
        const Symbol name = entry->Node(0).LiteralSym(entry);
        const eastl::vector<PromptInfo> prompts = _ParsePromptInfo(entry);
        mRuntime.mOverridePromptInfo[name] = prompts;
    }
}

// Reconstructed from eboot.elf at 0xED4B80.
void RBHelpbarCom::_DumpPromptInfo() {
    for (auto it = mRuntime.mLayoutPromptInfo.begin(); it != mRuntime.mLayoutPromptInfo.end(); ++it) {
    }
    for (auto it = mRuntime.mOverridePromptInfo.begin(); it != mRuntime.mOverridePromptInfo.end(); ++it) {
    }
}

// Reconstructed from eboot.elf at 0xED4BE0.
void RBHelpbarCom::_UpdatePromptInfo() {
    const eastl::vector<PromptInfo>& current = _GetCurrentPromptInfo();
    if (&current != &mRuntime.mCurrentPromptInfo) {
        mRuntime.mCurrentPromptInfo = current;
    }
    _UpdateButtonOverride(mRuntime.mCurrentPromptInfo);
    int last = -1;
    for (unsigned long index = 0; index < mRuntime.mCurrentPromptInfo.size(); ++index) {
        if (mRuntime.mCurrentPromptInfo[index].mToken != Symbol()) {
            last = static_cast<int>(index);
        }
    }
    mRuntime.mPromptCount = last >= 0 ? static_cast<unsigned long>(last + 1) : 0;
    _ClearButtonOverrideChanges();
    _UpdatePromptProps();
}

// Reconstructed from eboot.elf at 0xED4E90.
void RBHelpbarCom::_Enter() {
    // Component begins with MsgSink's slots; the model does not derive it.
    MsgSink* const sink = reinterpret_cast<MsgSink*>(this);
    Entity* const scene = FindSceneEntity(mObject->mEntity);
    if (scene != nullptr) {
        mRuntime.mSceneEntity = scene;
        RBUIInputCom* const input = scene->GetRoot()->GetCom<RBUIInputCom>();
        if (input != nullptr) {
            input->RegisterCom(this);
        }
        static Symbol transitionDone;
        if (transitionDone == Symbol()) {
            transitionDone = Symbol("transition_done");
        }
        scene->AddSink(&mRuntime.mTransitionDoneSink, sink, transitionDone, Symbol());
    }
    theRBMetaState.AddSink(&mRuntime.mHelpbarOverrideSink, sink, HelpbarOverrideMsg::Event(), Symbol());
    theRBMetaState.AddSink(&mRuntime.mHelpbarButtonOverrideSink, sink, HelpbarButtonOverrideMsg::Event(), Symbol());
    if (theBandUserMgr != nullptr) {
        static Symbol userSlotCleared;
        if (userSlotCleared == Symbol()) {
            userSlotCleared = Symbol("user_slot_cleared");
        }
        static Symbol userSlotSet;
        if (userSlotSet == Symbol()) {
            userSlotSet = Symbol("user_slot_set");
        }
        theBandUserMgr->AddSink(&mRuntime.mUserSlotClearedSink, sink, userSlotCleared, Symbol());
        theBandUserMgr->AddSink(&mRuntime.mUserSlotSetSink, sink, userSlotSet, Symbol());
    }
}

// Reconstructed from eboot.elf at 0xED51D0.
void RBHelpbarCom::_Poll() {
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (mRuntime.mLastLayout == layout && !mRuntime.mConfigChanged && !_AnyButtonOverrideChanged()) {
        return;
    }
    mRuntime.mConfigChanged = false;
    mRuntime.mLastLayout = layout;
    _UpdatePromptInfo();
    static Symbol configChanged;
    if (configChanged == Symbol()) {
        configChanged = Symbol("helpbar_config_changed");
    }
    const Message<0> changedConfig(configChanged);
    mObject->mEntity->Export(changedConfig.mData);
    if (!theUI->IsInTransition()) {
        const HelpbarChangedMsg changed;
        mObject->mEntity->Export(changed.mData);
    }
}

// Reconstructed from eboot.elf at 0xED5430.
bool RBHelpbarCom::_AnyButtonOverrideChanged() const {
    for (const bool changed : mRuntime.mButtonOverrideChanged) {
        if (changed) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xED54C0. The sound plays once: for a
// handled message, or else when a layout is gone to.
bool RBHelpbarCom::_GoBack(int padNum) {
    bool handled = false;
    if (mRuntime.mSceneEntity != nullptr) {
        static Symbol helpbarBack;
        if (helpbarBack == Symbol()) {
            helpbarBack = Symbol("helpbar_back");
        }
        Message<1> back(helpbarBack);
        back.mData->Node(2) = DataNode(padNum);
        handled = mRuntime.mSceneEntity->GetRoot()->ExportToSiblings(this, back.mData);
        if (handled && mBackSound != Symbol()) {
            PlayHelpbarSound(mBackSound);
        }
    }
    if (mPreviousLayout == kLayoutInvalid) {
        return handled;
    }
    theRBMetaState.Goto(mPreviousLayout, false);
    if (!handled && mBackSound != Symbol()) {
        PlayHelpbarSound(mBackSound);
    }
    return true;
}

// Reconstructed from eboot.elf at 0xED5700. Unlike _GoBack, the sound plays
// before the layout is gone to.
bool RBHelpbarCom::_Accept(int padNum) {
    bool handled = false;
    if (mRuntime.mSceneEntity != nullptr) {
        static Symbol helpbarAccept;
        if (helpbarAccept == Symbol()) {
            helpbarAccept = Symbol("helpbar_accept");
        }
        Message<1> accept(helpbarAccept);
        accept.mData->Node(2) = DataNode(padNum);
        handled = mRuntime.mSceneEntity->GetRoot()->ExportToSiblings(this, accept.mData);
        if (handled && mAcceptSound != Symbol()) {
            PlayHelpbarSound(mAcceptSound);
        }
    }
    if (mNextLayout == kLayoutInvalid) {
        return handled;
    }
    if (!handled && mAcceptSound != Symbol()) {
        PlayHelpbarSound(mAcceptSound);
    }
    theRBMetaState.Goto(mNextLayout, false);
    return true;
}

// Reconstructed from eboot.elf at 0xED5930. The entry's arrays after its
// key fill the prompts in order; the release build does not check that
// they fit.
eastl::vector<RBHelpbarCom::PromptInfo> RBHelpbarCom::_ParsePromptInfo(const DataArrayPtr& entry) {
    eastl::vector<PromptInfo> prompts;
    prompts.resize(kNumPrompts);
    for (int index = 1; index < entry->Size(); ++index) {
        const DataArrayPtr prompt = entry->Node(index).Array(entry);
        PromptInfo& info = prompts[index - 1];
        info.mAction = static_cast<JoypadAction>(prompt->Node(0).LiteralInt(prompt));
        info.mToken = prompt->Node(1).LiteralSym(prompt);
        info.mSecondaryToken = prompt->Size() >= 3 ? prompt->Node(2).LiteralSym(prompt) : Symbol();
    }
    return prompts;
}

// Reconstructed from eboot.elf at 0xED5AC0.
RBHelpbarCom::PromptInfo RBHelpbarCom::_ParsePrompt(const DataArrayPtr& prompt) {
    PromptInfo info;
    info.mAction = static_cast<JoypadAction>(prompt->Node(0).LiteralInt(prompt));
    info.mToken = prompt->Node(1).LiteralSym(prompt);
    if (prompt->Size() >= 3) {
        info.mSecondaryToken = prompt->Node(2).LiteralSym(prompt);
    }
    return info;
}

// Reconstructed from eboot.elf at 0xED5B30. The release build keeps the
// layout names of its stripped warnings.
const eastl::vector<RBHelpbarCom::PromptInfo>& RBHelpbarCom::_GetCurrentPromptInfo() {
    const Symbol helpbarOverride = theRBMetaState.GetHelpbarOverride();
    const auto overrideInfo = mRuntime.mOverridePromptInfo.find(helpbarOverride);
    if (overrideInfo != mRuntime.mOverridePromptInfo.end()) {
        return overrideInfo->second;
    }
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (helpbarOverride != Symbol()) {
        static_cast<void>(RBUILayoutToSym(layout));
    }
    const auto layoutInfo = mRuntime.mLayoutPromptInfo.find(layout);
    if (layoutInfo != mRuntime.mLayoutPromptInfo.end()) {
        return layoutInfo->second;
    }
    if (layout != kLayoutInvalid) {
        static_cast<void>(RBUILayoutToSym(layout));
    }
    return sEmptyPromptInfo;
}

// Reconstructed from eboot.elf at 0xED5C70. The release build keeps the
// current prompts' lookup and the names of its stripped error.
bool RBHelpbarCom::_UpdateButtonOverride(eastl::vector<PromptInfo>& prompts) {
    static_cast<void>(_GetCurrentPromptInfo());
    bool changed = false;
    for (int action = 0; action < kNumHelpbarButtonOverrides; ++action) {
        const RBMetaState::ButtonOverride button =
            theRBMetaState.GetHelpbarButtonOverride(static_cast<JoypadAction>(action));
        bool found = false;
        for (PromptInfo& prompt : prompts) {
            if (prompt.mAction != action) {
                continue;
            }
            found = true;
            if (button.mLabel != Symbol()) {
                const bool hide = button.mLabel == mRuntime.mNoButtonOverride;
                prompt.mToken = hide ? Symbol() : button.mLabel;
                prompt.mSecondaryToken = hide ? Symbol() : button.mIcon;
                changed = true;
            }
        }
        if (!found && button.mLabel != Symbol()) {
            static_cast<void>(MakeErrorName());
            static_cast<void>(JoypadActionToName(static_cast<JoypadAction>(action)));
        }
    }
    return changed;
}

// Reconstructed from eboot.elf at 0xED5DB0.
void RBHelpbarCom::_ClearButtonOverrideChanges() {
    for (bool& changed : mRuntime.mButtonOverrideChanged) {
        changed = false;
    }
}

// Reconstructed from eboot.elf at 0xED5DD0.
void RBHelpbarCom::_UpdatePromptProps() {
    for (unsigned int index = 0; index < mRuntime.mPrompts.size(); ++index) {
        mRuntime.mPrompts[index] = PromptInfo();
    }
    for (unsigned long index = 0;
         index < mRuntime.mCurrentPromptInfo.size() && index < mRuntime.mPrompts.size();
         ++index) {
        mRuntime.mPrompts[index] = mRuntime.mCurrentPromptInfo[index];
    }
}

// Reconstructed from eboot.elf at 0xED5EA0.
RBHelpbarCom::PromptTokens RBHelpbarCom::_GetToken(
    const eastl::vector<PromptInfo>& prompts,
    JoypadAction action) const {
    for (const PromptInfo& prompt : prompts) {
        if (prompt.mAction == action) {
            return PromptTokens{prompt.mToken, prompt.mSecondaryToken};
        }
    }
    return PromptTokens{Symbol(), Symbol()};
}

// Reconstructed from eboot.elf at 0xED5EE0.
DataNode RBHelpbarCom::_OnUserSlotChanged() {
    const HelpbarChangedMsg changed;
    mObject->mEntity->Export(changed.mData);
    return DataNode();
}

// Reconstructed from eboot.elf at 0xED5FE0. The same body as
// _OnUserSlotChanged.
DataNode RBHelpbarCom::_OnTransitionDone() {
    const HelpbarChangedMsg changed;
    mObject->mEntity->Export(changed.mData);
    return DataNode();
}

// Reconstructed from eboot.elf at 0xED60E0.
DataNode RBHelpbarCom::OnMsg(const UIKeyboardKeyMsg& msg) {
    DataNode result;
    if (msg.GetKeyData()) {
        return DataNode();
    }
    const int key = msg.GetKey();
    if (key == kKeyBackspace) {
        if (_GoBack(kNoPad)) {
            result = DataNode(0);
        }
    } else if (key == kKeyEnter) {
        if (_Accept(kNoPad)) {
            result = DataNode(0);
        }
    }
    return result;
}

// Reconstructed from eboot.elf at 0xED6320.
DataNode RBHelpbarCom::OnMsg(const UIButtonDownMsg& msg) {
    const int action = msg.GetAction();
    if (action == kActionCancel) {
        if (_GoBack(msg.GetPadNum())) {
            return DataNode(0);
        }
    } else if (action == kActionConfirm) {
        if (_Accept(msg.GetPadNum())) {
            return DataNode(0);
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xED6540.
DataNode RBHelpbarCom::OnMsg(const HelpbarButtonOverrideMsg& msg) {
    mRuntime.mButtonOverrideChanged[msg.GetAction()] = true;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xED6580.
DataNode RBHelpbarCom::OnMsg(const HelpbarOverrideMsg& msg) {
    static_cast<void>(msg);
    mRuntime.mConfigChanged = true;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xED65A0. The slot and transition
// handlers answer unhandled, so their messages fall through to the end.
DataNode RBHelpbarCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol userSlotCleared;
    if (userSlotCleared == Symbol()) {
        userSlotCleared = Symbol("user_slot_cleared");
    }
    if (type == userSlotCleared) {
        DataNode result = _OnUserSlotChanged();
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol userSlotSet;
    if (userSlotSet == Symbol()) {
        userSlotSet = Symbol("user_slot_set");
    }
    if (type == userSlotSet) {
        DataNode result = _OnUserSlotChanged();
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol transitionDone;
    if (transitionDone == Symbol()) {
        transitionDone = Symbol("transition_done");
    }
    if (type == transitionDone) {
        DataNode result = _OnTransitionDone();
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == UIButtonDownMsg::Event()) {
        DataNode result = OnMsg(UIButtonDownMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == UIKeyboardKeyMsg::Event()) {
        DataNode result = OnMsg(UIKeyboardKeyMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == HelpbarButtonOverrideMsg::Event()) {
        DataNode result = OnMsg(HelpbarButtonOverrideMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == HelpbarOverrideMsg::Event()) {
        DataNode result = OnMsg(HelpbarOverrideMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xED6AA0.
Symbol RBHelpbarCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xED6AB0.
Symbol RBHelpbarCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xED6AC0.
int RBHelpbarCom::CurrentRev() const {
    return const_cast<RBHelpbarCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xED6AE0.
bool RBHelpbarCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xED6B10.
Component* RBHelpbarCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xED6B20. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBHelpbarCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBHelpbarCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBHelpbarCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xED6C60.
PropRegistry& RBHelpbarCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xED6C70.
ComMetaData& RBHelpbarCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBAA450. The binary emits the factory
// with the other rb_meta factories.
Component* RBHelpbarCom::_Create() {
    return new RBHelpbarCom();
}

// Reconstructed from eboot.elf at 0xB81280. The binary emits the
// registration with the other rb_meta classes'.
void RBHelpbarCom::Init() {
    InitComponentClass<RBHelpbarCom>();
}
