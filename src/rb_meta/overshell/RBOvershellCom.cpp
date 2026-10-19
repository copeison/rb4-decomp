// rb_meta/RBOvershellCom.o (0xBE6F60 to 0xBF727F). The properties'
// accessors are the lambdas after the identity slots (0xBF65B0-0xBF65F0).
// The compiler's code: the EASTL helpers (0xBF6BA0 for the slot vector's
// resize, 0xBF6EB0 for the online set's copy), the std::functions of the
// dialogs' callbacks (vtables 0x1971B60, 0x1971BB0 and 0x1971C00,
// 0xBF6CA0-0xBF7170) and the messages' constructors inlined from their
// headers (0xBF6860, 0xBF6970, 0xBF6A80). The static initializer is at
// 0xBF7180; the shared header statics it sets first (0x1B02988-0x1B029D8)
// belong to headers that are not modelled, apart from the null object id.
// The class's registration and factory are emitted elsewhere (0xB87E00,
// 0xBAB940).
#include "rb_meta/overshell/RBOvershellCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "rb_meta/overshell/RBOvershellSlotCom.h"
#include "render/scene/RndSceneResource.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1B02988, which the static initializer sets
// with the shared header's statics. Each object that uses it has its own
// copy. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Whether the set holds the slot. Name not in the reference map.
bool HasSlot(const eastl::set<unsigned long>& slots, unsigned long slot) {
    return slots.find(slot) != slots.end();
}

// The run-time state's subscriptions, in declaration order. Name not in
// the reference map.
constexpr int kNumSinks = 17;
void GetSinks(RBOvershellCom::RuntimeData& data, MsgSource::EventSinkElem* (&sinks)[kNumSinks]) {
    MsgSource::EventSinkElem* const all[kNumSinks] = {
        &data.mTransitionDoneSink,
        &data.mGameMicsChangedSink,
        &data.mDebugControllerTypesChangedSink,
        &data.mProfileAboutToBeRemovedSink,
        &data.mProfileFailedToSaveLoadSink,
        &data.mPendingProfilesResolvedSink,
        &data.mGameCreatedSink,
        &data.mGameStartedSink,
        &data.mGameEndedSink,
        &data.mGameAboutToBeDestroyedSink,
        &data.mFailScreenPresentedSink,
        &data.mSetSongOptionsRequiredSink,
        &data.mErrorZeroPlayersSink,
        &data.mSystemOverlayStatusChangedSink,
        &data.mGameModeChangedSink,
        &data.mJoypadSink,
        &data.mLostConnectionToServerSink,
    };
    for (int index = 0; index < kNumSinks; ++index) {
        sinks[index] = all[index];
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0xBF7180).
Symbol RBOvershellCom::sId("RBOvershellCom");
Symbol RBOvershellCom::sClassName("RBOvershellCom");
PropRegistry RBOvershellCom::sPropRegistry;
ComMetaData RBOvershellCom::sMetaData;

RBOvershellCom* theOvershell;

// Reconstructed from eboot.elf at 0xBE6F60.
const char* OvershellOnlineStateToString(OvershellOnlineState state) {
    static const char* const kNames[] = {"Invalid", "Offline", "Pending Offline", "Online", "Pending Online"};
    const unsigned int index = static_cast<unsigned int>(state + 1);
    return index <= 4 ? kNames[index] : "UNKNOWN_STATE";
}

// Reconstructed from eboot.elf at 0xBE6F80. Each label is also the
// description.
eastl::vector<AllowedValue<int>> GetOvershellOnlineStateAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(5);
    values.push_back(AllowedValue<int>{kOvershellOnlineStateInvalid, String("Invalid"), String("Invalid")});
    values.push_back(AllowedValue<int>{kOvershellOnlineStateOffline, String("Offline"), String("Offline")});
    values.push_back(AllowedValue<int>{
        kOvershellOnlineStatePendingOffline, String("Pending Offline"), String("Pending Offline")});
    values.push_back(AllowedValue<int>{kOvershellOnlineStateOnline, String("Online"), String("Online")});
    values.push_back(AllowedValue<int>{
        kOvershellOnlineStatePendingOnline, String("Pending Online"), String("Pending Online")});
    return values;
}

