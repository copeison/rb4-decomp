// ui/UIInputCom.o (0x89C8B0 to 0x8A481F). The registration and the factory
// are emitted in ui/UI.o. Only the class's structure, its property
// registration and the transition functions the layout uses are
// reconstructed.
#include "ui/input/UIInputCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndSceneResource.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "ui/input/UIInputReceiverCom.h"
#include "ui/layout/UILayoutResource.h"

namespace {

// The null object id of a shared header, which the static initializer sets
// to -1 (0x1AC8748); Exit activates the state graph for no object. Name not
// in the reference map.
GameObjectId gNoObject = {0xFFFFFFFFU};

// The state graph driver on the root of the component's entity, or null.
// Inlined into the transition functions. Name not in the reference map.
StateGraphDriverCom* RootStateGraphDriver(const Component* com) {
    return com->mObject->mEntity->GetRoot()->GetCom<StateGraphDriverCom>();
}

// StateGraphResource::Id(), inlined into _Init: the class id of the state
// graph resource (stategraph/StateGraphResource.o), whose local static is at
// 0x1AB2708. The class is not declared in this tree. Name not in the
// reference map.
Symbol StateGraphResourceId() {
    static Symbol id;
    if (id == Symbol()) {
        id = Symbol("StateGraphResource");
    }
    return id;
}

// The accessors _Init installs (0x8A35B0 to 0x8A36D0). Setting
// "current_focus" asks the next five post-polls to focus the object. Names
// not in the reference map.
int MaxPadNum(const StaticTagArgs& args) {
    static_cast<void>(args);
    return 7;
}

void OnEnterAnimationChanged(const PropAccessorArgs& args) {
    static_cast<void>(args);
}

void OnExitAnimationChanged(const PropAccessorArgs& args) {
    static_cast<void>(args);
}

void SetCurrentFocus(const PropAccessorArgs& args) {
    UIInputCom::RuntimeData& runtime = static_cast<UIInputCom*>(args.mComponent)->mRuntime;
    runtime.mPendingFocus = *static_cast<const GameObjectId*>(args.mValue);
    runtime.mFocusFrames = 5;
}

bool GetCurrentFocusInThisEntity(const PropAccessorArgs& args) {
    const UIInputCom& input = *static_cast<const UIInputCom*>(args.mComponent);
    const UIInputCom::ReceiverRef* const focus = input.FindFocus(0);
    return focus != nullptr && focus->mReceiver != nullptr &&
           input.mObject->mEntity == focus->mEntity;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8A4820).
// The 8 and 4 it sets after the null id come from the same shared header
// and are not modelled.
Symbol UIInputCom::sId("UIInputCom");
Symbol UIInputCom::sClassName("UIInputCom");
PropRegistry UIInputCom::sPropRegistry;
ComMetaData UIInputCom::sMetaData;

// Reconstructed from eboot.elf at 0x89C8B0.
UIInputCom::UIInputCom()
    : mRuntime(),
      mWrapNavigation(false),
      mNavigationDirection(kNavAll),
      mMultiFocus(false),
      mLockedToPadNum(-1),
      mFocusFirstOnEnter(true),
      mIsModal(false),
      mEnterAnimation(),
      mExitAnimation() {}

// Reconstructed from eboot.elf at 0x89CAD0. The deleting destructor is at
// 0x89CB00.
UIInputCom::~UIInputCom() {}

// Reconstructed from eboot.elf at 0x89ED90.
void UIInputCom::Exit() {
    StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver != nullptr && mExitAnimation.mPath != Symbol()) {
        driver->Activate(mExitAnimation, gNoObject);
    }
}

// Reconstructed from eboot.elf at 0x89EE10.
bool UIInputCom::IsEntering() const {
    const StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver == nullptr || mEnterAnimation.mPath == Symbol()) {
        return false;
    }
    return driver->IsActive(mEnterAnimation);
}

// Reconstructed from eboot.elf at 0x89EE90.
bool UIInputCom::IsExiting() const {
    const StateGraphDriverCom* const driver = RootStateGraphDriver(this);
    if (driver == nullptr || mExitAnimation.mPath == Symbol()) {
        return false;
    }
    return driver->IsActive(mExitAnimation);
}

