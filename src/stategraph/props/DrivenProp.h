#pragma once

#include <cstddef>
#include <functional>

#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/waveform/Waveform.h"

class GroupMetadata;
struct TagArgs;

// A value a state graph object either holds itself or reads from a
// property of another component (the map's DrivenProp<T>, instantiated in
// the stategraph objects: StateConditionComparisonInit_A.o,
// StateNodeAnimate.o and others). Its "value", "driven_prop" and
// "is_driven" properties are registered by Register. Field names are not
// in the reference map.
template <class T>
class DrivenProp {
public:
    DrivenProp() : mValue(), mProp(), mDriven(false) {}
    // An undriven value. Inlined into the owners' constructors, for example
    // StateTransition's at 0x7F5220.
    explicit DrivenProp(const T& value) : mValue(value), mProp(), mDriven(false) {}

    // The value: the driving property's, read through its component, when
    // the value is driven; the own value otherwise. A driving property that
    // cannot be found or read gives the type's default. The map has it out
    // of line in StateNodeAnimate.o; this build inlines it, for example into
    // StateTransitionInstance::Poll at 0x7F84B0.
    T GetValue(Entity* entity) const;

    // Registers the type as the struct property `name` at `offset`:
    // "value", "prop" (a property reference into the entity `entity`
    // gives) and the hidden "is_driven", with a ">" button that switches
    // between them, and returns the struct's metadata. The binary emits a
    // copy per user; RndSceneCom.o holds the float and colour ones
    // (0x4107C0, 0x410EF0). Not reconstructed.
    GroupMetadata& Register(
        PropRegistry& registry,
        Symbol name,
        int offset,
        const std::function<Entity*(const TagArgs&)>& entity);

    T mValue;
    PropRef mProp;
    bool mDriven;
};

// The value type the template reads a driving property as. Name not in the
// reference map.
template <class T>
struct DrivenPropType;

template <>
struct DrivenPropType<float> {
    static constexpr PropertyType kType = kPropertyFloat;
};

template <class T>
T DrivenProp<T>::GetValue(Entity* entity) const {
    if (!mDriven || !mProp.IsComplete()) {
        return mValue;
    }
    T value = T();
    GameObject* const object = mProp.Obj(entity);
    if (object == nullptr) {
        return value;
    }
    Component* const com = object->FindCom(mProp.mCom);
    if (com == nullptr) {
        return value;
    }
    void* storage = com;
    const PropInfo* const info = com->_GetPropRegistry().FindProp(mProp.mPath, &storage);
    if (info == nullptr || !_GetProp(*com, mProp.mPath, *info, storage, &value, DrivenPropType<T>::kType)) {
        // The report is compiled out; its arguments remain.
        static_cast<void>(com->MakeErrorName());
        char path[512] = {};
        mProp.mPath.ToString(path, sizeof(path));
    }
    return value;
}

// A driven waveform is read whole from the property
// (PropRef::Get<Waveform<float>>, 0x2D0470); an undriven one is copied,
// which draws a new evaluation state. Inlined into
// StateTransitionInstance::Poll (0x7F84B0).
template <>
inline Waveform<float> DrivenProp<Waveform<float>>::GetValue(Entity* entity) const {
    if (mDriven && mProp.IsComplete()) {
        return mProp.Get<Waveform<float>>(entity);
    }
    return mValue;
}

static_assert(offsetof(DrivenProp<float>, mProp) == 8);
static_assert(offsetof(DrivenProp<float>, mDriven) == 224);
static_assert(sizeof(DrivenProp<float>) == 232);
static_assert(offsetof(DrivenProp<Waveform<float>>, mProp) == 40);
static_assert(offsetof(DrivenProp<Waveform<float>>, mDriven) == 256);
static_assert(sizeof(DrivenProp<Waveform<float>>) == 264);
