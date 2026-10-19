#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Joypad.h"
#include "rb_game/core/Defines.h"
#include "utl/containers/Vector.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class DataArray;
class Entity;
class GameObject;
class User;

// The meta game's UI state (rb_meta/RBMetaStateCom.o, 0xBB0690-0xBC5810):
// the layout IDs and their tables, RBMetaState, which goes to layouts by
// ID and keeps the meta game's global state, and RBMetaStateCom, the
// component that exposes that state to the UI as read-only properties.

// The game's UI layouts. The map names the type; the enumerator names are
// the strings RBUILayoutToSym interns.
enum RBUILayout {
    kLayoutInvalid = -1,
#define RB_UI_LAYOUT(name, path) name,
#include "rb_meta/state/RBUILayout.inc"
#undef RB_UI_LAYOUT
};

static_assert(sizeof(RBUILayout) == 4);

constexpr int kNumRBUILayouts = 108;  // Name not in the reference map.

// The body part the character screens focus on. The map names the type
// (CharacterBodyPartToSym(CharacterBodyPartToFocus)); the enumerator names
// are the strings CharacterBodyPartToSym interns.
enum CharacterBodyPartToFocus {
    kCharacterBodyPartNone = -1,
    kCharacterBodyPartHead = 0,
    kCharacterBodyPartTorso = 1,
    kCharacterBodyPartLegs = 2,
    kCharacterBodyPartGuitar = 3,
    kCharacterBodyPartDrums = 4,
};

// The background music a layout wants, from the LAYOUT_MUSIC_STATES macro
// ("BG Music State"). Names not in the reference map; the enumerators
// follow the allowed values' labels.
enum BGMusicState {
    kBGMusicStateInvalid = -1,
    kBGMusicStateNone = 0,
    kBGMusicStateTitleScreen = 1,
    kBGMusicStateShell = 2,
    kBGMusicStateBandHub = 3,
    kBGMusicStateWeeklyChallenge = 4,
};

// The ambient sound a layout wants, from LAYOUT_AMBIENT_SOUND_STATES ("BG
// Ambient Sound State"). Names not in the reference map.
enum BGAmbientSoundState {
    kBGAmbientSoundStateInvalid = -1,
    kBGAmbientSoundStateNone = 0,
    kBGAmbientSoundStateCrowdOnly = 1,
    kBGAmbientSoundStateCrowdAndBand = 2,
    kBGAmbientSoundStateWorld = 3,
};

// Whether a layout ducks the background music, from
// LAYOUTS_WITH_DUCKED_MUSIC ("BG Duck Music State"). Names not in the
// reference map.
enum BGDuckMusicState {
    kBGDuckMusicStateInvalid = -1,
    kBGDuckMusicStateFullVolume = 0,
    kBGDuckMusicStateDucked = 1,
};

// What gameplay is doing while the game layout shows. Newer than the map;
// the type name is inferred and the enumerator names are the strings
// MetaGameplayStateToSym interns.
enum MetaGameplayState {
    kMetaGameplayStateInvalid = -1,
    kMetaGameplayStateNotInGameplay = 0,
    kMetaGameplayStateIntro = 1,
    kMetaGameplayStatePreSong = 2,
    kMetaGameplayStateInSong = 3,
    kMetaGameplayStateBetweenSongs = 4,
    kMetaGameplayStateWinning = 5,
    kMetaGameplayStateLosing = 6,
    kMetaGameplayStateOnResultsScreen = 7,
    kMetaGameplayStateWatchingCutscene = 8,
    kMetaGameplayStateMenu = 9,
    kMetaGameplayStateVoting = 10,
    kMetaGameplayStateExiting = 11,
};

// The number of joypad actions a helpbar button override can name. Name
// not in the reference map.
constexpr int kNumHelpbarButtonOverrides = 15;

// The layout's name, interned into a guarded table on the first call;
// "kLayoutInvalid" for kLayoutInvalid.
Symbol RBUILayoutToSym(RBUILayout layout);  // 0xBB06A0
// The body part's name, interned on the first call;
// "kCharacterBodyPartNone" for kCharacterBodyPartNone.
Symbol CharacterBodyPartToSym(CharacterBodyPartToFocus part);  // 0xBB2210
// The state's name, interned on the first call. Newer than the map; name
// not in the reference map.
Symbol MetaGameplayStateToSym(MetaGameplayState state);  // 0xBC4200
// Orders allowed layout values by label, the deprecated ones last.
bool LayoutSort(const AllowedValue<int>& a, const AllowedValue<int>& b);  // 0xBB50F0
// Registers the kLayout* defines for the UI layout IDs with the data
// system, once.
void InitRBLayoutDefines();  // 0xBC4760

