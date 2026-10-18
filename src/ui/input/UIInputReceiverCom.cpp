// ui/UIInputReceiverCom.o (0x8A4A60 to 0x8A93FF). The registration and the
// factory are emitted in ui/UI.o.
#include "ui/input/UIInputReceiverCom.h"

#include <new>

#include "entity/core/TransCom.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "os/joypads/Joypad.h"
#include "render/scene/RndDrawableEntityResource.h"

namespace {

// The editor's list of the states, which the state properties and event
// parameters offer. The binary builds it in place for each. Name not in the
// reference map.
eastl::vector<AllowedValue<int>> UIStateAllowedValues() {
    return eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kUIStateInvalid, String("Invalid"), String("")},
        AllowedValue<int>{kUIStateNormal, String("Normal"), String("")},
        AllowedValue<int>{kUIStateFocused, String("Focused"), String("")},
        AllowedValue<int>{kUIStateDisabled, String("Disabled"), String("")},
        AllowedValue<int>{kUIStateSelecting, String("Selecting"), String("")},
        AllowedValue<int>{kUIStateSelected, String("Selected"), String("")}};
}

// The parameters of a button event: a hidden first parameter, the button
// and the action by their names, and the pad, up to 7. Name not in the
// reference map.
void RegisterButtonParams(PropRegistry& params, int (*maxPadNum)(const StaticTagArgs& args)) {
    PropMetadata& hidden = *params.RegisterProp("_hidden_", -1, kPropertyInt, 0).mMetadata.Get();
    hidden.mIsHidden = true;

    const int buttonType = 0;
    IntMetadata<int>& button =
        TypeSpecificMetadata(params.RegisterProp("button", -1, kPropertyInt, 0), buttonType);
    static_cast<void>(Symbol("param"));
    button.mHelp = String("The joypad button associated to the button press.");
    eastl::vector<AllowedValue<int>> buttons;
    buttons.reserve(26);
    for (int index = 0; index != 26; ++index) {
        const char* const name = JoypadButtonToName(static_cast<JoypadButton>(index));
        buttons.push_back(AllowedValue<int>{index, String(name), String(name)});
    }
    button.mAllowedValues = buttons;

    const int actionType = 0;
    IntMetadata<int>& action =
        TypeSpecificMetadata(params.RegisterProp("action", -1, kPropertyInt, 0), actionType);
    static_cast<void>(Symbol("param"));
    action.mHelp = String("The joypad action associated to the button press.");
    eastl::vector<AllowedValue<int>> actions;
    actions.reserve(15);
    for (int index = 0; index != 15; ++index) {
        const char* const name = JoypadActionToName(static_cast<JoypadAction>(index));
        actions.push_back(AllowedValue<int>{index, String(name), String(name)});
    }
    action.mAllowedValues = actions;

    const int padType = 0;
    IntMetadata<int>& padNum =
        TypeSpecificMetadata(params.RegisterProp("pad_num", -1, kPropertyInt, 0), padType);
    static_cast<void>(Symbol("param"));
    padNum.mHelp = String("The joypad number associated to the button press.");
    padNum.mMin = 0;
    padNum.mMax._Set(maxPadNum);
}

// The accessors of "selection_state", "previous_state" and
// "in_first_focus" (0x8A8F60 to 0x8A8FA0). Setting the state queues it as
// not initiated by the user. Names not in the reference map.
int GetSelectionState(const PropAccessorArgs& args) {
    return static_cast<const UIInputReceiverCom*>(args.mComponent)->mRuntime.mState;
}

void SetSelectionState(const PropAccessorArgs& args) {
    UIInputReceiverCom::RuntimeData& runtime =
        static_cast<UIInputReceiverCom*>(args.mComponent)->mRuntime;
    runtime.mQueuedState = *static_cast<const UIState*>(args.mValue);
    runtime.mQueuedUserInitiated = false;
}

int GetPreviousState(const PropAccessorArgs& args) {
    return static_cast<const UIInputReceiverCom*>(args.mComponent)->mRuntime.mPreviousState;
}

bool GetInFirstFocus(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return false;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8A9340).
// The int it first sets to -1 (0x1AC8A48) comes from a shared header and is
// not modelled.
Symbol UIInputReceiverCom::sId("UIInputReceiverCom");
Symbol UIInputReceiverCom::sClassName("UIInputReceiverCom");
PropRegistry UIInputReceiverCom::sPropRegistry;
ComMetaData UIInputReceiverCom::sMetaData;

// Reconstructed from eboot.elf at 0x8A4A60.
UIInputReceiverCom::UIInputReceiverCom()
    : mRuntime(),
      mSelectToFocusParams{false, 30},
      mNumSelectingFrames(0),
      mCanSelect(true),
      mCanNavigateTo(true),
      mLockedToPadNum(-1),
      mAlternateNavigationTrans(GameObjectId{0xFFFFFFFFU}) {}