// Reconstructed from eboot.elf at 0xBE72D0. The slot vector starts with
// four empty slots; the requests and the queued users reserve room for
// ten.
RBOvershellCom::RuntimeData::RuntimeData()
    : mNeedsFindSlots(true),
      mSystemOverlayOpen(false),
      mSystemOverlayClosed(false),
      mSlots(),
      mJoinOrder(),
      mInputCom(nullptr),
      mLostConnectionToServer(false),
      mOverrideFlow(kOverrideFlowNone),
      mOverrideFlowRequests(),
      mActiveStatus(kOvershellInMeta),
      mInVenueMetaMode(false),
      mSongOptionsRequired(false),
      mUseExtendedMicArrows(false),
      mAllowsButtonPulse(true),
      mQueuedJoinUsers(),
      mPartRand(),
      mRandomSeed(0),
      mIdleMasterGuid(),
      mIdleMasterPadNum(-1),
      mIdleMasterProfile(nullptr),
      mIdleMasterSlot(static_cast<unsigned long>(-1)),
      mGameplayBlocking(false),
      mSompLayoutTransitionBlocking(false),
      mOnlineState(kOvershellOnlineStateOffline),
      mOnlineSlots(),
      mOnlineStateChanged(false),
      mReconnectDialogShowing(false),
      mJoinBlocked(false),
      mOnlineTransitionStartMs(-1.0F),
      mSompDropInStartMs(-1.0F),
      mMovingUser(false),
      mSompLayoutTransition(false),
      mSompLayoutTransitionTime(-1.0F),
      mNeedsUpdateAll(false),
      mNeedsDrawReset(false),
      mLightLayout(kLayoutInvalid) {
    MsgSource::EventSinkElem* sinks[kNumSinks];
    GetSinks(*this, sinks);
    for (MsgSource::EventSinkElem* const sink : sinks) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
    mSlots.resize(4);
    mOverrideFlowRequests.reserve(10);
    mQueuedJoinUsers.reserve(10);
}

