// stategraph/StateNodeTriggerAudioClip.o (0x7F3A20 to 0x7F5216). The
// class's registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeTriggerAudioClip.h"

#include <new>

#include "audio/core/components/AudioClipCom.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"

namespace {

// The id of no object, at 0x1ABDE58, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The labels of "finish_mode". The text keeps the binary's missing spaces.
// Name not in the reference map.
// Reconstructed from eboot.elf at 0x7F42C0.
eastl::vector<AllowedValue<int>> GetFinishModeAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{
        StateNodeTriggerAudioClip::kFinishImmediately,
        String("Immediately"),
        String("Don't wait for the clip to start or finish.")});
    values.push_back(AllowedValue<int>{
        StateNodeTriggerAudioClip::kFinishOnStart,
        String("On Start"),
        String(
            "Wait for the clip to start. If the clip is set up to start at a specific musical timing itis will not "
            "be \"started\" until that musical time is reached.")});
    values.push_back(AllowedValue<int>{
        StateNodeTriggerAudioClip::kFinishOnEnd,
        String("On End"),
        String(
            "Wait for the clip to be done playing. This will include the time between trigger and start,and the "
            "time it takes the audio clip to play. NOTE: If the clip is a looping clip this nodewill never report "
            "that it is finished.")});
    return values;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x7F5160).
Symbol StateNodeTriggerAudioClip::sId("StateNodeTriggerAudioClip");
Symbol StateNodeTriggerAudioClip::sClassName("StateNodeTriggerAudioClip");
PropRegistry StateNodeTriggerAudioClip::sPropRegistry;
ComMetaData StateNodeTriggerAudioClip::sMetaData;

// Inlined by every user.
inline AudioClipCom* StateNodeTriggerAudioClip::FindAudioClip(Entity* entity) const {
    if (mAudioClip.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* const object = entity->GetObject(mAudioClip);
    return object != nullptr ? object->GetBaseCom<AudioClipCom>() : nullptr;
}

// Reconstructed from eboot.elf at 0x7F3A20. The node waits for the clip to
// end and stops it when left early.
StateNodeTriggerAudioClip::StateNodeTriggerAudioClip()
    : mAudioClip(gNullObjectId), mFinishMode(kFinishOnEnd), mStopOnEarlyExit(true) {}

// Reconstructed from eboot.elf at 0x7F3A60. The tag functions are lambdas
// (0x7F4DF0, 0x7F4FD0 and 0x7F50E0).
void StateNodeTriggerAudioClip::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeTriggerAudioClip prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    GameObjectIdMetadata& audioClip = TypeSpecificMetadata(
        registry.RegisterProp(
            "audio_clip", offsetof(StateNodeTriggerAudioClip, mAudioClip), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mAudioClip);
    audioClip.mDefault = prototype.mAudioClip;
    static_cast<void>(Symbol("prop"));
    audioClip.mHelp = String("The AudioClipCom to trigger.");
    // At 0x7F4DF0.
    audioClip.mRequiredInterfaces._Set([](const TagArgs& args) {
        static_cast<void>(args);
        eastl::vector<Symbol> interfaces;
        interfaces.push_back(AudioClipCom::sClassName);
        return interfaces;
    });
    audioClip.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    audioClip.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();

    IntMetadata<int>& finishMode = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "finish_mode", offsetof(StateNodeTriggerAudioClip, mFinishMode), kPropertyInt, 4).mMetadata.Get());
    finishMode.mDefault = prototype.mFinishMode;
    static_cast<void>(Symbol("prop"));
    // At 0x7F4FD0.
    finishMode.mAllowedValues._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return GetFinishModeAllowedValues();
    });
    finishMode.mDefault = kFinishOnEnd;

    BoolMetadata& stopOnEarlyExit = TypeSpecificMetadata(
        registry.RegisterProp(
            "stop_on_early_exit", offsetof(StateNodeTriggerAudioClip, mStopOnEarlyExit), kPropertyBool, 1),
        prototype.mStopOnEarlyExit);
    stopOnEarlyExit.mDefault = prototype.mStopOnEarlyExit;
    static_cast<void>(Symbol("prop"));
    stopOnEarlyExit.mHelp = String(
        "In spite of setting the \"finish mode\" to something other than \"Immediately\", It is possible a "
        "transition out of this node may be occur before you expect it to. If that happens, should the audio clip "
        "be stopped?");
    stopOnEarlyExit.mDefault = true;
    // At 0x7F50E0.
    stopOnEarlyExit.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeTriggerAudioClip&>(args.mComponent).mFinishMode == kFinishImmediately;
    });

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A node that instantly triggers an AudioClipCom.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("");
}

