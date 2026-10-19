#pragma once

#include <cstddef>
#include <cstdio>
#include <functional>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/Resource.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "os/memory/MemMgr.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "stategraph/conditions/StateConditionEvent.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/Message.h"
#include "utl/streams/MemStream.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The class symbol of the parameter comparison for the property type:
// "StateConditionEventParameter_<type>", formatted in sBuf (0x1ABF2D0,
// emitted with stategraph/StateCondition.o). Inlined into every user.
inline Symbol StateConditionEventParameterComponentIdForType(PropertyType type) {
    static char sBuf[64];
    sprintf(sBuf, "StateConditionEventParameter_%s", TypeName(type, true));
    return Symbol(sBuf);
}

// Reads an event's parameter node as the value type: false when the node
// cannot stand in for the type. Numbers and booleans take integers (floats
// also floats), ids object ids, symbols and resource paths strings, and the
// rest their literal colours, property references and waveforms. The
// waveform copies are emitted out of line (0x8560B0 for Waveform<float>,
// 0x8595C0 for Waveform<Hmx::Color>); the others are inlined into
// StateConditionEventParameter<T>::Instance::_OnEvent. Name not in the
// reference map.
inline bool ReadEventParam(const DataNode& node, float& value) {
    if (!node.CompatibleType(kDataFloat)) {
        return false;
    }
    value = node.Type() == kDataInt ? static_cast<float>(node.mValue.integer) : node.mValue.real;
    return true;
}
inline bool ReadEventParam(const DataNode& node, int& value) {
    if (!node.CompatibleType(kDataInt)) {
        return false;
    }
    value = node.mValue.integer;
    return true;
}
inline bool ReadEventParam(const DataNode& node, bool& value) {
    if (!node.CompatibleType(kDataInt)) {
        return false;
    }
    value = node.mValue.integer != 0;
    return true;
}
inline bool ReadEventParam(const DataNode& node, GameObjectId& value) {
    if (!node.CompatibleType(kDataGameObjectId)) {
        return false;
    }
    value.mId = static_cast<unsigned int>(node.mValue.integer);
    return true;
}
inline bool ReadEventParam(const DataNode& node, Symbol& value) {
    if (!node.CompatibleType(kDataString)) {
        return false;
    }
    value = Symbol(EventNodeText(node));
    return true;
}
inline bool ReadEventParam(const DataNode& node, ResourcePath& value) {
    if (!node.CompatibleType(kDataString)) {
        return false;
    }
    value = EventNodeText(node);
    return true;
}
inline bool ReadEventParam(const DataNode& node, Hmx::Color& value) {
    if (!node.CompatibleType(kDataColor)) {
        return false;
    }
    value = node.LiteralColor(false, nullptr);
    return true;
}
inline bool ReadEventParam(const DataNode& node, PropRef& value) {
    if (!node.CompatibleType(kDataPropRef)) {
        return false;
    }
    value = node.LiteralPropRef(nullptr);
    return true;
}
inline bool ReadEventParam(const DataNode& node, Waveform<float>& value) {
    if (!node.CompatibleType(kDataWaveformFloat)) {
        return false;
    }
    value = node.LiteralWaveformFloat(nullptr);
    return true;
}
inline bool ReadEventParam(const DataNode& node, Waveform<Hmx::Color>& value) {
    if (!node.CompatibleType(kDataWaveformColor)) {
        return false;
    }
    value = node.LiteralWaveformColor(nullptr);
    return true;
}

// The attribute's value for the entity's property at the path, with no
// component at hand: the constant or what the static function gives; the
// type's default otherwise. Inlined into the parameter list of
// StateConditionEventParameter<T>::_Init (0x83A610 for float). Name not in
// the reference map.
template <class T>
T StaticTagValue(const TagBase<T>& tag, Entity* entity, const PropPath& path) {
    if (tag.mMode == TagBase<T>::kModeStaticFunction) {
        return tag.mStaticFunction(StaticTagArgs(entity, path));
    }
    if (tag.mMode == TagBase<T>::kModeConstant) {
        return T(tag.mValue);
    }
    return T();
}

