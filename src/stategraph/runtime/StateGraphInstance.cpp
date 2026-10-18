// stategraph/StateGraphInstance.o (0x703CC0 to 0x706C38).
#include "stategraph/runtime/StateGraphInstance.h"

#include "anim/drivers/AnimDriverCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "math/random/Rand.h"
#include "os/profiling/PerfMgr.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/nodes/StateNodeRoot.h"
#include "stategraph/resources/StateGraphResource.h"
#include "utl/data/DataArray.h"

// Reconstructed from eboot.elf at 0x703CC0. A graph that is not inlined or
// failed to load is reported; the report is compiled out.
StateGraphInstance::StateGraphInstance(Entity* entity, TimeMgr::Clock* clock, ResourcePath path)
    : mEntity(entity),
      mAnimDriver(entity->GetRoot()->GetCom<AnimDriverCom>()),
      mResource(entity->mResource->TryGetInline<StateGraphResource>(path)),
      mRandom(-1.0F),
      mTransitions(),
      mNodeInstances(),
      mPollIndex(static_cast<unsigned long>(-1)),
      mPerfTimer(static_cast<unsigned long>(-1)),
      mScopes(),
      mScopeIndex(static_cast<unsigned long>(-1)),
      mTimingPosition(0.0F),
      mSkipTimeUpdate(true),
      mPollPhase(kPollEveryFrame),
      mClock(clock) {
    if (mResource == nullptr || mResource->Fail()) {
        static_cast<void>(entity->MakeErrorName());
    }
}

// Reconstructed from eboot.elf at 0x703EB0. The deleting destructor is at
// 0x703FD0. The node instances are deleted without exiting.
StateGraphInstance::~StateGraphInstance() {
    while (!mNodeInstances.empty()) {
        delete mNodeInstances.back();
        mNodeInstances.pop_back();
    }
    _PopScopes(0);
}

// Reconstructed from eboot.elf at 0x703FC0.
void StateGraphInstance::_ClearScopes() {
    _PopScopes(0);
}

// Reconstructed from eboot.elf at 0x704000.
StateGraphInstance::TransitionEntry::TransitionEntry(StateTransition* transition)
    : mTransition(transition), mCondition(nullptr), mSynchronized(transition->IsSynchronized()) {}

// Reconstructed from eboot.elf at 0x704030.
StateGraphInstance::TransitionScope::TransitionScope(const StateNode* node)
    : mNode(node), mTransitions(), mElapsed{}, mIncomingEnded(false), mNodeFinished(false) {}

// Reconstructed from eboot.elf at 0x704080. A transition without an origin
// is polled through a null instance, as in the binary.
void StateGraphInstance::Activate(GameObjectId object) {
    if (mResource == nullptr || mResource->Fail()) {
        return;
    }
    const StateNode* node;
    StateTransition* transition = nullptr;
    if (object.mId == 0xFFFFFFFFU) {
        node = mResource->GetRootNode();
    } else {
        GameObject* const graphObject = mResource->mEntity->GetObject(object);
        node = graphObject->GetBaseCom<StateNode>();
        if (node == nullptr) {
            transition = graphObject->GetBaseCom<StateTransition>();
            if (transition == nullptr) {
                return;
            }
            node = transition->mRuntime.mOriginNode;
        }
    }
    StateNodeInstance* instance = nullptr;
    if (node != nullptr) {
        instance = _EnterNode(node);
        _EndIncomingTransitions();
        instance->Enter();
        instance->SetWeights(1.0F, 1.0F);
    }
    if (transition != nullptr) {
        instance->Poll();
        _ActivateTransition(transition);
    }
}

// Reconstructed from eboot.elf at 0x704290.
StateNodeInstance* StateGraphInstance::_EnterNode(const StateNode* node) {
    _PopScopes(_PushScopes(node));
    while (node->GetId() == StateNodeFolder::sId) {
        node = static_cast<const StateNodeFolder*>(node)->mStartNode;
        _PushScope(node);
        StateTransition* const transition = _FindTransition(mScopes.size() - 1, false, true);
        if (transition != nullptr) {
            node = transition->mRuntime.mTargetNode;
            _PopScopes(mScopes.size() - 1);
            _PushScope(node);
        }
    }
    StateNodeInstance* const instance = node->CreateInstance(*this);
    mNodeInstances.insert(mNodeInstances.begin(), instance);
    return instance;
}