// Reconstructed from eboot.elf at 0x8A4AF0. The deleting destructor is at
// 0x8A4B00.
UIInputReceiverCom::~UIInputReceiverCom() {}

// Reconstructed from eboot.elf at 0x8A55B0.
void UIInputReceiverCom::_Enter() {
    if (mRuntime.mState == kUIStateInvalid && mRuntime.mQueuedState == kUIStateInvalid) {
        mRuntime.mQueuedState = kUIStateNormal;
        mRuntime.mQueuedUserInitiated = false;
    }
}

// Reconstructed from eboot.elf at 0x8A5830. The events' parameters are
// registered while tag functions that need a component are not allowed.
void UIInputReceiverCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that receives all input from the UI Manager.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mInterface = sClassName;
    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);

    {
        PropRegistry& stateChange =
            metadata.AddExportedEvent(Symbol("ui_state_change"), String("The UI state has changed."));
        const GameObjectId objectType = GameObjectId{0xFFFFFFFFU};
        GameObjectIdMetadata& object =
            TypeSpecificMetadata(stateChange.RegisterProp("object", -1, kPropertyGameObjectId, 0), objectType);
        static_cast<void>(Symbol("param"));
        object.mHelp = String("The game object ID that had the state change.");
        object.mRequiredComs = {sId};

        const Symbol nameType;
        SymbolMetadata& objectName =
            TypeSpecificMetadata(stateChange.RegisterProp("object_name", -1, kPropertySymbol, 0), nameType);
        static_cast<void>(Symbol("param"));
        objectName.mHelp = String("The game object name that had the state change.");

        const int previousType = 0;
        IntMetadata<int>& previous = TypeSpecificMetadata(
            stateChange.RegisterProp("previous_state", -1, kPropertyInt, 0), previousType);
        static_cast<void>(Symbol("param"));
        previous.mHelp = String("The previous state.");
        previous.mAllowedValues = UIStateAllowedValues();

        const int currentType = 0;
        IntMetadata<int>& current = TypeSpecificMetadata(
            stateChange.RegisterProp("current_state", -1, kPropertyInt, 0), currentType);
        static_cast<void>(Symbol("param"));
        current.mHelp = String("The current state.");
        current.mAllowedValues = UIStateAllowedValues();

        const bool userType = false;
        BoolMetadata& user = TypeSpecificMetadata(
            stateChange.RegisterProp("user_initiated", -1, kPropertyBool, 0), userType);
        static_cast<void>(Symbol("param"));
        user.mHelp = String(
            "Was this state change initiated by the user's input (navigation, selection, etc.)?");
    }
    RegisterButtonParams(
        metadata.AddExportedEvent(
            Symbol("ui_button_down"),
            String("A button was pressed- check action or button parameter to find out what it was!  "
                   "Use this message if you want to handle button presses.")),
        +[](const StaticTagArgs& args) {
            static_cast<void>(args);
            return 7;
        });
    RegisterButtonParams(
        metadata.AddExportedEvent(
            Symbol("ui_button_down_feedback"),
            String("A button was pressed- check action or button parameter to find out what it was!  "
                   "Use this message if you want to react to button presses.")),
        +[](const StaticTagArgs& args) {
            static_cast<void>(args);
            return 7;
        });
    SetNonStaticTagFunctionsAllowed(nonStatic);

    UIInputReceiverCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    {
        PropRegistry& params = static_cast<GroupMetadata&>(
            *registry
                 .RegisterProp(
                     "select_to_focus_params",
                     offsetof(UIInputReceiverCom, mSelectToFocusParams),
                     kPropertyStruct,
                     sizeof(SelectToFocusParams))
                 .mMetadata.Get()).mRegistry;
        static_cast<void>(Symbol("struct"));
        const SelectToFocusParams paramsPrototype = {false, 30};

        BoolMetadata& enable = TypeSpecificMetadata(
            params.RegisterProp("enable", offsetof(SelectToFocusParams, mEnable), kPropertyBool, 1),
            paramsPrototype.mEnable);
        enable.mDefault = paramsPrototype.mEnable;
        static_cast<void>(Symbol("prop"));
        enable.mHelp = String("Are we going to go from selected back to focused state?");

        IntMetadata<int>& frames = TypeSpecificMetadata(
            params.RegisterProp("frames", offsetof(SelectToFocusParams, mFrames), kPropertyInt, 4),
            paramsPrototype.mFrames);
        frames.mDefault = paramsPrototype.mFrames;
        static_cast<void>(Symbol("prop"));
        frames.mHelp = String(
            "The number of frames we are waiting in selected state before going back to focus state.");
        frames.mDefault = 30;
        frames.mMin = 0;
        frames.mReadOnly._Set([](const TagArgs& args) {
            return !static_cast<const UIInputReceiverCom&>(args.mComponent).mSelectToFocusParams.mEnable;
        });
    }

    IntMetadata<int>& selectingFrames = TypeSpecificMetadata(
        registry.RegisterProp(
            "num_selecting_frames", offsetof(UIInputReceiverCom, mNumSelectingFrames), kPropertyInt, 4),
        prototype.mNumSelectingFrames);
    selectingFrames.mDefault = prototype.mNumSelectingFrames;
    static_cast<void>(Symbol("prop"));
    selectingFrames.mMin = 0;
    selectingFrames.mHelp =
        String("The number of frames we wait after user selects object to be in selected state.");

    BoolMetadata& canSelect = TypeSpecificMetadata(
        registry.RegisterProp("can_select", offsetof(UIInputReceiverCom, mCanSelect), kPropertyBool, 1),
        prototype.mCanSelect);
    canSelect.mDefault = prototype.mCanSelect;
    static_cast<void>(Symbol("prop"));
    canSelect.mHelp = String("Can this object be selected (aka receive select messages)?");

    BoolMetadata& canNavigate = TypeSpecificMetadata(
        registry.RegisterProp(
            "can_navigate_to", offsetof(UIInputReceiverCom, mCanNavigateTo), kPropertyBool, 1),
        prototype.mCanNavigateTo);
    canNavigate.mDefault = prototype.mCanNavigateTo;
    static_cast<void>(Symbol("prop"));
    canNavigate.mHelp = String("Can the standard UIInput navigation focus this object?");

    IntMetadata<int>& lockedPad = TypeSpecificMetadata(
        registry.RegisterProp(
            "locked_to_pad_num", offsetof(UIInputReceiverCom, mLockedToPadNum), kPropertyInt, 4),
        prototype.mLockedToPadNum);
    lockedPad.mDefault = prototype.mLockedToPadNum;
    static_cast<void>(Symbol("prop"));
    lockedPad.mHelp = String(
        "You can lock all input queries to a specific joypad number. -1 means it's not locked.");
    lockedPad.mMin = -1;
    lockedPad.mMax = 8;

    const int stateType = 0;
    IntMetadata<int>& selectionState =
        TypeSpecificMetadata(registry.RegisterProp("selection_state", -1, kPropertyInt, 0), stateType);
    static_cast<void>(Symbol("prop"));
    selectionState.mAllowedValues = UIStateAllowedValues();
    selectionState.mHistory = false;
    selectionState.mGetter = GetSelectionState;
    selectionState.mSetter = SetSelectionState;

    const int previousType = 0;
    IntMetadata<int>& previousState =
        TypeSpecificMetadata(registry.RegisterProp("previous_state", -1, kPropertyInt, 0), previousType);
    static_cast<void>(Symbol("prop"));
    previousState.mAllowedValues = UIStateAllowedValues();
    previousState.mGetter = GetPreviousState;
    previousState.mReadOnly = true;

    GameObjectIdMetadata& navigationTrans = TypeSpecificMetadata(
        registry.RegisterProp(
            "alternate_navigation_trans",
            offsetof(UIInputReceiverCom, mAlternateNavigationTrans),
            kPropertyGameObjectId,
            4),
        prototype.mAlternateNavigationTrans);
    navigationTrans.mDefault = prototype.mAlternateNavigationTrans;
    static_cast<void>(Symbol("prop"));
    navigationTrans.mHelp = String(
        "Use this transform for navigation instead of the trans of the object we're attached to.");
    navigationTrans.mRequiredComs = {TransCom::sId};

    const bool firstFocusType = false;
    BoolMetadata& firstFocus =
        TypeSpecificMetadata(registry.RegisterProp("in_first_focus", -1, kPropertyBool, 0), firstFocusType);
    static_cast<void>(Symbol("prop"));
    firstFocus.mGetter = GetInFirstFocus;
    firstFocus.mReadOnly = true;
    firstFocus.mHelp = String("!!!DEPRECATED - DON'T USE ANYMORE!!!");

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev <= 0) {
                static_cast<UIInputReceiverCom*>(args.mComponent)->mNumSelectingFrames = 0;
            }
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    }
}

// Reconstructed from eboot.elf at 0x8A8440.
Symbol UIInputReceiverCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8A8450.
Symbol UIInputReceiverCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8A8460.
int UIInputReceiverCom::CurrentRev() const {
    return const_cast<UIInputReceiverCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8A8480.
bool UIInputReceiverCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8A84B0.
Component* UIInputReceiverCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8A84C0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIInputReceiverCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIInputReceiverCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIInputReceiverCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8A8630.
PropRegistry& UIInputReceiverCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8A8640.
ComMetaData& UIInputReceiverCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x8A8650.
bool UIInputReceiverCom::_HasPostPoll() const {
    return true;
}

// Reconstructed from eboot.elf at 0x895020. The binary emits the factory
// in ui/UI.o.
Component* UIInputReceiverCom::_Create() {
    return new UIInputReceiverCom();
}

// Reconstructed from eboot.elf at 0x891F80. The binary emits the
// registration in ui/UI.o; the class is a superclass root.
void UIInputReceiverCom::Init() {
    InitComponentClass<UIInputReceiverCom>(true, true);
}