// The condition that compares a parameter of its parent StateConditionEvent's
// event with a value (stategraph/StateConditionEventParameter.o,
// 0x835D40-0x85968A; class ids "StateConditionEventParameter_<type>"),
// for each parameter type: float, int, bool, GameObjectId, Symbol,
// ResourcePath, Hmx::Color, PropRef, Waveform<float> and
// Waveform<Hmx::Color>. "param" names the parameter; a parameter of another
// type replaces the component with the comparison of that type. The
// object's code is grouped by type: the template statics' guarded
// initializers (0x8388E0-0x8399CF), then each type's members, comparison
// base members and instances (float 0x8399D0-0x83CFAF, int from 0x83CFB0,
// bool 0x8403A0, GameObjectId 0x843740, Symbol 0x846AE0, ResourcePath
// 0x849EF0, Hmx::Color 0x84D2D0, PropRef 0x84FE70, Waveform<float>
// 0x852C70 and Waveform<Hmx::Color> 0x856180). The vtable has 59 slots
// (0x1954E40 for float). The object is 528 bytes for float.
template <class T>
class StateConditionEventParameter : public StateConditionComparison<T> {
public:
    using Base = StateConditionComparison<T>;
    using typename Base::ComparisonOperator;

    // The condition's run-time state: the parameter's value from the last
    // event, compared when it comes, and whether a comparison was met since
    // the frame's reset. The map's StateConditionEventParameter<T>::
    // Instance. The vtable has nine slots (0x1955028 for float). The
    // object is 64 bytes for float, from the small-block pool.
    class Instance : public Base::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x83C2F0 for float); the waveform
        // copies are out of line (0x855830, 0x858D40). The map's
        // constructor takes the node instance as well.
        Instance(const StateConditionEventParameter& condition, StateGraphInstance& graph)
            : Base::Instance(condition, graph), mParameter(&condition), mMet(false) {}
        // Slots 0-1 (0x83C770 and 0x83C780 for float).
        ~Instance() override {}

        // Slot 2: whether an event met the comparison (0x83C7B0 for float).
        bool _IsMet(bool useCached) override;
        // Slot 3 (0x83C7C0 for float).
        void _ResetPerFrameState() override;
        // Slot 4: reads the parameter from the event, whose nodes from 2 on
        // are the parameters in the registry's order, and compares it
        // (0x83C7D0 for float).
        bool _OnEvent(DataArray* event) override;
        // Slot 8: the parameter's last value (0x83C8A0 for float).
        bool _GetValueToCompare(T& value) const override;

        // Field names are not in the reference map.
        const StateConditionEventParameter* mParameter;
        T mValue;
        bool mMet;
    };

    StateConditionEventParameter() : mParent{0xFFFFFFFFU}, mParam(), mParentEvent(nullptr) {}
    // Copies the condition for an imprint; the parent starts unresolved.
    // Inlined into _Imprint (0x83C120 for float).
    StateConditionEventParameter(const StateConditionEventParameter& other)
        : Base(other), mParent(other.mParent), mParam(other.mParam), mParentEvent(nullptr) {}
    // Slots 0-1 (0x83A2F0, the Component destructor, and 0x83C080 for
    // float).
    ~StateConditionEventParameter() override {}

    // Slot 4: sTypeId (0x83C0A0 for float).
    Symbol GetId() const override;
    // Slot 5: sId (0x83C0B0 for float).
    Symbol GetClassName() const override;
    int CurrentRev() const override;        // slot 7 (0x83C0C0 for float)
    bool IsA(Symbol type) const override;   // slot 8 (0x83C0E0 for float)
    Component* AsComponent() override;      // slot 9 (0x83C110 for float)
    // Slot 10 (0x83C120 for float). The map's _Imprint(char*, Component*&,
    // bool).
    char* _Imprint(char* buffer, Component** imprint) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22 (0x83C2A0 for float)
    ComMetaData& _GetMetaData() override;       // slot 23 (0x83C2B0 for float)
    // Slot 41: resolves the parent event (0x83C2C0 for float).
    void _Fixup() override;
    // Slot 42 (0x83C2F0 for float). The map's _CreateInstance(
    // StateNodeInstance const&, StateGraphInstance&) const.
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;
    // Slot 52: kParameterComparison (0x83C3D0 for float).
    int _GetType() const override;
    // Slot 53: valid under an Event condition whose event has the parameter
    // with the type (0x83C3E0 for float).
    bool _IsValid(const StateCondition* parent, String* error) const override;
    // Slot 54: "event parameter" (0x83C6D0 for float).
    const char* _GetValueToCompareGeneralDescription() const override;
    // Slot 55: "Param: <param>" (0x83C6E0 for float).
    const char* _GetValueToCompareDescription() const override;
    // Slot 56: whether a parameter is named (0x83C750 for float).
    bool _HasValueToCompare() const override;

    // Registers the class: InitComponentClass's body, with sId as the
    // class symbol. The map has it inline; this build emits it in the
    // object (0x835D80 for float), and StateConditionEventParameterInit
    // calls it.
    static void Init();
    // The class factory (0x83A1A0 for float).
    static Component* _Create();
    // 0x8399D0 for float.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);

    // Guarded template statics. "StateConditionEventParameter_<type>", the
    // class symbol StateCondition::Create and Init take; the float copy's
    // guarded initializer is in stategraph/StateCondition.o (0x8098C0;
    // 0x1ABF040).
    static Symbol sId;
    // Also "StateConditionEventParameter_<type>", which GetId returns
    // (0x1AC26B8 for float). Name not in the reference map.
    static Symbol sTypeId;
    static PropRegistry sPropRegistry;  // 0x1AC2600 for float
    static ComMetaData sMetaData;       // 0x1AC2448 for float

    // Field names are not in the reference map.
    // "parent" (hidden): the Event condition's object. It sits in the
    // comparison's tail padding.
    GameObjectId mParent;
    // "param".
    Symbol mParam;
    // The parent, resolved by _Fixup and the "parent" setter.
    StateConditionEvent* mParentEvent;
};