// Reconstructed from eboot.elf at 0x7043D0.
void StateGraphInstance::_EndIncomingTransitions() {
    for (mScopeIndex = mScopes.size() - 1; mScopeIndex != static_cast<unsigned long>(-1); --mScopeIndex) {
        bool& ended = mScopes[mScopeIndex].mIncomingEnded;
        if (ended) {
            break;
        }
        ended = true;
    }
    mScopeIndex = static_cast<unsigned long>(-1);
}

// Reconstructed from eboot.elf at 0x704450. A blending target looks for a
// blend partner among the older instances. The target's timing position
// becomes the one the next search starts from.
void StateGraphInstance::_ActivateTransition(StateTransition* transition) {
    mTransitions.insert(mTransitions.begin(), StateTransitionInstance(*this, *transition));
    StateTransitionInstance& running = mTransitions.front();
    StateNodeInstance* const previous = mNodeInstances.front();
    StateNodeInstance* const next = _EnterNode(transition->mRuntime.mTargetNode);
    const bool blending = running.Begin(previous, next);
    if (blending) {
        if (next->WantsBlendPartner()) {
            for (unsigned long index = 1; index < mNodeInstances.size(); ++index) {
                if (next->TakeBlendPartner(mNodeInstances[index])) {
                    break;
                }
            }
        }
    } else {
        while (!mTransitions.empty()) {
            mTransitions.back().End();
            mTransitions.pop_back();
        }
        while (mNodeInstances.size() > 1) {
            StateNodeInstance* const instance = mNodeInstances.back();
            instance->Exit();
            delete instance;
            mNodeInstances.pop_back();
        }
        _EndIncomingTransitions();
    }
    next->Enter();
    const float weight = blending ? 0.0F : 1.0F;
    next->SetWeights(weight, weight);
    mPollIndex = 0;
    next->Poll();
    mPollIndex = static_cast<unsigned long>(-1);
    if (next->mNode->HasValidTiming()) {
        mTimingPosition = next->GetTimingPosition();
    }
}

// Reconstructed from eboot.elf at 0x7046F0.
bool StateGraphInstance::Poll() {
    return _Poll();
}

// Reconstructed from eboot.elf at 0x704700. The timer's use is compiled
// out.
bool StateGraphInstance::PollTimed() {
    if (mPerfTimer != static_cast<unsigned long>(-1)) {
        static_cast<void>(thePerfMgr.GetTimer(mPerfTimer));
    }
    return _Poll();
}

