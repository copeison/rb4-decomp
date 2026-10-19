// rb_meta/RBUIInputCom.o (0xCB1A00 to 0xCB3CDF). The static initializer is
// at 0xCB3B90; the shared header value it sets first (0x1B0F6A8) is kept
// below. The scene focus map's destructor (0xCB3740, 0xCB3750), its list's
// assignment (0xCB37F0) and swap (0xCB39D0), and the std::function of
// "auto_strum_lock"'s hidden tag (vtable 0x1978138, call operator 0xCB3950)
// are the compiler's. The class's registration and factory are emitted
// elsewhere (0xB92B20, 0xBACAF0).
#include "rb_meta/input/RBUIInputCom.h"

#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "os/joypads/JoypadMsgs.h"
#include "os/users/User.h"
#include "rb_game/users/BandUser.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "ui/input/UIInputReceiverCom.h"
#include "ui/lists/UIListCom.h"
#include "ui/lists/UIListData.h"
#include "ui/manager/UIMgr.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"

namespace {

// ---- Declarations from rb_game that its headers do not have yet.
// BandUser derives from User at offset 0; rb_game/users/BandUser.h and
// BandUserMgr.h belong to that module's reconstruction and declare neither
// the base nor the members below.

// BandUser's virtual GetControllerType (slot 8, 0xA3F2B0) and
// GetGameplayOptions (slot 19, 0x92ADB0), and IsLefty(ControllerType)
// (slot 1) of the options it returns (RBGameplayOptions::IsLefty in the
// map). BandUser.h does not declare the class's virtual methods, so the
// calls go through its vtable. Names not in the reference map.
struct BandUserVtable {
    void* mSlots0To7[8];
    int (*mGetControllerType)(const BandUser* user);
    void* mSlots9To18[10];
    const void* (*mGetGameplayOptions)(const BandUser* user);
};
struct GameplayOptionsVtable {
    void* mSlot0;
    bool (*mIsLefty)(const void* options, int controllerType);
};

// The user a BandUser is. Name not in the reference map.
User* AsUser(BandUser* user) {
    return reinterpret_cast<User*>(user);
}

// The binary calls BandUserMgr::GetBandUser(User*) (0xA404F0), which
// returns its argument. Name not in the reference map.
BandUser* AsBandUser(User* user) {
    return reinterpret_cast<BandUser*>(user);
}

// Whether the user plays its controller left-handed:
// user->GetGameplayOptions()->IsLefty(user->GetControllerType()). Name not
// in the reference map.
bool IsLefty(const BandUser* user) {
    const BandUserVtable* const userVtable = *reinterpret_cast<const BandUserVtable* const*>(user);
    const void* const options = userVtable->mGetGameplayOptions(user);
    const GameplayOptionsVtable* const optionsVtable =
        *static_cast<const GameplayOptionsVtable* const*>(options);
    return optionsVtable->mIsLefty(options, userVtable->mGetControllerType(user));
}

// ---- End of the rb_game declarations.

// The id of no object, at 0x1B0F6A8, which the static initializer sets.
// Each object that uses it has its own copy. Name not in the reference
// map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The joypad actions and buttons the filters test (JoypadActionToName's
// "Confirm", "Cancel", "Option", "Up", "Right", "Down", "Left", "Shell
// Option", and "kPad_DUp", "kPad_DDown"). Names not in the reference map.
constexpr JoypadAction kActionNone = static_cast<JoypadAction>(0);
constexpr JoypadAction kActionConfirm = static_cast<JoypadAction>(1);
constexpr JoypadAction kActionCancel = static_cast<JoypadAction>(2);
constexpr JoypadAction kActionOption = static_cast<JoypadAction>(3);
constexpr JoypadAction kActionUp = static_cast<JoypadAction>(6);
constexpr JoypadAction kActionRight = static_cast<JoypadAction>(7);
constexpr JoypadAction kActionDown = static_cast<JoypadAction>(8);
constexpr JoypadAction kActionLeft = static_cast<JoypadAction>(9);
constexpr JoypadAction kActionShellOption = static_cast<JoypadAction>(14);
constexpr JoypadAction kNumActions = static_cast<JoypadAction>(15);
constexpr JoypadButton kPadDUp = static_cast<JoypadButton>(12);
constexpr JoypadButton kPadDDown = static_cast<JoypadButton>(14);

// A strum this soon after the pad's last one is dropped. Name not in the
// reference map.
constexpr float kDoubleStrumMs = 50.0F;

// The pad whose focus a scene remembers. Name not in the reference map.
constexpr int kFocusPad = 0;

// Copies a path over another, reusing its nodes, as EASTL's
// list::DoAssign does (0xCB37F0). utl/containers/List.h's list has no
// assignment. Name not in the reference map.
void AssignPath(eastl::list<GameObjectId>& to, const eastl::list<GameObjectId>& from) {
    eastl::ListNodeBase* node = to.mNode.mpNext;
    const eastl::ListNodeBase* input = from.mNode.mpNext;
    for (; node != &to.mNode && input != &from.mNode; node = node->mpNext, input = input->mpNext) {
        static_cast<eastl::list<GameObjectId>::node_type*>(node)->mValue =
            static_cast<const eastl::list<GameObjectId>::node_type*>(input)->mValue;
    }
    if (input != &from.mNode) {
        for (; input != &from.mNode; input = input->mpNext) {
            to.push_back(static_cast<const eastl::list<GameObjectId>::node_type*>(input)->mValue);
        }
    } else {
        while (node != &to.mNode) {
            node = to.erase(eastl::list<GameObjectId>::iterator(node)).mpNode;
        }
    }
}

// A scene's remembered focus. Name and field names not in the reference
// map.
struct SceneFocusState {
    SceneFocusState() : mHasListFocus(false), mListFocusIndex(static_cast<unsigned long>(-1)) {}
    SceneFocusState(const SceneFocusState& other)
        : mHasListFocus(other.mHasListFocus),
          mListFocusIndex(other.mListFocusIndex),
          mFocusPath(other.mFocusPath),
          mFocusName(other.mFocusName) {}
    SceneFocusState& operator=(const SceneFocusState& other) {
        mHasListFocus = other.mHasListFocus;
        mListFocusIndex = other.mListFocusIndex;
        if (this != &other) {
            AssignPath(mFocusPath, other.mFocusPath);
        }
        mFocusName = other.mFocusName;
        return *this;
    }

