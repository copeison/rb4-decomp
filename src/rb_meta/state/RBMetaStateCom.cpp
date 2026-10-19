// rb_meta/RBMetaStateCom.o (0xBB0690 to 0xBC5810). The object's other
// code is the compiler's: the guarded lookups of the layout macros' symbols,
// the eastl::sort over the allowed layout values (0x156A00, 0x156D80), the
// property accessors after the identity slots (0xBC4C50-0xBC50A0) and the
// std::function of Init's timeline callback (vtable 0x1970940, call operator
// 0xBC5470). The business-intelligence reports (0xBB7550-0xBC3BB0: the map's
// SendGameOptionsBI, SendScoresBI, _GetBIBandAndSongInfo, SendSongAbortBI,
// SendVoteBI and SendPurchaseInfoBI) are not reconstructed. The static
// initializer is at 0xBC5570; the shared header statics it sets first
// (0x1AFEEB0-0x1AFEF1C, among them an "igs_tutorial_event" symbol) belong to
// headers that are not modelled. The class's registration and factory are
// emitted elsewhere (0xB840D0, 0xBAA980).
#include "rb_meta/state/RBMetaStateCom.h"

#include <cstring>
#include <functional>
#include <new>

#include "anim/drivers/AnimDriverCom.h"
#include "audio/core/system/SoundManager.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropInfo.h"
#include "entity/resources/Resource.h"
#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/system/System.h"
#include "persistence/saveload/SaveLoadMgr.h"
#include "rb_game/mics/GameMicManager.h"
#include "rb_meta/character/RBCharacterMenuMgr.h"
#include "rb_meta/clans/RBClanMgr.h"
#include "rb_meta/dialog/DialogMgrCom.h"
#include "rb_meta/onboarding/RBIGSOnboarding.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/somp/RBSompSessionMgr.h"
#include "rb_meta/ui/RBUILayoutCom.h"
#include "rb_world/characters/RBCharOutfitDesc.h"
#include "render/scene/RndSceneResource.h"
#include "ui/manager/UIMgr.h"
#include "utl/containers/Sort.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/data/DataUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The UI layouts whose animations keep the timeline their options give,
// from the "use_authored_anim_timelines" config (Init). Name not in the
// reference map.
eastl::vector<ResourcePath> gAuthoredTimelineLayouts;  // 0x1AFEF20

// "__no_button_override". Name not in the reference map.
Symbol gNoButtonOverride("__no_button_override");  // 0x1AFEF40

// Whether a layout macro's array lists the layout. A symbol in the array
// is skipped; the release build keeps the text of the warning it gave.
// Inlined into the macro lookups. Name not in the reference map.
bool MacroListsLayout(DataArray* layouts, RBUILayout layout) {
    for (int i = 0; i < layouts->Size(); i++) {
        DataNode node;
        node = layouts->Node(i);
        if (node.mType == kDataSymbol) {
            static_cast<void>(node.Str(nullptr));
            continue;
        }
        if (node.Int(nullptr) == layout) {
            return true;
        }
    }
    return false;
}

// The state of the layout in a macro of (state (layouts...)) entries, or 0.
// Inlined into the state lookups. Name not in the reference map.
int MacroLayoutState(DataArray* states, RBUILayout layout) {
    for (int i = 0; i < states->Size(); i++) {
        DataArray* const entry = states->Node(i).Array(states);
        DataArray* const layouts = entry->Node(1).Array(entry);
        for (int j = 0; j < layouts->Size(); j++) {
            DataNode node;
            node = layouts->Node(j);
            if (node.mType == kDataSymbol) {
                static_cast<void>(node.Str(nullptr));
                continue;
            }
            if (node.Int(nullptr) == layout) {
                return entry->Int(0);
            }
        }
    }
    return 0;
}

// An allowed value whose label is the text and whose description is
// empty. Name not in the reference map.
AllowedValue<int> NamedValue(int value, const char* name) {
    return AllowedValue<int>{value, String(name), String("")};
}

// The instruments a character screen can focus on (Bass to Mic, then
// None). Called by GetAllowedEntityFocusValues and _Init. Name not in the
// reference map.
eastl::vector<AllowedValue<int>> GetFocusInstrumentAllowedValues() {  // 0xBB5160
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{0, String("Bass"), String("Bassist")});
    values.push_back(AllowedValue<int>{1, String("Drum"), String("Drummer")});
    values.push_back(AllowedValue<int>{2, String("Guitar"), String("Guitarist")});
    values.push_back(AllowedValue<int>{3, String("Mic"), String("Vocalist")});
    values.push_back(AllowedValue<int>{4, String("None"), String("No Instrument")});
    return values;
}

// The layout of the nearest UI layout entity: the RBUILayoutCom on the
// entity's root object, else on the root of the entity whose object
// instances it, and so on outwards. Inlined into GetObjectUILayout and
// GetEntityUILayout. Name not in the reference map.
RBUILayout FindUILayout(const Entity* entity, bool requireEntered, bool fallBack) {
    for (;;) {
        const RBUILayoutCom* const com = entity->GetRoot()->GetCom<RBUILayoutCom>();
        if (com != nullptr) {
            if (!requireEntered) {
                return com->mLayout;
            }
            // Entered (0x8 or 0x10) in the game mode (1).
            const unsigned int flags = com->mObject->mEntity->mFlags;
            const bool entered = (flags & 0xE) == 0xA || (flags & 0x16) == 0x12;
            if (!entered && fallBack) {
                return theRBMetaState.mCurrentLayout;
            }
            return entered ? com->mLayout : kLayoutInvalid;
        }
        if (entity->mParentObject == nullptr || entity->mParentObject->mEntity == nullptr) {
            break;
        }
        entity = entity->mParentObject->mEntity;
    }
    if (theRBMetaState.mPendingLayout == kLayoutInvalid && fallBack) {
        return theRBMetaState.mCurrentLayout;
    }
    return theRBMetaState.mPendingLayout;
}

}  // namespace

// The object's statics, in the order of its static initializer (0xBC5570).
Symbol RBMetaStateCom::sId("RBMetaStateCom");
Symbol RBMetaStateCom::sClassName("RBMetaStateCom");
PropRegistry RBMetaStateCom::sPropRegistry;
ComMetaData RBMetaStateCom::sMetaData;
RBMetaState theRBMetaState;

// Reconstructed from eboot.elf at 0xBB0690.
Symbol RBMetaState::GetNoButtonOverride() {
    return gNoButtonOverride;
}

// Reconstructed from eboot.elf at 0xBB06A0. The names are interned into a
// guarded function-local table on the first call.
Symbol RBUILayoutToSym(RBUILayout layout) {
    if (layout == kLayoutInvalid) {
        return Symbol("kLayoutInvalid");
    }

    static const Symbol sSymbols[kNumRBUILayouts] = {
#define RB_UI_LAYOUT(name, path) Symbol(#name),
#include "rb_meta/state/RBUILayout.inc"
#undef RB_UI_LAYOUT
    };
    return sSymbols[layout];
}

// Reconstructed from eboot.elf at 0xBB0F40.
bool IsRockShopLayout(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("ROCK_SHOP_LAYOUTS");
    }
    return MacroListsLayout(DataGetMacro(macro), layout);
}

// Reconstructed from eboot.elf at 0xBB1160.
bool IsRivalsLayout(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("RIVALS_LAYOUTS");
    }
    return MacroListsLayout(DataGetMacro(macro), layout);
}