// Reconstructed from eboot.elf at 0xBE78E0. Each linked subscription leaves
// its source, the last one first, before the links unlink themselves.
RBOvershellCom::RuntimeData::~RuntimeData() {
    MsgSource::EventSinkElem* sinks[kNumSinks];
    GetSinks(*this, sinks);
    for (int index = kNumSinks - 1; index >= 0; --index) {
        MsgSource::EventSinkElem* const sink = sinks[index];
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0xBE7EE0. The prototype publishes itself
// as theOvershell while the registry is built.
void RBOvershellCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBOvershellCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }
    metadata.mDescription = "Total control of the overshell scene.";
    metadata.mAuthor = "Sheila Porter";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());

    PropInfo& slots = registry.RegisterPropArray(
        "overshell_slots", offsetof(RBOvershellCom, mOvershellSlots), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& slotsMetadata = static_cast<ArrayMetadata&>(*slots.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(slots, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId nullId = {0xFFFFFFFFU};
    GameObjectIdMetadata& slot = TypeSpecificMetadata(slotsMetadata.mItemInfo, nullId);
    static_cast<void>(Symbol("item"));
    slot.mRequiredComs = {RBOvershellSlotCom::sId};
    slotsMetadata.mFixedSize = true;

    SymbolMetadata& status =
        TypeSpecificMetadata(registry.RegisterProp("active_status", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    {
        static Symbol inactive;
        if (inactive == Symbol()) {
            inactive = Symbol("kOvershellInactive");
        }
        static Symbol inMeta;
        if (inMeta == Symbol()) {
            inMeta = Symbol("kOvershellInMeta");
        }
        static Symbol inSong;
        if (inSong == Symbol()) {
            inSong = Symbol("kOvershellInSong");
        }
        eastl::vector<AllowedValue<Symbol>> values;
        values.push_back(AllowedValue<Symbol>{
            inactive, String(inactive.Str()), String("Overshell is inactive (loading screens, etc).")});
        values.push_back(AllowedValue<Symbol>{inMeta, String(inMeta.Str()), String("Game is in a meta screen.")});
        values.push_back(AllowedValue<Symbol>{inSong, String(inSong.Str()), String("In gameplay.")});
        status.mAllowedValues = values;
        // At 0xBF65B0 and 0xBF65C0. The setter writes the status without
        // SetActiveStatus.
        status.mGetter = [](const PropAccessorArgs& args) {
            return OvershellActiveStatusToSymbol(static_cast<RBOvershellCom*>(args.mComponent)->mRuntime.mActiveStatus);
        };
        status.mSetter = [](const PropAccessorArgs& args) {
            static_cast<RBOvershellCom*>(args.mComponent)->mRuntime.mActiveStatus =
                SymbolToOvershellActiveStatus(*static_cast<const Symbol*>(args.mValue));
        };
        status.mSaves = false;
        status.mHistory = false;
    }

    BoolMetadata& showing =
        TypeSpecificMetadata(registry.RegisterProp("should_be_showing", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    // At 0xBF65F0.
    showing.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<RBOvershellCom*>(args.mComponent)->ShouldBeShowing();
    };
    showing.mReadOnly = true;
    showing.mSaves = false;
}

// Reconstructed from eboot.elf at 0xBE8810. The four slot objects start
// null.
RBOvershellCom::RBOvershellCom() : mRuntime(), mOvershellSlots() {
    theOvershell = this;
    mOvershellSlots.Resize(4);
    for (unsigned int index = 0; index < 4; ++index) {
        mOvershellSlots[index] = gNullObjectId;
    }
}

// Reconstructed from eboot.elf at 0xBE88D0. The deleting destructor is at
// 0xBE8970.
RBOvershellCom::~RBOvershellCom() {
    theOvershell = nullptr;
}

// Reconstructed from eboot.elf at 0xBE8B10.
RBOvershellSlotCom* RBOvershellCom::GetSlot(unsigned long index) const {
    if (index >= mRuntime.mSlots.size()) {
        return nullptr;
    }
    return mRuntime.mSlots[index];
}

// Reconstructed from eboot.elf at 0xBE8E20.
void RBOvershellCom::SetSongOptionsRequired(bool required) {
    mRuntime.mSongOptionsRequired = required;
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBE9110.
OvershellActiveStatus RBOvershellCom::GetActiveStatus(bool* inVenueMetaMode) const {
    if (inVenueMetaMode != nullptr) {
        *inVenueMetaMode = mRuntime.mActiveStatus == kOvershellInSong ? mRuntime.mInVenueMetaMode : false;
    }
    return mRuntime.mActiveStatus;
}

// Reconstructed from eboot.elf at 0xBE9130.
void RBOvershellCom::EnterVenueMetaMode() {
    if (!mRuntime.mInVenueMetaMode) {
        SetActiveStatus(kOvershellInMeta, true);
    }
}

// Reconstructed from eboot.elf at 0xBE9250.
void RBOvershellCom::_ChooseActiveStatusFromMetaState() {
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    OvershellActiveStatus status = kOvershellInSong;
    if (layout != kLayoutGame) {
        status = layout != kLayoutLoading ? kOvershellInMeta : kOvershellInactive;
    }
    SetActiveStatus(status, false);
}

// Reconstructed from eboot.elf at 0xBE9280.
void RBOvershellCom::SetUseExtendedMicArrows(bool use) {
    mRuntime.mUseExtendedMicArrows = use;
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBE9290.
void RBOvershellCom::SetAllowsButtonPulse(bool allows) {
    mRuntime.mAllowsButtonPulse = allows;
}

// Reconstructed from eboot.elf at 0xBE92A0.
void RBOvershellCom::SetIdleMasterState(
    const UserGuid& guid,
    int padNum,
    const RBProfile* profile,
    const unsigned long& slot) {
    mRuntime.mIdleMasterGuid = guid;
    mRuntime.mIdleMasterPadNum = padNum;
    mRuntime.mIdleMasterProfile = profile;
    mRuntime.mIdleMasterSlot = slot;
    theRBMetaState.mFocusSlot = slot != static_cast<unsigned long>(-1) ? static_cast<GameSlot>(slot) : kNumGameSlots;
}

// Reconstructed from eboot.elf at 0xBE92E0.
void RBOvershellCom::ClearIdleMaster() {
    mRuntime.mIdleMasterGuid = UserGuid();
    mRuntime.mIdleMasterPadNum = -1;
    mRuntime.mIdleMasterProfile = nullptr;
    mRuntime.mIdleMasterSlot = static_cast<unsigned long>(-1);
    theRBMetaState.mFocusSlot = kNumGameSlots;
}

// Reconstructed from eboot.elf at 0xBEAAB0.
void RBOvershellCom::_Export(DataArray* msg) {
    mObject->mEntity->Export(msg);
}

// Reconstructed from eboot.elf at 0xBEC1E0.
bool RBOvershellCom::InSong() const {
    return mRuntime.mActiveStatus == kOvershellInSong;
}

// Reconstructed from eboot.elf at 0xBEC4C0.
bool RBOvershellCom::_IsInGame() const {
    return mRuntime.mActiveStatus == kOvershellInSong;
}

// Reconstructed from eboot.elf at 0xBECC20.
void RBOvershellCom::QueueUpdateAll() {
    mRuntime.mNeedsUpdateAll = true;
}

// Reconstructed from eboot.elf at 0xBED090.
bool RBOvershellCom::IsOnline() const {
    return static_cast<unsigned int>(mRuntime.mOnlineState - 1) < 2;
}

// Reconstructed from eboot.elf at 0xBEE7F0.
bool RBOvershellCom::IsPendingOnline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline;
}

// Reconstructed from eboot.elf at 0xBEE800.
bool RBOvershellCom::IsPendingOffline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStatePendingOffline;
}

// Reconstructed from eboot.elf at 0xBEF710.
bool RBOvershellCom::IsSlotOnline(unsigned long slot) const {
    if (IsOffline()) {
        return true;
    }
    return HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF790.
bool RBOvershellCom::IsOffline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStateOffline ||
        mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline;
}

// Reconstructed from eboot.elf at 0xBEF7B0.
bool RBOvershellCom::_IsOnlineSlot(unsigned long slot) const {
    return HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF820.
bool RBOvershellCom::IsSlotOffline(unsigned long slot) const {
    if (IsOffline()) {
        return false;
    }
    return !HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF8A0.
void RBOvershellCom::MoveOnlineSlot(unsigned long from, unsigned long to) {
    if (HasSlot(mRuntime.mOnlineSlots, from)) {
        mRuntime.mOnlineSlots.erase(from);
    }
    mRuntime.mOnlineSlots.insert(to);
}

// Reconstructed from eboot.elf at 0xBF07A0.
bool RBOvershellCom::IsNotOnlineSlot(unsigned long slot) const {
    return !HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBF2830.
void RBOvershellCom::_PollQueuedUpdateAll() {
    if (mRuntime.mNeedsUpdateAll) {
        UpdateAll();
        mRuntime.mNeedsUpdateAll = false;
    }
}

// Reconstructed from eboot.elf at 0xBF3C70.
void RBOvershellCom::_EditPoll() {
    if (mRuntime.mNeedsFindSlots) {
        _FindSlots();
        mRuntime.mNeedsFindSlots = false;
    }
}

// Reconstructed from eboot.elf at 0xBF4190.
void RBOvershellCom::_OnSystemOverlayOpened() {
    mRuntime.mSystemOverlayOpen = true;
}

// Reconstructed from eboot.elf at 0xBF41A0. _Poll handles the closed
// overlay once the platform reports it gone.
void RBOvershellCom::_OnSystemOverlayClosed() {
    mRuntime.mSystemOverlayOpen = false;
    mRuntime.mSystemOverlayClosed = true;
}

// Reconstructed from eboot.elf at 0xBF6370.
Symbol RBOvershellCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xBF6380.
Symbol RBOvershellCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xBF6390.
int RBOvershellCom::CurrentRev() const {
    return const_cast<RBOvershellCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xBF63B0.
bool RBOvershellCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBF63E0.
Component* RBOvershellCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xBF6580.
PropRegistry& RBOvershellCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xBF6590.
ComMetaData& RBOvershellCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBAB940. The binary emits the factory
// with the other rb_meta factories.
Component* RBOvershellCom::_Create() {
    return new RBOvershellCom();
}

// Reconstructed from eboot.elf at 0xB87E00. The binary emits the
// registration with the other rb_meta classes'.
void RBOvershellCom::Init() {
    InitComponentClass<RBOvershellCom>();
}