// Reconstructed from eboot.elf at 0x704730. Each transition takes its
// blend's share of what the newer transitions left; the oldest node gets
// the rest. A finished transition drops the older transitions and nodes.
// When no transition runs and the current node finished, the scopes are
// marked finished from the innermost out until one has transitions; with
// none left the graph ends. The reports of a runaway graph are compiled
// out.
bool StateGraphInstance::_Poll() {
    if (mNodeInstances.empty()) {
        return false;
    }
    if (mSkipTimeUpdate) {
        mSkipTimeUpdate = false;
    } else {
        _UpdateElapsedTime();
    }
    if (!mTransitions.empty()) {
        float remaining = 1.0F;
        float blend = 0.0F;
        unsigned long index = 0;
        for (; index < mTransitions.size(); ++index) {
            StateNodeInstance* const next = mNodeInstances[index];
            const bool blending = mTransitions[index].Poll(mNodeInstances[index + 1], next, &blend);
            float share = blend * remaining;
            share = remaining < share ? remaining : (share > 0.0F ? share : 0.0F);
            next->SetWeights(share, blend);
            remaining -= share;
            if (!blending) {
                break;
            }
        }
        if (index == mTransitions.size()) {
            mNodeInstances[index]->SetWeights(remaining, 1.0F - blend);
        } else {
            while (mTransitions.size() > index) {
                mTransitions.back().End();
                mTransitions.pop_back();
            }
            while (mNodeInstances.size() > index + 1) {
                StateNodeInstance* const instance = mNodeInstances.back();
                instance->Exit();
                delete instance;
                mNodeInstances.pop_back();
            }
            _EndIncomingTransitions();
        }
    }
    for (mPollIndex = mNodeInstances.size() - 1; mPollIndex != static_cast<unsigned long>(-1); --mPollIndex) {
        mNodeInstances[mPollIndex]->Poll();
    }
    for (unsigned long taken = 0;;) {
        const bool transitioning = !mTransitions.empty();
        if (transitioning) {
            if (!mTransitions.front().mTransition->mCanBeInterrupted) {
                break;
            }
        } else if (!mScopes.back().mNodeFinished && mNodeInstances.front()->mFinished) {
            TransitionScope* end = mScopes.end();
            while (end != mScopes.begin()) {
                TransitionScope& scope = end[-1];
                scope.mNodeFinished = true;
                if (!scope.mTransitions.empty()) {
                    break;
                }
                --end;
            }
            if (end == mScopes.begin()) {
                while (!mNodeInstances.empty()) {
                    StateNodeInstance* const instance = mNodeInstances.back();
                    instance->Exit();
                    delete instance;
                    mNodeInstances.pop_back();
                }
                _PopScopes(0);
                break;
            }
        }
        mRandom = -1.0F;
        StateTransition* const transition = _FindTransition(0, transitioning, false);
        if (transition == nullptr) {
            break;
        }
        _ActivateTransition(transition);
        if (++taken == 1000) {
            static_cast<void>(mNodeInstances[0]->mNode->mObject->GetSafeName());
        } else if (mNodeInstances.size() == 32) {
            for (unsigned long node = 0; node != 5; ++node) {
                static_cast<void>(mNodeInstances[node]->mNode->mObject->GetSafeName());
            }
        } else {
            continue;
        }
        while (!mTransitions.empty()) {
            mTransitions.back().End();
            mTransitions.pop_back();
        }
        while (!mNodeInstances.empty()) {
            StateNodeInstance* const instance = mNodeInstances.back();
            instance->Exit();
            delete instance;
            mNodeInstances.pop_back();
        }
        _PopScopes(0);
        break;
    }
    return !mNodeInstances.empty();
}