// Whether the layout is listed in the data macro. Newer than the map;
// names not in the reference map.
bool IsRockShopLayout(RBUILayout layout);          // 0xBB0F40, ROCK_SHOP_LAYOUTS
bool IsRivalsLayout(RBUILayout layout);            // 0xBB1160, RIVALS_LAYOUTS
bool IsPotentialSompLayout(RBUILayout layout);     // 0xBB1380, POTENTIAL_SOMP_LAYOUTS
bool UsesMetaBackground(RBUILayout layout);        // 0xBB15A0, META_BACKGROUND_LAYOUTS
bool IsLayoutWithDuckedMusic(RBUILayout layout);   // 0xBB1AE0, LAYOUTS_WITH_DUCKED_MUSIC
// The layout's state in LAYOUT_MUSIC_STATES and LAYOUT_AMBIENT_SOUND_STATES
// (state (layouts...)) entries, or 0 when no entry lists it. Names not in
// the reference map.
BGMusicState GetLayoutMusicState(RBUILayout layout);                 // 0xBB17C0
BGAmbientSoundState GetLayoutAmbientSoundState(RBUILayout layout);  // 0xBB1D00

// The layout the object's UI layout entity shows: the RBUILayoutCom of the
// nearest root object, walking out through the parent objects of instanced
// entities. With `requireEntered`, a component whose entity is not entered
// in the game mode does not count (and with `fallBack` set yields the
// current layout). Without a component, the layout being gone to, else the
// current layout when `fallBack` is set, else kLayoutInvalid. Names not in
// the reference map.
RBUILayout GetObjectUILayout(const GameObject* object, bool requireEntered, bool fallBack);  // 0xBB2020
RBUILayout GetEntityUILayout(const Entity* entity, bool requireEntered, bool fallBack);      // 0xBB2110

// The meta game's UI state (theRBMetaState): the current layout and the
// layout being gone to, the character-screen focus, the helpbar overrides,
// the joined slots and the meta gameplay state. RBMetaStateCom's
// properties read and write it, and the layout components report to it.
// The vtable at 0x1970888 keeps MsgSource's slots but for Handle. The map's
// object is 304 bytes; this build's is 392. Field names are not in the
// reference map.
class RBMetaState : public MsgSource {
public:
    // A helpbar override for one joypad action: the two symbols of the
    // "helpbar_button_override_msg". The evidence for the field names is
    // weak. Name not in the reference map.
    struct ButtonOverride {
        Symbol mLabel;
        Symbol mIcon;
    };

    // Inlined into the static initializer (0xBC5570).
    RBMetaState();
    // The complete destructor is MsgSource's (0xBB50E0); the deleting
    // destructor is at 0xBC4C30.
    ~RBMetaState() override {}

    // Slot 2: "set_calibration_user" (user) and "goto" (layout). Other
    // messages are unhandled; MsgSource's subscriptions are not offered.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xBC48F0

    // Publishes the state as the "rb_meta_state" script global and picks
    // the timeline of the animations a UI layout starts.
    void Init();  // 0xBB5590
    // Starts the shell music once its scene is loaded, follows the SFX and
    // crowd volume options, plays a pending back sound, and counts the
    // joined slots and the connected microphones.
    void Poll();  // 0xBB58C0
    // Goes to the layout through UIMgr::Goto unless it is current, and drops
    // the queued dialogs. The map's Goto(RBUILayout); this build adds the
    // flag, which skips the layout's name report (compiled out but for its
    // RBUILayoutToSym call) that a session of the SOMP mode enables. The
    // startup clears it.
    void Goto(RBUILayout layout, bool quiet);  // 0xBB5D40

