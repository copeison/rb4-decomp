#include "rb_game/users/RBBandUser.h"

#include "audio/fmod/system/FmodPlatform.h"
#include "net/OnlineID.h"
#include "os/locale/Locale.h"
#include "rb_game/options/GameplayOptions.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_world/characters/RBCharManager.h"
#include "rb_world/characters/RBOutfitPieceLoader.h"
#include "utl/text/MakeString.h"

// Reconstructed from eboot.elf at 0x9A9A00.
RBBandUser::RBBandUser()
    : mPreviewCharType(kCharTypeNone),
      mCurrentCharType(kCharTypeNone),
      mCharUsage(0),
      mCharUsageFlags(),
      mSompRequestState(-1),
      mCharFlags(),
      mSompHost(false),
      mSompLeader(false),
      mSompReady(false),
      mCharIds{-1, -1, -1, -1} {}

// Reconstructed from eboot.elf at 0x9A9AE0.
int RBBandUser::GetPlatformUserId() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        return profile->mPadNum;
    }
    return mPlatformUserId;
}

// Reconstructed from eboot.elf at 0x9A9B10.
void RBBandUser::_Reset() {
    mPreviewCharName = Symbol();
    mPreviewCharType = kCharTypeNone;
    const Symbol none;
    const CharType noType = kCharTypeNone;
    _SetCurrentCharName(none, noType, nullptr);
    mSompParts.clear();
    mSompPartStates.clear();
    mSompRequestState = -1;
    mSompReady = false;
    mCharFlags[0] = false;
    mCharFlags[1] = false;
    mSompHost = false;
    mSompLeader = false;
}

// Reconstructed from eboot.elf at 0x9A9BB0. The profile update is
// _UpdateProfileCurrentCharIfOwner, inlined.
void RBBandUser::_SetCurrentCharName(const Symbol& name, const CharType& type, const UserGuid* owner) {
    mCurrentCharName = name;
    mCurrentCharType = type;
    mCurrentCharOwner = owner != nullptr ? *owner : UserGuid();
    _UpdateProfileCurrentCharIfOwner();
    _UpdateSompMember();
}

// Reconstructed from eboot.elf at 0x9A9CA0.
void RBBandUser::_ApplyTrackVolumeSettings() {
    if (GetTrackType() == kTrackGuitar) {
        FmodBusInterface* const bus = gFmodBusInterface;
        bus->SetVcaVolumeDb("vca:/IGS", 2, GetGameplayOptions()->GetIGSVolumeDecibels(), 0.0f);
    }
}

// Reconstructed from eboot.elf at 0x9A9F50.
bool RBBandUser::IsUsingOtherController() const {
    if (!IsLocal()) {
        return false;
    }
    return ConnectedControllerType() != GetControllerType();
}

// Reconstructed from eboot.elf at 0x9A9F90.
IRBAchievementLogger* RBBandUser::GetAchievementData() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile == nullptr) {
        return nullptr;
    }
    return profile->mAchData;
}

// Reconstructed from eboot.elf at 0x9A9FD0.
const char* RBBandUser::GetPlatformName() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        return profile->GetPlatformName();
    }
    static Symbol guest;
    if (guest == Symbol()) {
        guest = Symbol("guest");
    }
    const char* const localized = Localize(guest, nullptr, 0, TheLocale);
    FormatString name("%s");
    name << localized;
    return name.Str();
}

// Reconstructed from eboot.elf at 0x9AA110.
bool RBBandUser::GetPreviewCharacterDesc(RBChar::RBCharDesc& desc) const {
    if (mPreviewCharName != Symbol()) {
        return _GetCharDesc(mPreviewCharName, mPreviewCharType, mPreviewCharOwner, desc);
    }
    desc = RBChar::RBCharDesc();
    return false;
}

