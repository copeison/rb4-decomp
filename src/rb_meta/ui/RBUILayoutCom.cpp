// rb_meta/RBUILayoutCom.o (0xCB49F0 to 0xCB5B0F). The std::function of
// _DoInputRegistration's scene search (vtable 0x19784A0, call operator
// 0xCB5980) is the compiler's. The static initializer is at 0xCB5A10; the
// shared header statics it sets first (0x1B0FC08-0x1B0FC5C) belong to
// headers that are not modelled. The class's registration and factory are
// emitted elsewhere (0xB936F0, 0xBACBE0).
#include "rb_meta/ui/RBUILayoutCom.h"

#include <functional>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "persistence/saveload/SaveLoadMgr.h"
#include "rb_meta/character/RBCharacterLayoutTrackerCom.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/ui/RBUIInputCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "ui/input/UIMsg.h"
#include "ui/layout/UILayoutResource.h"
#include "ui/manager/UIMgr.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"

namespace {

// The joypad action that goes back (a JoypadAction). Name not in the
// reference map.
constexpr int kActionCancel = 2;

// The frames a layout holds its drawing back at least, and at most. Names
// not in the reference map.
constexpr int kMinFramesToSkipDraw = 10;
constexpr int kMaxFramesToSkipDraw = 300;

// The layout resource of the component's entity, or null. Name not in the
// reference map.
UILayoutResource* LayoutResource(const Component& component) {
    return static_cast<UILayoutResource*>(component.mObject->mEntity->mResource);
}

}  // namespace

// The object's statics, in the order of its static initializer (0xCB5A10).
Symbol RBUILayoutCom::sId("RBUILayoutCom");
Symbol RBUILayoutCom::sClassName("RBUILayoutCom");
PropRegistry RBUILayoutCom::sPropRegistry;
ComMetaData RBUILayoutCom::sMetaData;

// Reconstructed from eboot.elf at 0xCB49F0.
void RBUILayoutCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBUILayoutCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    metadata.mDescription = "Allows screens to set state about themselves.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(UILayoutResource::Id());

    IntMetadata<int>& layout = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("layout", offsetof(RBUILayoutCom, mLayout), kPropertyInt, sizeof(RBUILayout))
             .mMetadata.Get());
    layout.mDefault = prototype.mLayout;
    static_cast<void>(Symbol("prop"));
    layout.mAllowedValues = RBMetaState::GetAllowedLayoutValues(false);

    IntMetadata<int>& goBackToLayout = static_cast<IntMetadata<int>&>(
        *registry
             .RegisterProp(
                 "go_back_to_layout", offsetof(RBUILayoutCom, mGoBackToLayout), kPropertyInt, sizeof(RBUILayout))
             .mMetadata.Get());
    goBackToLayout.mDefault = prototype.mGoBackToLayout;
    static_cast<void>(Symbol("prop"));
    goBackToLayout.mAllowedValues = RBMetaState::GetAllowedLayoutValues(true);

    IntMetadata<int>& initialFramesToSkipDraw = TypeSpecificMetadata(
        registry.RegisterProp(
            "initial_frames_to_skip_draw",
            offsetof(RBUILayoutCom, mInitialFramesToSkipDraw),
            kPropertyInt,
            sizeof(int)),
        prototype.mInitialFramesToSkipDraw);
    initialFramesToSkipDraw.mDefault = prototype.mInitialFramesToSkipDraw;
    static_cast<void>(Symbol("prop"));
    initialFramesToSkipDraw.mDefault = 0;
    initialFramesToSkipDraw.mMin = 0;
    initialFramesToSkipDraw.mMax = kMaxFramesToSkipDraw;
}

// Reconstructed from eboot.elf at 0xCB4E80. The layout defaults to the
// first one, kLayoutTitle.
RBUILayoutCom::RBUILayoutCom()
    : mRuntime(), mLayout(kLayoutTitle), mGoBackToLayout(kLayoutInvalid), mInitialFramesToSkipDraw(0) {}

// Reconstructed from eboot.elf at 0xCB4EF0. The deleting destructor is at
// 0xCB4F50. A linked subscription leaves its source before the link
// unlinks itself.
RBUILayoutCom::~RBUILayoutCom() {
    LinkedList::Node& link = mRuntime.mTransitionStartedSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mRuntime.mTransitionStartedSink.mSource->RemoveSink(&mRuntime.mTransitionStartedSink);
    }
}

// Reconstructed from eboot.elf at 0xCB4FC0.
void RBUILayoutCom::_Enter() {
    theRBMetaState._SetCurrentLayout(mLayout);
    _DoInputRegistration();
    static Symbol transitionStarted;
    if (transitionStarted == Symbol()) {
        transitionStarted = Symbol("transition_started");
    }
    static Symbol onTransitionStarted;
    if (onTransitionStarted == Symbol()) {
        onTransitionStarted = Symbol("on_transition_started");
    }
    if (theUI != nullptr) {
        // Component begins with MsgSink's slots; the model does not derive
        // it.
        theUI->AddSink(
            &mRuntime.mTransitionStartedSink, reinterpret_cast<MsgSink*>(this), transitionStarted, onTransitionStarted);
    }
    const int frames = mInitialFramesToSkipDraw;
    _SetFramesToSkipDraw(frames > kMinFramesToSkipDraw ? frames : kMinFramesToSkipDraw);
    mRuntime.mSkippingDraw = frames > 0;
}