    // The helpbar configuration the layouts show in place of their own,
    // exported as "helpbar_override_msg" when it changes.
    void SetHelpbarOverride(Symbol config);  // 0xBB70A0
    void ClearHelpbarOverride();             // 0xBB7190
    Symbol GetHelpbarOverride() const {
        return mHelpbarOverride;
    }
    // The override of the action's helpbar button, exported as
    // "helpbar_button_override_msg" (action label icon) when it changes.
    // The map's SetHelpbarButtonOverride(JoypadAction, Symbol); this build
    // passes the label and the icon, separately or together.
    void SetHelpbarButtonOverride(JoypadAction action, Symbol label, Symbol icon);  // 0xBB71F0
    void SetHelpbarButtonOverride(JoypadAction action, const ButtonOverride& button);  // 0xBB7280
    // The map's GetHelpbarButtonOverride(JoypadAction) const; this build
    // returns both symbols.
    ButtonOverride GetHelpbarButtonOverride(JoypadAction action) const;  // 0xBB7330
    // Clears the override of every action. The map has the single-action
    // ClearHelpbarButtonOverride(JoypadAction); name not in the reference
    // map.
    void ClearHelpbarButtonOverrides();  // 0xBB7420

    // Whether the slot's player is joined and the prompts reflect it: every
    // joined slot when mFocusSlot is kNumGameSlots, else the focused one.
    bool IsSlotJoined(GameSlot slot) const;  // 0xBB7510
    // Exports "booting_player". Name not in the reference map.
    void ExportBootingPlayer();  // 0xBC3BE0
    // The meta gameplay state, kMetaGameplayStateNotInGameplay outside the
    // game layout. Names not in the reference map.
    MetaGameplayState GetMetaGameplayState() const;   // 0xBC41D0
    void SetMetaGameplayState(MetaGameplayState state);  // 0xBC41E0
    // Whether the meta gameplay state is kMetaGameplayStateWatchingCutscene
    // or kMetaGameplayStateVoting. Name not in the reference map.
    bool IsWatchingCutsceneOrVoting() const;  // 0xBC41F0

    // The business-intelligence reports. Not reconstructed.
    void SendGameOptionsBI() const;  // 0xBB7550

    // Allowed values of RBMetaStateCom's properties. `includeInvalid`
    // starts the list with an invalid entry. The map has
    // GetAllowedLayoutValues(bool), GetAllowedEntityFocusValues() and
    // GetAllowedBodyPartFocusValues(); the rest are newer.
    static eastl::vector<AllowedValue<int>> GetAllowedLayoutValues(bool includeInvalid);  // 0xBB3910
    static eastl::vector<AllowedValue<int>> GetAllowedEntityFocusValues();                // 0xBB3E80
    static eastl::vector<AllowedValue<int>> GetAllowedBodyPartFocusValues();              // 0xBB3EA0
    static eastl::vector<AllowedValue<int>> GetAllowedMusicStateValues(bool includeInvalid);    // 0xBB4070
    static eastl::vector<AllowedValue<int>> GetAllowedAmbientStateValues(bool includeInvalid);  // 0xBB4450
    static eastl::vector<AllowedValue<int>> GetAllowedDuckStateValues(bool includeInvalid);     // 0xBB47A0
    static eastl::vector<AllowedValue<int>> GetAllowedMetaGameplayStateValues(bool includeInvalid);  // 0xBB49C0

    // The patch number of the "patch_versions" config's "current_patch",
    // or 0. Name not in the reference map.
    static int GetCurrentPatch();  // 0xBC3D40
    // The current patch's "version_number" text from "patch_version_text",
    // or the empty symbol. Name not in the reference map.
    static Symbol GetGameVersionText();  // 0xBC3DE0
    // The name of the band being played or edited. Name not in the
    // reference map. Not reconstructed: it reads game managers of the
    // rb_game module that are not modelled.
    static Symbol GetCurrentBandName();  // 0xBC4080
    // The music, ambient sound and ducking the layout wants; a rock shop
    // layout of a character session uses the layout the session started
    // from. Names not in the reference map.
    static BGMusicState GetMusicStateForLayout(RBUILayout layout);  // 0xBC4140
    static BGAmbientSoundState GetAmbientSoundStateForLayout(RBUILayout layout);  // 0xBC4180
    static bool IsMusicDuckedForLayout(RBUILayout layout);  // 0xBC41C0
    // Whether the song library is filtered for a challenge: the challenge
    // setup says so, or a SOMP session filters it. Name not in the
    // reference map.
    static bool IsLibraryFilteredForChallenge();  // 0xBC3CD0
    // The "__no_button_override" symbol. Name not in the reference map.
    static Symbol GetNoButtonOverride();  // 0xBB0690

