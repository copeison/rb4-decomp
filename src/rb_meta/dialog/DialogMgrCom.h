#pragma once

#include <cfloat>
#include <cstddef>
#include <functional>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class BandUser;
class DataArray;
class UIButtonDownMsg;
class UISelectMsg;

// The component that throws a dialog up on top of the current screen
// ("Throws a dialog up on top of any current screen."; rb_meta/
// DialogMgrCom.o, 0xADAE60-0xAE2620 with the static initializer at
// 0xAE23E0). The game fills a DialogArgs through one of the InitFor*
// functions and passes it to ShowDialog, which copies it to the current
// dialog and adds the dialog scene (kDialogScenePath) to the UI; the
// scene's DialogMgrCom becomes the active one when it enters and shows the
// current dialog's texts and icons through its properties. A pressed
// button hides the dialog and calls the dialog's callback with it.
// Dialogs requested while one shows or the UI is in a transition wait in a
// queue under a lock until Poll shows them. The vtable at 0x196E628 has 41
// slots. The object is 464 bytes.
class DialogMgrCom : public Component {
public:
    // How the dialog is laid out ("dialog_layout_type"). Newer than the
    // map; the type and enumerator names are inferred from the allowed
    // values' labels.
    enum DialogLayoutType : int {
        // "Standard Text Popup".
        kDialogLayoutStandardText = 0,
        // "Onboarding - Text Interstitial".
        kDialogLayoutOnboardingText = 1,
        // "Onboarding - Single Icon".
        kDialogLayoutOnboardingOneIcon = 2,
        // "Onboarding - Two Icon".
        kDialogLayoutOnboardingTwoIcon = 3,
        // "Unlock - Text only".
        kDialogLayoutUnlockText = 4,
        // "Vocal Improv": the animated Vocal Improv onboarding screen.
        kDialogLayoutVocalImprov = 5,
        // "Unlock - Banner", for the Positive Grid foot pedal unlocks.
        kDialogLayoutUnlockBanner = 6,
        // "Patch Notes".
        kDialogLayoutPatchNotes = 7,
        // "Setlist Choice".
        kDialogLayoutSetlistChoice = 8,
        // "Custom Text Popup": the standard popup with Yes/No text override.
        kDialogLayoutCustomText = 9,
        // "Onboarding - Text Directed": tied to a UI element, at the
        // position overrides.
        kDialogLayoutOnboardingTextDirected = 10,
    };

    // What the dialog waits for ("dialog_type"). The map names the type;
    // the enumerator names are inferred from the allowed values' labels.
    enum DialogType : int {
        kDialogTypeOk = 0,
        kDialogTypeYesNo = 1,
        // Waits for the condition function to return true.
        kDialogTypeConditional = 2,
        kDialogTypeYesNoCancel = 3,
        kDialogTypeFourOptions = 4,
        // Waits for the condition, or for Red/B (the cancel action).
        kDialogTypeConditionalWithCancel = 5,
        kDialogTypeTwoOptions = 6,
        kDialogTypeThreeOptions = 7,
    };

    // Whether the dialog's scene takes the input modally. The map names the
    // type; the enumerator names are inferred, as is which value means
    // modal (the InitFor* functions set DialogArgs::mModal for 1).
    enum DialogModalType : int {
        kDialogNonModal = 0,
        kDialogModal = 1,
    };

    // Whether a text the InitFor* functions take is a localization token
    // or text to show as it is. The map names the type; the enumerator
    // names are inferred and the evidence is weak.
    enum DialogTextLocType : int {
        // Localized through Localize.
        kDialogTextToken = 0,
        kDialogTextLiteral = 1,
    };

    // The answer a dialog was hidden with, which its callback receives. The
    // map names the type; the enumerator names are inferred from the button
    // arrays OnMsg(UISelectMsg const&) checks.
    enum Button : int {
        kButtonOk = 0,
        kButtonYes = 1,
        kButtonNo = 2,
        kButtonCancel = 3,
        kButtonOptionA = 4,
        kButtonOptionB = 5,
        kButtonOptionC = 6,
        kButtonOptionD = 7,
        // The condition of a conditional dialog was met
        // (_UpdateConditionDialog).
        kButtonConditionMet = 8,
    };

