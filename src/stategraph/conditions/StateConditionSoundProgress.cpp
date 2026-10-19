// stategraph/StateConditionSoundProgress.o (0x88F1B0 to 0x8903E6). The
// class's registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionSoundProgress.h"

#include <new>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"

// The object's statics, in the order of its static initializer (0x890330).
// The id of no object it first sets (0x1AC77F8) comes from a shared header
// and is not used here.
Symbol StateConditionSoundProgress::sId("StateConditionSoundProgress");
Symbol StateConditionSoundProgress::sClassName("StateConditionSoundProgress");
PropRegistry StateConditionSoundProgress::sPropRegistry;
ComMetaData StateConditionSoundProgress::sMetaData;

// Reconstructed from eboot.elf at 0x88F1B0.
StateConditionSoundProgress::StateConditionSoundProgress()
    : mInstanceName(), mInstanceNameIsGlobal(false), mProgressMode(kProgressFromEnd), mProceedIfNotPlaying(true) {
    mOperator = kLessEqual;
}

// Reconstructed from eboot.elf at 0x88F250. The instance name and the mode
// redraw the condition when they change (0x890200, 0x890210).
void StateConditionSoundProgress::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionSoundProgress prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    SymbolMetadata& instanceName = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name", offsetof(StateConditionSoundProgress, mInstanceName), kPropertySymbol, sizeof(Symbol)),
        prototype.mInstanceName);
    instanceName.mDefault = prototype.mInstanceName;
    static_cast<void>(Symbol("prop"));
    instanceName.mHelp = String("The name by which this particular instance of the sound is interacted with.");
    instanceName.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    BoolMetadata& isGlobal = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name_is_global", offsetof(StateConditionSoundProgress, mInstanceNameIsGlobal), kPropertyBool, 1),
        prototype.mInstanceNameIsGlobal);
    isGlobal.mDefault = prototype.mInstanceNameIsGlobal;
    static_cast<void>(Symbol("prop"));
    isGlobal.mHelp = String(
        "If true, the \"instance name\" is searched for on the list of \"global\" sounds. Otherwise the local list of "
        "sounds is searched.");
    // At 0x8902A0.
    isGlobal.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateConditionSoundProgress&>(args.mComponent).mInstanceName == Symbol();
    });

    IntMetadata<int>& progressMode = TypeSpecificMetadata(
        registry.RegisterProp("progress_mode", offsetof(StateConditionSoundProgress, mProgressMode), kPropertyInt, 4),
        prototype.mProgressMode);
    progressMode.mDefault = prototype.mProgressMode;
    static_cast<void>(Symbol("prop"));
    progressMode.mHelp = String("Reference to use for comparison.");
    progressMode.mDisplayName = String("progress mode");
    progressMode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kProgressFromStart,
            String("From start"),
            String("Use the time from the beginning of the sound, in seconds.")},
        AllowedValue<int>{
            kProgressFromEnd, String("From end"), String("Use the time until the end of the sound, in seconds.")},
        AllowedValue<int>{
            kProgressPercent, String("Percent"), String("Use the percent of the way though the sound (0-100).")}};
    progressMode.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    BoolMetadata& proceed = TypeSpecificMetadata(
        registry.RegisterProp(
            "proceed_if_not_playing", offsetof(StateConditionSoundProgress, mProceedIfNotPlaying), kPropertyBool, 1),
        prototype.mProceedIfNotPlaying);
    proceed.mDefault = prototype.mProceedIfNotPlaying;
    static_cast<void>(Symbol("prop"));
    proceed.mHelp = String(
        "If true, the condition will be satisfied if the named sound is not yet playing, or has already finished "
        "playing. If false, the condition will not be satisfied.");

    StateConditionComparison<float>::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Compares progress through a sound against a specific value or another property.";
    metadata.mAuthor = "Buzz Burrowes";
}

// Reconstructed from eboot.elf at 0x88F9D0.
const char* StateConditionSoundProgress::_GetValueToCompareGeneralDescription() const {
    if (mProgressMode == kProgressFromEnd) {
        return "Seconds until end";
    }
    return mProgressMode == kProgressFromStart ? "Seconds from beginning" : "Percent complete";
}