    // The layout ID's symbol from the allowed layout values; the release
    // build keeps only this lookup of the map's layout reports.
    Symbol _GetLayoutSym(RBUILayout layout);  // 0xBB6FC0
    // Records the layout as current and clears the pending one; the
    // previous layout keeps the last layout other than kLayoutLoading.
    void _SetCurrentLayout(RBUILayout layout);  // 0xBC3D00
    // Copies whether each slot's user is joined. Not reconstructed: it calls
    // a User virtual that is not modelled.
    void _UpdateJoinedSlots();  // 0xBB5C60
    // Switches the "main" and "gpu" heaps to strategy 3 while an expensive
    // layout loads and restores them afterwards. Name not in the reference
    // map.
    void _SetLoadingHeapStrategy(bool loading);  // 0xBB6EC0

    RBUILayout mCurrentLayout;
    // The layout Goto went to; _SetCurrentLayout clears it.
    RBUILayout mPendingLayout;
    // A layout to return to, which its reader resets (0xEE13DF). The
    // evidence for the name is weak.
    RBUILayout mReturnLayout;
    // The last current layout other than kLayoutLoading.
    RBUILayout mPreviousLayout;
    unsigned char mPadding[8];  // Never read or written.
    // "set_calibration_user"'s user.
    User* mCalibrationUser;
    // "is_first_time_calibration".
    bool mFirstTimeCalibration;
    // "current_focus_entity": the instrument whose character the
    // character screens show (0 bass to 3 microphone, 4 none).
    int mFocusEntity;
    // "current_focus_body_part".
    CharacterBodyPartToFocus mFocusBodyPart;
    // "current_focus_gender", left uninitialized.
    int mFocusGender;
    // "helpbar_override_config".
    Symbol mHelpbarOverride;
    ButtonOverride mButtonOverrides[kNumHelpbarButtonOverrides];
    // Set by the back handler (0xCB2E40); Poll plays the back sound when a
    // layout change happened in the same frame and clears both flags.
    bool mBackPressed;
    bool mLayoutChanged;
    // "is_any_slot_joinable".
    bool mAnySlotJoinable;
    // Whether each slot's user is joined, by GameSlot.
    bool mSlotJoined[kNumGameSlots];
    // The slot whose instrument the prompts reflect; kNumGameSlots for
    // every joined slot.
    GameSlot mFocusSlot;
    // Written by the song list (0xB02811, 0xC9A726) before it changes
    // layout. The evidence for the name is weak.
    int mSongListIndex;
    // Written by the practice section select (0xC40381) and cleared when
    // kLayoutPracticeSectionSelect is entered from elsewhere than the game
    // or the speed select. The evidence for the name is weak.
    int mPracticeSection;
    // The SFX and crowd volume options last applied to the VCAs.
    int mSfxVolume;
    int mCrowdVolume;
    // Whether Poll applies the volume options; nothing in this build sets
    // it.
    bool mApplyVolumeOptions;
    // "num_microphones_connected", counted by Poll.
    int mNumMicsConnected;
    // "current_meta_gameplay_state".
    MetaGameplayState mMetaGameplayState;
    // "go_directly_to_edit_band".
    bool mGoDirectlyToEditBand;
    // Set once the shell music bus is attached.
    bool mShellMusicStarted;
    // Whether the loading heap strategy is in effect.
    bool mLoadingHeapStrategy;
    // The "main" and "gpu" heaps, found on first use, and their strategies
    // before the loading strategy.
    long mMainHeap;
    long mGpuHeap;
    int mMainHeapStrategy;
    int mGpuHeapStrategy;
};