    // The users whose answer counts (DialogArgs::mRequiredInstrument): a
    // user who plays another track can answer only when no band user plays
    // that one. Newer than the map; names not in the reference map.
    enum RequiredInstrument : int {
        kRequireAnyInstrument = 0,
        kRequireGuitar = 1,
        kRequireBass = 2,
        kRequireVocals = 3,
        kRequireDrums = 4,
    };

    // A dialog to show: its layout, its texts and icons, and its callbacks.
    // The constructor, the destructor and Clear are inline; the map emits
    // them in other objects (for example rb_audio/RBSong.o), and this build
    // keeps Clear at 0x8F82D0. 256 bytes. Field names are not in the
    // reference map; most follow DialogMgrCom's properties.
    struct DialogArgs {
        DialogArgs() {
            Clear();
        }

        // Resets every field: no texts, no icons, no callbacks, a modal
        // standard OK dialog, and the position overrides at FLT_MIN (none).
        void Clear() {
            mPending = false;
            mLayoutType = kDialogLayoutStandardText;
            mType = kDialogTypeOk;
            mName = Symbol("");
            mHeaderText = Symbol("");
            mSubHeaderText = Symbol("");
            mBody1Text = Symbol("");
            mBody2Text = Symbol("");
            mOkButtonTextToken = Symbol("");
            mYesButtonTextToken = Symbol("");
            mNoButtonTextToken = Symbol("");
            mOptionAButtonTextToken = Symbol("");
            mOptionBButtonTextToken = Symbol("");
            mOptionCButtonTextToken = Symbol("");
            mOptionDButtonTextToken = Symbol("");
            mIcon1Image = "";
            mIcon2Image = "";
            mInitialButtonFocus = 0;
            mXPosOverride = FLT_MIN;
            mZPosOverride = FLT_MIN;
            mModal = true;
            mRequiredInstrument = kRequireAnyInstrument;
            mOnHide = nullptr;
            mCondition = nullptr;
        }

        // "dialog_layout_type".
        DialogLayoutType mLayoutType;
        // "dialog_type".
        DialogType mType;
        // The dialog's id, IsDialogActive(Symbol) compares: the token of its
        // main text.
        Symbol mName;
        // "header_text", "sub_header_text", "body1_text" and "body2_text".
        Symbol mHeaderText;
        Symbol mSubHeaderText;
        Symbol mBody1Text;
        Symbol mBody2Text;
        // The button label tokens ("ok_button_text_token" and so on).
        Symbol mOkButtonTextToken;
        Symbol mYesButtonTextToken;
        Symbol mNoButtonTextToken;
        Symbol mOptionAButtonTextToken;
        Symbol mOptionBButtonTextToken;
        Symbol mOptionCButtonTextToken;
        Symbol mOptionDButtonTextToken;
        // "icon1_image" and "icon2_image", RndTexture2DResources.
        ResourcePath mIcon1Image;
        ResourcePath mIcon2Image;
        // "initial_button_focus": 0 lets the input component choose, 1
        // focuses Yes, 2 No and 3 Cancel.
        int mInitialButtonFocus;
        // "x_pos_override" and "z_pos_override", for the directed layout.
        float mXPosOverride;
        float mZPosOverride;
        // Whether the dialog's scene takes the input modally
        // (UIInputCom::SetIsModal).
        bool mModal;
        RequiredInstrument mRequiredInstrument;
        // Called with the answer and its user once the dialog hides.
        std::function<void(Button, BandUser*)> mOnHide;
        // A conditional dialog hides once it returns true.
        std::function<bool()> mCondition;
        // Set by the InitFor* functions and cleared by Clear: the current
        // dialog is shown or its scene is being added. The name is weak.
        bool mPending;
    };

    DialogMgrCom();  // 0xADAF90
    // Copies the button arrays for an imprint; the texts, the icons and the
    // position overrides start empty. Inlined into _Imprint (0xAE09D0); the
    // binary's copy is at 0xAE0D30.
    DialogMgrCom(const DialogMgrCom& other)
        : Component(other),
          mIcon1Image(),
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
          mOkButtons(other.mOkButtons),
          mYesButtons(other.mYesButtons),
          mNoButtons(other.mNoButtons),
          mCancelButtons(other.mCancelButtons),
          mOptionAButtons(other.mOptionAButtons),
          mOptionBButtons(other.mOptionBButtons),
          mOptionCButtons(other.mOptionCButtons),
          mOptionDButtons(other.mOptionDButtons) {}
    // Slots 0-1: 0xADB150, 0xADB420.
    ~DialogMgrCom() override;