// Reconstructed from eboot.elf at 0x89F430. The events' parameters are
// registered while tag functions that need a component are not allowed.
// "current_focus" is read-only for a multi-focus scene.
void UIInputCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that handles all input from the UI Manager.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mRootOnly = true;
    metadata.mInterface = sClassName;
    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);

    PropRegistry& select = metadata.AddExportedEvent(
        Symbol("ui_select"),
        String("Something with a UIInputReceiver was 'selected'- use the parameters to find out which "
               "object it was!"));
    const GameObjectId objectType = {0xFFFFFFFFU};
    GameObjectIdMetadata& objectId =
        TypeSpecificMetadata(select.RegisterProp("object_id", -1, kPropertyGameObjectId, 0), objectType);
    static_cast<void>(Symbol("param"));
    objectId.mHelp = String("The game object ID that received the select message.");
    const Symbol nameType;
    SymbolMetadata& objectName =
        TypeSpecificMetadata(select.RegisterProp("object_name", -1, kPropertySymbol, 0), nameType);
    static_cast<void>(Symbol("param"));
    objectName.mHelp = String("The game object name that received the select message.");
    const int padType = 0;
    IntMetadata<int>& padNum =
        TypeSpecificMetadata(select.RegisterProp("pad_num", -1, kPropertyInt, 0), padType);
    static_cast<void>(Symbol("param"));
    padNum.mHelp = String("The joypad number associated to the select message.");
    padNum.mMin = 0;
    padNum.mMax._Set(MaxPadNum);

    PropRegistry& transitionDone = metadata.AddExportedEvent(
        Symbol("transition_done"),
        String("A layout<->layout transition is done. Can only check for this event in the 'to' scene "
               "since the 'from' one will be unloaded by this point."));
    const ResourcePath fromType;
    ResourcePathMetadata& from =
        TypeSpecificMetadata(transitionDone.RegisterProp("from", -1, kPropertyResourcePath, 0), fromType);
    static_cast<void>(Symbol("param"));
    from.mHelp = String("The layout file (resource path) that we just came from.");
    from.mResourceId = UILayoutResource::Id();
    const ResourcePath toType;
    ResourcePathMetadata& to =
        TypeSpecificMetadata(transitionDone.RegisterProp("to", -1, kPropertyResourcePath, 0), toType);
    static_cast<void>(Symbol("param"));
    to.mHelp = String("The layout file we transitioned to. Should be the current.");
    to.mResourceId = UILayoutResource::Id();
    SetNonStaticTagFunctionsAllowed(nonStatic);

    UIInputCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& wrap = TypeSpecificMetadata(
        registry.RegisterProp("wrap_navigation", offsetof(UIInputCom, mWrapNavigation), kPropertyBool, 1),
        prototype.mWrapNavigation);
    wrap.mDefault = prototype.mWrapNavigation;
    static_cast<void>(Symbol("prop"));
    wrap.mHelp = String("Should navigation wrap?");

    BoolMetadata& multiFocus = TypeSpecificMetadata(
        registry.RegisterProp("multi_focus", offsetof(UIInputCom, mMultiFocus), kPropertyBool, 1),
        prototype.mMultiFocus);
    multiFocus.mDefault = prototype.mMultiFocus;
    static_cast<void>(Symbol("prop"));
    multiFocus.mHelp = String("Does this input allow multiply focused objects by user/pad number?");

    IntMetadata<int>& lockedPad = TypeSpecificMetadata(
        registry.RegisterProp("locked_to_pad_num", offsetof(UIInputCom, mLockedToPadNum), kPropertyInt, 4),
        prototype.mLockedToPadNum);
    lockedPad.mDefault = prototype.mLockedToPadNum;
    static_cast<void>(Symbol("prop"));
    lockedPad.mHelp = String("Are all the inputs locked to a specific user/pad number?");
    lockedPad.mMin = -1;
    lockedPad.mMax = 8;

    BoolMetadata& focusFirst = TypeSpecificMetadata(
        registry.RegisterProp(
            "focus_first_on_enter", offsetof(UIInputCom, mFocusFirstOnEnter), kPropertyBool, 1),
        prototype.mFocusFirstOnEnter);
    focusFirst.mDefault = prototype.mFocusFirstOnEnter;
    static_cast<void>(Symbol("prop"));
    focusFirst.mHelp = String("Do we focus the first UIInputReceiverCom we find on enter?");

    BoolMetadata& modal = TypeSpecificMetadata(
        registry.RegisterProp("is_modal", offsetof(UIInputCom, mIsModal), kPropertyBool, 1),
        prototype.mIsModal);
    modal.mDefault = prototype.mIsModal;
    static_cast<void>(Symbol("prop"));
    modal.mHelp = String("Should this input be modal?");

    IntMetadata<int>& direction = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "navigation_direction", offsetof(UIInputCom, mNavigationDirection), kPropertyInt, 4).mMetadata.Get());
    direction.mDefault = prototype.mNavigationDirection;
    static_cast<void>(Symbol("prop"));
    direction.mHelp = String("Which direction should support navigation?");
    direction.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kNavVertical,
            String("Vertical"),
            String("Navigation is only done in vertical directions (up, down)")},
        AllowedValue<int>{
            kNavHorizontal,
            String("Horizontal"),
            String("Navigation is only done in horizontal directions (left, right)")},
        AllowedValue<int>{
            kNavGrid,
            String("Grid"),
            String("Navigation is done in all directions (left, right, up, down), and requires "
                   "transforms to line up in a grid.")},
        AllowedValue<int>{
            kNavAll,
            String("All"),
            String("Navigation is done in all directions (left, right, up, down)")}};

    ResourcePathMetadata& enterAnimation = TypeSpecificMetadata(
        registry.RegisterProp(
            "enter_animation", offsetof(UIInputCom, mEnterAnimation), kPropertyResourcePath, 8),
        prototype.mEnterAnimation);
    enterAnimation.mDefault = prototype.mEnterAnimation;
    static_cast<void>(Symbol("prop"));
    enterAnimation.mHelp = String("Which state graph to run when we initially enter the scene.");
    enterAnimation.mResourceId = StateGraphResourceId();
    enterAnimation.mOnChanged = OnEnterAnimationChanged;

    ResourcePathMetadata& exitAnimation = TypeSpecificMetadata(
        registry.RegisterProp(
            "exit_animation", offsetof(UIInputCom, mExitAnimation), kPropertyResourcePath, 8),
        prototype.mExitAnimation);
    exitAnimation.mDefault = prototype.mExitAnimation;
    static_cast<void>(Symbol("prop"));
    exitAnimation.mHelp = String("Which state graph to run when we are exiting the scene.");
    exitAnimation.mResourceId = StateGraphResourceId();
    exitAnimation.mOnChanged = OnExitAnimationChanged;

    const GameObjectId focusType = {0xFFFFFFFFU};
    GameObjectIdMetadata& currentFocus = TypeSpecificMetadata(
        registry.RegisterProp("current_focus", -1, kPropertyGameObjectId, 0), focusType);
    static_cast<void>(Symbol("prop"));
    currentFocus.mHelp = String(
        "Will return the game object ID of the currently focused object (or the GameObject containing "
        "the InstanceCom of the focused object); setting this property will attempt to change the "
        "focused object. Using this property assumes we are not a multi-focused scene.");
    currentFocus.mGetter = GetUIInputCurrentFocus;
    currentFocus.mSetter = SetCurrentFocus;
    currentFocus.mHistory = false;
    currentFocus.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const UIInputCom&>(args.mComponent).mMultiFocus;
    });
    currentFocus.mSaves = false;

    const bool inEntityType = false;
    BoolMetadata& inEntity = TypeSpecificMetadata(
        registry.RegisterProp("current_focus_in_this_entity", -1, kPropertyBool, 0), inEntityType);
    static_cast<void>(Symbol("prop"));
    inEntity.mHelp = String(
        "If TRUE (checked), the currently-focused UIInputReceiverCom is inside the same Entity as this "
        "Com.  Otherwise, it is in a child entity inside an InstanceCom.");
    inEntity.mGetter = GetCurrentFocusInThisEntity;
    inEntity.mReadOnly = true;
    inEntity.mSaves = false;
    inEntity.mHistory = false;
}

// Reconstructed from eboot.elf at 0x8A2E60.
Symbol UIInputCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8A2E70.
Symbol UIInputCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8A2E80.
int UIInputCom::CurrentRev() const {
    return const_cast<UIInputCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8A2EA0.
bool UIInputCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8A2ED0.
Component* UIInputCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8A2EE0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIInputCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIInputCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIInputCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8A31B0.
PropRegistry& UIInputCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8A31C0.
ComMetaData& UIInputCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x8A31D0.
bool UIInputCom::_HasPostPoll() const {
    return true;
}

// Reconstructed from eboot.elf at 0x8A31E0. A linked subscription leaves
// its source before its link unlinks itself.
UIInputCom::RuntimeData::~RuntimeData() {
    for (MsgSource::EventSinkElem* sink :
         {&mJoypadConnectSink, &mButtonUpSink, &mButtonDownSink, &mKeyReleaseSink, &mKeySink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x894FD0. The binary emits the factory
// in ui/UI.o.
Component* UIInputCom::_Create() {
    return new UIInputCom();
}

// Reconstructed from eboot.elf at 0x891BA0. The binary emits the
// registration in ui/UI.o; the class is a superclass root.
void UIInputCom::Init() {
    InitComponentClass<UIInputCom>(true, true);
}