    // Whether the focused object holds a UIListCom, and its focused row.
    bool mHasListFocus;
    unsigned long mListFocusIndex;
    // The focused object's id and those of the instancing objects above
    // it, the scene entity's own first.
    eastl::list<GameObjectId> mFocusPath;
    // The focused object's name. Nothing in this build reads it; the name
    // is weak.
    Symbol mFocusName;
};

static_assert(offsetof(SceneFocusState, mFocusPath) == 16);
static_assert(offsetof(SceneFocusState, mFocusName) == 48);
static_assert(sizeof(SceneFocusState) == 56);

using SceneFocusMap = eastl::map<ResourcePath, SceneFocusState>;

}  // namespace

// utl/containers/Map.h's map assigns a new node's value in place and frees
// nodes without destroying them, which a list cannot take; this map
// constructs and destroys its values, as EASTL's does (the node destructor
// is inlined into 0xCB3750 and _ClearSceneFocusState).
namespace eastl {

template <>
SceneFocusMap::node_type* SceneFocusMap::DoCreateNode(const ResourcePath& key) {
    auto* node = static_cast<node_type*>(mAllocator.allocate(sizeof(node_type)));
    new (&node->mValue) value_type{key, SceneFocusState()};
    return node;
}

template <>
void SceneFocusMap::DoFreeNode(node_type* node) {
    node->mValue.~value_type();
    mAllocator.deallocate(node, sizeof(node_type));
}

}  // namespace eastl