    // Slot 2: "ui_select" and "ui_button_down" go to the OnMsg handlers;
    // other messages are unhandled. The map's Handle(DataArray*, bool).
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xAE0780
    Symbol GetId() const override;         // slot 4: 0xAE0950
    Symbol GetClassName() const override;  // slot 5: 0xAE0960
    int CurrentRev() const override;       // slot 7: 0xAE0970
    bool IsA(Symbol type) const override;  // slot 8: 0xAE0990
    Component* AsComponent() override;     // slot 9: 0xAE09C0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xAE09D0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xAE0AE0
    ComMetaData& _GetMetaData() override;       // slot 23: 0xAE0AF0
    // Slot 31: becomes the active dialog, takes the current dialog's texts
    // and icons, makes the scene's input modal and puts the dialog's
    // helpbar in place of the layout's. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xADE350
    // Slot 33: hides a conditional dialog whose condition is met. The map's
    // _Poll(ObjPtr const&).
    void _Poll() override;  // 0xADE660

    // Hides the dialog with the answer of the button the user selected,
    // when the user's input goes to the shell. Always handled.
    DataNode OnMsg(const UISelectMsg& msg);  // 0xADE6D0
    // Hides a dialog with a cancel answer on the cancel action, or the
    // directed onboarding dialog on the confirm action. Newer than the map.
    DataNode OnMsg(const UIButtonDownMsg& msg);  // 0xADEAF0

    // Whether the object is one of the button array's. The map has
    // _IsOkButton, _IsYesButton and _IsNoButton out of line; this build
    // inlines the checks of the eight arrays into OnMsg(UISelectMsg
    // const&). The other five names are not in the reference map.
    bool _IsOkButton(const GameObjectId& id) const {
        return _IsButton(mOkButtons, id);
    }
    bool _IsYesButton(const GameObjectId& id) const {
        return _IsButton(mYesButtons, id);
    }
    bool _IsNoButton(const GameObjectId& id) const {
        return _IsButton(mNoButtons, id);
    }
    bool _IsCancelButton(const GameObjectId& id) const {
        return _IsButton(mCancelButtons, id);
    }
    bool _IsOptionAButton(const GameObjectId& id) const {
        return _IsButton(mOptionAButtons, id);
    }
    bool _IsOptionBButton(const GameObjectId& id) const {
        return _IsButton(mOptionBButtons, id);
    }
    bool _IsOptionCButton(const GameObjectId& id) const {
        return _IsButton(mOptionCButtons, id);
    }
    bool _IsOptionDButton(const GameObjectId& id) const {
        return _IsButton(mOptionDButtons, id);
    }
    // Name not in the reference map.
    static bool _IsButton(const PropArray<GameObjectId>& buttons, const GameObjectId& id) {
        for (unsigned long index = 0; index < buttons.size(); ++index) {
            if (buttons[index].mId == id.mId) {
                return true;
            }
        }
        return false;
    }

    // Shows the dialog when it is safe (_SafeToShowDialog); otherwise
    // queues a copy. A conditional dialog without a condition is dropped.
    static void ShowDialog(const DialogArgs& args);  // 0xADD560
    // Shows the first queued dialog once it is safe, and drops it from the
    // queue. Called once per frame by App::RunOneFrame.
    static void Poll();  // 0xADE120
    // Drops the current dialog and the queue, removing a dialog scene that
    // has not entered yet. RBMetaState::Goto calls it. Name not in the
    // reference map.
    static void ClearAllDialogs();  // 0xADDB80
    // Hides the active dialog with its cancel answer: No for a Yes/No
    // dialog, Cancel for the dialogs with a cancel or options, OK
    // otherwise. False when no dialog is active. Name not in the reference
    // map.
    static bool CancelDialog(bool immediate);  // 0xADD8B0
    static bool IsDialogActive();              // 0xADE100
    // Whether the current dialog has the id, active or not.
    static bool IsDialogActive(Symbol name);  // 0xADE110