// Reconstructed from eboot.elf at 0xCB5140. The search keeps the last
// instanced scene with a child entity; a scene without an RBUIInputCom
// leaves only the release build's error name.
void RBUILayoutCom::_DoInputRegistration() {
    const UILayoutResource* const layout = LayoutResource(*this);
    if (layout == nullptr) {
        return;
    }
    RndSceneInstanceCom* scene = nullptr;
    const std::function<void(RndSceneInstanceCom*)> findScene = [&scene](RndSceneInstanceCom* instance) {
        if (instance != nullptr && instance->mRuntime.mEntity != nullptr) {
            scene = instance;
        }
    };
    layout->ForEachScene(findScene);
    if (scene == nullptr) {
        return;
    }
    RBUIInputCom* const input = scene->mRuntime.mEntity->GetRoot()->GetCom<RBUIInputCom>();
    if (input != nullptr) {
        mRuntime.mInputCom = input;
        input->RegisterCom(this);
    } else {
        static_cast<void>(MakeErrorName());
    }
}

// Reconstructed from eboot.elf at 0xCB52A0.
void RBUILayoutCom::_SetFramesToSkipDraw(int frames) {
    UILayoutResource* const layout = LayoutResource(*this);
    const int atLeast = frames > 0 ? frames : 0;
    const int clamped = atLeast < kMaxFramesToSkipDraw ? atLeast : kMaxFramesToSkipDraw;
    if (layout != nullptr) {
        layout->mFramesToSkipDraw = clamped;
        if (clamped != 0) {
            mRuntime.mSkippingDraw = true;
        }
    }
}

// Reconstructed from eboot.elf at 0xCB52E0.
void RBUILayoutCom::_Poll() {
    if (!mRuntime.mSkippingDraw) {
        return;
    }
    const RBCharacterLayoutTrackerCom* const tracker =
        mObject->mEntity->GetRoot()->GetCom<RBCharacterLayoutTrackerCom>();
    if (tracker == nullptr) {
        mRuntime.mSkippingDraw = false;
        _SetFramesToSkipDraw(kMinFramesToSkipDraw);
    } else if (!tracker->AnyPlayerEntityIsLoading()) {
        _SetFramesToSkipDraw(kMinFramesToSkipDraw);
        mRuntime.mSkippingDraw = false;
    }
}

// Reconstructed from eboot.elf at 0xCB53A0.
void RBUILayoutCom::_UndoInputRegistration() {
    if (mRuntime.mInputCom != nullptr) {
        mRuntime.mInputCom->UnregisterCom(this);
    }
}

// Reconstructed from eboot.elf at 0xCB53C0.
DataNode RBUILayoutCom::OnMsg(const UIButtonDownMsg& msg) {
    if (msg.GetAction() == kActionCancel && mGoBackToLayout != kLayoutInvalid) {
        theRBMetaState.Goto(mGoBackToLayout, false);
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xCB5420. The options are cleaned through
// their vtable.
DataNode RBUILayoutCom::_OnTransitionStarted(const Symbol& from, const Symbol& to) {
    static_cast<void>(from);
    static_cast<void>(to);
    _UndoInputRegistration();
    if (mRuntime.mTransitionStartedSink.mSource != nullptr) {
        mRuntime.mTransitionStartedSink.mSource->RemoveSink(&mRuntime.mTransitionStartedSink);
    }
    theSaveLoadMgr->SaveAll(false);
    SystemOptions* const options = &theRBProfileMgr->mSystemOptions;
    options->MakeClean();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xCB54A0.
DataNode RBUILayoutCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == UIButtonDownMsg::Event()) {
        DataNode result = OnMsg(UIButtonDownMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol onTransitionStarted;
    if (onTransitionStarted == Symbol()) {
        onTransitionStarted = Symbol("on_transition_started");
    }
    if (type == onTransitionStarted) {
        return _OnTransitionStarted(msg->Sym(2), msg->Sym(3));
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xCB5700.
Symbol RBUILayoutCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xCB5710.
Symbol RBUILayoutCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xCB5720.
int RBUILayoutCom::CurrentRev() const {
    return const_cast<RBUILayoutCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xCB5740.
bool RBUILayoutCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xCB5770.
Component* RBUILayoutCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xCB5780. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBUILayoutCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBUILayoutCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBUILayoutCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xCB58E0.
PropRegistry& RBUILayoutCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xCB58F0.
ComMetaData& RBUILayoutCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBACBE0. The binary emits the factory
// with the other rb_meta factories.
Component* RBUILayoutCom::_Create() {
    return new RBUILayoutCom();
}

// Reconstructed from eboot.elf at 0xB936F0. The binary emits the
// registration with the other rb_meta classes'.
void RBUILayoutCom::Init() {
    InitComponentClass<RBUILayoutCom>();
}