// Reconstructed from eboot.elf at 0xBB1380.
bool IsPotentialSompLayout(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("POTENTIAL_SOMP_LAYOUTS");
    }
    return MacroListsLayout(DataGetMacro(macro), layout);
}

// Reconstructed from eboot.elf at 0xBB15A0.
bool UsesMetaBackground(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("META_BACKGROUND_LAYOUTS");
    }
    return MacroListsLayout(DataGetMacro(macro), layout);
}

// Reconstructed from eboot.elf at 0xBB17C0.
BGMusicState GetLayoutMusicState(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("LAYOUT_MUSIC_STATES");
    }
    return static_cast<BGMusicState>(MacroLayoutState(DataGetMacro(macro), layout));
}

// Reconstructed from eboot.elf at 0xBB1AE0.
bool IsLayoutWithDuckedMusic(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("LAYOUTS_WITH_DUCKED_MUSIC");
    }
    return MacroListsLayout(DataGetMacro(macro), layout);
}

// Reconstructed from eboot.elf at 0xBB1D00.
BGAmbientSoundState GetLayoutAmbientSoundState(RBUILayout layout) {
    static Symbol macro;
    if (macro == Symbol()) {
        macro = Symbol("LAYOUT_AMBIENT_SOUND_STATES");
    }
    return static_cast<BGAmbientSoundState>(MacroLayoutState(DataGetMacro(macro), layout));
}

// Reconstructed from eboot.elf at 0xBB2020.
RBUILayout GetObjectUILayout(const GameObject* object, bool requireEntered, bool fallBack) {
    return FindUILayout(object->mEntity, requireEntered, fallBack);
}

// Reconstructed from eboot.elf at 0xBB2110. The walk starts from the root
// object's entity.
RBUILayout GetEntityUILayout(const Entity* entity, bool requireEntered, bool fallBack) {
    return FindUILayout(entity->GetRoot()->mEntity, requireEntered, fallBack);
}

// Reconstructed from eboot.elf at 0xBB2210.
Symbol CharacterBodyPartToSym(CharacterBodyPartToFocus part) {
    if (part == kCharacterBodyPartNone) {
        return Symbol("kCharacterBodyPartNone");
    }

    static const Symbol sSymbols[] = {
        Symbol("kCharacterBodyPartHead"),
        Symbol("kCharacterBodyPartTorso"),
        Symbol("kCharacterBodyPartLegs"),
        Symbol("kCharacterBodyPartGuitar"),
        Symbol("kCharacterBodyPartDrums"),
    };
    return sSymbols[part];
}