static_assert(sizeof(RBMetaState::ButtonOverride) == 16);
static_assert(offsetof(RBMetaState, mCurrentLayout) == 24);
static_assert(offsetof(RBMetaState, mPreviousLayout) == 36);
static_assert(offsetof(RBMetaState, mCalibrationUser) == 48);
static_assert(offsetof(RBMetaState, mFirstTimeCalibration) == 56);
static_assert(offsetof(RBMetaState, mFocusEntity) == 60);
static_assert(offsetof(RBMetaState, mFocusGender) == 68);
static_assert(offsetof(RBMetaState, mHelpbarOverride) == 72);
static_assert(offsetof(RBMetaState, mButtonOverrides) == 80);
static_assert(offsetof(RBMetaState, mBackPressed) == 320);
static_assert(offsetof(RBMetaState, mSlotJoined) == 323);
static_assert(offsetof(RBMetaState, mFocusSlot) == 328);
static_assert(offsetof(RBMetaState, mSfxVolume) == 340);
static_assert(offsetof(RBMetaState, mApplyVolumeOptions) == 348);
static_assert(offsetof(RBMetaState, mNumMicsConnected) == 352);
static_assert(offsetof(RBMetaState, mGoDirectlyToEditBand) == 360);
static_assert(offsetof(RBMetaState, mLoadingHeapStrategy) == 362);
static_assert(offsetof(RBMetaState, mMainHeap) == 368);
static_assert(offsetof(RBMetaState, mGpuHeapStrategy) == 388);
static_assert(sizeof(RBMetaState) == 392);

extern RBMetaState theRBMetaState;  // 0x1AFF588

// "helpbar_override_msg", which RBMetaState exports when the helpbar
// override changes. The complete destructor jumps to Message<0>'s
// (0xBB7180); the vtable is at 0x19708D0. Name not in the reference map.
class HelpbarOverrideMsg : public Message<0> {
public:
    HelpbarOverrideMsg() : Message<0>(Event()) {}
    ~HelpbarOverrideMsg() override {}  // deleting destructor 0xBC50B0

    // The message type, built on first use (0x1B002C8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("helpbar_override_msg");
        }
        return t;
    }
};

// "helpbar_button_override_msg" (action label icon). The vtable is at
// 0x19708F0. Name not in the reference map.
class HelpbarButtonOverrideMsg : public Message<3> {
public:
    HelpbarButtonOverrideMsg(JoypadAction action, Symbol label, Symbol icon);  // 0xBC50D0
    // The complete destructor jumps to Message<3>'s (0xBB7320); the
    // deleting destructor is at 0xBC53B0.
    ~HelpbarButtonOverrideMsg() override {}

    // The message type, built on first use (0x1B002D8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("helpbar_button_override_msg");
        }
        return t;
    }
};

// "booting_player". The complete destructor jumps to Message<0>'s
// (0xBC3CC0); the deleting destructor is at 0xBC53D0 and the vtable at
// 0x1970910. Name not in the reference map.
class BootingPlayerMsg : public Message<0> {
public:
    BootingPlayerMsg() : Message<0>(Event()) {}
    ~BootingPlayerMsg() override {}

    // The message type, built on first use (0x1B002E8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("booting_player");
        }
        return t;
    }
};

// The component that shows the meta state to the UI: "Read-only state for
// rb meta globals." It has no fields; every property reads or writes
// theRBMetaState through its accessors. The vtable at 0x1970730 has 41
// slots.
class RBMetaStateCom : public Component {
public:
    RBMetaStateCom();  // 0xBB5080
    // Slots 0-1: the complete destructor is Component's (0xBB50B0); the
    // deleting destructor is at 0xBB50C0.
    ~RBMetaStateCom() override {}

    Symbol GetId() const override;          // slot 4: 0xBC4A70
    Symbol GetClassName() const override;   // slot 5: 0xBC4A80
    int CurrentRev() const override;        // slot 7: 0xBC4A90
    bool IsA(Symbol type) const override;   // slot 8: 0xBC4AB0
    Component* AsComponent() override;      // slot 9: 0xBC4AE0
    char* _Imprint(char* buffer, Component** imprint) override;  // slot 10: 0xBC4AF0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xBC4C10
    ComMetaData& _GetMetaData() override;       // slot 23: 0xBC4C20

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it at 0xB840D0.
    static void Init();
    // The class factory and its placement version, emitted at 0xBAA980 and
    // 0xBAA9B0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xBB2300

    // "RBMetaStateCom", constructed by the static initializer at 0xBC5570.
    static Symbol sId;  // 0x1AFEF48
    // Also "RBMetaStateCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1AFEF50
    static PropRegistry sPropRegistry;  // 0x1AFEF60
    static ComMetaData sMetaData;       // 0x1AFF000
};

static_assert(sizeof(RBMetaStateCom) == 24);