// Reconstructed from eboot.elf at 0x9AA250. The release build only reads
// the names and the owner's id for its reports.
bool RBBandUser::_GetCharDesc(const Symbol& name, const CharType& type, const UserGuid& owner,
                              RBChar::RBCharDesc& desc) const {
    switch (type) {
    case kCharTypeProfile: {
        RBProfile* const profile = GetProfile(false);
        if (profile != nullptr) {
            return profile->mCharacterData.GetCharacterByName(name, desc);
        }
        static_cast<void>(GetPlatformName());
        break;
    }
    case kCharTypePrefab: {
        const RBChar::RBCharDesc* const prefab = theCharManager->GetPrefabCharDesc(name);
        if (prefab != nullptr) {
            desc = *prefab;
            return true;
        }
        break;
    }
    case kCharTypeOtherProfile: {
        if (GetProfile(false) != nullptr) {
            static_cast<void>(GetPlatformName());
        }
        BandUser* const user = theBandUserMgr->GetBandUser(owner, false);
        if (user == nullptr) {
            static_cast<void>(owner.ToShortString());
            break;
        }
        if (user->GetProfile(false) != nullptr && user->GetProfile(false)->mCharacterData.CharacterExists(name)) {
            return user->GetProfile(false)->mCharacterData.GetCharacterByName(name, desc);
        }
        static_cast<void>(user->GetPlatformName());
        break;
    }
    default:
        return false;
    }
    desc = RBChar::RBCharDesc();
    return false;
}

// Reconstructed from eboot.elf at 0x9AA6D0. The release build keeps an
// unused lookup of another user's character's owner.
bool RBBandUser::SetPreviewCharName(const Symbol& name, const CharType& type, const UserGuid* owner) {
    if (name == Symbol()) {
        mPreviewCharName = Symbol();
        mPreviewCharType = kCharTypeNone;
        mPreviewCharOwner = UserGuid();
        return true;
    }
    if (!_VerifyExistenceOfCharacter(name, type, owner)) {
        mPreviewCharName = Symbol();
        mPreviewCharType = kCharTypeNone;
        mPreviewCharOwner = UserGuid();
        return false;
    }
    if (type == kCharTypeOtherProfile) {
        static_cast<void>(theBandUserMgr->GetBandUser(*owner, false));
    }
    mPreviewCharName = name;
    mPreviewCharType = type;
    mPreviewCharOwner = owner != nullptr ? *owner : UserGuid();
    return true;
}