// Reconstructed from eboot.elf at 0x704C50.
void StateGraphInstance::_UpdateElapsedTime() {
    float deltas[4];
    for (int units = 0; units != 4; ++units) {
        deltas[units] = mClock->DeltaTime(static_cast<TimeUnits>(units));
    }
    for (TransitionScope& scope : mScopes) {
        for (int units = 0; units != 4; ++units) {
            scope.mElapsed[kSinceIncomingStart][units] += deltas[units];
            if (scope.mIncomingEnded) {
                scope.mElapsed[kSinceIncomingEnd][units] += deltas[units];
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x704D80.
void StateGraphInstance::Deactivate() {
    while (!mTransitions.empty()) {
        mTransitions.back().End();
        mTransitions.pop_back();
    }
    while (!mNodeInstances.empty()) {
        StateNodeInstance* const instance = mNodeInstances.back();
        instance->Exit();
        delete instance;
        mNodeInstances.pop_back();
    }
    _PopScopes(0);
}

// Reconstructed from eboot.elf at 0x704E10. A synchronized transition may
// start only when the current node's timing crossed its start time since
// the last search; the start time one below the window is read as the
// window's end. Searching moves the last timing position on unless a
// transition is found.
StateTransition* StateGraphInstance::_FindTransition(unsigned long scope, bool interrupting, bool entering) {
    mScopeIndex = static_cast<unsigned long>(-1);
    float position = 0.0F;
    if (!entering) {
        StateNodeInstance* const current = mNodeInstances[0];
        if (current->mNode->HasValidTiming()) {
            position = current->GetTimingPosition();
        }
    }
    const float previous = mTimingPosition;
    mScopeIndex = scope;
    if (scope < mScopes.size()) {
        const float moved = position - previous;
        do {
            TransitionScope& searched = mScopes[mScopeIndex];
            if (!searched.mTransitions.empty()) {
                StateTransition* fallback = nullptr;
                for (TransitionEntry& entry : searched.mTransitions) {
                    if (entry.mCondition == nullptr && !entry.mSynchronized) {
                        fallback = entry.mTransition;
                        continue;
                    }
                    if (interrupting && !entry.mTransition->mCanInterrupt) {
                        continue;
                    }
                    const bool met = entry.mCondition != nullptr ? entry.mCondition->IsMet(false) : true;
                    if (entry.mSynchronized) {
                        float start = entry.mTransition->mAnimationBlendStartTime;
                        const float window = entry.mTransition->GetStartWindow();
                        if (start == window + -1.0F) {
                            start = window;
                        }
                        const float wrapped = entry.mTransition->WrapToStartWindow(mTimingPosition);
                        if (!(wrapped < start && wrapped + moved >= start)) {
                            continue;
                        }
                    }
                    if (met) {
                        mScopeIndex = static_cast<unsigned long>(-1);
                        return entry.mTransition;
                    }
                }
                if (fallback != nullptr && !interrupting) {
                    mScopeIndex = static_cast<unsigned long>(-1);
                    return fallback;
                }
            }
            for (TransitionEntry& entry : searched.mTransitions) {
                if (entry.mCondition != nullptr) {
                    entry.mCondition->ResetPerFrameState();
                }
            }
        } while (++mScopeIndex < mScopes.size());
    }
    mTimingPosition = position;
    mScopeIndex = static_cast<unsigned long>(-1);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x705170.
StateGraphResource* StateGraphInstance::GetResource() const {
    return mResource.Get();
}

// Reconstructed from eboot.elf at 0x705180.
float StateGraphInstance::GetElapsedTime(unsigned int clock, int units) const {
    return mScopes[mScopeIndex].mElapsed[clock][units];
}

// Reconstructed from eboot.elf at 0x7051B0.
void StateGraphInstance::_PopScopes(unsigned long index) {
    for (unsigned long scope = index; scope < mScopes.size(); ++scope) {
        for (TransitionEntry& entry : mScopes[scope].mTransitions) {
            delete entry.mCondition;
        }
    }
    mScopes.erase(mScopes.begin() + index, mScopes.end());
}

// Reconstructed from eboot.elf at 0x705300.
int StateGraphInstance::_PushScopes(const StateNode* node) {
    const int depth = node->mRuntime.mFolderNode != nullptr ? _PushScopes(node->mRuntime.mFolderNode) : 0;
    if (static_cast<unsigned long>(depth) < mScopes.size()) {
        TransitionScope& scope = mScopes[depth];
        if (scope.mNode == node) {
            if (scope.mNodeFinished) {
                scope.mNodeFinished = false;
            }
        } else {
            _PopScopes(depth);
        }
    }
    if (static_cast<unsigned long>(depth) == mScopes.size()) {
        _PushScope(node);
    }
    return depth + 1;
}

// Reconstructed from eboot.elf at 0x7053C0.
void StateGraphInstance::_PushScope(const StateNode* node) {
    mScopes.push_back(TransitionScope(node));
    TransitionScope& scope = mScopes.back();
    const unsigned int count = node->mTransitions.size();
    scope.mTransitions.reserve(count);
    for (unsigned long index = 0; index != count; ++index) {
        StateTransition* const transition = node->mRuntime.mTransitions[index];
        const StateCondition* const condition = transition->mRuntime.mCondition;
        const bool synchronized = transition->IsSynchronized();
        StateConditionInstance* const instance = condition != nullptr ? condition->CreateInstance(*this) : nullptr;
        scope.mTransitions.push_back(TransitionEntry(transition, instance, synchronized));
    }
    mScopeIndex = mScopes.size() - 1;
}

// Reconstructed from eboot.elf at 0x705760.
void StateGraphInstance::_PopScope() {
    _PopScopes(mScopes.size() - 1);
}

// Reconstructed from eboot.elf at 0x7058B0.
bool StateGraphInstance::IsScopeNodeFinished() const {
    return mScopes[mScopeIndex].mNodeFinished;
}

// Reconstructed from eboot.elf at 0x7058D0.
float StateGraphInstance::GetRandom() {
    if (mRandom < 0.0F) {
        mRandom = gRand.Float();
    }
    return mRandom;
}

// Reconstructed from eboot.elf at 0x705930.
bool StateGraphInstance::_IsBlendingIn() const {
    const unsigned long count = mNodeInstances.size();
    return count >= 2 && mPollIndex != count - 1;
}

// Reconstructed from eboot.elf at 0x705950.
bool StateGraphInstance::_IsBlendingOut() const {
    const unsigned long count = mNodeInstances.size();
    return count >= 2 && mPollIndex == count - 1;
}

// Reconstructed from eboot.elf at 0x705970.
bool StateGraphInstance::UseHSVColorBlending() const {
    return _GetPolledTransition()->mTransition->mHsvColorBlending;
}

// Reconstructed from eboot.elf at 0x7059A0.
const StateTransitionInstance* StateGraphInstance::_GetPolledTransition() const {
    return &mTransitions[mPollIndex - (mPollIndex == mTransitions.size())];
}

// Reconstructed from eboot.elf at 0x7059D0.
int StateGraphInstance::GetRotationBlending() const {
    return _GetPolledTransition()->mTransition->mRotationBlending;
}

// Reconstructed from eboot.elf at 0x705A00.
StateNodeInstance* StateGraphInstance::FindNodeInstance(const GameObject* object) {
    for (StateNodeInstance* instance : mNodeInstances) {
        StateNodeInstance* const found = instance->FindInstance(object);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x705A50.
int StateGraphInstance::ResolveTimeUnits(int units) const {
    return units == 4 ? mAnimDriver->mDefaultTimeline : units;
}

// Reconstructed from eboot.elf at 0x705A60.
bool StateGraphInstance::ShouldPoll(unsigned long count) const {
    if (mPollPhase == kPollEvenFrames) {
        return (count & 1) == 0;
    }
    if (mPollPhase == kPollOddFrames) {
        return (count & 1) != 0;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x705A80.
DataNode StateGraphInstance::_OnEntityDestroyed(const DataArray* msg) {
    static_cast<void>(msg);
    Deactivate();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x705B30.
DataNode StateGraphInstance::_OnObjectDestroyed(const DataArray* msg) {
    _DeactivateIfActive(msg->Node(2).ObjectId(msg));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x705B70.
void StateGraphInstance::_DeactivateIfActive(GameObjectId object) {
    for (StateNodeInstance* instance : mNodeInstances) {
        if (instance->UsesObject(object)) {
            Deactivate();
            return;
        }
    }
    for (const StateTransitionInstance& running : mTransitions) {
        if (running.mTransition->mObject->mId.mId == object.mId) {
            Deactivate();
            return;
        }
    }
    for (const TransitionScope& scope : mScopes) {
        if (scope.mNode->mObject->mId.mId == object.mId) {
            Deactivate();
            return;
        }
        for (const TransitionEntry& entry : scope.mTransitions) {
            if (entry.mTransition->mObject->mId.mId == object.mId ||
                entry.mTransition->mTargetState.mId == object.mId ||
                (entry.mCondition != nullptr && entry.mCondition->UsesObject(object))) {
                Deactivate();
                return;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x705F30. Only the graph component
// classes matter.
DataNode StateGraphInstance::_OnComponentDestroyed(const DataArray* msg) {
    const GameObjectId object = msg->Node(2).ObjectId(msg);
    const ComMetaData* const metaData = ComMetaData::GetMetaData(msg->Sym(3));
    if (metaData != nullptr) {
        const Symbol iface = metaData->mInterface;
        if (iface == StateNode::sClassName || iface == StateTransition::sClassName ||
            iface == StateCondition::sClassName) {
            _DeactivateIfActive(object);
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x705FD0. Each message type is a lazily
// built static symbol.
DataNode StateGraphInstance::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onEntityDestroyed;
    if (onEntityDestroyed == Symbol()) {
        onEntityDestroyed = Symbol("on_entity_destroyed");
    }
    if (type == onEntityDestroyed) {
        return _OnEntityDestroyed(msg);
    }
    static Symbol onObjectDestroyed;
    if (onObjectDestroyed == Symbol()) {
        onObjectDestroyed = Symbol("on_object_destroyed");
    }
    if (type == onObjectDestroyed) {
        return _OnObjectDestroyed(msg);
    }
    static Symbol onComponentDestroyed;
    if (onComponentDestroyed == Symbol()) {
        onComponentDestroyed = Symbol("on_component_destroyed");
    }
    if (type == onComponentDestroyed) {
        return _OnComponentDestroyed(msg);
    }
    return DataNode();
}