static_assert(offsetof(StateConditionEventParameter<float>, mParent) == 508);
static_assert(offsetof(StateConditionEventParameter<float>, mParam) == 512);
static_assert(offsetof(StateConditionEventParameter<float>, mParentEvent) == 520);
static_assert(sizeof(StateConditionEventParameter<float>) == 528);
static_assert(sizeof(StateConditionEventParameter<int>) == 528);
static_assert(sizeof(StateConditionEventParameter<bool>) == 528);
static_assert(sizeof(StateConditionEventParameter<GameObjectId>) == 528);
static_assert(sizeof(StateConditionEventParameter<Symbol>) == 528);
static_assert(sizeof(StateConditionEventParameter<ResourcePath>) == 528);
static_assert(sizeof(StateConditionEventParameter<Hmx::Color>) == 544);
static_assert(sizeof(StateConditionEventParameter<PropRef>) == 944);
static_assert(sizeof(StateConditionEventParameter<Waveform<float>>) == 592);
static_assert(sizeof(StateConditionEventParameter<Waveform<Hmx::Color>>) == 592);
static_assert(offsetof(StateConditionEventParameter<float>::Instance, mParameter) == 48);
static_assert(offsetof(StateConditionEventParameter<float>::Instance, mValue) == 56);
static_assert(offsetof(StateConditionEventParameter<float>::Instance, mMet) == 60);
static_assert(sizeof(StateConditionEventParameter<float>::Instance) == 64);
static_assert(offsetof(StateConditionEventParameter<bool>::Instance, mMet) == 57);
static_assert(sizeof(StateConditionEventParameter<Symbol>::Instance) == 72);
static_assert(sizeof(StateConditionEventParameter<Hmx::Color>::Instance) == 80);
static_assert(sizeof(StateConditionEventParameter<PropRef>::Instance) == 280);
static_assert(sizeof(StateConditionEventParameter<Waveform<float>>::Instance) == 104);

// The statics, in the order of the object's guarded initializers.
template <class T>
Symbol StateConditionEventParameter<T>::sId(StateConditionEventParameterComponentIdForType(DrivenPropType<T>::kType));
template <class T>
ComMetaData StateConditionEventParameter<T>::sMetaData;
template <class T>
PropRegistry StateConditionEventParameter<T>::sPropRegistry;
template <class T>
Symbol StateConditionEventParameter<T>::sTypeId(
    StateConditionEventParameterComponentIdForType(DrivenPropType<T>::kType));