namespace {

// The object an id path names: the first id in the entity, the rest in the
// entities its instance components instance. Null for an empty path, the
// null id or a missing object. Name not in the reference map.
GameObject* FindObjectByPath(const Entity* entity, const eastl::list<GameObjectId>& path) {
    if (path.size() == 0) {
        return nullptr;
    }
    eastl::list<GameObjectId> rest(path);
    const GameObjectId id = rest.front();
    rest.erase(rest.begin());
    GameObject* object = id.mId == gNullObjectId.mId ? nullptr : entity->GetObject(id);
    if (object != nullptr && rest.size() != 0) {
        object = FindObjectByPath(object->GetExistingBaseCom<InstanceCom>()->mRuntime.mEntity, rest);
    }
    return object;
}

// Adds the cycles since the last split while the timer runs. Inlined. Name
// not in the reference map.
void SplitTimer(Hmx::Timer& timer) {
    if (timer.mRunning > 0) {
        const unsigned long now = Hmx::Timer::GetCycleCounter();
        timer.mCycles += now - timer.mStart;
        timer.mStart = now;
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0xCB3B90).
Symbol RBUIInputCom::sId("RBUIInputCom");
Symbol RBUIInputCom::sClassName("RBUIInputCom");
PropRegistry RBUIInputCom::sPropRegistry;
ComMetaData RBUIInputCom::sMetaData;

namespace {

// Each scene's remembered focus, by the scene's resource path. Name not in
// the reference map.
SceneFocusMap sSceneFocusStates;  // 0x1B0F908

}  // namespace

// Reconstructed from eboot.elf at 0xCB1A00. The strum timer starts
// running.
RBUIInputCom::RBUIInputCom()
    : mRestoreFocusPending(false),
      mLastAction(kNumActions),
      mFilterButtonDown(true),
      mFilterButtonUp(true),
      mStrumTimer{0, 0, 1},
      mLastStrumMs(),
      mNoDrumNavigationRemap(false),
      mTransitionStartedSink(),
      mIgnoreStickyFocus(false),
      mAutoStrumLock(false),
      mDisableStrumLock(true) {
    mStrumTimer.mStart = Hmx::Timer::GetCycleCounter();
}

RBUIInputCom::RBUIInputCom(const RBUIInputCom& other)
    : UIInputCom(other),
      mRestoreFocusPending(false),
      mLastAction(kNumActions),
      mFilterButtonDown(true),
      mFilterButtonUp(true),
      mStrumTimer{0, 0, 1},
      mLastStrumMs(),
      mNoDrumNavigationRemap(false),
      mTransitionStartedSink(),
      mIgnoreStickyFocus(other.mIgnoreStickyFocus),
      mAutoStrumLock(other.mAutoStrumLock),
      mDisableStrumLock(other.mDisableStrumLock) {
    mStrumTimer.mStart = Hmx::Timer::GetCycleCounter();
}

// Reconstructed from eboot.elf at 0xCB1AC0. UIInputCom's properties come
// first, and the class is found by UIInputCom's interface.
void RBUIInputCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    UIInputCom::_InitAsSuperclass(registry, metadata);
    metadata.mInterface = UIInputCom::sClassName;
    RBUIInputCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }

    BoolMetadata& ignoreStickyFocus = TypeSpecificMetadata(
        registry.RegisterProp(
            "ignore_sticky_focus", offsetof(RBUIInputCom, mIgnoreStickyFocus), kPropertyBool, sizeof(bool)),
        prototype.mIgnoreStickyFocus);
    ignoreStickyFocus.mDefault = prototype.mIgnoreStickyFocus;
    static_cast<void>(Symbol("prop"));
    ignoreStickyFocus.mHelp = String(
        "Turn this on if you want your scene to deal with focus itself, rather than having RBUIInputCom attempt "
        "to handle it generically.");