// Reconstructed from eboot.elf at 0x88FA00.
const char* StateConditionSoundProgress::_GetValueToCompareDescription() const {
    const char* format = mProgressMode == kProgressFromStart ? "Seconds from beginning (%s)" : "Percent complete (%s)";
    if (mProgressMode == kProgressFromEnd) {
        format = "Seconds until end (%s)";
    }
    return (FormatString(format) << mInstanceName).Str();
}

// Reconstructed from eboot.elf at 0x88FAA0.
bool StateConditionSoundProgress::_HasValueToCompare() const {
    return mInstanceName != Symbol();
}

// Reconstructed from eboot.elf at 0x88FAC0.
bool StateConditionSoundProgress::_AllowOperator(ComparisonOperator op) const {
    return static_cast<unsigned int>(op - kLess) < 5;
}

// Reconstructed from eboot.elf at 0x88FAD0.
StateConditionSoundProgress::Instance::Instance(
    const StateConditionSoundProgress& condition,
    StateGraphInstance& graph)
    : StateConditionComparison<float>::Instance(condition, graph), mSoundProgress(&condition) {}

// Reconstructed from eboot.elf at 0x88FB80. The generator is released
// before the comparison.
bool StateConditionSoundProgress::Instance::_IsMet(bool useCached) {
    AudioGenerator* const generator = theSoundManager.LockIfOwned(_GetSoundHandle());
    if (generator == nullptr) {
        return mSoundProgress->mProceedIfNotPlaying;
    }
    --generator->mRefCount;
    return StateConditionComparison<float>::Instance::_IsMet(useCached);
}

// Reconstructed from eboot.elf at 0x88FC70.
unsigned int StateConditionSoundProgress::Instance::_GetSoundHandle() const {
    unsigned int handle = 0;
    unsigned int found;
    StateGraphDriverCom* const driver = mGraph->mEntity->GetRoot()->GetCom<StateGraphDriverCom>();
    if (driver->HasSoundHandle(
            mSoundProgress->mInstanceName,
            found,
            static_cast<SoundHandleScope>(mSoundProgress->mInstanceNameIsGlobal))) {
        handle = found;
    }
    return handle;
}

// Reconstructed from eboot.elf at 0x88FD30. The generator's times are in
// milliseconds; a sound without a length counts as complete.
bool StateConditionSoundProgress::Instance::_GetValueToCompare(float& value) const {
    value = 0.0F;
    AudioGenerator* const generator = theSoundManager.LockIfOwned(_GetSoundHandle());
    if (generator == nullptr) {
        return true;
    }
    value = generator->GetTimelineMs();
    if (mSoundProgress->mProgressMode == kProgressFromStart) {
        value *= 0.001F;
    }
    switch (mSoundProgress->mProgressMode) {
    case kProgressFromEnd:
        value = (generator->GetLengthMs() - value) * 0.001F;
        break;
    case kProgressPercent: {
        const float length = generator->GetLengthMs();
        value = length > 0.0F ? value / length * 100.0F : 100.0F;
        break;
    }
    default:
        break;
    }
    --generator->mRefCount;
    return true;
}

// Reconstructed from eboot.elf at 0x88FEC0.
Symbol StateConditionSoundProgress::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x88FED0.
Symbol StateConditionSoundProgress::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x88FEE0.
int StateConditionSoundProgress::CurrentRev() const {
    return const_cast<StateConditionSoundProgress*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x88FF00.
bool StateConditionSoundProgress::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x88FF30.
Component* StateConditionSoundProgress::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x88FF40. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionSoundProgress::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionSoundProgress*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionSoundProgress(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8900B0.
PropRegistry& StateConditionSoundProgress::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8900C0.
ComMetaData& StateConditionSoundProgress::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x8900D0.
StateConditionInstance* StateConditionSoundProgress::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x8901B0.
int StateConditionSoundProgress::_GetType() const {
    return kSoundProgress;
}

// Reconstructed from eboot.elf at 0x7012F0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionSoundProgress::_Create() {
    return new StateConditionSoundProgress();
}

// Reconstructed from eboot.elf at 0x6FDC70. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionSoundProgress::Init() {
    InitComponentClass<StateConditionSoundProgress>();
}