// Reconstructed from eboot.elf at 0x8399D0 (the float copy). The parameter
// is chosen from the parent event's; the own values take the metadata of
// the event's parameter, and the evaluation mode is hidden, as an event
// parameter is met when an event comes.
template <class T>
void StateConditionEventParameter<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionEventParameter prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sId, sPropRegistry);
    }
    GameObjectIdMetadata& parent = TypeSpecificMetadata(
        registry.RegisterProp("parent", offsetof(StateConditionEventParameter, mParent), kPropertyGameObjectId, 4),
        prototype.mParent);
    parent.mDefault = prototype.mParent;
    static_cast<void>(Symbol("prop"));
    parent.mIsHidden = true;
    // At 0x83A3B0.
    parent.mSetter = [](const PropAccessorArgs& args) {
        StateConditionEventParameter* const condition = static_cast<StateConditionEventParameter*>(args.mComponent);
        condition->mParent = *static_cast<const GameObjectId*>(args.mValue);
        condition->mParentEvent = condition->template _GetObjectCom<StateConditionEvent>(condition->mParent, true);
    };

    SymbolMetadata& param = TypeSpecificMetadata(
        registry.RegisterProp("param", offsetof(StateConditionEventParameter, mParam), kPropertySymbol, sizeof(Symbol)),
        prototype.mParam);
    param.mDefault = prototype.mParam;
    static_cast<void>(Symbol("prop"));
    param.mHelp = String("The parameter of the parent event to compare against.");
    // At 0x83A610: the parent event's parameters that are not hidden, with
    // their help.
    param.mAllowedValues =
        std::function<eastl::vector<AllowedValue<Symbol>>(const TagArgs&)>([](const TagArgs& args) {
            eastl::vector<AllowedValue<Symbol>> values;
            const StateConditionEventParameter& condition =
                static_cast<const StateConditionEventParameter&>(args.mComponent);
            const PropRegistry* const params = condition.mParentEvent->GetEventRegistry();
            if (params == nullptr) {
                return values;
            }
            Entity* const entity = condition._GetContextEntityPrototype();
            for (const PropRegistry::PropEntry& entry : params->mProps) {
                const Symbol name = entry.mName;
                const PropMetadata* const info = entry.mInfo.mMetadata.Get();
                if (StaticTagValue(info->mIsHidden, entity, PropPath(name))) {
                    continue;
                }
                const String help = StaticTagValue(info->mHelp, entity, PropPath(name));
                values.push_back(AllowedValue<Symbol>{name, String(nullptr), String(help.c_str())});
            }
            return values;
        });
    // At 0x83A990: a parameter of another type replaces the component with
    // the comparison of that type, which takes the stored properties; the
    // comparison values lose their driving properties, and an ordering
    // operator falls back to "==" for a type that is not a number.
    param.mOnChanged = [](const PropAccessorArgs& args) {
        StateConditionEventParameter* const condition = static_cast<StateConditionEventParameter*>(args.mComponent);
        const PropRegistry* const params = condition->mParentEvent->GetEventRegistry();
        if (params != nullptr) {
            const PropInfo* const info =
                params->FindProp(PropPath(*static_cast<const Symbol*>(args.mValue)), nullptr);
            if (info != nullptr && info->mType != DrivenPropType<T>::kType) {
                const PropertyType type = info->mType;
                GameObject* const object = condition->mObject;
                MemStream stream(false);
                _StorePropState(*condition, PropPath(), stream, nullptr);
                const int op = condition->mOperator;
                object->DestroyComponent(condition->GetId(), false);
                Component* const com =
                    object->CreateComponent(StateConditionEventParameterComponentIdForType(type), false);
                object->_ReenterComponents();
                stream.Seek(0, kSeekBegin);
                _ApplyStoredPropState(*com, PropPath(), stream, true);
                PropPath::Node member;
                PropPath::Node field;
                SetPropValue(*com, PropPath(member = Symbol("value_a"), field = Symbol("prop")), PropRef());
                SetPropValue(*com, PropPath(member = Symbol("value_b"), field = Symbol("prop")), PropRef());
                if (op >= StateCondition::kLess && !IsNumber(type)) {
                    SetPropValue(*com, PropPath(Symbol("operator")), static_cast<int>(StateCondition::kEqual));
                }
                Message<1> msg(Symbol("property_comparison_condition_type_changed"));
                DataNode id;
                id.mType = kDataGameObjectId;
                id.mValue.var = object->mId.mId;
                msg.mData->Node(2) = id;
                object->mEntity->Export(msg.mData);
                return;
            }
        }
        condition->_SendDisplayTextUpdated();
    };

    // At 0x83B650.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev <= 0) {
            static_cast<StateConditionEventParameter*>(args.mComponent)->_LoadOldValues(*args.mSavedProps);
        }
    };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
    }

    Base::_InitAsSuperclass(registry, metadata);
    // At 0x83BEA0: the parent event's parameter answers.
    const TagRemapFunc remap = [](TagRemapArgs& args) {
        const StateConditionEventParameter& condition =
            static_cast<const StateConditionEventParameter&>(args.mComponent);
        args.mEntity = condition._GetContextEntityPrototype();
        args.mTargetPath = PropPath(condition.mParam);
        if (condition.mParentEvent != nullptr) {
            const PropRegistry* const params = condition.mParentEvent->GetEventRegistry();
            if (params != nullptr) {
                args.mTargetInfo = params->FindProp(PropPath(condition.mParam), nullptr);
            }
        }
    };
    PropPath::Node member;
    PropPath::Node field;
    const T type = T();
    RemapPropMetadata(
        TypeSpecificMetadata(
            const_cast<PropInfo&>(
                *registry.FindProp(PropPath(member = Symbol("value_a"), field = Symbol("value")), nullptr)),
            type),
        remap);
    RemapPropMetadata(
        TypeSpecificMetadata(
            const_cast<PropInfo&>(
                *registry.FindProp(PropPath(member = Symbol("value_b"), field = Symbol("value")), nullptr)),
            type),
        remap);
    static_cast<void>(registry.FindProp(PropPath(Symbol("evaluation_mode")), nullptr));
    PropMetadata& mode = *registry.FindProp(PropPath(Symbol("evaluation_mode")), nullptr)->mMetadata.Get();
    static_cast<void>(Symbol("prop"));
    mode.mIsHidden = true;
    metadata.mDescription = "Compares a property value against a specific value or another property.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x83C0A0 (the float copy).