// Reconstructed from eboot.elf at 0xBB2300. Every property is read through
// its accessors from theRBMetaState or the managers it summarizes; none is
// stored in the component.
void RBMetaStateCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBMetaStateCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    metadata.mDescription = "Read-only state for rb meta globals.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());

    const int intType = 0;
    const bool boolType = false;
    const Symbol symbolType;

    IntMetadata<int>& currentLayout =
        TypeSpecificMetadata(registry.RegisterProp("current_layout", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    currentLayout.mHelp =
        String("Returns the active layout; setting this property will attemp to goto the layout (transition).");
    currentLayout.mAllowedValues = RBMetaState::GetAllowedLayoutValues(false);
    currentLayout.mGetter = [](const PropAccessorArgs&) { return static_cast<int>(theRBMetaState.mCurrentLayout); };
    currentLayout.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.Goto(static_cast<RBUILayout>(*static_cast<const int*>(args.mValue)), false);
    };
    currentLayout.mHistory = false;

    BoolMetadata& isRockShop =
        TypeSpecificMetadata(registry.RegisterProp("current_layout_is_rock_shop", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    isRockShop.mHelp = String("Is the active layout part of the rock shop?");
    isRockShop.mGetter = [](const PropAccessorArgs&) { return IsRockShopLayout(theRBMetaState.mCurrentLayout); };
    isRockShop.mReadOnly = true;

    BoolMetadata& usesMetaBackground = TypeSpecificMetadata(
        registry.RegisterProp("current_layout_uses_meta_background", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    usesMetaBackground.mHelp = String("Does the active layout use the standard meta background scene?");
    usesMetaBackground.mGetter = [](const PropAccessorArgs&) {
        return UsesMetaBackground(theRBMetaState.mCurrentLayout);
    };
    usesMetaBackground.mReadOnly = true;

    IntMetadata<int>& focusEntity =
        TypeSpecificMetadata(registry.RegisterProp("current_focus_entity", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    focusEntity.mDisplayName = String("Character Entity To Focus");
    focusEntity.mHelp = String("Which player entity should be focused (on character creator/rock shop screens)");
    focusEntity.mAllowedValues = GetFocusInstrumentAllowedValues();
    focusEntity.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mFocusEntity; };
    focusEntity.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.mFocusEntity = *static_cast<const int*>(args.mValue);
    };
    focusEntity.mSaves = false;

    IntMetadata<int>& focusBodyPart =
        TypeSpecificMetadata(registry.RegisterProp("current_focus_body_part", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    focusBodyPart.mDisplayName = String("Character Body Part To Focus");
    focusBodyPart.mHelp = String("Which body part should be focused (on character creator/rock shop screens)");
    focusBodyPart.mAllowedValues = RBMetaState::GetAllowedBodyPartFocusValues();
    focusBodyPart.mGetter = [](const PropAccessorArgs&) { return static_cast<int>(theRBMetaState.mFocusBodyPart); };
    focusBodyPart.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.mFocusBodyPart = static_cast<CharacterBodyPartToFocus>(*static_cast<const int*>(args.mValue));
    };
    focusBodyPart.mSaves = false;

    IntMetadata<int>& focusGender =
        TypeSpecificMetadata(registry.RegisterProp("current_focus_gender", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    focusGender.mDisplayName = String("Character Entity Gender");
    focusGender.mHelp = String("The gender of the focused character entity (on character creator/rock shop screens)");
    focusGender.mAllowedValues = RBChar::gCharGenderAllowedValues;
    focusGender.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mFocusGender; };
    focusGender.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.mFocusGender = *static_cast<const int*>(args.mValue);
    };
    focusGender.mSaves = false;

    IntMetadata<int>& musicState =
        TypeSpecificMetadata(registry.RegisterProp("current_bg_music_state", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    musicState.mDisplayName = String("BG Music State");
    musicState.mHelp = String("Indicates what kind of background music should be playing in the current layout.");
    musicState.mAllowedValues = RBMetaState::GetAllowedMusicStateValues(false);
    musicState.mGetter = [](const PropAccessorArgs&) {
        return static_cast<int>(RBMetaState::GetMusicStateForLayout(theRBMetaState.mCurrentLayout));
    };
    musicState.mReadOnly = true;
    musicState.mSaves = false;

    IntMetadata<int>& ambientState =
        TypeSpecificMetadata(registry.RegisterProp("current_bg_ambient_sound_state", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    ambientState.mDisplayName = String("BG Ambient Sound State");
    ambientState.mHelp = String("Indicates what kind of ambient sounds should be playing in the current layout.");
    ambientState.mAllowedValues = RBMetaState::GetAllowedAmbientStateValues(false);
    ambientState.mGetter = [](const PropAccessorArgs&) {
        return static_cast<int>(RBMetaState::GetAmbientSoundStateForLayout(theRBMetaState.mCurrentLayout));
    };
    ambientState.mReadOnly = true;
    ambientState.mSaves = false;

    IntMetadata<int>& duckState =
        TypeSpecificMetadata(registry.RegisterProp("current_bg_duck_music_state", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    duckState.mDisplayName = String("BG Duck Music State");
    duckState.mHelp = String("Indicates whether background music should be ducked in the current layout.");
    duckState.mAllowedValues = RBMetaState::GetAllowedDuckStateValues(false);
    duckState.mGetter = [](const PropAccessorArgs&) {
        return static_cast<int>(IsLayoutWithDuckedMusic(theRBMetaState.mCurrentLayout));
    };
    duckState.mReadOnly = true;
    duckState.mSaves = false;

    IntMetadata<int>& gameplayState =
        TypeSpecificMetadata(registry.RegisterProp("current_meta_gameplay_state", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    gameplayState.mDisplayName = String("Meta Gameplay State");
    gameplayState.mHelp = String(
        "When we're in the gameplay layout, this provides more detail about what is going on with gameplay.");
    gameplayState.mAllowedValues = RBMetaState::GetAllowedMetaGameplayStateValues(false);
    gameplayState.mGetter = [](const PropAccessorArgs&) {
        return static_cast<int>(theRBMetaState.GetMetaGameplayState());
    };
    gameplayState.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.SetMetaGameplayState(static_cast<MetaGameplayState>(*static_cast<const int*>(args.mValue)));
    };
    gameplayState.mSaves = false;

    SymbolMetadata& helpbarOverride =
        TypeSpecificMetadata(registry.RegisterProp("helpbar_override_config", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    helpbarOverride.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.GetHelpbarOverride(); };
    helpbarOverride.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.SetHelpbarOverride(*static_cast<const Symbol*>(args.mValue));
    };
    helpbarOverride.mHistory = false;

    BoolMetadata& dialogActive =
        TypeSpecificMetadata(registry.RegisterProp("is_dialog_active", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    dialogActive.mGetter = [](const PropAccessorArgs&) { return DialogMgrCom::IsDialogActive(); };
    dialogActive.mReadOnly = true;

    BoolMetadata& firstCalibration =
        TypeSpecificMetadata(registry.RegisterProp("is_first_time_calibration", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    firstCalibration.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mFirstTimeCalibration; };
    firstCalibration.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.mFirstTimeCalibration = *static_cast<const bool*>(args.mValue);
    };
    firstCalibration.mReadOnly = true;

    SymbolMetadata& versionText =
        TypeSpecificMetadata(registry.RegisterProp("game_version_text", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    versionText.mGetter = [](const PropAccessorArgs&) { return RBMetaState::GetGameVersionText(); };
    versionText.mReadOnly = true;
    versionText.mHistory = false;

    SymbolMetadata& bandName =
        TypeSpecificMetadata(registry.RegisterProp("current_band_name", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    bandName.mGetter = [](const PropAccessorArgs&) { return RBMetaState::GetCurrentBandName(); };
    bandName.mReadOnly = true;
    bandName.mSaves = false;
    bandName.mHistory = false;

    BoolMetadata& slotJoinable =
        TypeSpecificMetadata(registry.RegisterProp("is_any_slot_joinable", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    slotJoinable.mHelp = String("Is any overshell slot currently joinable?");
    slotJoinable.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mAnySlotJoinable; };
    slotJoinable.mSetter = [](const PropAccessorArgs& args) {
        theRBMetaState.mAnySlotJoinable = *static_cast<const bool*>(args.mValue);
    };
    slotJoinable.mSaves = false;

    BoolMetadata& soloInvite = TypeSpecificMetadata(
        registry.RegisterProp("show_guitar_solo_tutorial_invite", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    soloInvite.mHelp = String(
        "True if we should go from the intro animations to the guitar solo invite. False if we should go to the "
        "main menu instead.");
    soloInvite.mReadOnly = true;
    soloInvite.mSaves = false;
    soloInvite.mGetter = [](const PropAccessorArgs&) { return RBIGSOnboarding::ShouldShowInvite(); };

    BoolMetadata& autosaving = TypeSpecificMetadata(registry.RegisterProp("is_autosaving", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    autosaving.mHelp = String("Is the game autosaving?");
    autosaving.mGetter = [](const PropAccessorArgs&) { return theSaveLoadMgr->mIsBusy; };
    autosaving.mSaves = false;
    autosaving.mReadOnly = true;

    IntMetadata<int>& microphones =
        TypeSpecificMetadata(registry.RegisterProp("num_microphones_connected", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    microphones.mHelp = String("Is there at least one microphone connected?");
    microphones.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mNumMicsConnected; };
    microphones.mSaves = false;
    microphones.mReadOnly = true;

    // The joined slots, a struct of accessor-only members.
    GroupMetadata& joinedSlots =
        static_cast<GroupMetadata&>(*registry.RegisterProp("joined_slots", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    joinedSlots.mHelp = String("Whether or not button prompts should reflect the instrument in each slot");
    PropRegistry& slots = joinedSlots.mRegistry;

    BoolMetadata& guitar0 = TypeSpecificMetadata(slots.RegisterProp("guitar0", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    guitar0.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.IsSlotJoined(kGameSlotGuitar0); };
    guitar0.mReadOnly = true;

    BoolMetadata& drum = TypeSpecificMetadata(slots.RegisterProp("drum", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    drum.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.IsSlotJoined(kGameSlotDrum); };
    drum.mReadOnly = true;

    BoolMetadata& vocals = TypeSpecificMetadata(slots.RegisterProp("vocals", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    vocals.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.IsSlotJoined(kGameSlotVocals); };
    vocals.mReadOnly = true;

    BoolMetadata& guitar1 = TypeSpecificMetadata(slots.RegisterProp("guitar1", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    guitar1.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.IsSlotJoined(kGameSlotGuitar1); };
    guitar1.mReadOnly = true;

    BoolMetadata& eitherGuitar =
        TypeSpecificMetadata(slots.RegisterProp("either_guitar_joined", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    eitherGuitar.mGetter = [](const PropAccessorArgs&) {
        return theRBMetaState.IsSlotJoined(kGameSlotGuitar0) || theRBMetaState.IsSlotJoined(kGameSlotGuitar1);
    };
    eitherGuitar.mReadOnly = true;

    BoolMetadata& isSomp = TypeSpecificMetadata(registry.RegisterProp("is_somp", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    isSomp.mGetter = [](const PropAccessorArgs&) { return theRBSompSessionMgr.IsSompSession(); };
    isSomp.mReadOnly = true;

    BoolMetadata& filtered = TypeSpecificMetadata(
        registry.RegisterProp("is_library_filtered_for_challenge", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    filtered.mReadOnly = true;
    filtered.mGetter = [](const PropAccessorArgs&) { return RBMetaState::IsLibraryFilteredForChallenge(); };

    BoolMetadata& editBand =
        TypeSpecificMetadata(registry.RegisterProp("go_directly_to_edit_band", -1, kPropertyBool, 0), boolType);
    static_cast<void>(Symbol("prop"));
    editBand.mReadOnly = true;
    editBand.mSaves = false;
    editBand.mGetter = [](const PropAccessorArgs&) { return theRBMetaState.mGoDirectlyToEditBand; };
}

// Reconstructed from eboot.elf at 0xBB3910. The layouts are ordered by
// LayoutSort.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedLayoutValues(bool includeInvalid) {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(kNumRBUILayouts);
    if (includeInvalid) {
        values.push_back(NamedValue(kLayoutInvalid, Symbol("kLayoutInvalid").Str()));
    }
    for (int layout = 0; layout != kNumRBUILayouts; layout++) {
        values.push_back(NamedValue(layout, RBUILayoutToSym(static_cast<RBUILayout>(layout)).Str()));
    }
    eastl::sort(values.begin(), values.end(), LayoutSort);
    return values;
}

// Reconstructed from eboot.elf at 0xBB3E80.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedEntityFocusValues() {
    return GetFocusInstrumentAllowedValues();
}

// Reconstructed from eboot.elf at 0xBB3EA0.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedBodyPartFocusValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(6);
    values.push_back(NamedValue(kCharacterBodyPartNone, Symbol("kCharacterBodyPartNone").Str()));
    for (int part = 0; part != 5; part++) {
        values.push_back(NamedValue(part, CharacterBodyPartToSym(static_cast<CharacterBodyPartToFocus>(part)).Str()));
    }
    return values;
}

// Reconstructed from eboot.elf at 0xBB4070.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedMusicStateValues(bool includeInvalid) {
    eastl::vector<AllowedValue<int>> values;
    if (includeInvalid) {
        values.push_back(AllowedValue<int>{kBGMusicStateInvalid, String("Invalid"), String("Invalid")});
    }
    values.push_back(AllowedValue<int>{kBGMusicStateNone, String("No Music"), String("No Music")});
    values.push_back(
        AllowedValue<int>{kBGMusicStateTitleScreen, String("Title Screen Music"), String("Title Screen Music")});
    values.push_back(AllowedValue<int>{kBGMusicStateShell, String("Shell Music"), String("Shell Music")});
    values.push_back(AllowedValue<int>{kBGMusicStateBandHub, String("Band Hub Music"), String("Band Hub Music")});
    values.push_back(AllowedValue<int>{
        kBGMusicStateWeeklyChallenge, String("Weekly Challenge Music"), String("Weekly Challenge Music")});
    return values;
}

// Reconstructed from eboot.elf at 0xBB4450.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedAmbientStateValues(bool includeInvalid) {
    eastl::vector<AllowedValue<int>> values;
    if (includeInvalid) {
        values.push_back(AllowedValue<int>{kBGAmbientSoundStateInvalid, String("Invalid"), String("Invalid")});
    }
    values.push_back(
        AllowedValue<int>{kBGAmbientSoundStateNone, String("No Ambient Sound"), String("No Ambient Sound")});
    values.push_back(
        AllowedValue<int>{kBGAmbientSoundStateCrowdOnly, String("Ambient Crowd Only"), String("Ambient Crowd Only")});
    values.push_back(AllowedValue<int>{
        kBGAmbientSoundStateCrowdAndBand, String("Ambient Crowd And Band"), String("Ambient Crowd And Band")});
    values.push_back(AllowedValue<int>{kBGAmbientSoundStateWorld, String("Ambient World"), String("Ambient World")});
    return values;
}

// Reconstructed from eboot.elf at 0xBB47A0.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedDuckStateValues(bool includeInvalid) {
    eastl::vector<AllowedValue<int>> values;
    if (includeInvalid) {
        values.push_back(AllowedValue<int>{kBGDuckMusicStateInvalid, String("Invalid"), String("Invalid")});
    }
    values.push_back(AllowedValue<int>{kBGDuckMusicStateFullVolume, String("Full Volume"), String("Full Volume")});
    values.push_back(AllowedValue<int>{kBGDuckMusicStateDucked, String("Ducked"), String("Ducked")});
    return values;
}

// Reconstructed from eboot.elf at 0xBB49C0. The between-songs and exiting
// states are not offered.
eastl::vector<AllowedValue<int>> RBMetaState::GetAllowedMetaGameplayStateValues(bool includeInvalid) {
    eastl::vector<AllowedValue<int>> values;
    if (includeInvalid) {
        values.push_back(NamedValue(kMetaGameplayStateInvalid, "kMetaGameplayStateInvalid"));
    }
    values.push_back(NamedValue(kMetaGameplayStateNotInGameplay, "kMetaGameplayStateNotInGameplay"));
    values.push_back(NamedValue(kMetaGameplayStateIntro, "kMetaGameplayStateIntro"));
    values.push_back(NamedValue(kMetaGameplayStatePreSong, "kMetaGameplayStatePreSong"));
    values.push_back(NamedValue(kMetaGameplayStateInSong, "kMetaGameplayStateInSong"));
    values.push_back(NamedValue(kMetaGameplayStateWinning, "kMetaGameplayStateWinning"));
    values.push_back(NamedValue(kMetaGameplayStateLosing, "kMetaGameplayStateLosing"));
    values.push_back(NamedValue(kMetaGameplayStateOnResultsScreen, "kMetaGameplayStateOnResultsScreen"));
    values.push_back(NamedValue(kMetaGameplayStateWatchingCutscene, "kMetaGameplayStateWatchingCutscene"));
    values.push_back(NamedValue(kMetaGameplayStateMenu, "kMetaGameplayStateMenu"));
    values.push_back(NamedValue(kMetaGameplayStateVoting, "kMetaGameplayStateVoting"));
    return values;
}

// Reconstructed from eboot.elf at 0xBB5080.
RBMetaStateCom::RBMetaStateCom() {}

// Reconstructed from eboot.elf at 0xBB50F0.
bool LayoutSort(const AllowedValue<int>& a, const AllowedValue<int>& b) {
    const bool aDeprecated = a.mName.contains("_DEPRECATED");
    const bool bDeprecated = b.mName.contains("_DEPRECATED");
    if (aDeprecated && !bDeprecated) {
        return false;
    }
    if (bDeprecated && !aDeprecated) {
        return true;
    }
    return strcmp(a.mName.c_str(), b.mName.c_str()) < 0;
}

// Reconstructed from eboot.elf at 0xBC5570, the object's static
// initializer, which inlines it.
RBMetaState::RBMetaState()
    : mCurrentLayout(kLayoutInvalid),
      mPendingLayout(kLayoutInvalid),
      mReturnLayout(kLayoutInvalid),
      mPreviousLayout(kLayoutInvalid),
      mCalibrationUser(nullptr),
      mFirstTimeCalibration(false),
      mFocusEntity(4),
      mFocusBodyPart(kCharacterBodyPartNone),
      mHelpbarOverride(),
      mButtonOverrides(),
      mBackPressed(false),
      mLayoutChanged(false),
      mAnySlotJoinable(false),
      mSlotJoined(),
      mFocusSlot(kNumGameSlots),
      mSongListIndex(0),
      mPracticeSection(0),
      mSfxVolume(-1),
      mCrowdVolume(-1),
      mApplyVolumeOptions(false),
      mNumMicsConnected(0),
      mMetaGameplayState(kMetaGameplayStateNotInGameplay),
      mGoDirectlyToEditBand(false),
      mShellMusicStarted(false),
      mLoadingHeapStrategy(false),
      mMainHeap(-1),
      mGpuHeap(-1),
      mMainHeapStrategy(-1),
      mGpuHeapStrategy(-1) {}

// Reconstructed from eboot.elf at 0xBB5590. The timeline callback lets the
// animations of a UI layout run on the UI clock unless the layout is one of
// the "use_authored_anim_timelines" layouts; animations of musician
// characters keep their options' timeline.
void RBMetaState::Init() {
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = this;
    DataSetGlobal(Symbol("rb_meta_state"), self);

    DataArray* const layouts = SystemConfig(Symbol("use_authored_anim_timelines"));
    for (int i = 1; i < layouts->Size(); i++) {
        gAuthoredTimelineLayouts.push_back(ResourcePath(layouts->Node(i).Str(nullptr)));
    }

    AnimDriverCom::sProjectTimelineCallback = [](Entity* entity, const Resource* resource) {
        const Entity* const layout = GetUILayoutEntity(entity);
        if (layout == nullptr || strstr(resource->mPath.Str(), "char/musician/") != nullptr) {
            return kNumTimeUnits;
        }
        const EntityResource* const layoutResource = layout->mResource;
        if (layoutResource != nullptr) {
            for (const ResourcePath& path : gAuthoredTimelineLayouts) {
                if (path == layoutResource->mPath) {
                    return kNumTimeUnits;
                }
            }
        }
        return kTimeUnitsUISeconds;
    };
}

// Reconstructed from eboot.elf at 0xBB58C0.
void RBMetaState::Poll() {
    if (!mShellMusicStarted) {
        const ResourcePath path("audio_layers/shell_banks.scene");
        static_cast<void>(RndSceneResource::Id());
        if (ResourcePtr<RndSceneResource>(static_cast<RndSceneResource*>(Resource::Get(path).Get())) != nullptr) {
            mShellMusicStarted = gFmodBusInterface->AttachBusToMusicPort(Symbol("bus:/MUSICMAIN").Str());
        }
    }

    if (mApplyVolumeOptions) {
        const RBSystemOptions& options = theRBProfileMgr->mSystemOptions;
        const int sfx = static_cast<int>(options.mSFXVolume);
        if (sfx != mSfxVolume && gFmodBusInterface->SetVcaVolumeDb("vca:/SFX", 2, options.GetSFXVolumeDb(), 0.0f)) {
            mSfxVolume = sfx;
        }
        const int crowd = static_cast<int>(options.mCrowdVolume);
        if (crowd != mCrowdVolume
            && gFmodBusInterface->SetVcaVolumeDb("vca:/Crowd", 2, options.GetCrowdVolumeDb(), 0.0f)) {
            mCrowdVolume = crowd;
        }
    }

    if (mLayoutChanged && mBackPressed) {
        theSoundManager.PlaySound(Symbol("event:/Shell/UI/os_back"), nullptr, false);
    }
    mBackPressed = false;
    mLayoutChanged = false;
    _UpdateJoinedSlots();

    mNumMicsConnected = 0;
    for (int slot = 0; slot < kNumGameSlots; slot++) {
        if (TheGameMicManager->HasMic(MicClientID{slot, -1})) {
            mNumMicsConnected++;
        }
    }
}

// Reconstructed from eboot.elf at 0xBB5D40. The deprecated layouts without
// a file do nothing; the dropped ones and the developer menu fall back to
// the title. Going to a SOMP session's create or browse layout also goes
// through the layouts after it.
void RBMetaState::Goto(RBUILayout layout, bool quiet) {
    if (theRBSompSessionMgr.IsSompSession() && !quiet) {
        static_cast<void>(RBUILayoutToSym(layout));
    }
    if (mCurrentLayout == layout) {
        return;
    }
    mPendingLayout = layout;
    const bool expensive = layout == kLayoutDifficultySelect || layout == kLayoutGame || layout == kLayoutLoading
        || layout == kLayoutBTMChallenge;
    _SetLoadingHeapStrategy(expensive);

    switch (layout) {
    case kLayoutTitle:
        theUI->Goto(ResourcePath("ui/title/title.layout"));
        break;
    case kLayoutSongSelect:
        theUI->Goto(ResourcePath("ui/songs/songs.layout"));
        break;
    case kLayoutDifficultySelect:
        theUI->Goto(ResourcePath("ui/difficulty/difficulty.layout"));
        break;
    case kLayoutGame:
        theUI->Goto(ResourcePath("ui/game/game.layout"));
        break;
    case kLayoutMainMenu:
        theUI->Goto(ResourcePath("ui/main_menu/main_menu.layout"));
        break;
    case kLayoutStoreCategories:
        theUI->Goto(ResourcePath("ui/store/store_categories.layout"));
        break;
    case kLayoutStoreSongs:
        theUI->Goto(ResourcePath("ui/store/store_songs.layout"));
        break;
    case kLayoutCampaignGigCompletionResults:
        theUI->Goto(ResourcePath("ui/results/campaign_gig_complete_results.layout"));
        break;
    case kLayoutLoading:
        theUI->Goto(ResourcePath("ui/loading/loading.layout"));
        break;
    case kLayoutBandEdit:
        theUI->Goto(ResourcePath("ui/band_edit/band_edit.layout"));
        break;
    case kLayoutStoreSearch:
        theUI->Goto(ResourcePath("ui/store/store_search.layout"));
        break;
    case kLayoutBandSelection:
        theUI->Goto(ResourcePath("ui/band_select/band_select.layout"));
        break;
    case kLayoutCitySelection:
        theUI->Goto(ResourcePath("ui/city_select/city_select.layout"));
        break;
    case kLayoutLeaderboards:
        theUI->Goto(ResourcePath("ui/leaderboards/leaderboards.layout"));
        break;
    case kLayoutPlayerStats:
        theUI->Goto(ResourcePath("ui/player_stats/player_stats.layout"));
        break;
    case kLayoutCampaignNarrativeUpdate:
        theUI->Goto(ResourcePath("ui/campaign_narrative/campaign_narrative.layout"));
        break;
    case kLayoutStoreHome:
        theUI->Goto(ResourcePath("ui/store/store_home.layout"));
        break;
    case kLayoutGigSelection:
        theUI->Goto(ResourcePath("ui/campaign_gig_select/campaign_gig_select.layout"));
        break;
    case kLayoutTourSelection:
        theUI->Goto(ResourcePath("ui/tour_select/tour_select.layout"));
        break;
    case kLayoutOptionsSystemSettings:
        theUI->Goto(ResourcePath("ui/options/options_system_settings.layout"));
        break;
    case kLayoutOptionsVolumeControls:
        theUI->Goto(ResourcePath("ui/options/options_volume_controls.layout"));
        break;
    case kLayoutBandNaming:
        theUI->Goto(ResourcePath("ui/band_naming/band_naming.layout"));
        break;
    case kLayoutOptionsCredits:
        theUI->Goto(ResourcePath("ui/options/options_credits.layout"));
        break;
    case kLayoutOptionsModifiers:
        theUI->Goto(ResourcePath("ui/options/options_modifiers.layout"));
        break;
    case kLayoutLegacyEntitlements_DEPRECATED:
        theUI->Goto(ResourcePath("ui/legacy_entitlements/legacy_entitlements.layout"));
        break;
    case kLayoutCharacterCreate:
        theUI->Goto(ResourcePath("ui/character/character_create/character_create.layout"));
        break;
    case kLayoutRockShopMain:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize.layout"));
        break;
    case kLayoutQuickplayResults:
        theUI->Goto(ResourcePath("ui/results/quickplay_results.layout"));
        break;
    case kLayoutCharacterCreateHead:
        theUI->Goto(ResourcePath("ui/character/character_create/character_create_head.layout"));
        break;
    case kLayoutCharacterCreateHair:
        theUI->Goto(ResourcePath("ui/character/character_create/character_create_hair.layout"));
        break;
    case kLayoutRockShopClothing:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_clothing.layout"));
        break;
    case kLayoutRockShopHairMakeup:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_hair_makeup.layout"));
        break;
    case kLayoutRockShopInstruments:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_instruments.layout"));
        break;
    case kLayoutRockShopDetail:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_detail.layout"));
        break;
    case kLayoutBandHub:
        theUI->Goto(ResourcePath("ui/band_hub/band_hub.layout"));
        break;
    case kLayoutBandMemberSelect:
        theUI->Goto(ResourcePath("ui/band_member_select/band_member_select.layout"));
        break;
    case kLayoutGameStartup:
        theUI->Goto(ResourcePath("ui/startup/startup.layout"));
        break;
    case kLayoutCalibrationAuto:
        theUI->Goto(ResourcePath("ui/calibration/calibration_auto.layout"));
        break;
    case kLayoutCalibrationManual:
        theUI->Goto(ResourcePath("ui/calibration/calibration_manual.layout"));
        break;
    case kLayoutCalibrationEnterNumbers:
        theUI->Goto(ResourcePath("ui/calibration/calibration_enter_numbers.layout"));
        break;
    case kLayoutRockShopBandSelect:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_band_select.layout"));
        break;
    case kLayoutSessionMusicianSelect:
        theUI->Goto(ResourcePath("ui/band_member_select/session_member_select.layout"));
        break;
    case kLayoutCampaignSetlistCreate:
        theUI->Goto(ResourcePath("ui/campaign_setlist/setlist.layout"));
        break;
    case kLayoutSongSelectSearch:
        theUI->Goto(ResourcePath("ui/songs/songs_search.layout"));
        break;
    case kLayoutGuitarSoloTutorialInvite:
        theUI->Goto(ResourcePath("ui/guitar_solo_invite/guitar_solo_invite.layout"));
        break;
    case kLayoutCampaignBandHistory:
        theUI->Goto(ResourcePath("ui/band_history/band_history.layout"));
        break;
    case kLayoutNotificationFeed:
        theUI->Goto(ResourcePath("ui/notifications/notifications.layout"));
        break;
    case kLayoutSetlistHub:
        theUI->Goto(ResourcePath("ui/user_setlist/setlist_hub.layout"));
        break;
    case kLayoutSetlistNaming:
        theUI->Goto(ResourcePath("ui/user_setlist_naming/setlist_naming.layout"));
        break;
    case kLayoutPracticeSpeedSelect:
        theUI->Goto(ResourcePath("ui/practice/practice_speed_select.layout"));
        break;
    case kLayoutBTMSetlistCreate:
        theUI->Goto(ResourcePath("ui/btm/btm_setlist.layout"));
        break;
    case kLayoutBandWarsHub:
        theUI->Goto(ResourcePath("ui/band_wars/clan_rivals_hub.layout"));
        break;
    case kLayoutPlayerVsPlayer_DEPRECATED:
        theUI->Goto(ResourcePath("ui/player_stats/player_v_player.layout"));
        break;
    case kLayoutClanVsClan:
        theUI->Goto(ResourcePath("ui/player_stats/clan_v_clan.layout"));
        break;
    case kLayoutFindPlayers:
        theUI->Goto(ResourcePath("ui/band_wars/find_players.layout"));
        break;
    case kLayoutMyInvitations:
        theUI->Goto(ResourcePath("ui/band_wars/my_invitations.layout"));
        break;
    case kLayoutBTMHub:
        theUI->Goto(ResourcePath("ui/btm/btm_hub.layout"));
        break;
    case kLayoutBTMBandSelection:
        theUI->Goto(ResourcePath("ui/btm/btm_band_select.layout"));
        break;
    case kLayoutBTMIntroChoice:
        theUI->Goto(ResourcePath("ui/btm/btm_intro_choice.layout"));
        break;
    case kLayoutBTMWinLoss:
        theUI->Goto(ResourcePath("ui/btm/btm_win_loss.layout"));
        break;
    case kLayoutBTMChapterCompletion:
        theUI->Goto(ResourcePath("ui/btm/btm_chapter_completion.layout"));
        break;
    case kLayoutFindClans:
        theUI->Goto(ResourcePath("ui/band_wars/find_groups.layout"));
        break;
    case kLayoutBTMBandNaming:
        theUI->Goto(ResourcePath("ui/band_naming/btm_band_naming.layout"));
        break;
    case kLayoutClanNotifications:
        theUI->Goto(ResourcePath("ui/band_wars/clan_notifications.layout"));
        break;
    case kLayoutClanStats:
        theUI->Goto(ResourcePath("ui/band_wars_clan_stats/clan_stats.layout"));
        break;
    case kLayoutClanBadges:
        theUI->Goto(ResourcePath("ui/band_wars/clan_badges.layout"));
        break;
    case kLayoutPlayerBadges:
        theUI->Goto(ResourcePath("ui/player_stats/player_badges.layout"));
        break;
    case kLayoutPlayerPortrait:
        theUI->Goto(ResourcePath("ui/portrait/player_portrait.layout"));
        break;
    case kLayoutClanCreateEdit:
        theUI->Goto(ResourcePath("ui/band_wars/clan_create_edit.layout"));
        break;
    case kLayoutBTMModeCompletion:
        theUI->Goto(ResourcePath("ui/btm/btm_mode_completion.layout"));
        break;
    case kLayoutClanRoster:
        theUI->Goto(ResourcePath("ui/band_wars/clan_roster.layout"));
        break;
    case kLayoutClanWeeklyChallenge:
        theUI->Goto(ResourcePath("ui/band_wars/band_wars_weekly_challenges.layout"));
        break;
    case kLayoutClanTierStatus:
        theUI->Goto(ResourcePath("ui/band_wars/clan_tier_status.layout"));
        break;
    case kLayoutClanEventLeaderboard:
        theUI->Goto(ResourcePath("ui/band_wars/clan_rivals_event_leaderboard.layout"));
        break;
    case kLayoutBTMChallenge:
        theUI->Goto(ResourcePath("ui/btm/btm_challenge.layout"));
        break;
    case kLayoutSompGreenRoom:
        theUI->Goto(ResourcePath("ui/somp/somp_green_room/somp_green_room.layout"));
        break;
    case kLayoutSompMenu:
        theUI->Goto(ResourcePath("ui/somp/somp_menu.layout"));
        break;
    case kLayoutSompHub:
        theUI->Goto(ResourcePath("ui/somp/somp_hub/somp_hub.layout"));
        break;
    case kLayoutSompInviteFriends:
        theUI->Goto(ResourcePath("ui/somp/somp_invite_friends/somp_invite_friends.layout"));
        break;
    case kLayoutMissions:
        theUI->Goto(ResourcePath("ui/missions/missions.layout"));
        break;
    case kLayoutClanUserSettings:
        theUI->Goto(ResourcePath("ui/band_wars/band_wars_user_settings.layout"));
        break;
    case kLayoutBandWarsLookingForClan:
        theUI->Goto(ResourcePath("ui/band_wars/clan_rivals_looking_for_clan.layout"));
        break;
    case kLayoutBandWarsLookingForClanInvitations:
        theUI->Goto(ResourcePath("ui/band_wars/clan_rivals_invitations.layout"));
        break;
    case kLayoutOptionsGraphics:
        theUI->Goto(ResourcePath("ui/options/options_graphics.layout"));
        break;
    case kLayoutClanInfoLanding:
        theUI->Goto(ResourcePath("ui/band_wars/clan_info_landing.layout"));
        break;
    case kLayoutRockShopCustomizeTrack:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_track.layout"));
        break;
    case kLayoutRockShopCustomizeTrackR2T:
        theUI->Goto(ResourcePath("ui/character/character_customize/character_customize_track_r2t.layout"));
        break;
    case kLayoutOptions_DEPRECATED:
    case kLayoutVenueSelect_DEPRECATED:
    case kLayoutOptionsCalibration_DEPRECATED:
    case kLayoutCharacterCreateFacialHair_DEPRECATED:
    case kLayoutIntro_DEPRECATED:
    case kLayoutGuitarSoloExtras_DEPRECATED:
    case kLayoutPlayerProfileStats_DEPRECATED:
    case kLayoutClanProfileStats_DEPRECATED:
    case kLayoutBTMIntro_DEPRECATED:
    case kLayoutClanWeeklyChallengeResult_DEPRECATED:
        break;
    case kLayoutPracticeSectionSelect:
        if (mCurrentLayout != kLayoutGame && mCurrentLayout != kLayoutPracticeSpeedSelect) {
            mPracticeSection = 0;
        }
        theUI->Goto(ResourcePath("ui/practice/practice_section_select.layout"));
        break;
    case kLayoutClanAchievements_DEPRECATED:
        static_cast<void>(_GetLayoutSym(layout));
        theUI->Goto(ResourcePath("ui/band_wars/clan_badges.layout"));
        break;
    case kLayoutSompSessionCreateEdit:
        theUI->Goto(ResourcePath("ui/somp/somp_session_create_edit/somp_session_create_edit.layout"));
        [[fallthrough]];
    case kLayoutSompBrowseSessions:
        theUI->Goto(ResourcePath("ui/somp/somp_browse_sessions/somp_browse_sessions.layout"));
        [[fallthrough]];
    case kLayoutSompSessionPreferences:
        theUI->Goto(ResourcePath("ui/somp/somp_session_preferences/somp_session_preferences.layout"));
        break;
    default:
        static_cast<void>(_GetLayoutSym(layout));
        Goto(kLayoutTitle, false);
        break;
    }
    mLayoutChanged = true;
    DialogMgrCom::ClearAllDialogs();
}

// Reconstructed from eboot.elf at 0xBB6EC0. Strategy 3 is MemHeap's
// strategy for large transient loads.
void RBMetaState::_SetLoadingHeapStrategy(bool loading) {
    if (mMainHeap == -1) {
        mMainHeap = MemFindHeap("main");
        if (mMainHeap == -1) {
            return;
        }
    }
    if (mGpuHeap == -1) {
        mGpuHeap = MemFindHeap("gpu");
        if (mGpuHeap == -1) {
            return;
        }
    }
    if (loading == mLoadingHeapStrategy) {
        return;
    }
    mLoadingHeapStrategy = loading;
    if (loading) {
        mMainHeapStrategy = MemHeapStrategy(mMainHeap);
        MemSetHeapStrategy(mMainHeap, 3);
        mGpuHeapStrategy = MemHeapStrategy(mGpuHeap);
        MemSetHeapStrategy(mGpuHeap, 3);
    } else {
        MemSetHeapStrategy(mMainHeap, mMainHeapStrategy);
        MemSetHeapStrategy(mGpuHeap, mGpuHeapStrategy);
    }
}

// Reconstructed from eboot.elf at 0xBB6FC0.
Symbol RBMetaState::_GetLayoutSym(RBUILayout layout) {
    const eastl::vector<AllowedValue<int>> values = GetAllowedLayoutValues(false);
    const char* name = "";
    for (const AllowedValue<int>& value : values) {
        if (value.mValue == layout) {
            name = value.mName.c_str();
            break;
        }
    }
    return Symbol(name);
}

// Reconstructed from eboot.elf at 0xBB70A0.
void RBMetaState::SetHelpbarOverride(Symbol config) {
    if (mHelpbarOverride != config) {
        mHelpbarOverride = config;
        HelpbarOverrideMsg msg;
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0xBB7190.
void RBMetaState::ClearHelpbarOverride() {
    SetHelpbarOverride(Symbol(""));
}

// Reconstructed from eboot.elf at 0xBB71F0.
void RBMetaState::SetHelpbarButtonOverride(JoypadAction action, Symbol label, Symbol icon) {
    ButtonOverride& button = mButtonOverrides[action];
    if (button.mLabel != label || button.mIcon != icon) {
        button.mLabel = label;
        button.mIcon = icon;
        HelpbarButtonOverrideMsg msg(action, label, icon);
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0xBB7280.
void RBMetaState::SetHelpbarButtonOverride(JoypadAction action, const ButtonOverride& button) {
    ButtonOverride& current = mButtonOverrides[action];
    if (current.mLabel != button.mLabel || current.mIcon != button.mIcon) {
        current = button;
        HelpbarButtonOverrideMsg msg(action, button.mLabel, button.mIcon);
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0xBB7330.
RBMetaState::ButtonOverride RBMetaState::GetHelpbarButtonOverride(JoypadAction action) const {
    return mButtonOverrides[action];
}

// Reconstructed from eboot.elf at 0xBB7420.
void RBMetaState::ClearHelpbarButtonOverrides() {
    for (int action = 0; action != kNumHelpbarButtonOverrides; action++) {
        SetHelpbarButtonOverride(static_cast<JoypadAction>(action), Symbol(""), Symbol(""));
    }
}

// Reconstructed from eboot.elf at 0xBB7510.
bool RBMetaState::IsSlotJoined(GameSlot slot) const {
    return mSlotJoined[slot] && (mFocusSlot == kNumGameSlots || mFocusSlot == slot);
}

// Reconstructed from eboot.elf at 0xBC3BE0.
void RBMetaState::ExportBootingPlayer() {
    BootingPlayerMsg msg;
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xBC3CD0.
bool RBMetaState::IsLibraryFilteredForChallenge() {
    if (theRBClanMgr->mLibraryFilteredForChallenge) {
        return true;
    }
    return theRBSompSessionMgr.IsSongLibraryFiltered();
}

// Reconstructed from eboot.elf at 0xBC3D00. The release build keeps only
// the symbol lookups of the report it gave when the layout was not the one
// being gone to.
void RBMetaState::_SetCurrentLayout(RBUILayout layout) {
    if (mCurrentLayout != kLayoutLoading) {
        mPreviousLayout = mCurrentLayout;
    }
    if (mPendingLayout != layout) {
        static_cast<void>(_GetLayoutSym(layout));
        static_cast<void>(_GetLayoutSym(mPendingLayout));
    }
    mCurrentLayout = layout;
    mPendingLayout = kLayoutInvalid;
}

// Reconstructed from eboot.elf at 0xBC3D40.
int RBMetaState::GetCurrentPatch() {
    DataArray* const versions = SystemConfig(Symbol("patch_versions"));
    if (versions == nullptr) {
        return 0;
    }
    DataArray* const patch = versions->FindArray(Symbol("current_patch"), false);
    if (patch == nullptr) {
        return 0;
    }
    return patch->Int(1);
}

// Reconstructed from eboot.elf at 0xBC3DE0. Each (patch text) entry of
// "patch_version_text" whose patch is the current one formats its text into
// the localized "version_number"; the last match wins.
Symbol RBMetaState::GetGameVersionText() {
    const int patch = GetCurrentPatch();
    Symbol text;
    DataArray* const versions = SystemConfig(Symbol("patch_versions"));
    if (versions != nullptr) {
        DataArray* const texts = versions->FindArray(Symbol("patch_version_text"), false);
        if (texts != nullptr) {
            for (int i = 1; i < texts->Size(); i++) {
                DataArray* const entry = texts->Node(i).Array(texts);
                if (entry != nullptr && entry->Size() == 2 && entry->Int(0) == patch) {
                    static Symbol versionNumber;
                    if (versionNumber == Symbol()) {
                        versionNumber = Symbol("version_number");
                    }
                    const char* const format = Localize(versionNumber, nullptr, 0, TheLocale);
                    FormatString string(format);
                    string << entry->Str(1);
                    text = Symbol(string.Str());
                }
            }
        }
    }
    return text;
}

// Reconstructed from eboot.elf at 0xBC4140. The release build keeps the
// rock shop check of an assertion on the session's layout.
BGMusicState RBMetaState::GetMusicStateForLayout(RBUILayout layout) {
    if (IsRockShopLayout(layout) && RBCharacterMenuMgr::SessionIsActive()) {
        layout = RBCharacterMenuMgr::GetPreviousLayout();
        static_cast<void>(IsRockShopLayout(layout));
    }
    return GetLayoutMusicState(layout);
}

// Reconstructed from eboot.elf at 0xBC4180.
BGAmbientSoundState RBMetaState::GetAmbientSoundStateForLayout(RBUILayout layout) {
    if (IsRockShopLayout(layout) && RBCharacterMenuMgr::SessionIsActive()) {
        layout = RBCharacterMenuMgr::GetPreviousLayout();
        static_cast<void>(IsRockShopLayout(layout));
    }
    return GetLayoutAmbientSoundState(layout);
}

// Reconstructed from eboot.elf at 0xBC41C0.
bool RBMetaState::IsMusicDuckedForLayout(RBUILayout layout) {
    return IsLayoutWithDuckedMusic(layout);
}

// Reconstructed from eboot.elf at 0xBC41D0.
MetaGameplayState RBMetaState::GetMetaGameplayState() const {
    if (mCurrentLayout != kLayoutGame) {
        return kMetaGameplayStateNotInGameplay;
    }
    return mMetaGameplayState;
}

// Reconstructed from eboot.elf at 0xBC41E0.
void RBMetaState::SetMetaGameplayState(MetaGameplayState state) {
    if (mMetaGameplayState != state) {
        mMetaGameplayState = state;
    }
}

// Reconstructed from eboot.elf at 0xBC41F0.
bool RBMetaState::IsWatchingCutsceneOrVoting() const {
    return mMetaGameplayState == kMetaGameplayStateWatchingCutscene || mMetaGameplayState == kMetaGameplayStateVoting;
}

// Reconstructed from eboot.elf at 0xBC4200. Every name is interned before
// the lookup.
Symbol MetaGameplayStateToSym(MetaGameplayState state) {
    static Symbol notInGameplay;
    if (notInGameplay == Symbol()) {
        notInGameplay = Symbol("kMetaGameplayStateNotInGameplay");
    }
    static Symbol intro;
    if (intro == Symbol()) {
        intro = Symbol("kMetaGameplayStateIntro");
    }
    static Symbol preSong;
    if (preSong == Symbol()) {
        preSong = Symbol("kMetaGameplayStatePreSong");
    }
    static Symbol inSong;
    if (inSong == Symbol()) {
        inSong = Symbol("kMetaGameplayStateInSong");
    }
    static Symbol betweenSongs;
    if (betweenSongs == Symbol()) {
        betweenSongs = Symbol("kMetaGameplayStateBetweenSongs");
    }
    static Symbol winning;
    if (winning == Symbol()) {
        winning = Symbol("kMetaGameplayStateWinning");
    }
    static Symbol losing;
    if (losing == Symbol()) {
        losing = Symbol("kMetaGameplayStateLosing");
    }
    static Symbol onResultsScreen;
    if (onResultsScreen == Symbol()) {
        onResultsScreen = Symbol("kMetaGameplayStateOnResultsScreen");
    }
    static Symbol watchingCutscene;
    if (watchingCutscene == Symbol()) {
        watchingCutscene = Symbol("kMetaGameplayStateWatchingCutscene");
    }
    static Symbol menu;
    if (menu == Symbol()) {
        menu = Symbol("kMetaGameplayStateMenu");
    }
    static Symbol voting;
    if (voting == Symbol()) {
        voting = Symbol("kMetaGameplayStateVoting");
    }
    static Symbol exiting;
    if (exiting == Symbol()) {
        exiting = Symbol("kMetaGameplayStateExiting");
    }
    static Symbol invalid;
    if (invalid == Symbol()) {
        invalid = Symbol("kMetaGameplayStateInvalid");
    }

    switch (state) {
    case kMetaGameplayStateInvalid:
        return invalid;
    case kMetaGameplayStateNotInGameplay:
        return notInGameplay;
    case kMetaGameplayStateIntro:
        return intro;
    case kMetaGameplayStatePreSong:
        return preSong;
    case kMetaGameplayStateInSong:
        return inSong;
    case kMetaGameplayStateBetweenSongs:
        return betweenSongs;
    case kMetaGameplayStateWinning:
        return winning;
    case kMetaGameplayStateLosing:
        return losing;
    case kMetaGameplayStateOnResultsScreen:
        return onResultsScreen;
    case kMetaGameplayStateWatchingCutscene:
        return watchingCutscene;
    case kMetaGameplayStateMenu:
        return menu;
    case kMetaGameplayStateVoting:
        return voting;
    case kMetaGameplayStateExiting:
        return exiting;
    default:
        return Symbol("INVALID_GAMEPLAY_STATE!");
    }
}

// Reconstructed from eboot.elf at 0xBC4760. Each define is a one-node array
// holding the layout's value, kLayoutInvalid included.
void InitRBLayoutDefines() {
    static bool sDefined = false;  // 0x1B002A0
    if (sDefined) {
        return;
    }
    sDefined = true;
    for (int layout = kLayoutInvalid; layout != kNumRBUILayouts; layout++) {
        const Symbol name = RBUILayoutToSym(static_cast<RBUILayout>(layout));
        DataArray* const value = new DataArray(1);
        value->AddRef();
        value->Node(0) = DataNode(layout);
        DataSetMacro(name, value);
        value->Release();
    }
}

// Reconstructed from eboot.elf at 0xBC48F0.
DataNode RBMetaState::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol setCalibrationUser;
    if (setCalibrationUser == Symbol()) {
        setCalibrationUser = Symbol("set_calibration_user");
    }
    if (type == setCalibrationUser) {
        mCalibrationUser = static_cast<User*>(msg->Evaluate(2).LiteralSink(msg));
        return DataNode(0);
    }
    static Symbol go;
    if (go == Symbol()) {
        go = Symbol("goto");
    }
    if (type == go) {
        Goto(static_cast<RBUILayout>(msg->Int(2)), false);
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xBC4A70.
Symbol RBMetaStateCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xBC4A80.
Symbol RBMetaStateCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xBC4A90.
int RBMetaStateCom::CurrentRev() const {
    return const_cast<RBMetaStateCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xBC4AB0.
bool RBMetaStateCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBC4AE0.
Component* RBMetaStateCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xBC4AF0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBMetaStateCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBMetaStateCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBMetaStateCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xBC4C10.
PropRegistry& RBMetaStateCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xBC4C20.
ComMetaData& RBMetaStateCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBC50D0.
HelpbarButtonOverrideMsg::HelpbarButtonOverrideMsg(JoypadAction action, Symbol label, Symbol icon)
    : Message<3>(Event()) {
    mData->Node(2) = DataNode(static_cast<int>(action));
    mData->Node(3) = DataNode(label);
    mData->Node(4) = DataNode(icon);
}

// Reconstructed from eboot.elf at 0xBAA980. The binary emits the factory
// with the other rb_meta factories.
Component* RBMetaStateCom::_Create() {
    return new RBMetaStateCom();
}

// Reconstructed from eboot.elf at 0xB840D0. The binary emits the
// registration with the other rb_meta classes'.
void RBMetaStateCom::Init() {
    InitComponentClass<RBMetaStateCom>();
}