    BoolMetadata& autoStrumLock = TypeSpecificMetadata(
        registry.RegisterProp("auto_strum_lock", offsetof(RBUIInputCom, mAutoStrumLock), kPropertyBool, sizeof(bool)),
        prototype.mAutoStrumLock);
    autoStrumLock.mDefault = prototype.mAutoStrumLock;
    static_cast<void>(Symbol("prop"));
    autoStrumLock.mHelp = String(
        "Set this to true if you want RBUIInputRecievers in this scene to automatically strumLock when focused.");
    autoStrumLock.mIsHidden = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        return static_cast<const RBUIInputCom&>(args.mComponent).mDisableStrumLock;
    });

    BoolMetadata& disableStrumLock = TypeSpecificMetadata(
        registry.RegisterProp(
            "disable_strum_lock", offsetof(RBUIInputCom, mDisableStrumLock), kPropertyBool, sizeof(bool)),
        prototype.mDisableStrumLock);
    disableStrumLock.mDefault = prototype.mDisableStrumLock;
    static_cast<void>(Symbol("prop"));
    disableStrumLock.mHelp = String(
        "Set to true to disable all use of strum lock in this scene.  (This is equivalent to checking 'disable "
        "strum lock handling' on all RBUIInputReceiverComs in the scene.)");
    disableStrumLock.mDefault = true;
}

// Reconstructed from eboot.elf at 0xCB1F70. The deleting destructor is at
// 0xCB32B0. A linked subscription leaves its source before the link
// unlinks itself.
RBUIInputCom::~RBUIInputCom() {
    LinkedList::Node& link = mTransitionStartedSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mTransitionStartedSink.mSource->RemoveSink(&mTransitionStartedSink);
    }
}

// Reconstructed from eboot.elf at 0xCB1FF0.
bool RBUIInputCom::ShouldAutoStrumLock() const {
    if (mDisableStrumLock) {
        return false;
    }
    return mAutoStrumLock;
}

// Reconstructed from eboot.elf at 0xCB2010.
bool RBUIInputCom::IsStrumLockEnabled() const {
    return !mDisableStrumLock;
}

// Reconstructed from eboot.elf at 0xCB2020.
void RBUIInputCom::_Enter() {
    UIInputCom::_Enter();
    mRestoreFocusPending = true;
    static Symbol transitionStarted;
    if (transitionStarted == Symbol()) {
        transitionStarted = Symbol("transition_started");
    }
    static Symbol onTransitionStarted;
    if (onTransitionStarted == Symbol()) {
        onTransitionStarted = Symbol("on_transition_started");
    }
    // Component begins with MsgSink's slots; the model does not derive it.
    theUI->AddSink(
        &mTransitionStartedSink, reinterpret_cast<MsgSink*>(this), transitionStarted, onTransitionStarted);
}

// Reconstructed from eboot.elf at 0xCB2150.
void RBUIInputCom::_Poll() {
    UIInputCom::_Poll();
    if (mRestoreFocusPending) {
        _RestoreSceneFocusState();
        mRestoreFocusPending = false;
    }
}

// Reconstructed from eboot.elf at 0xCB2180. The saved object and its
// receiver are expected to exist.
void RBUIInputCom::_RestoreSceneFocusState() {
    if (mIgnoreStickyFocus) {
        return;
    }
    const Entity* const entity = mObject->mEntity;
    const EntityResource* const resource = entity->mResource;
    if (resource == nullptr) {
        return;
    }
    const auto found = sSceneFocusStates.find(resource->mPath);
    if (found == sSceneFocusStates.end()) {
        return;
    }
    const SceneFocusState& state = found->second;
    GameObject* const object = FindObjectByPath(entity, state.mFocusPath);
    UIInputReceiverCom* const receiver = object->GetBaseCom<UIInputReceiverCom>();
    SetFocusedObject(receiver, kFocusPad, false);
    SetFocusFirstOnEnter(false);
    if (state.mHasListFocus) {
        UIListCom* const list = receiver->mObject->GetCom<UIListCom>();
        const UIListData* const data = list->mRuntime.mData;
        if (data != nullptr && state.mListFocusIndex < data->GetNumRows()) {
            list->SetFocusByDataIndex(state.mListFocusIndex);
        }
    }
}

// Reconstructed from eboot.elf at 0xCB2340.
void RBUIInputCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
}