    // Fill the arguments for a layout. Each clears them first, sets the
    // layout, the type and the modality, takes the texts (localized unless
    // their DialogTextLocType says otherwise; the dialog's id is the token
    // of the main text), and sets the callbacks. Overloads newer than the
    // map add the initial focus, the required instrument or the OK label;
    // the names of the layouts the map lacks are inferred.
    static void InitForStandardTextPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const int& initialFocus,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADEBA0
    // With the OK button's label token.
    static void InitForStandardTextPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const int& initialFocus,
        const Symbol& okText,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADED20
    // The standard popup with Yes/No labels. Name not in the reference map.
    static void InitForCustomTextPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const Symbol& yesText,
        const Symbol& noText,
        const int& initialFocus,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADEE50
    // With a header; the body is the dialog's id.
    static void InitForCustomTextPopup(
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
        std::function<bool()> condition);  // 0xADEFE0
    // The body is the dialog's id.
    static void InitForTextOnlyOnboardingPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& header,
        const DialogTextLocType& headerLoc,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADF1C0
    // With the instrument whose player must answer.
    static void InitForTextOnlyOnboardingPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& header,
        const DialogTextLocType& headerLoc,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const RequiredInstrument& instrument,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADF370
    static void InitForOneIconOnboardingPopup(
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
        std::function<bool()> condition);  // 0xADF530
    static void InitForTwoIconOnboardingPopup(
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
        std::function<bool()> condition);  // 0xADF730
    // At the position overrides; the body is the dialog's id. Name not in
    // the reference map.
    static void InitForTextDirectedOnboardingPopup(
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
        std::function<bool()> condition);  // 0xADF970
    static void InitForBannerUnlockPopup(
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
        std::function<bool()> condition);  // 0xADFB40
    static void InitForTextOnlyUnlockPopup(
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
        std::function<bool()> condition);  // 0xADFD80
    // No caller in this build. Name not in the reference map.
    static void InitForPatchNotesPopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& header,
        const DialogTextLocType& headerLoc,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xADFF70
    // Focuses Yes. Name not in the reference map.
    static void InitForSetlistChoicePopup(
        DialogArgs& args,
        const DialogType& type,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const Symbol& yesText,
        const Symbol& noText,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xAE0120
    // Custom text popups that wait for one of two, three or four options,
    // with their labels. The first two have no caller in this build. Names
    // not in the reference map.
    static void InitForTwoOptionPopup(
        DialogArgs& args,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const Symbol& optionAText,
        const Symbol& optionBText,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xAE02B0
    static void InitForThreeOptionPopup(
        DialogArgs& args,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const Symbol& optionAText,
        const Symbol& optionBText,
        const Symbol& optionCText,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xAE0440
    static void InitForFourOptionPopup(
        DialogArgs& args,
        const DialogModalType& modal,
        const Symbol& body,
        const DialogTextLocType& bodyLoc,
        const Symbol& optionAText,
        const Symbol& optionBText,
        const Symbol& optionCText,
        const Symbol& optionDText,
        std::function<void(Button, BandUser*)> onHide,
        std::function<bool()> condition);  // 0xAE05E0

    // Removes the dialog scene, clears the current dialog and calls its
    // callback with the answer. An answer from a user who neither has the
    // focused slot nor plays the required instrument is ignored while a
    // band user plays it. The map's _HideDialog(Button, BandUser*); this
    // build passes the scene removal's kSceneQueueNow flag.
    static void _HideDialog(Button button, BandUser* user, bool immediate);  // 0xADD8F0
    // Whether no dialog is active or pending and the UI is not in a
    // transition. Out of line in the map; inlined into ShowDialog and Poll.
    // Defined in DialogMgrCom.cpp, which owns the state.
    static bool _SafeToShowDialog();
    // Hides the current dialog with kButtonConditionMet once its condition
    // returns true. Inlined into _Poll.
    static void _UpdateConditionDialog();  // 0xADE6A0
    // Whether the user plays the current dialog's required instrument.
    // Inlined into _HideDialog. Name not in the reference map.
    static bool _IsUserRequiredInstrument(BandUser* user);  // 0xADE0A0
    // The "dialog_type" allowed values. Name not in the reference map.
    static eastl::vector<AllowedValue<int>> GetDialogTypeAllowedValues();  // 0xADD090

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it at 0xB64000.
    static void Init();
    // The class factory and its placement version, emitted at 0xBA0B80 and
    // 0xBA0BB0.
    static Component* _Create();
    // Describes the class and registers its properties. The texts, the
    // types and the focus read and write the current dialog (or the
    // component's copies of them); none is saved.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xADB440

    // "DialogMgrCom", constructed by the static initializer at 0xAE23E0.
    static Symbol sId;  // 0x1AF49F8
    // Also "DialogMgrCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AF4A00
    static PropRegistry sPropRegistry;  // 0x1AF4A10
    static ComMetaData sMetaData;       // 0x1AF4AB0
    // "ui/dialog/dialog.scene".
    static const char* kDialogScenePath;  // 0x19BE248

    // The current dialog's icons and texts, copied by _Enter; the
    // properties of the same names read and write them. Field names are
    // not in the reference map.
    ResourcePath mIcon1Image;
    ResourcePath mIcon2Image;
    Symbol mHeaderText;
    Symbol mSubHeaderText;
    Symbol mBody1Text;
    Symbol mBody2Text;
    Symbol mOkButtonTextToken;
    Symbol mYesButtonTextToken;
    Symbol mNoButtonTextToken;
    Symbol mOptionAButtonTextToken;
    Symbol mOptionBButtonTextToken;
    Symbol mOptionCButtonTextToken;
    Symbol mOptionDButtonTextToken;
    // The helpbar override _Enter replaced, put back by the destructor
    // unless it was a dialog's own.
    Symbol mPreviousHelpbarOverride;
    // "x_pos_override" and "z_pos_override", copied by _Enter. The
    // constructors leave them uninitialized.
    float mXPosOverride;
    float mZPosOverride;
    // The objects whose selection gives each answer ("ok_buttons",
    // "yes_buttons", "no_buttons", "cancel_buttons" and
    // "option_a_buttons" to "option_d_buttons").
    PropArray<GameObjectId> mOkButtons;
    PropArray<GameObjectId> mYesButtons;
    PropArray<GameObjectId> mNoButtons;
    PropArray<GameObjectId> mCancelButtons;
    PropArray<GameObjectId> mOptionAButtons;
    PropArray<GameObjectId> mOptionBButtons;
    PropArray<GameObjectId> mOptionCButtons;
    PropArray<GameObjectId> mOptionDButtons;
};

static_assert(offsetof(DialogMgrCom::DialogArgs, mName) == 8);
static_assert(offsetof(DialogMgrCom::DialogArgs, mHeaderText) == 16);
static_assert(offsetof(DialogMgrCom::DialogArgs, mOkButtonTextToken) == 48);
static_assert(offsetof(DialogMgrCom::DialogArgs, mOptionDButtonTextToken) == 96);
static_assert(offsetof(DialogMgrCom::DialogArgs, mIcon1Image) == 104);
static_assert(offsetof(DialogMgrCom::DialogArgs, mInitialButtonFocus) == 120);
static_assert(offsetof(DialogMgrCom::DialogArgs, mXPosOverride) == 124);
static_assert(offsetof(DialogMgrCom::DialogArgs, mModal) == 132);
static_assert(offsetof(DialogMgrCom::DialogArgs, mRequiredInstrument) == 136);
static_assert(offsetof(DialogMgrCom::DialogArgs, mOnHide) == 144);
static_assert(offsetof(DialogMgrCom::DialogArgs, mCondition) == 192);
static_assert(offsetof(DialogMgrCom::DialogArgs, mPending) == 240);
static_assert(sizeof(DialogMgrCom::DialogArgs) == 256);
static_assert(offsetof(DialogMgrCom, mIcon1Image) == 24);
static_assert(offsetof(DialogMgrCom, mHeaderText) == 40);
static_assert(offsetof(DialogMgrCom, mOkButtonTextToken) == 72);
static_assert(offsetof(DialogMgrCom, mOptionDButtonTextToken) == 120);
static_assert(offsetof(DialogMgrCom, mPreviousHelpbarOverride) == 128);
static_assert(offsetof(DialogMgrCom, mXPosOverride) == 136);
static_assert(offsetof(DialogMgrCom, mZPosOverride) == 140);
static_assert(offsetof(DialogMgrCom, mOkButtons) == 144);
static_assert(offsetof(DialogMgrCom, mCancelButtons) == 264);
static_assert(offsetof(DialogMgrCom, mOptionDButtons) == 424);
static_assert(sizeof(DialogMgrCom) == 464);
