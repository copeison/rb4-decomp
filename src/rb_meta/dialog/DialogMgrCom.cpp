// rb_meta/DialogMgrCom.o (0xADAE60 to 0xAE2620). The registrations of the
// property accessors are the lambdas after the identity slots
// (0xAE0B00-0xAE0D20). The vector of queued dialogs uses the compiler's
// EASTL helpers (0xADDDB0 swap and 0xADDF40 shrink in ClearAllDialogs, and
// 0xAE0D30-0xAE2170 for the copies, the insertion and the erasure). The
// static initializer is at 0xAE23E0; the factory at 0xAE25D0 and the
// function after it belong to the next object.
#include "rb_meta/dialog/DialogMgrCom.h"

#include <cfloat>
#include <new>

#include "entity/core/Entity.h"
#include "os/locale/Locale.h"
#include "os/threading/CritSec.h"
#include "rb_game/users/BandUser.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "rb_meta/overshell/RBOvershellCom.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/textures/RndTexture2DResource.h"
#include "ui/input/UIMsg.h"
#include "ui/manager/UIMgr.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1AF49E8, which the static initializer sets
// with the shared header's thread-group widths. Each object that uses it
// has its own copy. Names not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};
[[maybe_unused]] int gDialogGroupSize2D = 8;  // 0x1AF49EC
[[maybe_unused]] int gDialogGroupSize3D = 4;  // 0x1AF49F0

}  // namespace

// The object's statics, in the order of its static initializer (0xAE23E0).
Symbol DialogMgrCom::sId("DialogMgrCom");
Symbol DialogMgrCom::sClassName("DialogMgrCom");
PropRegistry DialogMgrCom::sPropRegistry;
ComMetaData DialogMgrCom::sMetaData;
const char* DialogMgrCom::kDialogScenePath = "ui/dialog/dialog.scene";

