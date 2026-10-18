// stategraph/StateTransition.o (0x7F5220 to 0x7FB537). The class's
// registration and factory are emitted in stategraph/StateGraph.o. The
// registry builder (0x7F5420) and its tag functions (0x7F9620 on) are not
// reconstructed.
#include "stategraph/transitions/StateTransition.h"

#include <math.h>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropArray.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/runtime/StateGraphInstance.h"

// The object's statics, in the order of its static initializer (0x7FB480).
Symbol StateTransition::sId("StateTransition");
Symbol StateTransition::sClassName("StateTransition");
PropRegistry StateTransition::sPropRegistry;
ComMetaData StateTransition::sMetaData;

// Reconstructed from eboot.elf at 0x7F5220. The blend curve takes the
// default float waveform, drawing its evaluation state twice: for the
// temporary and for the copy.
StateTransition::StateTransition()
    : mOriginState{0xFFFFFFFFU},
      mTargetState{0xFFFFFFFFU},
      mCondition{0xFFFFFFFFU},
      mDuration(),
      mUnits(4),
      mBlendCurve(Waveform<float>(GetDefaultFloatWaveformResource().Get())),
      mCanInterrupt(true),
      mCanBeInterrupted(false),
      mHsvColorBlending(false),
      mBlendMode(kBlendAnimation),
      mRotationBlending(0),
      mAnimationBlendStartTime(0.9F),
      mAnimationBlendDuration(0.1F),
      mAnimationBlendOffset(0.0F),
      mRuntime{nullptr, nullptr, nullptr} {}

// Reconstructed from eboot.elf at 0x7F7C40. The deleting destructor is at
// 0x7F8A30; the blend curve releases its resources.
StateTransition::~StateTransition() {}

// Reconstructed from eboot.elf at 0x7F7C90.
void StateTransition::_Fixup() {
    mRuntime.mOriginNode = _GetObjectBaseCom<StateNode>(mOriginState, true);
    mRuntime.mTargetNode = _GetObjectBaseCom<StateNode>(mTargetState, true);
    mRuntime.mCondition = _GetObjectBaseCom<StateCondition>(mCondition, true);
}

// Reconstructed from eboot.elf at 0x7F7CF0.
void StateTransition::SetOrigin(GameObjectId origin) {
    mOriginState = origin;
    mRuntime.mOriginNode = _GetObjectBaseCom<StateNode>(origin, true);
}

// Reconstructed from eboot.elf at 0x7F7D20.
void StateTransition::SetTarget(GameObjectId target) {
    mTargetState = target;
    mRuntime.mTargetNode = _GetObjectBaseCom<StateNode>(target, true);
}

// Reconstructed from eboot.elf at 0x7F7D50.
void StateTransition::SetCondition(GameObjectId condition) {
    if (mCondition.mId != condition.mId) {
        mCondition = condition;
        mRuntime.mCondition = _GetObjectBaseCom<StateCondition>(condition, true);
        _SendDisplayTextUpdated();
    }
}

// Reconstructed from eboot.elf at 0x7F7D90.
void StateTransition::_LoadBlendCurve() {
    static Symbol blendCurve;
    if (blendCurve == Symbol()) {
        blendCurve = Symbol("blend_curve");
    }
    const ResourcePath path = GetInlineWaveformPath(*this, PropPath(blendCurve));
    const ResourcePtr<WaveformResource<float>> resource =
        mObject->mEntity->mResource->TryGetInline<WaveformResource<float>>(path);
    if (resource != nullptr) {
        mBlendCurve.mValue.SetResource(resource.Get());
    }
}

// Reconstructed from eboot.elf at 0x7F7FD0.
bool StateTransition::IsTargetExclusive() const {
    return mRuntime.mTargetNode->IsExclusive();
}

// Reconstructed from eboot.elf at 0x7F7FE0.
bool StateTransition::IsTargetBlocking() const {
    return mRuntime.mTargetNode->IsBlocking();
}

// Reconstructed from eboot.elf at 0x7F7FF0.
bool StateTransition::TakesTime() const {
    return IsSynchronized() || mDuration.mDriven || 0.0F < mDuration.mValue;
}

// Reconstructed from eboot.elf at 0x7F8050.
bool StateTransition::IsSynchronized() const {
    return mBlendMode == kBlendAnimation && mRuntime.mOriginNode->HasValidTiming() &&
           mRuntime.mTargetNode->HasValidTiming();
}

// Reconstructed from eboot.elf at 0x7F8090.
float StateTransition::GetStartWindow() const {
    return static_cast<float>(static_cast<int>(mAnimationBlendStartTime)) + 1.0F;
}

// Reconstructed from eboot.elf at 0x7F80B0.
float StateTransition::WrapToStartWindow(float position) const {
    const float window = static_cast<float>(static_cast<int>(mAnimationBlendStartTime)) + 1.0F;
    float wrapped = 0.0F;
    if (window != 0.0F) {
        const float remainder = fmodf(position, window);
        wrapped = remainder < 0.0F ? remainder + window : remainder;
    }
    return wrapped;
}

// Reconstructed from eboot.elf at 0x7F8100.
bool StateTransition::IsFromFolderStart() const {
    const StateNode* const origin = mRuntime.mOriginNode;
    const StateNodeFolder* const folder = origin->mRuntime.mFolderNode;
    return folder != nullptr && folder->mStart.mId == origin->mObject->mId.mId;
}

// Reconstructed from eboot.elf at 0x7F8130.
const StateNodeTiming* StateTransition::GetOriginTiming() const {
    return mRuntime.mOriginNode->GetTimingSource();
}