// Reconstructed from eboot.elf at 0xCB2350. With drum navigation, a held
// pad shift button turns Option into Up and Shell Option into Down;
// without it, a held pad or cymbal shift button leaves no action. Neither
// applies while a cutscene or a vote shows. A lefty player's directions
// are flipped when the controller type flips them.
JoypadAction RBUIInputCom::_FilterAction(BandUser* user, JoypadAction action) {
    const int padNum = AsUser(user)->GetPadNum();
    const Symbol type = JoypadControllerTypePadNum(padNum);
    static Symbol none;
    if (none == Symbol()) {
        none = Symbol("none");
    }
    if (type == none) {
        return action;
    }
    if (!theRBMetaState.IsWatchingCutsceneOrVoting()) {
        const JoypadButton padShift = JoypadTypePadShiftButton(type);
        const JoypadButton cymbalShift = JoypadTypeCymbalShiftButton(type);
        const unsigned int buttons = JoypadGetPadData(padNum)->mButtons;
        const unsigned int padShiftHeld = buttons & (1U << padShift);
        if (theRBProfileMgr->mSystemOptions.mIsDrumNavigationAllowed) {
            if (padShiftHeld != 0 && !mNoDrumNavigationRemap) {
                if (action == kActionOption) {
                    return kActionUp;
                }
                if (action == kActionShellOption) {
                    return kActionDown;
                }
            }
        } else if (padShiftHeld != 0 || (buttons & (1U << cymbalShift)) != 0) {
            return kActionNone;
        }
    }
    if (JoypadTypeHasLeftyFlip(type) && IsLefty(user)) {
        switch (action) {
        case kActionUp:
            return kActionDown;
        case kActionRight:
            return kActionLeft;
        case kActionDown:
            return kActionUp;
        case kActionLeft:
            return kActionRight;
        default:
            break;
        }
    }
    return action;
}

// Reconstructed from eboot.elf at 0xCB2550. Only the strum buttons (the
// d-pad's up and down) count; every press splits the timer.
bool RBUIInputCom::_IsDoubleStrum(BandUser* user, JoypadButton button) {
    SplitTimer(mStrumTimer);
    if (button != kPadDUp && button != kPadDDown) {
        return false;
    }
    const int padNum = AsUser(user)->GetPadNum();
    const float now = static_cast<float>(Hmx::Timer::CyclesToMs(mStrumTimer.mCycles));
    const float elapsed = now - mLastStrumMs[padNum];
    mLastStrumMs[padNum] = now;
    return elapsed < kDoubleStrumMs;
}

// Reconstructed from eboot.elf at 0xCB25E0. The path ends at the focused
// object and climbs through the instancing objects to the scene's entity.
void RBUIInputCom::_SaveSceneFocusState() {
    if (mIgnoreStickyFocus) {
        return;
    }
    const EntityResource* const resource = mObject->mEntity->mResource;
    if (resource == nullptr) {
        return;
    }
    // The binary calls UIInputCom::FindFocus(int) const (0x89D8E0), which
    // returns the receiver.
    const ReceiverRef* const focus = FindFocus(kFocusPad);
    UIInputReceiverCom* const receiver = focus != nullptr ? focus->mReceiver : nullptr;
    if (receiver == nullptr) {
        return;
    }
    SceneFocusState state;
    state.mFocusName = receiver->mObject->mName;
    const Entity* const entity = mObject->mEntity;
    eastl::list<GameObjectId> path;
    const GameObject* object = receiver->mObject;
    path.push_back(object->mId);
    while (object->mEntity != entity) {
        object = object->mEntity->mParentObject;
        path.push_front(object->mId);
    }
    // EASTL's move assignment: clear, then swap.
    state.mFocusPath.clear();
    state.mFocusPath.swap(path);
    // The binary calls UIListCom::GetDataFocusIndex(bool) const (0x8B7E40).
    const UIListCom* const list = receiver->mObject->GetCom<UIListCom>();
    if (list != nullptr) {
        state.mHasListFocus = true;
        state.mListFocusIndex = static_cast<unsigned long>(static_cast<long>(list->GetDataFocusIndex(true)));
    }
    sSceneFocusStates[resource->mPath] = state;
}