namespace {

// The dialog being shown, or about to be shown once its scene enters. The
// properties read and write it. Name not in the reference map.
DialogMgrCom::DialogArgs sDialogArgs;  // 0x1AF4C60
// The dialogs ShowDialog could not show yet, in order, and the lock that
// guards them. Names not in the reference map.
eastl::vector<DialogMgrCom::DialogArgs> sDialogQueue;  // 0x1AF4D60
CritSec sDialogQueueLock;                              // 0x1AF4D80
// The component of the shown dialog's scene, set by its _Enter. Name not
// in the reference map.
DialogMgrCom* sActiveDialog;  // 0x1AF4D90

// The JoypadAction values OnMsg(UIButtonDownMsg const&) checks. Names not
// in the reference map; they follow the engine's kAction_Confirm and
// kAction_Cancel.
constexpr int kActionConfirm = 1;
constexpr int kActionCancel = 2;

// A text as the InitFor* functions store it: the token itself, or its
// localized text. Inlined into each of them. Name not in the reference
// map.
Symbol DialogText(const Symbol& text, const DialogMgrCom::DialogTextLocType& loc) {
    if (loc != DialogMgrCom::kDialogTextToken) {
        return text;
    }
    return Symbol(Localize(text, nullptr, 0, TheLocale));
}

// The component whose property is accessed. Name not in the reference map.
DialogMgrCom& Dialog(const PropAccessorArgs& args) {
    return *static_cast<DialogMgrCom*>(args.mComponent);
}

// Registers one of the text properties, which read and write the
// component's copy of the text and are neither saved nor kept in the
// history. Name not in the reference map.
template <Symbol DialogMgrCom::*Text>
void RegisterTextProp(PropRegistry& registry, const char* name) {
    SymbolMetadata& text = TypeSpecificMetadata(registry.RegisterProp(name, -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    text.mGetter = [](const PropAccessorArgs& args) { return Dialog(args).*Text; };
    text.mSetter = [](const PropAccessorArgs& args) { Dialog(args).*Text = *static_cast<const Symbol*>(args.mValue); };
    text.mSaves = false;
    text.mHistory = false;
}

// Registers one of the button arrays: objects other than null, each at
// most once. Name not in the reference map.
void RegisterButtonArray(PropRegistry& registry, const char* name, int offset, const char* help) {
    PropInfo& buttons = registry.RegisterPropArray(name, offset, sizeof(PropArray<GameObjectId>));
    ArrayMetadata& buttonsMetadata = static_cast<ArrayMetadata&>(*buttons.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(buttons, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId nullId = {0xFFFFFFFFU};
    GameObjectIdMetadata& button = TypeSpecificMetadata(buttonsMetadata.mItemInfo, nullId);
    static_cast<void>(Symbol("item"));
    button.mAllowsNull = false;
    button.mAllowsDuplicates = false;
    button.mHelp = String(help);
}

}  // namespace

// Reconstructed from eboot.elf at 0xADAF90. The position overrides are left
// uninitialized.
DialogMgrCom::DialogMgrCom()
    : mIcon1Image(),
      mIcon2Image(),
      mHeaderText(),
      mSubHeaderText(),
      mBody1Text(),
      mBody2Text(),
      mOkButtonTextToken(),
      mYesButtonTextToken(),
      mNoButtonTextToken(),
      mOptionAButtonTextToken(),
      mOptionBButtonTextToken(),
      mOptionCButtonTextToken(),
      mOptionDButtonTextToken(),
      mPreviousHelpbarOverride(),
      mOkButtons(),
      mYesButtons(),
      mNoButtons(),
      mCancelButtons(),
      mOptionAButtons(),
      mOptionBButtons(),
      mOptionCButtons(),
      mOptionDButtons() {}

// Reconstructed from eboot.elf at 0xADB150. The active dialog, or any
// component while none is active, puts the replaced helpbar override back;
// the active one also drops the current dialog. The deleting destructor is
// at 0xADB420.
DialogMgrCom::~DialogMgrCom() {
    if (sActiveDialog == this || sActiveDialog == nullptr) {
        theRBMetaState.ClearHelpbarOverride();
        if (mPreviousHelpbarOverride != Symbol()) {
            theRBMetaState.SetHelpbarOverride(mPreviousHelpbarOverride);
        }
        if (sActiveDialog == this) {
            sDialogArgs.Clear();
            sActiveDialog = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0xADB440. The registry is at revision 0.
// The queue gets room for 16 dialogs.
void DialogMgrCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    DialogMgrCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }
    sDialogQueue.reserve(16);

    metadata.mDescription = "Throws a dialog up on top of any current screen.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());

    IntMetadata<int>& layoutType = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("dialog_layout_type", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    layoutType.mHelp = String("The type of dialog.");
    layoutType.mGetter = [](const PropAccessorArgs&) { return static_cast<int>(sDialogArgs.mLayoutType); };
    layoutType.mSetter = [](const PropAccessorArgs& args) {
        sDialogArgs.mLayoutType = static_cast<DialogLayoutType>(*static_cast<const int*>(args.mValue));
    };
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{
            kDialogLayoutStandardText,
            String("Standard Text Popup"),
            String("The standard pop-up text-only dialog display.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutOnboardingText,
            String("Onboarding - Text Interstitial"),
            String("Text only Onboarding tutorial display")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutOnboardingOneIcon,
            String("Onboarding - Single Icon"),
            String("Onboarding tutorial with a single large icon.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutOnboardingTwoIcon,
            String("Onboarding - Two Icon"),
            String("Onboarding tutorial with two icons.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutUnlockText,
            String("Unlock - Text only"),
            String("A 3 text-field layout for displaying unlocked content info.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutVocalImprov,
            String("Vocal Improv"),
            String("A special-case popup for the animated Vocal Improv onboarding screen.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutUnlockBanner,
            String("Unlock - Banner"),
            String("A banner display for unlocking, intended for the Positive Grid foot pedal unlocks.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutPatchNotes, String("Patch Notes"), String("Special popup for displaying patch notes.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutSetlistChoice,
            String("Setlist Choice"),
            String("Special popup for choosing what to do with a setlist.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutCustomText,
            String("Custom Text Popup"),
            String("The standard pop-up text-only dialog display, but with Yes/No text override.")});
        values.push_back(AllowedValue<int>{
            kDialogLayoutOnboardingTextDirected,
            String("Onboarding - Text Directed"),
            String("Text only Onboarding tutorial display, tied to a UI Element")});
        layoutType.mAllowedValues = values;
        layoutType.mSaves = false;
    }

    IntMetadata<int>& type = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("dialog_type", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    type.mHelp = String("The type of dialog.");
    type.mGetter = [](const PropAccessorArgs&) { return static_cast<int>(sDialogArgs.mType); };
    type.mSetter = [](const PropAccessorArgs& args) {
        sDialogArgs.mType = static_cast<DialogType>(*static_cast<const int*>(args.mValue));
    };
    type.mAllowedValues = GetDialogTypeAllowedValues();
    type.mSaves = false;

    ResourcePathMetadata& icon1 =
        TypeSpecificMetadata(registry.RegisterProp("icon1_image", -1, kPropertyResourcePath, 0), ResourcePath());
    static_cast<void>(Symbol("prop"));
    icon1.mResourceId = RndTexture2DResource::Id();
    icon1.mSetter = [](const PropAccessorArgs& args) {
        Dialog(args).mIcon1Image = *static_cast<const ResourcePath*>(args.mValue);
    };
    icon1.mGetter = [](const PropAccessorArgs& args) { return Dialog(args).mIcon1Image; };
    icon1.mHistory = false;

    ResourcePathMetadata& icon2 =
        TypeSpecificMetadata(registry.RegisterProp("icon2_image", -1, kPropertyResourcePath, 0), ResourcePath());
    static_cast<void>(Symbol("prop"));
    icon2.mResourceId = RndTexture2DResource::Id();
    icon2.mSetter = [](const PropAccessorArgs& args) {
        Dialog(args).mIcon2Image = *static_cast<const ResourcePath*>(args.mValue);
    };
    icon2.mGetter = [](const PropAccessorArgs& args) { return Dialog(args).mIcon2Image; };
    icon2.mSaves = false;
    icon2.mHistory = false;

    RegisterTextProp<&DialogMgrCom::mHeaderText>(registry, "header_text");
    RegisterTextProp<&DialogMgrCom::mSubHeaderText>(registry, "sub_header_text");
    RegisterTextProp<&DialogMgrCom::mBody1Text>(registry, "body1_text");
    RegisterTextProp<&DialogMgrCom::mBody2Text>(registry, "body2_text");
    RegisterTextProp<&DialogMgrCom::mOkButtonTextToken>(registry, "ok_button_text_token");
    RegisterTextProp<&DialogMgrCom::mYesButtonTextToken>(registry, "yes_button_text_token");
    RegisterTextProp<&DialogMgrCom::mNoButtonTextToken>(registry, "no_button_text_token");
    RegisterTextProp<&DialogMgrCom::mOptionAButtonTextToken>(registry, "option_a_button_text_token");
    RegisterTextProp<&DialogMgrCom::mOptionBButtonTextToken>(registry, "option_b_button_text_token");
    RegisterTextProp<&DialogMgrCom::mOptionCButtonTextToken>(registry, "option_c_button_text_token");
    RegisterTextProp<&DialogMgrCom::mOptionDButtonTextToken>(registry, "option_d_button_text_token");

    FloatMetadata& xPos = TypeSpecificMetadata(registry.RegisterProp("x_pos_override", -1, kPropertyFloat, 0), 0.0F);
    static_cast<void>(Symbol("prop"));
    xPos.mGetter = [](const PropAccessorArgs& args) { return Dialog(args).mXPosOverride; };
    xPos.mSetter = [](const PropAccessorArgs& args) {
        Dialog(args).mXPosOverride = *static_cast<const float*>(args.mValue);
    };
    xPos.mSaves = false;
    xPos.mHistory = false;

    FloatMetadata& zPos = TypeSpecificMetadata(registry.RegisterProp("z_pos_override", -1, kPropertyFloat, 0), 0.0F);
    static_cast<void>(Symbol("prop"));
    zPos.mGetter = [](const PropAccessorArgs& args) { return Dialog(args).mZPosOverride; };
    zPos.mSetter = [](const PropAccessorArgs& args) {
        Dialog(args).mZPosOverride = *static_cast<const float*>(args.mValue);
    };
    zPos.mSaves = false;
    zPos.mHistory = false;

    // Read-only: the current dialog's focus.
    IntMetadata<int>& focus = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("initial_button_focus", -1, kPropertyInt, 0).mMetadata.Get());
    static_cast<void>(Symbol("prop"));
    focus.mHelp = String("Which button to focus.");
    focus.mGetter = [](const PropAccessorArgs&) { return sDialogArgs.mInitialButtonFocus; };
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{0, String("Default"), String("Let the input com focus what it wants")});
        values.push_back(AllowedValue<int>{1, String("Yes"), String("Start with the Yes button focused")});
        values.push_back(AllowedValue<int>{2, String("No"), String("Start with the No button focused")});
        values.push_back(AllowedValue<int>{3, String("Cancel"), String("Start with the Cancel button focused")});
        focus.mAllowedValues = values;
        focus.mSaves = false;
    }

    RegisterButtonArray(
        registry, "ok_buttons", offsetof(DialogMgrCom, mOkButtons), "Object for any button that we hit ok on.");
    RegisterButtonArray(
        registry,
        "yes_buttons",
        offsetof(DialogMgrCom, mYesButtons),
        "Object for any button that functions as a 'yes' choice.");
    RegisterButtonArray(
        registry,
        "no_buttons",
        offsetof(DialogMgrCom, mNoButtons),
        "Object for any button that functions as a 'no' choice.");
    RegisterButtonArray(
        registry,
        "cancel_buttons",
        offsetof(DialogMgrCom, mCancelButtons),
        "Object for any button that functions as a 'cancel' choice.");
    RegisterButtonArray(
        registry,
        "option_a_buttons",
        offsetof(DialogMgrCom, mOptionAButtons),
        "Object for any button that functions as an 'Option A' choice.");
    RegisterButtonArray(
        registry,
        "option_b_buttons",
        offsetof(DialogMgrCom, mOptionBButtons),
        "Object for any button that functions as an 'Option B' choice.");
    RegisterButtonArray(
        registry,
        "option_c_buttons",
        offsetof(DialogMgrCom, mOptionCButtons),
        "Object for any button that functions as an 'Option C' choice.");
    RegisterButtonArray(
        registry,
        "option_d_buttons",
        offsetof(DialogMgrCom, mOptionDButtons),
        "Object for any button that functions as an 'Option D' choice.");
}

// Reconstructed from eboot.elf at 0xADD090. The labels are listed out of
// value order.
eastl::vector<AllowedValue<int>> DialogMgrCom::GetDialogTypeAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{kDialogTypeOk, String("OK"), String("Waits for OK button press")});
    values.push_back(
        AllowedValue<int>{kDialogTypeConditional, String("Conditional"), String("Waits for a condition to be met")});
    values.push_back(AllowedValue<int>{kDialogTypeYesNo, String("Yes/No"), String("Waits for Yes/No to be pressed")});
    values.push_back(AllowedValue<int>{
        kDialogTypeYesNoCancel, String("Yes/No/Cancel"), String("Waits for Yes/No/Cancel to be pressed")});
    values.push_back(AllowedValue<int>{
        kDialogTypeFourOptions, String("4 Options"), String("Waits for any of 4 options to be pressed")});
    values.push_back(AllowedValue<int>{
        kDialogTypeConditionalWithCancel,
        String("Conditional with Cancel"),
        String("Waits for a condition to be met, or for Red/B to be pressed")});
    values.push_back(AllowedValue<int>{
        kDialogTypeTwoOptions, String("2 Options"), String("Waits for any of 2 options to be pressed")});
    values.push_back(AllowedValue<int>{
        kDialogTypeThreeOptions, String("3 Options"), String("Waits for any of 3 options to be pressed")});
    return values;
}

// Inlined into ShowDialog and Poll; this build emits no copy.
inline bool DialogMgrCom::_SafeToShowDialog() {
    return sActiveDialog == nullptr && !sDialogArgs.mPending && !theUI->IsInTransition();
}

// Reconstructed from eboot.elf at 0xADD560.
void DialogMgrCom::ShowDialog(const DialogArgs& args) {
    if (!_SafeToShowDialog()) {
        ScopedCritSec lock(sDialogQueueLock);
        sDialogQueue.push_back(args);
        return;
    }
    if ((args.mType != kDialogTypeConditionalWithCancel && args.mType != kDialogTypeConditional)
        || args.mCondition) {
        sDialogArgs = args;
        ResourcePath path;
        path = kDialogScenePath;
        theUI->AddScene(path, 0);
    }
}

// Reconstructed from eboot.elf at 0xADD8B0.
bool DialogMgrCom::CancelDialog(bool immediate) {
    if (sActiveDialog == nullptr) {
        return false;
    }
    Button button = kButtonOk;
    switch (sDialogArgs.mType) {
    case kDialogTypeYesNo:
        button = kButtonNo;
        break;
    case kDialogTypeConditional:
        button = kButtonOk;
        break;
    case kDialogTypeYesNoCancel:
    case kDialogTypeFourOptions:
    case kDialogTypeConditionalWithCancel:
    case kDialogTypeTwoOptions:
    case kDialogTypeThreeOptions:
        button = kButtonCancel;
        break;
    default:
        break;
    }
    _HideDialog(button, nullptr, immediate);
    return true;
}

// Reconstructed from eboot.elf at 0xADD8F0. The callback is copied before
// the current dialog is cleared, so it may show the next dialog.
void DialogMgrCom::_HideDialog(Button button, BandUser* user, bool immediate) {
    if (sActiveDialog == nullptr) {
        return;
    }
    if (user != nullptr && user->GetSlot() != static_cast<unsigned long>(theRBMetaState.mFocusSlot)
        && sDialogArgs.mRequiredInstrument != kRequireAnyInstrument && !_IsUserRequiredInstrument(user)) {
        const eastl::vector<BandUser*> users = theBandUserMgr->GetBandUsers();
        for (BandUser* bandUser : users) {
            if (_IsUserRequiredInstrument(bandUser)) {
                return;
            }
        }
    }
    ResourcePath path;
    path = kDialogScenePath;
    theUI->RemoveScene(path, immediate);
    std::function<void(Button, BandUser*)> onHide = sDialogArgs.mOnHide;
    sDialogArgs.Clear();
    sActiveDialog = nullptr;
    if (onHide) {
        onHide(button, user);
    }
}

// Reconstructed from eboot.elf at 0xADDB80. The release build keeps a call
// of UIMgr::IsInTransition whose result it does not use. The binary empties
// the queue, swaps it with an empty vector and shrinks it, which releases
// its storage.
void DialogMgrCom::ClearAllDialogs() {
    static_cast<void>(theUI->IsInTransition());
    ScopedCritSec lock(sDialogQueueLock);
    if (sActiveDialog == nullptr && theUI->GetCurrentLayout() != nullptr) {
        ResourcePath path;
        path = kDialogScenePath;
        theUI->RemoveScene(path, 0);
    }
    sDialogArgs.Clear();
    sActiveDialog = nullptr;
    sDialogQueue.clear();
    sDialogQueue.set_capacity(0);
}

// Reconstructed from eboot.elf at 0xADE0A0.
bool DialogMgrCom::_IsUserRequiredInstrument(BandUser* user) {
    switch (sDialogArgs.mRequiredInstrument) {
    case kRequireAnyInstrument:
        return true;
    case kRequireGuitar:
        return user->GetTrackType() == kTrackGuitar;
    case kRequireBass:
        return user->GetTrackType() == kTrackBass;
    case kRequireVocals:
        return user->GetTrackType() == kTrackVocals;
    case kRequireDrums:
        return user->GetTrackType() == kTrackDrum;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0xADE100.
bool DialogMgrCom::IsDialogActive() {
    return sActiveDialog != nullptr;
}

// Reconstructed from eboot.elf at 0xADE110.
bool DialogMgrCom::IsDialogActive(Symbol name) {
    return sDialogArgs.mName == name;
}

// Reconstructed from eboot.elf at 0xADE120. The dialog is copied out of the
// queue before ShowDialog, which may queue it again.
void DialogMgrCom::Poll() {
    if (sDialogQueue.empty() || !_SafeToShowDialog()) {
        return;
    }
    ScopedCritSec lock(sDialogQueueLock);
    const DialogArgs args(sDialogQueue.front());
    ShowDialog(args);
    sDialogQueue.erase(sDialogQueue.begin());
}

// Reconstructed from eboot.elf at 0xADE350. The scene's RBUIInputCom
// leaves the focus to the dialog and gives it the input first. The helpbar
// is "dialog_directed_override" for the directed layout,
// "dialog_cancel_only_override" for a conditional dialog with a cancel,
// "dialog_cancel_override" for the other dialogs with a cancel or options
// and "dialog_override" otherwise; a replaced override that is none of
// these is kept for the destructor.
void DialogMgrCom::_Enter() {
    sActiveDialog = this;
    mHeaderText = sDialogArgs.mHeaderText;
    mSubHeaderText = sDialogArgs.mSubHeaderText;
    mBody1Text = sDialogArgs.mBody1Text;
    mBody2Text = sDialogArgs.mBody2Text;
    mOkButtonTextToken = sDialogArgs.mOkButtonTextToken;
    mYesButtonTextToken = sDialogArgs.mYesButtonTextToken;
    mNoButtonTextToken = sDialogArgs.mNoButtonTextToken;
    mOptionAButtonTextToken = sDialogArgs.mOptionAButtonTextToken;
    mOptionBButtonTextToken = sDialogArgs.mOptionBButtonTextToken;
    mOptionCButtonTextToken = sDialogArgs.mOptionCButtonTextToken;
    mOptionDButtonTextToken = sDialogArgs.mOptionDButtonTextToken;
    mXPosOverride = sDialogArgs.mXPosOverride;
    mZPosOverride = sDialogArgs.mZPosOverride;
    mIcon1Image = sDialogArgs.mIcon1Image;
    mIcon2Image = sDialogArgs.mIcon2Image;

    RBUIInputCom* const input = mObject->mEntity->GetRoot()->GetCom<RBUIInputCom>();
    input->SetIsModal(sDialogArgs.mModal);
    input->mIgnoreStickyFocus = true;
    input->RegisterCom(this);

    static Symbol dialogOverride;
    if (dialogOverride == Symbol()) {
        dialogOverride = Symbol("dialog_override");
    }
    static Symbol cancelOverride;
    if (cancelOverride == Symbol()) {
        cancelOverride = Symbol("dialog_cancel_override");
    }
    static Symbol cancelOnlyOverride;
    if (cancelOnlyOverride == Symbol()) {
        cancelOnlyOverride = Symbol("dialog_cancel_only_override");
    }
    static Symbol directedOverride;
    if (directedOverride == Symbol()) {
        directedOverride = Symbol("dialog_directed_override");
    }

    const Symbol previous = theRBMetaState.GetHelpbarOverride();
    Symbol helpbar = directedOverride;
    if (sDialogArgs.mLayoutType != kDialogLayoutOnboardingTextDirected) {
        switch (sDialogArgs.mType) {
        case kDialogTypeYesNoCancel:
        case kDialogTypeFourOptions:
        case kDialogTypeTwoOptions:
        case kDialogTypeThreeOptions:
            helpbar = cancelOverride;
            break;
        case kDialogTypeConditionalWithCancel:
            helpbar = cancelOnlyOverride;
            break;
        default:
            helpbar = dialogOverride;
            break;
        }
    }
    theRBMetaState.SetHelpbarOverride(helpbar);
    if (previous != directedOverride && previous != cancelOverride && previous != cancelOnlyOverride
        && previous != dialogOverride) {
        mPreviousHelpbarOverride = previous;
    }
}

// Reconstructed from eboot.elf at 0xADE660. The binary also compares the
// dialog's type with kDialogTypeConditional and
// kDialogTypeConditionalWithCancel, but both outcomes run the check.
void DialogMgrCom::_Poll() {
    _UpdateConditionDialog();
}

// Reconstructed from eboot.elf at 0xADE6A0.
void DialogMgrCom::_UpdateConditionDialog() {
    if (sDialogArgs.mCondition && sDialogArgs.mCondition()) {
        _HideDialog(kButtonConditionMet, nullptr, false);
    }
}

// Reconstructed from eboot.elf at 0xADE6D0.
DataNode DialogMgrCom::OnMsg(const UISelectMsg& msg) {
    BandUser* const user = BandUserMgr::GetBandUser(msg.GetUser());
    if (theOvershell->IsUserAllowingInputToShell(user)) {
        const GameObjectId selected = msg.GetSelectedObject();
        if (_IsOkButton(selected)) {
            _HideDialog(kButtonOk, user, false);
        } else if (_IsYesButton(selected)) {
            _HideDialog(kButtonYes, user, false);
        } else if (_IsNoButton(selected)) {
            _HideDialog(kButtonNo, user, false);
        } else if (_IsCancelButton(selected)) {
            _HideDialog(kButtonCancel, user, false);
        } else if (_IsOptionAButton(selected)) {
            _HideDialog(kButtonOptionA, user, false);
        } else if (_IsOptionBButton(selected)) {
            _HideDialog(kButtonOptionB, user, false);
        } else if (_IsOptionCButton(selected)) {
            _HideDialog(kButtonOptionC, user, false);
        } else if (_IsOptionDButton(selected)) {
            _HideDialog(kButtonOptionD, user, false);
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xADEAF0. Input the overshell keeps is
// handled and ignored.
DataNode DialogMgrCom::OnMsg(const UIButtonDownMsg& msg) {
    if (!theOvershell->IsUserAllowingInputToShell(BandUserMgr::GetBandUser(msg.GetUser()))) {
        return DataNode(0);
    }
    const int action = msg.GetAction();
    if (action == kActionCancel) {
        switch (sDialogArgs.mType) {
        case kDialogTypeYesNoCancel:
        case kDialogTypeFourOptions:
        case kDialogTypeConditionalWithCancel:
        case kDialogTypeTwoOptions:
        case kDialogTypeThreeOptions:
            _HideDialog(kButtonCancel, BandUserMgr::GetBandUser(msg.GetUser()), false);
            return DataNode(0);
        default:
            return DataNode();
        }
    }
    if (action == kActionConfirm && sDialogArgs.mLayoutType == kDialogLayoutOnboardingTextDirected) {
        _HideDialog(kButtonOk, BandUserMgr::GetBandUser(msg.GetUser()), false);
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xADEBA0.
void DialogMgrCom::InitForStandardTextPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const int& initialFocus,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutStandardText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mInitialButtonFocus = initialFocus;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADED20.
void DialogMgrCom::InitForStandardTextPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const int& initialFocus,
    const Symbol& okText,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    InitForStandardTextPopup(args, type, modal, body, bodyLoc, initialFocus, onHide, condition);
    args.mOkButtonTextToken = okText;
}

// Reconstructed from eboot.elf at 0xADEE50.
void DialogMgrCom::InitForCustomTextPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& yesText,
    const Symbol& noText,
    const int& initialFocus,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutCustomText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mYesButtonTextToken = yesText;
    args.mNoButtonTextToken = noText;
    args.mInitialButtonFocus = initialFocus;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADEFE0.
void DialogMgrCom::InitForCustomTextPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& yesText,
    const Symbol& noText,
    const int& initialFocus,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutCustomText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mYesButtonTextToken = yesText;
    args.mNoButtonTextToken = noText;
    args.mInitialButtonFocus = initialFocus;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADF1C0.
void DialogMgrCom::InitForTextOnlyOnboardingPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutOnboardingText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADF370.
void DialogMgrCom::InitForTextOnlyOnboardingPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const RequiredInstrument& instrument,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutOnboardingText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mRequiredInstrument = instrument;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADF530.
void DialogMgrCom::InitForOneIconOnboardingPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& subHeader,
    const DialogTextLocType& subHeaderLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const ResourcePath& icon,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutOnboardingOneIcon;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = header;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mSubHeaderText = DialogText(subHeader, subHeaderLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mIcon1Image = icon;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADF730.
void DialogMgrCom::InitForTwoIconOnboardingPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& subHeader,
    const DialogTextLocType& subHeaderLoc,
    const Symbol& body1,
    const DialogTextLocType& body1Loc,
    const Symbol& body2,
    const DialogTextLocType& body2Loc,
    const ResourcePath& icon1,
    const ResourcePath& icon2,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutOnboardingTwoIcon;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = header;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mSubHeaderText = DialogText(subHeader, subHeaderLoc);
    args.mBody1Text = DialogText(body1, body1Loc);
    args.mBody2Text = DialogText(body2, body2Loc);
    args.mIcon1Image = icon1;
    args.mIcon2Image = icon2;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADF970.
void DialogMgrCom::InitForTextDirectedOnboardingPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    float xPos,
    float zPos,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutOnboardingTextDirected;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mXPosOverride = xPos;
    args.mZPosOverride = zPos;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADFB40.
void DialogMgrCom::InitForBannerUnlockPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& subHeader,
    const DialogTextLocType& subHeaderLoc,
    const Symbol& body1,
    const DialogTextLocType& body1Loc,
    const Symbol& body2,
    const DialogTextLocType& body2Loc,
    const ResourcePath& icon,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutUnlockBanner;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = header;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mSubHeaderText = DialogText(subHeader, subHeaderLoc);
    args.mBody1Text = DialogText(body1, body1Loc);
    args.mBody2Text = DialogText(body2, body2Loc);
    args.mIcon1Image = icon;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADFD80.
void DialogMgrCom::InitForTextOnlyUnlockPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body1,
    const DialogTextLocType& body1Loc,
    const Symbol& body2,
    const DialogTextLocType& body2Loc,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutUnlockText;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = header;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body1, body1Loc);
    args.mBody2Text = DialogText(body2, body2Loc);
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xADFF70.
void DialogMgrCom::InitForPatchNotesPopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& header,
    const DialogTextLocType& headerLoc,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutPatchNotes;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = header;
    args.mHeaderText = DialogText(header, headerLoc);
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xAE0120.
void DialogMgrCom::InitForSetlistChoicePopup(
    DialogArgs& args,
    const DialogType& type,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& yesText,
    const Symbol& noText,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutSetlistChoice;
    args.mType = type;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mYesButtonTextToken = yesText;
    args.mNoButtonTextToken = noText;
    args.mInitialButtonFocus = 1;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xAE02B0.
void DialogMgrCom::InitForTwoOptionPopup(
    DialogArgs& args,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& optionAText,
    const Symbol& optionBText,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutCustomText;
    args.mType = kDialogTypeTwoOptions;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mOptionAButtonTextToken = optionAText;
    args.mOptionBButtonTextToken = optionBText;
    args.mInitialButtonFocus = 0;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xAE0440.
void DialogMgrCom::InitForThreeOptionPopup(
    DialogArgs& args,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& optionAText,
    const Symbol& optionBText,
    const Symbol& optionCText,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutCustomText;
    args.mType = kDialogTypeThreeOptions;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mOptionAButtonTextToken = optionAText;
    args.mOptionBButtonTextToken = optionBText;
    args.mOptionCButtonTextToken = optionCText;
    args.mInitialButtonFocus = 0;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xAE05E0.
void DialogMgrCom::InitForFourOptionPopup(
    DialogArgs& args,
    const DialogModalType& modal,
    const Symbol& body,
    const DialogTextLocType& bodyLoc,
    const Symbol& optionAText,
    const Symbol& optionBText,
    const Symbol& optionCText,
    const Symbol& optionDText,
    std::function<void(Button, BandUser*)> onHide,
    std::function<bool()> condition) {
    args.Clear();
    args.mLayoutType = kDialogLayoutCustomText;
    args.mType = kDialogTypeFourOptions;
    args.mModal = modal == kDialogModal;
    args.mName = body;
    args.mBody1Text = DialogText(body, bodyLoc);
    args.mOptionAButtonTextToken = optionAText;
    args.mOptionBButtonTextToken = optionBText;
    args.mOptionCButtonTextToken = optionCText;
    args.mOptionDButtonTextToken = optionDText;
    args.mInitialButtonFocus = 0;
    args.mOnHide = onHide;
    args.mCondition = condition;
    args.mPending = true;
}

// Reconstructed from eboot.elf at 0xAE0780. Component's handler is not
// asked.
DataNode DialogMgrCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == UISelectMsg::Event()) {
        DataNode result = OnMsg(UISelectMsg(msg));
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
    return DataNode();
}

// Reconstructed from eboot.elf at 0xAE0950.
Symbol DialogMgrCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xAE0960.
Symbol DialogMgrCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xAE0970.
int DialogMgrCom::CurrentRev() const {
    return const_cast<DialogMgrCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xAE0990.
bool DialogMgrCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xAE09C0.
Component* DialogMgrCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xAE09D0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* DialogMgrCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<DialogMgrCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) DialogMgrCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xAE0AE0.
PropRegistry& DialogMgrCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xAE0AF0.
ComMetaData& DialogMgrCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBA0B80. The binary emits the factory
// with the other rb_meta components' factories.
Component* DialogMgrCom::_Create() {
    return new DialogMgrCom();
}

// Reconstructed from eboot.elf at 0xB64000. The binary emits the
// registration with the other rb_meta components' registrations.
void DialogMgrCom::Init() {
    InitComponentClass<DialogMgrCom>();
}
