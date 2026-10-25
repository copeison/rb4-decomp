#pragma once

#include "utl/text/Symbol.h"

// The overshell's shared definitions (rb_meta/RBOvershellDefines.o,
// 0xBF7280-0xBF7???). Declared only; the object is not reconstructed.

// Whether the overshell runs, and where ("active_status"). The enumerator
// names are the strings OvershellActiveStatusToSymbol interns.
enum OvershellActiveStatus : int {
    // "Overshell is inactive (loading screens, etc)."
    kOvershellInactive = 0,
    // "Game is in a meta screen."
    kOvershellInMeta = 1,
    // "In gameplay."
    kOvershellInSong = 2,
};

// The flow that takes every slot through one screen. The map names the type
// (RBOvershellCom::BeginOverrideFlow(OvershellOverrideFlow)); the enumerator
// names are not in the reference map and follow the slot states' flags
// (RBOvershellSlotState) that tell whether a slot is in the flow.
enum OvershellOverrideFlow : int {
    kOverrideFlowNone = 0,
    // "song_settings_flow".
    kOverrideFlowSongSettings = 1,
    // "choose_band_members_flow".
    kOverrideFlowChooseBandMembers = 2,
    // "idle_flow".
    kOverrideFlowIdle = 3,
    // "disabled_flow": the overshell is inactive or a gameplay state keeps
    // it hidden.
    kOverrideFlowDisabled = 4,
    // "somp_transition_flow": a SOMP session changes the layout.
    kOverrideFlowSompTransition = 5,
};

// A slot's state (RBOvershellSlotState::GetStateID), indexing
// OvershellSlotStateToString's table of 84 names (0x1971C70). The map names
// the type; the enumerator names are the table's strings. Ids 2, 22 and
// 58-60 have no name ("-UnknownSlotState-").
enum OvershellSlotStateID : int {
    kState_NoInstrument = 0,
    kState_Join = 1,
    kState_ConnectMic = 3,
    kState_JoinedDefault = 4,
    kState_Options = 5,
    kState_InSongMenu = 6,
    kState_ChoosePart = 7,
    kState_ChoosePartWait = 8,
    kState_ChooseDiff = 9,
    kState_SongOptionsCancel = 10,
    kState_ReadyToPlay = 11,
    kState_ReadyToPlayWait = 12,
    kState_ChooseProfile = 13,
    kState_ChooseProfileConfirm = 14,
    kState_OptionsEndGame = 15,
    kState_Modifiers = 16,
    kState_OptionsDrumInSongSettings_DEPRECATED = 17,
    kState_OptionsDrumMessage = 18,
    kState_OptionsDrum = 19,
    kState_OptionsVocal = 20,
    kState_OptionsSlider = 21,
    kState_OptionsGuitar = 23,
    kState_ChooseDiffConfirm = 24,
    kState_ModifierUnlock = 25,
    kState_ReconnectController = 26,
    kState_NoJoinInGame = 27,
    kState_ModifiersDrumWarning = 28,
    kState_ToggleNoFailDenial = 29,
    kState_ChoosePartDenial = 30,
    kState_ModifierDelayedEffect = 31,
    kState_QuitEarlyConfirm = 32,
    kState_SkipSongConfirm = 33,
    kState_ChoosePartWarn = 34,
    kState_RestartSongConfirm = 35,
    kState_ChooseProfileDenial = 36,
    kState_RemoveUserInSongConfirm = 37,
    kState_ChooseCharacter = 38,
    kState_CharacterManagement = 39,
    kState_CharacterDeleteConfirm = 40,
    kState_CreateCharacterDenialNoRoom = 41,
    kState_CharacterError = 42,
    kState_ChooseBandMember = 43,
    kState_ChooseBandMemberReady = 44,
    kState_VenueMetaMenu = 45,
    kState_Idle = 46,
    kState_IdleMaster = 47,
    kState_IdleNotTheMaster = 48,
    kState_IdleNoMaster = 49,
    kState_NoBandError = 50,
    kState_ConfirmSoloToggle = 51,
    kState_Disabled = 52,
    kState_ChooseVocalPartSpotlightWarn = 53,
    kState_ChooseDrumPartSpotlightWarn = 54,
    kState_SharedProfileScoresDontCountWarn = 55,
    kState_RestartShowConfirm = 56,
    kState_ToggleCymbalConfirm = 57,
    kState_MirrorRemoteUser = 61,
    kState_ChooseDifficultyWait = 62,
    kState_ChoosePartServerQueryWait = 63,
    kState_OnlineSettings = 64,
    kState_OnlineKickPlayer = 65,
    kState_OnlinePlayerKickConfirm = 66,
    kState_OnlinePlayerKickResultWait = 67,
    kState_OnlinePlayerKickResultDisplay = 68,
    kState_OnlineError = 69,
    kState_OnlineLeaderReturnToGreenRoomConfirm = 70,
    kState_OnlineLeaderReturnToGreenRoomWait = 71,
    kState_OnlineLeaderReturnToMusicLibraryConfirm = 72,
    kState_OnlineLeaderReturnToMusicLibraryWait = 73,
    kState_OnlineLeaderDropOutConfirm = 74,
    kState_OnlineNonLeaderDropOutConfirm = 75,
    kState_OnlineQuitSessionConfirm = 76,
    kState_OnlineLeaderSkipToNextSongWait = 77,
    kState_OnlineExitSubmenu = 78,
    kState_OnlineScreenTransition = 79,
    kState_OnlinePickEmoteCategory = 80,
    kState_OnlinePickEmote = 81,
    kState_OnlinePickEmoteWait = 82,
    kState_OnlinePickEmoteRateThrottle = 83,
    kOvershellSlotStateCount = 84,
};

// The state's name, "-UnknownSlotState-" out of range.
const char* OvershellSlotStateToString(OvershellSlotStateID state);  // 0xBF7280
// The status's name ("kOvershellInactive", ...), interned on first use.
Symbol OvershellActiveStatusToSymbol(OvershellActiveStatus status);  // 0xBF72A0
// The status the name stands for; kOvershellInMeta for any other name.
OvershellActiveStatus SymbolToOvershellActiveStatus(Symbol status);  // 0xBF7440