// Reconstructed from eboot.elf at 0x7F41E0.
const char* StateNodeTriggerAudioClip::GetEditorText() const {
    if (mAudioClip.mId != gNullObjectId.mId) {
        Entity* const entity = _GetContextEntityPrototype();
        if (entity != nullptr) {
            const AudioClipCom* const clip = FindAudioClip(entity);
            if (clip != nullptr) {
                return clip->GetDescription();
            }
        }
    }
    return "(none)";
}

// Reconstructed from eboot.elf at 0x7F44F0. The clip is triggered even
// when the object has none, as the binary does not check.
void StateNodeTriggerAudioClip::Instance::_Enter() {
    mFinished = false;
    Entity* const entity = _GetContextEntity();
    mTriggerAudioClip->FindAudioClip(entity)->Trigger();
}

// Reconstructed from eboot.elf at 0x7F45B0.
bool StateNodeTriggerAudioClip::Instance::_Poll() {
    switch (mTriggerAudioClip->mFinishMode) {
    case kFinishImmediately:
        break;
    case kFinishOnStart:
        if (!_IsClipStarted()) {
            return true;
        }
        break;
    case kFinishOnEnd:
        if (!_IsClipDone()) {
            return true;
        }
        break;
    default:
        mFinished = false;
        return false;
    }
    mFinished = true;
    return false;
}

// Reconstructed from eboot.elf at 0x7F4770.
bool StateNodeTriggerAudioClip::Instance::_IsClipStarted() const {
    Entity* const entity = _GetContextEntity();
    if (entity == nullptr) {
        return false;
    }
    const AudioClipCom* const clip = mTriggerAudioClip->FindAudioClip(entity);
    return clip == nullptr || !clip->WaitsForStart() || clip->HasStarted();
}

// Reconstructed from eboot.elf at 0x7F4840.
bool StateNodeTriggerAudioClip::Instance::_IsClipDone() const {
    Entity* const entity = _GetContextEntity();
    if (entity == nullptr) {
        return false;
    }
    const AudioClipCom* const clip = mTriggerAudioClip->FindAudioClip(entity);
    return clip == nullptr || !clip->IsActive();
}

// Reconstructed from eboot.elf at 0x7F4900.
void StateNodeTriggerAudioClip::Instance::_Exit() {
    if (!mTriggerAudioClip->mStopOnEarlyExit || mFinished) {
        return;
    }
    Entity* const entity = _GetContextEntity();
    if (entity == nullptr) {
        return;
    }
    AudioClipCom* const clip = mTriggerAudioClip->FindAudioClip(entity);
    if (clip != nullptr) {
        clip->Stop();
    }
}

// Reconstructed from eboot.elf at 0x7F4A70.
Symbol StateNodeTriggerAudioClip::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7F4A80.
Symbol StateNodeTriggerAudioClip::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7F4A90.
int StateNodeTriggerAudioClip::CurrentRev() const {
    return const_cast<StateNodeTriggerAudioClip*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7F4AB0.
bool StateNodeTriggerAudioClip::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7F4AE0.
Component* StateNodeTriggerAudioClip::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7F4AF0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeTriggerAudioClip::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeTriggerAudioClip*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeTriggerAudioClip(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7F4CB0.
PropRegistry& StateNodeTriggerAudioClip::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7F4CC0.
ComMetaData& StateNodeTriggerAudioClip::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7F4CD0.
StateNodeInstance* StateNodeTriggerAudioClip::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x7005C0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeTriggerAudioClip::_Create() {
    return new StateNodeTriggerAudioClip();
}

// Reconstructed from eboot.elf at 0x6FAC90. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeTriggerAudioClip::Init() {
    InitComponentClass<StateNodeTriggerAudioClip>();
}