// Reconstructed from eboot.elf at 0x9AA7F0. The release build only reads
// the names and the owner's id for its reports.
bool RBBandUser::_VerifyExistenceOfCharacter(const Symbol& name, const CharType& type,
                                             const UserGuid* owner) const {
    const BandUser* user = this;
    switch (type) {
    case kCharTypeOtherProfile:
        if (owner == nullptr) {
            return false;
        }
        if (GetProfile(false) != nullptr) {
            static_cast<void>(GetPlatformName());
        }
        user = theBandUserMgr->GetBandUser(*owner, false);
        if (user == nullptr) {
            static_cast<void>(owner->ToShortString());
            return false;
        }
        [[fallthrough]];
    case kCharTypeProfile: {
        RBProfile* const profile = user->GetProfile(false);
        if (profile != nullptr && profile->mCharacterData.CharacterExists(name)) {
            return true;
        }
        static_cast<void>(user->GetPlatformName());
        return false;
    }
    case kCharTypePrefab:
        return theCharManager->GetPrefabCharDesc(name) != nullptr;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x9AA8C0.
Symbol RBBandUser::GetCurrentCharName(CharType& type, UserGuid& owner) const {
    type = mCurrentCharType;
    owner = mCurrentCharOwner;
    return mCurrentCharName;
}

// Reconstructed from eboot.elf at 0x9AA8E0.
Symbol RBBandUser::GetCurrentCharName() const {
    return mCurrentCharName;
}

// Reconstructed from eboot.elf at 0x9AA8F0.
bool RBBandUser::IsUsingPrefabChar(const Symbol& name) const {
    return mCurrentCharType == kCharTypePrefab && mCurrentCharName == name;
}

// Reconstructed from eboot.elf at 0x9AA910.
bool RBBandUser::IsUsingProfileChar(const Symbol& name) const {
    return mCurrentCharType == kCharTypeProfile && mCurrentCharName == name;
}

// Reconstructed from eboot.elf at 0x9AA930.
bool RBBandUser::IsUsingOtherProfileChar(const Symbol& name, const UserGuid& owner) const {
    return mCurrentCharType == kCharTypeOtherProfile && mCurrentCharName == name && mCurrentCharOwner == owner;
}

// Reconstructed from eboot.elf at 0x9AA950.
bool RBBandUser::GetCurrentCharacterDesc(RBChar::RBCharDesc& desc) const {
    if (mCurrentCharName != Symbol()) {
        return _GetCharDesc(mCurrentCharName, mCurrentCharType, mCurrentCharOwner, desc);
    }
    desc = RBChar::RBCharDesc();
    return false;
}

// Reconstructed from eboot.elf at 0x9AAA90. The release build keeps an
// unused lookup of another user's character's owner and reads the user's
// name for its report.
bool RBBandUser::SetCurrentCharName(const Symbol& name, const CharType& type, const UserGuid* owner) {
    const CharType noType = kCharTypeNone;
    SetPreviewCharName(Symbol(""), noType, nullptr);
    if (name == mCurrentCharName && type == mCurrentCharType &&
        (type != kCharTypeOtherProfile || (owner != nullptr && mCurrentCharOwner == *owner))) {
        return true;
    }
    _ClearCurrentCharUsage();
    if (name == Symbol()) {
        _SetCurrentCharName(Symbol(), noType, nullptr);
        return true;
    }
    UserGuid ownerGuid;
    if (owner != nullptr) {
        ownerGuid = *owner;
    }
    if (_TryToLockCharacter(name, type, ownerGuid)) {
        if (type == kCharTypeOtherProfile) {
            static_cast<void>(theBandUserMgr->GetBandUser(*owner, false));
        }
        _SetCurrentCharName(name, type, owner);
        return true;
    }
    static_cast<void>(GetPlatformName());
    _SetCurrentCharName(Symbol(), noType, nullptr);
    return false;
}

// Reconstructed from eboot.elf at 0x9AAC00.
void RBBandUser::_ClearCurrentCharUsage() {
    if (mCurrentCharName == Symbol()) {
        return;
    }
    switch (mCurrentCharType) {
    case kCharTypeProfile: {
        RBProfile* const profile = GetProfile(false);
        if (profile != nullptr) {
            profile->mCharacterData.ClearCharInUse(mCurrentCharName, GetUserGuid());
        }
        break;
    }
    case kCharTypePrefab:
        theCharManager->ClearPrefabInUse(mCurrentCharName, GetUserGuid());
        break;
    case kCharTypeOtherProfile: {
        BandUser* const user = theBandUserMgr->GetBandUser(mCurrentCharOwner, false);
        if (user == nullptr) {
            break;
        }
        RBProfile* const profile = user->GetProfile(false);
        if (profile != nullptr) {
            profile->mCharacterData.ClearCharInUse(mCurrentCharName, GetUserGuid());
        }
        break;
    }
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x9AACC0. The release build only reads
// the names and the owner's id for its reports.
bool RBBandUser::_TryToLockCharacter(const Symbol& name, const CharType& type, const UserGuid& owner) {
    if (!_VerifyExistenceOfCharacter(name, type, &owner)) {
        static_cast<void>(GetPlatformName());
        return false;
    }
    switch (type) {
    case kCharTypeProfile:
        return GetProfile(false)->mCharacterData.TryToMarkCharInUse(name, GetUserGuid());
    case kCharTypePrefab:
        return theCharManager->TryToMarkPrefabInUse(name, GetUserGuid());
    case kCharTypeOtherProfile: {
        BandUser* const user = theBandUserMgr->GetBandUser(owner, false);
        if (user == nullptr) {
            static_cast<void>(owner.ToShortString());
            return false;
        }
        return user->GetProfile(false)->mCharacterData.TryToMarkCharInUse(name, GetUserGuid());
    }
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x9AAD90.
bool RBBandUser::SetRandomPrefabChar() {
    const CharType noType = kCharTypeNone;
    SetPreviewCharName(Symbol(""), noType, nullptr);
    _ClearCurrentCharUsage();
    Symbol name;
    if (theCharManager->TryToGetRandomPrefabAndMarkInUse(GetUserGuid(), name)) {
        const CharType prefab = kCharTypePrefab;
        _SetCurrentCharName(name, prefab, nullptr);
        return true;
    }
    static_cast<void>(GetPlatformName());
    _SetCurrentCharName(Symbol(), noType, nullptr);
    return false;
}

// Reconstructed from eboot.elf at 0x9AAE70. The release build only reads
// the names and the owner's id for its reports.
void RBBandUser::CheckCharacterUsage() {
    if (mCurrentCharName == Symbol()) {
        return;
    }
    switch (mCurrentCharType) {
    case kCharTypeProfile: {
        RBProfile* const profile = GetProfile(false);
        if (profile != nullptr && profile->mCharacterData.UserIsUsingChar(GetUserGuid(), mCurrentCharName)) {
            return;
        }
        static_cast<void>(GetPlatformName());
        break;
    }
    case kCharTypePrefab:
        if (theCharManager->UserIsUsingPrefabChar(GetUserGuid(), mCurrentCharName)) {
            return;
        }
        static_cast<void>(GetPlatformName());
        break;
    case kCharTypeOtherProfile: {
        BandUser* const owner = theBandUserMgr->GetBandUser(mCurrentCharOwner, false);
        if (owner == nullptr) {
            static_cast<void>(GetPlatformName());
            static_cast<void>(mCurrentCharOwner.ToShortString());
            break;
        }
        RBProfile* const profile = owner->GetProfile(false);
        if (profile != nullptr && profile->mCharacterData.UserIsUsingChar(GetUserGuid(), mCurrentCharName)) {
            return;
        }
        static_cast<void>(GetPlatformName());
        static_cast<void>(mCurrentCharOwner.ToShortString());
        static_cast<void>(owner->GetPlatformName());
        break;
    }
    default:
        return;
    }
    const CharType noType = kCharTypeNone;
    _SetCurrentCharName(Symbol(), noType, nullptr);
}

// Reconstructed from eboot.elf at 0x9AB010.
void RBBandUser::_ForceClearCurrentCharUsage() {
    const CharType noType = kCharTypeNone;
    _SetCurrentCharName(Symbol(), noType, nullptr);
}

// Reconstructed from eboot.elf at 0x9AB070. The release build only reads
// the user's name for its report.
void RBBandUser::_UpdateProfileCurrentCharIfOwner() {
    RBProfile* const profile = GetProfile(false);
    if (profile == nullptr) {
        return;
    }
    if (mCurrentCharType == kCharTypeOtherProfile) {
        static_cast<void>(GetPlatformName());
        return;
    }
    if (theRBProfileMgr->SlotOwnsProfileSettings(GetSlot()) && mCurrentCharName != Symbol()) {
        profile->mCharacterData.SetSavedCharName(mCurrentCharName, mCurrentCharType == kCharTypePrefab);
    }
}

// Reconstructed from eboot.elf at 0x9AB100.
bool RBBandUser::PushCurrentCharDescToEntity() {
    if (theCharManager == nullptr) {
        return false;
    }
    const bool force = true;
    theCharManager->UpdateCharsForUser(this, force);
    return true;
}

// Reconstructed from eboot.elf at 0x9AB160. PushCurrentCharDescToEntity
// is inlined.
void RBBandUser::SwitchCurrentCharToExistingChar(const bool& allowRandomPrefab) {
    RBProfile* const profile = GetProfile(false);
    Symbol name;
    if (profile != nullptr && profile->mCharacterData.TryToGetFirstNotInUseCharAndMarkIt(GetUserGuid(), name)) {
        const CharType type = kCharTypeProfile;
        _SetCurrentCharName(name, type, nullptr);
    } else if (allowRandomPrefab) {
        SetRandomPrefabChar();
    } else {
        const CharType noType = kCharTypeNone;
        SetCurrentCharName(Symbol(""), noType, nullptr);
    }
    PushCurrentCharDescToEntity();
}

// Reconstructed from eboot.elf at 0x9AB250.
void RBBandUser::SwitchAwayFromOtherProfileCharIfNeeded() {
    if (mCurrentCharType != kCharTypeOtherProfile) {
        return;
    }
    const CharType noType = kCharTypeNone;
    SetCurrentCharName(Symbol(), noType, nullptr);
    const bool allowRandomPrefab = false;
    SwitchCurrentCharToExistingChar(allowRandomPrefab);
}

// Reconstructed from eboot.elf at 0x9AB2D0.
bool RBBandUser::HasAnyProfileChars() {
    RBProfile* const profile = GetProfile(false);
    return profile != nullptr && profile->mCharacterData.GetCharacterCount() != 0;
}

// Reconstructed from eboot.elf at 0x9AB310.
bool RBBandUser::HasProfileCharWithName(const Symbol& name) {
    RBProfile* const profile = GetProfile(false);
    return profile != nullptr && profile->mCharacterData.CharacterExists(name);
}

// Reconstructed from eboot.elf at 0x9AB350.
bool RBBandUser::HasUnviewedPiecesForSlot(const RBChar::RBCharOutfitSlot& slot,
                                          const RBChar::RBCharGender& gender) const {
    const RBProfile* const profile = GetProfile(false);
    if (profile == nullptr || theOutfitPieceLoader == nullptr) {
        return false;
    }
    eastl::vector<Symbol> unviewed;
    profile->mRockShopData.GetUnviewedPieces(unviewed);
    eastl::vector<ResourcePath> files;
    theOutfitPieceLoader->GetRockShopFilesForOutfitSlot(slot, gender, nullptr, files);
    eastl::vector<Symbol> names;
    names.reserve(files.size());
    for (const ResourcePath& file : files) {
        names.push_back(theOutfitPieceLoader->GetShortNameForPiece(file));
    }
    for (const Symbol& piece : unviewed) {
        for (const Symbol& shortName : names) {
            if (piece == shortName) {
                return true;
            }
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x9AB6A0.
bool RBBandUser::HasAnyUnviewedItems() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile == nullptr) {
        return false;
    }
    return profile->mRockShopData.AnyUnviewedPieces();
}

// Reconstructed from eboot.elf at 0x9AB6D0.
bool RBBandUser::HasUnviewedClothing(const RBChar::RBCharGender& gender) const {
    const RBChar::RBCharOutfitSlot top = RBChar::kOutfitSlotTop;
    if (HasUnviewedPiecesForSlot(top, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot bottom = RBChar::kOutfitSlotBottom;
    if (HasUnviewedPiecesForSlot(bottom, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot glasses = RBChar::kOutfitSlotGlasses;
    return HasUnviewedPiecesForSlot(glasses, gender);
}

// Reconstructed from eboot.elf at 0x9AB770.
bool RBBandUser::HasUnviewedHairOrMakeup(const RBChar::RBCharGender& gender) const {
    const RBChar::RBCharOutfitSlot hair = RBChar::kOutfitSlotHair;
    if (HasUnviewedPiecesForSlot(hair, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot facialHair = RBChar::kOutfitSlotFacialHair;
    if (HasUnviewedPiecesForSlot(facialHair, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot makeup = RBChar::kOutfitSlotMakeupFullFace;
    return HasUnviewedPiecesForSlot(makeup, gender);
}

// Reconstructed from eboot.elf at 0x9AB810.
bool RBBandUser::HasUnviewedInstruments(const RBChar::RBCharGender& gender) const {
    const RBChar::RBCharOutfitSlot bass = RBChar::kOutfitSlotBass;
    if (HasUnviewedPiecesForSlot(bass, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot guitar = RBChar::kOutfitSlotGuitar;
    if (HasUnviewedPiecesForSlot(guitar, gender)) {
        return true;
    }
    const RBChar::RBCharOutfitSlot drums = RBChar::kOutfitSlotDrums;
    return HasUnviewedPiecesForSlot(drums, gender);
}

// Reconstructed from eboot.elf at 0x9AB8B0.
Symbol RBBandUser::GetOnlineName() const {
    OnlineID id;
    GetOnlineID(id);
    const Symbol text(id.ToString());
    if (text != Symbol()) {
        return text;
    }
    return Symbol(GetPlatformName());
}

// Reconstructed from eboot.elf at 0x9AB960.
int RBBandUser::GetSompRequestState() const {
    return mSompRequestState;
}

// Reconstructed from eboot.elf at 0x9AB970.
void RBBandUser::SetSompRequestState(int state) {
    mSompRequestState = state;
}

// Reconstructed from eboot.elf at 0x9AB980.
eastl::vector<Symbol>& RBBandUser::GetSompParts() {
    return mSompParts;
}

// Reconstructed from eboot.elf at 0x9AB990.
eastl::vector<int>& RBBandUser::GetSompPartStates() {
    return mSompPartStates;
}

// Reconstructed from eboot.elf at 0x9AB9A0.
void RBBandUser::ResetSompRequests() {
    mSompRequestState = kSompRequestIdle;
    mSompParts.clear();
    mSompPartStates.clear();
}

// Reconstructed from eboot.elf at 0x9ABA70. The release build keeps the
// local user check and reads the user's pad and name for its report of an
// unexpected answer.
void RBBandUser::SetSompParts(const eastl::vector<Symbol>& parts, const eastl::vector<int>& partStates,
                              bool requested) {
    static_cast<void>(IsLocal());
    if (mSompRequestState != (requested ? kSompPartsRequested : kSompPartsReceived)) {
        static_cast<void>(GetPlatformUserId());
        static_cast<void>(GetPlatformName());
    }
    mSompParts = parts;
    mSompPartStates = partStates;
    mSompRequestState = kSompPartsReceived;
}

// Reconstructed from eboot.elf at 0x9ABC70. The parts are the guitar, the
// bass, the drums, the pro drums, the vocals and the harmonies; the guitar
// parts need a guitar controller in a guitar slot, the drum parts the drums
// in the drum slot, the vocal parts the microphone in the vocal slot. The
// release build reads the user's pad and name for its reports.
void RBBandUser::SetSompPart(int part) {
    static_cast<void>(IsLocal());
    TrackType track = kTrackNone;
    ScoreType score = kScoreBand;
    bool fits = false;
    switch (part) {
    case 0:
    case 1: {
        track = part == 0 ? kTrackGuitar : kTrackBass;
        score = part == 0 ? kScoreGuitar : kScoreBass;
        const ControllerType controller = GetControllerType();
        const unsigned long slot = GetSlot();
        fits = controller == kControllerGuitar && (slot == kGameSlotGuitar0 || slot == kGameSlotGuitar1);
        break;
    }
    case 2:
    case 3: {
        track = kTrackDrum;
        score = part == 2 ? kScoreDrum : kScoreRealDrum;
        const ControllerType controller = GetControllerType();
        const unsigned long slot = GetSlot();
        fits = controller == kControllerDrum && slot == kGameSlotDrum;
        break;
    }
    case 4:
    case 5: {
        track = kTrackVocals;
        score = part == 4 ? kScoreVocals : kScoreHarmony;
        const ControllerType controller = GetControllerType();
        const unsigned long slot = GetSlot();
        fits = controller == kControllerVocals && slot == kGameSlotVocals;
        break;
    }
    default:
        break;
    }
    if (!fits) {
        static_cast<void>(GetPlatformUserId());
        static_cast<void>(GetPlatformName());
        return;
    }
    SetTrackType(track);
    SetPreferredScoreType(score);
    if (mSompRequestState != kSompPartRequested) {
        static_cast<void>(GetPlatformUserId());
        static_cast<void>(GetPlatformName());
    }
    mSompRequestState = kSompPartGranted;
}

// Reconstructed from eboot.elf at 0x9ABE50. The release build reads the
// user's pad and name for its report of an unexpected answer.
void RBBandUser::SetSompDifficulty(Difficulty difficulty) {
    static_cast<void>(IsLocal());
    SetDifficulty(difficulty, false);
    if (mSompRequestState == kSompDifficultyRequested) {
        mSompRequestState = kSompDifficultyGranted;
        return;
    }
    static_cast<void>(GetPlatformUserId());
    static_cast<void>(GetPlatformName());
}

// Reconstructed from eboot.elf at 0x9ABEB0.
Symbol RBBandUser::GetSompPreferenceSymbol() const {
    if (!GetPreferenceFlag0()) {
        return Symbol();
    }
    if (GetPreferenceFlag2()) {
        return GetPickedSymbol();
    }
    return GetPreferenceSymbol();
}

// Reconstructed from eboot.elf at 0x9ABF00.
long RBBandUser::GetSompPreferenceIndex() const {
    if (!GetPreferenceFlag0()) {
        return -1;
    }
    if (GetPreferenceFlag2() || GetPreferenceFlag3()) {
        return GetPickedIndex();
    }
    return GetPreferenceIndex();
}

// Reconstructed from eboot.elf at 0x9ABF70.
RBBandUser::~RBBandUser() {}