// Reconstructed from eboot.elf at 0xCB2CB0.
void RBUIInputCom::_ClearSceneFocusState() {
    if (mIgnoreStickyFocus) {
        return;
    }
    const EntityResource* const resource = mObject->mEntity->mResource;
    if (resource == nullptr) {
        return;
    }
    const auto found = sSceneFocusStates.find(resource->mPath);
    if (found != sSceneFocusStates.end()) {
        sSceneFocusStates.erase(found);
    }
}

// Reconstructed from eboot.elf at 0xCB2DE0. A press of Cancel marks the
// meta state's back press. A press whose action the filter changes goes on
// as a new message with the filtered action; a double strum is handled
// here and dropped.
DataNode RBUIInputCom::OnMsg(const ButtonDownMsg& msg) {
    BandUser* const user = AsBandUser(msg.GetUser());
    mLastAction = msg.GetAction();
    if (mLastAction == kActionCancel) {
        theRBMetaState.mBackPressed = true;
    }
    if (mFilterButtonDown) {
        const JoypadAction action = _FilterAction(user, msg.GetAction());
        if (action != msg.GetAction()) {
            mFilterButtonDown = false;
            DataNode result;
            {
                const ButtonDownMsg filtered(AsUser(user), msg.GetButton(), action, msg.GetPadNum(), msg.GetValue());
                result = UIInputCom::OnMsg(filtered);
            }
            mFilterButtonDown = true;
            return result;
        }
    }
    if (_IsDoubleStrum(user, msg.GetButton())) {
        return DataNode(0);
    }
    return UIInputCom::OnMsg(msg);
}

// Reconstructed from eboot.elf at 0xCB2FF0. A release whose action the
// filter changes goes on as a new message with the filtered action.
DataNode RBUIInputCom::OnMsg(const ButtonUpMsg& msg) {
    if (mFilterButtonUp) {
        BandUser* const user = AsBandUser(msg.GetUser());
        const JoypadAction action = _FilterAction(user, msg.GetAction());
        if (action != msg.GetAction()) {
            mFilterButtonUp = false;
            DataNode result;
            {
                const ButtonUpMsg filtered(AsUser(user), msg.GetButton(), action, msg.GetPadNum(), msg.GetValue());
                result = UIInputCom::OnMsg(filtered);
            }
            mFilterButtonUp = true;
            return result;
        }
    }
    return UIInputCom::OnMsg(msg);
}

// Reconstructed from eboot.elf at 0xCB3130.
DataNode RBUIInputCom::_OnTransitionStarted(const Symbol& from, const Symbol& to) {
    static_cast<void>(from);
    static_cast<void>(to);
    if (mLastAction == kActionConfirm) {
        _SaveSceneFocusState();
    } else {
        _ClearSceneFocusState();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xCB3170. UIInputCom handles the other
// messages without warnings.
DataNode RBUIInputCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onTransitionStarted;
    if (onTransitionStarted == Symbol()) {
        onTransitionStarted = Symbol("on_transition_started");
    }
    if (type == onTransitionStarted) {
        return _OnTransitionStarted(msg->Sym(2), msg->Sym(3));
    }
    DataNode result = UIInputCom::Handle(msg, false);
    if (result.Type() != kDataUnhandled) {
        return result;
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xCB3330.
Symbol RBUIInputCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xCB3340.
Symbol RBUIInputCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xCB3350.
int RBUIInputCom::CurrentRev() const {
    return const_cast<RBUIInputCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xCB3370.
bool RBUIInputCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xCB33A0.
Component* RBUIInputCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xCB33B0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBUIInputCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBUIInputCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBUIInputCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xCB3720.
PropRegistry& RBUIInputCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xCB3730.
ComMetaData& RBUIInputCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBACAF0. The binary emits the factory
// with the other rb_meta factories.
Component* RBUIInputCom::_Create() {
    return new RBUIInputCom();
}

// Reconstructed from eboot.elf at 0xB92B20. The binary emits the
// registration with the other rb_meta classes'.
void RBUIInputCom::Init() {
    InitComponentClass<RBUIInputCom>();
}