// Reconstructed from eboot.elf at 0x7F8140.
const StateNodeTiming* StateTransition::GetTargetTiming() const {
    return mRuntime.mTargetNode->GetTimingSource();
}

// Reconstructed from eboot.elf at 0x7F8150.
float StateTransition::ConvertToNodeTiming(const StateNode& node, float position) const {
    const StateNodeTiming* const origin = mRuntime.mOriginNode->GetTimingSource();
    const StateNodeTiming* const timing = node.GetTiming();
    if (origin == nullptr || timing == nullptr) {
        return 0.0F;
    }
    return timing->GetScale(origin->GetUnits(), 120.0F) * position;
}

// Reconstructed from eboot.elf at 0x7F81C0. Either timing missing leaves
// the position; the binary calls both whenever one exists.
void StateTransition::ConvertFromNodeTiming(const StateNode& node, float* position, float nodePosition) const {
    const StateNodeTiming* const origin = mRuntime.mOriginNode->GetTimingSource();
    const StateNodeTiming* const timing = node.GetTiming();
    if (origin == nullptr && timing == nullptr) {
        return;
    }
    const float scale = timing->GetScale(origin->GetUnits(), 120.0F);
    *position = scale != 0.0F ? nodePosition / scale : 0.0F;
}

// Reconstructed from eboot.elf at 0x7F8250.
float StateTransition::ScaleByNodeLength(const StateNode& node, float fraction) const {
    const StateNodeTiming* const timing = node.GetTiming();
    if (timing == nullptr) {
        return 0.0F;
    }
    float length = 0.0F;
    timing->GetLength(length);
    return fraction * length;
}

// Reconstructed from eboot.elf at 0x7F8340.
StateTransitionInstance::StateTransitionInstance(StateGraphInstance& graph, const StateTransition& transition)
    : mGraph(&graph), mTransition(&transition), mElapsed(0.0F), mOriginPosition(0.0F), mSyncProgress(0.0F) {}

// Reconstructed from eboot.elf at 0x7F8360. The synchronized blend's
// progress starts at the origin's position past the start time, wrapped
// into the start window.
bool StateTransitionInstance::Begin(StateNodeInstance* from, StateNodeInstance* to) {
    if (mTransition->IsSynchronized()) {
        to->SetTimingPosition(mTransition->mAnimationBlendOffset);
        mOriginPosition = from->GetTimingPosition();
        const float start = mTransition->mAnimationBlendStartTime;
        const float whole = static_cast<float>(static_cast<int>(start));
        const float window = whole + 1.0F;
        float wrapped = 0.0F;
        if (window != 0.0F) {
            const float remainder = fmodf(mOriginPosition, window);
            wrapped = remainder < 0.0F ? remainder + window : remainder;
        }
        mSyncProgress = (wrapped < start ? wrapped + 1.0F + whole : wrapped) - start;
        return true;
    }
    return mTransition->TakesTime();
}

// Reconstructed from eboot.elf at 0x7F84B0. A synchronized blend follows
// the origin's timing over the blend duration; any other runs for its
// duration in its units. The blend is the curve at the clamped fraction.
bool StateTransitionInstance::Poll(StateNodeInstance* from, StateNodeInstance* to, float* blend) {
    static_cast<void>(to);
    Entity* const entity = mGraph->mEntity;
    float fraction;
    if (mTransition->IsSynchronized()) {
        if (from->mFinished) {
            *blend = 1.0F;
            return false;
        }
        const float position = from->GetTimingPosition();
        mSyncProgress = position - mOriginPosition + mSyncProgress;
        const float duration = mTransition->mAnimationBlendDuration;
        mOriginPosition = position;
        if (duration == 0.0F) {
            *blend = 1.0F;
            return false;
        }
        const float upper = duration > 0.0F ? duration : 0.0F;
        const float lower = duration < 0.0F ? duration : 0.0F;
        fraction = (upper < mSyncProgress ? upper : (mSyncProgress > lower ? mSyncProgress : lower)) / duration;
    } else {
        const int units = mGraph->ResolveTimeUnits(mTransition->mUnits);
        mElapsed += mGraph->mClock->DeltaTime(static_cast<TimeUnits>(units));
        const float duration = mTransition->mDuration.GetValue(entity);
        if (!(duration > 0.0F)) {
            *blend = 1.0F;
            return false;
        }
        const float upper = duration > 0.0F ? duration : 0.0F;
        const float lower = duration < 0.0F ? duration : 0.0F;
        fraction = (upper < mElapsed ? upper : (mElapsed > lower ? mElapsed : lower)) / duration;
    }
    if (fraction == 1.0F) {
        *blend = 1.0F;
        return false;
    }
    Waveform<float> curve = mTransition->mBlendCurve.GetValue(entity);
    *blend = curve.Evaluate(fraction);
    return true;
}

// Reconstructed from eboot.elf at 0x7F8A20.
void StateTransitionInstance::End() {}

// Reconstructed from eboot.elf at 0x7F8A80.
Symbol StateTransition::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7F8A90.
Symbol StateTransition::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7F8AA0.
int StateTransition::CurrentRev() const {
    return const_cast<StateTransition*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7F8AC0.
bool StateTransition::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7F8AF0.
Component* StateTransition::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7F8B00. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateTransition::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateTransition*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateTransition(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7F8E30.
PropRegistry& StateTransition::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7F8E40.
ComMetaData& StateTransition::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x700E70. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateTransition::_Create() {
    return new StateTransition();
}

// Reconstructed from eboot.elf at 0x6FBC40. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateTransition::Init() {
    InitComponentClass<StateTransition>();
}