template <class T>
Symbol StateConditionEventParameter<T>::GetId() const {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x83C0B0 (the float copy).
template <class T>
Symbol StateConditionEventParameter<T>::GetClassName() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x83C0C0 (the float copy).
template <class T>
int StateConditionEventParameter<T>::CurrentRev() const {
    return const_cast<StateConditionEventParameter*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x83C0E0 (the float copy).
template <class T>
bool StateConditionEventParameter<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x83C110 (the float copy).
template <class T>
Component* StateConditionEventParameter<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x83C120 (the float copy). The copy goes
// at the buffer's next 8-byte boundary, and its properties follow it.
template <class T>
char* StateConditionEventParameter<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy =
        reinterpret_cast<StateConditionEventParameter*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionEventParameter(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x83C2A0 (the float copy).
template <class T>
PropRegistry& StateConditionEventParameter<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x83C2B0 (the float copy).
template <class T>
ComMetaData& StateConditionEventParameter<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x83C2C0 (the float copy).
template <class T>
void StateConditionEventParameter<T>::_Fixup() {
    mParentEvent = this->template _GetObjectCom<StateConditionEvent>(mParent, true);
}

// Reconstructed from eboot.elf at 0x83C2F0 (the float copy).
template <class T>
StateConditionInstance* StateConditionEventParameter<T>::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x83C3D0 (the float copy).
template <class T>
int StateConditionEventParameter<T>::_GetType() const {
    return StateCondition::kParameterComparison;
}

// Reconstructed from eboot.elf at 0x83C3E0 (the float copy).
template <class T>
bool StateConditionEventParameter<T>::_IsValid(const StateCondition* parent, String* error) const {
    if (parent == nullptr || !parent->IsA(StateConditionEvent::sClassName)) {
        if (error != nullptr) {
            *error = "Event Parameter conditions must have an Event condition as their parent.";
        }
        return false;
    }
    const PropRegistry* const params = static_cast<const StateConditionEvent*>(parent)->GetEventRegistry();
    if (params != nullptr) {
        const PropInfo* const info = params->FindProp(PropPath(mParam), nullptr);
        if (info != nullptr) {
            if (info->mType == DrivenPropType<T>::kType) {
                return true;
            }
            if (error != nullptr) {
                const char* const actual = TypeName(info->mType, true);
                const char* const expected = TypeName(DrivenPropType<T>::kType, true);
                FormatString text(
                    "The parent Event condition's parameter '%s' is of type %s, but this Event Parameter "
                    "condition expects type %s.");
                text << mParam << actual << expected;
                *error = text.Str();
            }
            return false;
        }
    }
    if (error != nullptr) {
        FormatString text("The parent Event condition does not have parameter '%s'.");
        text << mParam;
        *error = text.Str();
    }
    return false;
}

// Reconstructed from eboot.elf at 0x83C6D0 (the float copy).
template <class T>
const char* StateConditionEventParameter<T>::_GetValueToCompareGeneralDescription() const {
    return "event parameter";
}

// Reconstructed from eboot.elf at 0x83C6E0 (the float copy).
template <class T>
const char* StateConditionEventParameter<T>::_GetValueToCompareDescription() const {
    FormatString text("Param: %s");
    text << mParam;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x83C750 (the float copy).
template <class T>
bool StateConditionEventParameter<T>::_HasValueToCompare() const {
    return mParam != Symbol();
}

// Reconstructed from eboot.elf at 0x83C7B0 (the float copy).
template <class T>
bool StateConditionEventParameter<T>::Instance::_IsMet(bool useCached) {
    static_cast<void>(useCached);
    return mMet;
}

// Reconstructed from eboot.elf at 0x83C7C0 (the float copy).
template <class T>
void StateConditionEventParameter<T>::Instance::_ResetPerFrameState() {
    mMet = false;
}

// Reconstructed from eboot.elf at 0x83C7D0 (the float copy). An event
// without the parameter, or with a value of another type, does not meet
// the condition.
template <class T>
bool StateConditionEventParameter<T>::Instance::_OnEvent(DataArray* event) {
    const PropRegistry* const params = mParameter->mParentEvent->GetEventRegistry();
    if (params == nullptr) {
        return false;
    }
    unsigned int index = 2;
    for (const PropRegistry::PropEntry& entry : params->mProps) {
        if (entry.mName == mParameter->mParam) {
            bool met = false;
            if (ReadEventParam(event->Node(index), mValue)) {
                met = Base::Instance::_IsMet(false);
            }
            mMet |= met;
            return met;
        }
        ++index;
    }
    return false;
}

// Reconstructed from eboot.elf at 0x83C8A0 (the float copy).
template <class T>
bool StateConditionEventParameter<T>::Instance::_GetValueToCompare(T& value) const {
    value = mValue;
    return true;
}

// Reconstructed from eboot.elf at 0x83A1A0 (the float copy).
template <class T>
Component* StateConditionEventParameter<T>::_Create() {
    return new StateConditionEventParameter();
}

// Reconstructed from eboot.elf at 0x835D80 (the float copy).
template <class T>
void StateConditionEventParameter<T>::Init() {
    static const long sHeap = MemFindHeap("metadata");
    MemPushHeap(sHeap);
    const Symbol name = sId;
    sMetaData.mCreatable = true;
    sMetaData.mSuperclassRoot = false;
    sMetaData.mAllowMultiple = false;
    _Init(sPropRegistry, sMetaData);
    RegisterPropTypeSize<StateConditionEventParameter>(name);
    StateConditionEventParameter prototype;
    Component::sFactory[name] = _Create;
    Component::sVtables[name] = [](void* storage) -> Component* {
        return new (storage) StateConditionEventParameter();
    };
    const bool hasPostPoll = prototype._HasPostPoll();
    sMetaData.Init(name, name, sPropRegistry, sizeof(StateConditionEventParameter), hasPostPoll, false);
    MemPopHeap();
}

// The registration of the StateConditionEventParameter<T> condition types
// (stategraph/StateConditionEventParameter.o): it calls each type's Init.
void StateConditionEventParameterInit();  // 0x835D40
