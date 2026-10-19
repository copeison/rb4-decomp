#pragma once

#include <cstddef>
#include <cstdio>
#include <limits>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropUtl.h"
#include "entity/waveform/Key.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "math/random/Rand.h"
#include "math/rotation/Rot.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "os/memory/MemMgr.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/props/DrivenProp.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The property types of the vector, rotation and transform values, which
// only the node templates drive; they start at zero, no rotation and the
// identity, as the nodes' constructors show (0x7E4C20 for the transform).
// Names not in the reference map.
template <>
struct DrivenPropType<Vector3> {
    static constexpr PropertyType kType = kPropertyVector3;
    static Vector3 Default() {
        return Vector3{0.0F, 0.0F, 0.0F};
    }
};
template <>
struct DrivenPropType<PropRotation> {
    static constexpr PropertyType kType = kPropertyEuler;
    static PropRotation Default() {
        PropRotation value;
        value.x = 0.0F;
        value.y = 0.0F;
        value.z = 0.0F;
        return value;
    }
};
template <>
struct DrivenPropType<PropXfm> {
    static constexpr PropertyType kType = kPropertyXfm;
    static PropXfm Default() {
        return PropXfm{{1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}};
    }
};

// The buffer the class ids of the StateNodeSetValue<T> types are written
// to, one per translation unit (0x1AB2940 in stategraph/StateGraph.o,
// 0x1AB6D60 in StateNodeIncrementPropertyInit_A.o). Name not in the
// reference map.
static char gSetValueClassName[64];

// The class id of the node that sets values of type T,
// "StateNodeSetValue_<kType>" with the type's constant name (TypeName).
// The guarded template statics sId and sClassName build their symbols from
// it. Name not in the reference map.
template <class T>
const char* MakeSetValueClassName() {
    sprintf(gSetValueClassName, "StateNodeSetValue_%s", TypeName(DrivenPropType<T>::kType, true));
    return gSetValueClassName;
}

// The smallest and largest value a number property allows, as floats: the
// metadata's limits, or -1e30 and 1e30 when the limit is the type's own.
// Inlined into GetPropRefRange. Names not in the reference map.
template <class T>
inline float NumberRangeMin(T value) {
    return value == std::numeric_limits<T>::min() ? -1e30F : static_cast<float>(value);
}
template <class T>
inline float NumberRangeMax(T value) {
    return value == std::numeric_limits<T>::max() ? 1e30F : static_cast<float>(value);
}
template <>
inline float NumberRangeMin(float value) {
    return value;
}
template <>
inline float NumberRangeMax(float value) {
    return value;
}

// Reads the limits of the number property `info` describes in `min` and
// `max` when they are given. Inlined into GetPropRefRange. Name not in the
// reference map.
template <class T>
inline void GetNumberPropRange(
    const Component& com,
    const PropPath& path,
    const PropInfo& info,
    float* min,
    float* max) {
    const auto& metadata = TypeSpecificMetadata(info, T());
    if (min != nullptr) {
        *min = NumberRangeMin<T>(metadata.mMin.Get(com, path));
    }
    if (max != nullptr) {
        *max = NumberRangeMax<T>(metadata.mMax.Get(com, path));
    }
}

// The range of values the number property the reference names allows in
// the entity, in `min` and `max` when they are given; left as they are
// when the property is missing or not a number. The binary emits one copy
// (stategraph/StateNodeIncrementPropertyInit_A.o) for the value nodes'
// tag functions and StateNodeSetProperty<T>::_GetTargetRange. Name not in
// the reference map.
// Reconstructed from eboot.elf at 0x724CE0.
inline void GetPropRefRange(Entity* entity, const PropRef& ref, float* min, float* max) {
    GameObject* object = nullptr;
    Component* com = nullptr;
    const PropInfo* info = nullptr;
    if (!ref.Resolve(entity, object, com, info)) {
        return;
    }
    switch (info->mType) {
    case kPropertyFloat:
        GetNumberPropRange<float>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyInt8:
        GetNumberPropRange<signed char>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyInt16:
        GetNumberPropRange<short>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyInt:
        GetNumberPropRange<int>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyInt64:
        GetNumberPropRange<long>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyUInt8:
        GetNumberPropRange<unsigned char>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyUInt16:
        GetNumberPropRange<unsigned short>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyUInt:
        GetNumberPropRange<unsigned int>(*com, ref.mPath, *info, min, max);
        break;
    case kPropertyUInt64:
        GetNumberPropRange<unsigned long>(*com, ref.mPath, *info, min, max);
        break;
    default:
        break;
    }
}

template <class T>
struct StateNodeValueOps;

// A node that sets a value of type T (the map's StateNodeSetValue<T>, in
// stategraph/StateNodeSetValue.o): the value, its own or driven by another
// property, which numbers can first convert from an input range to an
// output range, directly or along a curve. The node does not create an
// instance itself; StateNodeSetProperty<T> applies the value to a
// property, and StateNodeSetAnimFrame derives from the float copy.
// StateNodeSetValueInit registers the twenty types, from float to
// PropXfm, as "StateNodeSetValue_kFloat" and so on. The binary emits each
// copy in the first object that uses it: the float node in
// stategraph/StateGraph.o (0x6FFF80-0x700516; vtable 0x193A2C8 with 52
// slots), the others with the property nodes. The object is 448 bytes for
// the numbers, bools, ids, symbols and paths, 456 for colours, vectors and
// rotations, 480 for waveforms and transforms and 656 for property
// references.
template <class T>
class StateNodeSetValue : public StateNode {
public:
    // How "conversion" changes a number before it is applied. The labels
    // are the allowed values'; the enumerator names are not in the
    // reference map.
    enum Conversion : int {
        kConversionNone = 0,
        kConversionRange = 1,
        kConversionCurve = 2,
    };

    // The value's run-time state, the base of the property nodes'
    // instances (the map's StateNodeSetValue<T>::Instance): the value at
    // the start of a blend, the value to reach and the value applied, with
    // the blend between them. Instances of nodes that blend into each other
    // are linked, so that only the newest applies its value and a new one
    // starts from the value the older one applied. The float copy's vtable
    // at 0x193C578 has 20 slots, three of them pure. The object is 72 bytes
    // for float.
    class Instance : public StateNodeInstance {
    public:
        // Inlined into the subclasses' _CreateInstance, for example at
        // 0x723F90.
        Instance(const StateNodeSetValue& node, StateGraphInstance& graph)
            : StateNodeInstance(node, graph),
              mSetValueNode(&node),
              mBlendFrom(nullptr),
              mBlendTo(nullptr),
              mStartValue(),
              mTargetValue(),
              mValue(),
              mBlend(0.0F) {}
        // Slots 0-1 at 0x7248F0 and 0x724920 for float: unlinks the newer
        // instance blending from this one. Inlined into the subclasses'
        // destructors.
        ~Instance() override {
            if (mBlendTo != nullptr) {
                mBlendTo->mBlendFrom = nullptr;
            }
        }

        // Slot 2: takes the value to reach unless it is persistent, and
        // starts from the value the instance blending into this one
        // applied, or from the target's current value.
        void _Enter() override;  // 0x724300
        // Slot 4: the blend toward the value to reach.
        void _SetBlend(float blend) override;  // 0x724370
        // Slot 5: blends toward the value to reach while the graph blends
        // into the node, and applies the result unless a newer instance
        // blends from this one. True while the value is persistent, blends
        // or is still being applied.
        bool _Poll() override;  // 0x724380
        // Slot 7: whether the node blends and no instance blends into this
        // one yet.
        bool _WantsBlendPartner() const override;  // 0x724470
        // Slot 8: links the instance among the earlier one's that sets the
        // same target, to blend from it.
        bool _TakeBlendPartner(StateNodeInstance* previous) override;  // 0x7244A0
        // Slot 10: an instance no newer one blends from, setting the same
        // target as the other.
        bool _IsBlendPartner(const StateNodeInstance& other) const override;  // 0x7244D0

        // Slot 15: the target's current value. Name not in the reference
        // map.
        virtual bool _GetCurrentValue(T& value) const = 0;
        // Slot 16: the node's value, read from the driving property when it
        // is driven, and converted.
        virtual T _GetValueToApply() const;  // 0x724530
        // Slot 17: applies the value to the target.
        virtual void _ApplyValue(const T& value) const = 0;
        // Slot 18: whether the other instance, of the same class, sets the
        // same target. The map has an _IsBlendPartner override in each
        // subclass instead, so the slot is newer; name not in the reference
        // map.
        virtual bool _HasSameTarget(const StateNodeInstance& other) const = 0;
        // Slot 19: whether the value has been applied; true here.
        virtual bool _IsFinishedApplyingValue() const;  // 0x7248E0

        // Converts a number by the node's conversion; other values are left
        // alone. Name not in the reference map.
        void _ConvertValue(T& value) const;  // 0x724C00
        // The nearest entered instances this one blends from and that blend
        // from this one. Inlined. Names not in the reference map.
        Instance* _GetEnteredBlendFrom() const {
            Instance* instance = mBlendFrom;
            while (instance != nullptr && !instance->mEntered) {
                instance = instance->mBlendFrom;
            }
            return instance;
        }
        Instance* _GetEnteredBlendTo() const {
            Instance* instance = mBlendTo;
            while (instance != nullptr && !instance->mEntered) {
                instance = instance->mBlendTo;
            }
            return instance;
        }

        // Field names are not in the reference map.
        const StateNodeSetValue* mSetValueNode;
        // The older instance this one blends from, and the newer one that
        // blends from this.
        Instance* mBlendFrom;
        Instance* mBlendTo;
        T mStartValue;
        T mTargetValue;
        T mValue;
        float mBlend;
    };

    StateNodeSetValue()
        : mValue(),
          mPersistent(false),
          mConversion(kConversionNone),
          mInputRange{0.0F, 0.0F},
          mOutputRange{0.0F, 0.0F},
          mOutputCurve() {}
    // Copies the node for an imprint. Out of line at 0x700320 for float.
    StateNodeSetValue(const StateNodeSetValue& other) = default;
    // Slots 0-1: 0x6FFF80, 0x700040 for float.
    ~StateNodeSetValue() override {}

    Symbol GetId() const override;          // slot 4: 0x700110
    Symbol GetClassName() const override;   // slot 5: 0x700120
    int CurrentRev() const override;        // slot 7: 0x700130
    bool IsA(Symbol type) const override;   // slot 8: 0x700150
    Component* AsComponent() override;      // slot 9: 0x700180
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x700190
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7002A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7002B0
    // Slot 46 at 0x7002D0 for float: the value nodes blend.
    bool CanBlend() const override {
        return true;
    }
    // Slots 47-48 at 0x7002E0 and 0x7002F0 for float: colours (and colour
    // waveforms) blend as the transition's colour blending says, and
    // rotations and transforms as its rotation blending says.
    bool UsesColorBlending() const override {
        return DrivenPropType<T>::kType == kPropertyColor || DrivenPropType<T>::kType == kPropertyWaveformColor;
    }
    bool UsesRotationBlending() const override {
        return DrivenPropType<T>::kType == kPropertyEuler || DrivenPropType<T>::kType == kPropertyXfm;
    }

    // Slot 50: whether the node has somewhere to set its value; true here.
    virtual bool _HasValueToSet() const;  // 0x700300
    // Slot 51: the range of values the target allows, in `min` and `max`
    // when they are given; false when it is unknown, as here. Name not in
    // the reference map.
    virtual bool _GetTargetRange(float* min, float* max) const;  // 0x700310

    // The value as editor text: the driving property's reference, or the
    // value itself. Emitted with the property nodes, at 0x7240D0 for float.
    const char* _GetValueDescription() const;
    // Takes the value saved before revision 1 of the properties that hold
    // it: "value" and its driving "value_prop", which drives the value when
    // it is complete. The revision handlers of the subclasses call it.
    // Emitted with the property nodes, at 0x725EA0 for float.
    void _LoadOldValue(const Component& saved);
    // A curve that stays at zero, or the nearest value the target allows:
    // the default "output_curve". Emitted with the property nodes, at
    // 0x728510 for float. Name not in the reference map.
    ResourcePtr<WaveformResource<float>> _MakeDefaultCurve() const;

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateNodeSetValue.o, at 0x7E9390 for float.
    static void Init();
    // The class factory. Emitted in stategraph/StateNodeSetValue.o, at
    // 0x7EFF90 for float.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x726350
    // Makes the node the superclass of a subclass's metadata, as
    // StateNode::_InitAsSuperclass does. Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x723830

    // Guarded template statics; for float sId at 0x1AB2488 and sClassName
    // at 0x1AB2498, both "StateNodeSetValue_kFloat".
    static Symbol sId;
    // Name not in the reference map.
    static Symbol sClassName;
    static PropRegistry sPropRegistry;  // 0x1AB2660 for float
    static ComMetaData sMetaData;       // 0x1AB24A8 for float

    // Field names are not in the reference map; they follow the
    // properties.
    // "value": the value to set, or the property that drives it.
    DrivenProp<T> mValue;
    // "persistent": set the value every frame rather than once.
    bool mPersistent;
    // "conversion" (Conversion), for numbers.
    int mConversion;
    // "input_range" and "output_range" of the conversion.
    Vector2 mInputRange;
    Vector2 mOutputRange;
    // "output_curve" of the curve conversion, from the start to the end of
    // the input range.
    Waveform<float> mOutputCurve;
};

// The statics, each guarded where the copy is emitted: for float the ids'
// initializers are at 0x6FEBE0 and 0x6FEC30.
template <class T>
Symbol StateNodeSetValue<T>::sId(MakeSetValueClassName<T>());
template <class T>
Symbol StateNodeSetValue<T>::sClassName = Symbol(MakeSetValueClassName<T>());
template <class T>
PropRegistry StateNodeSetValue<T>::sPropRegistry;
template <class T>
ComMetaData StateNodeSetValue<T>::sMetaData;

static_assert(offsetof(StateNodeSetValue<float>, mValue) == 152);
static_assert(offsetof(StateNodeSetValue<float>, mPersistent) == 384);
static_assert(offsetof(StateNodeSetValue<float>, mConversion) == 388);
static_assert(offsetof(StateNodeSetValue<float>, mInputRange) == 392);
static_assert(offsetof(StateNodeSetValue<float>, mOutputRange) == 400);
static_assert(offsetof(StateNodeSetValue<float>, mOutputCurve) == 408);
static_assert(sizeof(StateNodeSetValue<float>) == 448);
static_assert(sizeof(StateNodeSetValue<bool>) == 448);
static_assert(sizeof(StateNodeSetValue<ResourcePath>) == 448);
static_assert(offsetof(StateNodeSetValue<Hmx::Color>, mPersistent) == 392);
static_assert(sizeof(StateNodeSetValue<Hmx::Color>) == 456);
static_assert(sizeof(StateNodeSetValue<PropRef>) == 656);
static_assert(sizeof(StateNodeSetValue<Waveform<float>>) == 480);
static_assert(sizeof(StateNodeSetValue<Waveform<Hmx::Color>>) == 480);
static_assert(sizeof(StateNodeSetValue<Vector3>) == 456);
static_assert(sizeof(StateNodeSetValue<PropRotation>) == 456);
static_assert(offsetof(StateNodeSetValue<PropXfm>, mPersistent) == 416);
static_assert(sizeof(StateNodeSetValue<PropXfm>) == 480);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mSetValueNode) == 32);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mBlendFrom) == 40);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mBlendTo) == 48);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mStartValue) == 56);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mTargetValue) == 60);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mValue) == 64);
static_assert(offsetof(StateNodeSetValue<float>::Instance, mBlend) == 68);
static_assert(sizeof(StateNodeSetValue<float>::Instance) == 72);
static_assert(offsetof(StateNodeSetValue<bool>::Instance, mBlend) == 60);
static_assert(offsetof(StateNodeSetValue<Symbol>::Instance, mBlend) == 80);
static_assert(offsetof(StateNodeSetValue<Hmx::Color>::Instance, mBlend) == 104);
static_assert(offsetof(StateNodeSetValue<Waveform<float>>::Instance, mBlend) == 176);
static_assert(offsetof(StateNodeSetValue<Vector3>::Instance, mBlend) == 92);
static_assert(offsetof(StateNodeSetValue<PropXfm>::Instance, mBlend) == 164);

// The conversion of a number, as a float: clamped into the input range and
// mapped to where it falls in it, then onto the output range or along the
// curve. An empty input range maps every value to its end. Inlined into
// each number type's _ConvertValue. Name not in the reference map.
template <class Node>
inline float ConvertSetValueNumber(const Node& node, float value) {
    const float in0 = node.mInputRange.x;
    const float in1 = node.mInputRange.y;
    float t = 1.0F;
    if (in1 != in0) {
        const float low = in0 < in1 ? in0 : in1;
        const float high = in1 > in0 ? in1 : in0;
        float clamped = low > value ? low : value;
        if (high < value) {
            clamped = high;
        }
        t = (clamped - in0) / (in1 - in0);
    }
    if (node.mConversion == Node::kConversionRange) {
        return t * (node.mOutputRange.y - node.mOutputRange.x) + node.mOutputRange.x;
    }
    return node.mOutputCurve.Evaluate(t);
}

// What the value nodes do with each type's values: the conversion, which
// only numbers have, and the blend toward the value to reach. A type that
// cannot blend reports it (the report is compiled out; its arguments
// remain) and blends to its empty value. Names not in the reference map.
template <class T>
struct StateNodeValueOps {
    static void Convert(const StateNodeSetValue<T>& node, T& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static T Blend(const StateGraphInstance& graph, const T& from, const T& to, float blend) {
        static_cast<void>(graph);
        static_cast<void>(from);
        static_cast<void>(to);
        static_cast<void>(blend);
        static_cast<void>(TypeName(DrivenPropType<T>::kType, true));
        return T();
    }
};

// Numbers convert and blend as floats.
template <class T>
struct StateNodeNumberOps {
    static void Convert(const StateNodeSetValue<T>& node, T& value) {
        if (node.mConversion == StateNodeSetValue<T>::kConversionNone) {
            return;
        }
        value = static_cast<T>(ConvertSetValueNumber(node, static_cast<float>(value)));
    }
    static T Blend(const StateGraphInstance& graph, const T& from, const T& to, float blend) {
        static_cast<void>(graph);
        return static_cast<T>((static_cast<float>(to) - static_cast<float>(from)) * blend + static_cast<float>(from));
    }
};

template <>
struct StateNodeValueOps<float> : StateNodeNumberOps<float> {};
template <>
struct StateNodeValueOps<signed char> : StateNodeNumberOps<signed char> {};
template <>
struct StateNodeValueOps<short> : StateNodeNumberOps<short> {};
template <>
struct StateNodeValueOps<int> : StateNodeNumberOps<int> {};
template <>
struct StateNodeValueOps<long> : StateNodeNumberOps<long> {};
template <>
struct StateNodeValueOps<unsigned char> : StateNodeNumberOps<unsigned char> {};
template <>
struct StateNodeValueOps<unsigned short> : StateNodeNumberOps<unsigned short> {};
template <>
struct StateNodeValueOps<unsigned int> : StateNodeNumberOps<unsigned int> {};
template <>
struct StateNodeValueOps<unsigned long> : StateNodeNumberOps<unsigned long> {};

// Colours blend in HSV when the transition asks for it, otherwise as their
// sRGB encodings do (0x7B9A10).
template <>
struct StateNodeValueOps<Hmx::Color> {
    static void Convert(const StateNodeSetValue<Hmx::Color>& node, Hmx::Color& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static Hmx::Color Blend(
        const StateGraphInstance& graph,
        const Hmx::Color& from,
        const Hmx::Color& to,
        float blend) {
        Hmx::Color result;
        if (graph.UseHSVColorBlending()) {
            InterpLinearAsGamma_sRGBInHSV(from, to, blend, result);
        } else {
            Hmx::Color fromGamma;
            Hmx::Color toGamma;
            LinearToGamma_sRGB(from, fromGamma);
            LinearToGamma_sRGB(to, toGamma);
            const Hmx::Color mixed(
                (toGamma.red - fromGamma.red) * blend + fromGamma.red,
                (toGamma.green - fromGamma.green) * blend + fromGamma.green,
                (toGamma.blue - fromGamma.blue) * blend + fromGamma.blue,
                (toGamma.alpha - fromGamma.alpha) * blend + fromGamma.alpha);
            GammaToLinear_sRGB(mixed, result);
        }
        return result;
    }
};

// Vectors interpolate component by component, ending exactly at either end
// (0x7D9580).
template <>
struct StateNodeValueOps<Vector3> {
    static void Convert(const StateNodeSetValue<Vector3>& node, Vector3& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static Vector3 Blend(const StateGraphInstance& graph, const Vector3& from, const Vector3& to, float blend) {
        static_cast<void>(graph);
        if (blend == 0.0F) {
            return from;
        }
        if (blend == 1.0F) {
            return to;
        }
        return Vector3{
            (to.x - from.x) * blend + from.x, (to.y - from.y) * blend + from.y, (to.z - from.z) * blend + from.z};
    }
};

// Rotations blend through quaternions or angle by angle, as the
// transition's rotation blending says (0x7E0EC0).
template <>
struct StateNodeValueOps<PropRotation> {
    static void Convert(const StateNodeSetValue<PropRotation>& node, PropRotation& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static PropRotation Blend(
        const StateGraphInstance& graph,
        const PropRotation& from,
        const PropRotation& to,
        float blend) {
        PropRotation result = DrivenPropType<PropRotation>::Default();
        if (graph.GetRotationBlending() != 0) {
            InterpEuler(from, to, blend, result);
        } else {
            InterpEulerLinear(from, to, blend, result);
        }
        return result;
    }
};

// Transforms blend from the identity as rotations do (0x7E8C00).
template <>
struct StateNodeValueOps<PropXfm> {
    static void Convert(const StateNodeSetValue<PropXfm>& node, PropXfm& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static PropXfm Blend(const StateGraphInstance& graph, const PropXfm& from, const PropXfm& to, float blend) {
        PropXfm result = DrivenPropType<PropXfm>::Default();
        if (graph.GetRotationBlending() != 0) {
            InterpXfm(from, to, blend, result);
        } else {
            InterpXfmLinear(from, to, blend, result);
        }
        return result;
    }
};

// Waveforms do not blend: the blend keeps the starting waveform's curves
// (0x7C97E0).
template <class T>
struct StateNodeWaveformOps {
    static void Convert(const StateNodeSetValue<Waveform<T>>& node, Waveform<T>& value) {
        static_cast<void>(node);
        static_cast<void>(value);
    }
    static Waveform<T> Blend(
        const StateGraphInstance& graph,
        const Waveform<T>& from,
        const Waveform<T>& to,
        float blend) {
        static_cast<void>(graph);
        static_cast<void>(to);
        static_cast<void>(blend);
        return Waveform<T>(from);
    }
};

template <>
struct StateNodeValueOps<Waveform<float>> : StateNodeWaveformOps<float> {};
template <>
struct StateNodeValueOps<Waveform<Hmx::Color>> : StateNodeWaveformOps<Hmx::Color> {};

// Reconstructed from eboot.elf at 0x700110.
template <class T>
Symbol StateNodeSetValue<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x700120.
template <class T>
Symbol StateNodeSetValue<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x700130.
template <class T>
int StateNodeSetValue<T>::CurrentRev() const {
    return const_cast<StateNodeSetValue*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x700150.
template <class T>
bool StateNodeSetValue<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x700180.
template <class T>
Component* StateNodeSetValue<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x700190. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* StateNodeSetValue<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeSetValue*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeSetValue(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7002A0.
template <class T>
PropRegistry& StateNodeSetValue<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7002B0.
template <class T>
ComMetaData& StateNodeSetValue<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x700300.
template <class T>
bool StateNodeSetValue<T>::_HasValueToSet() const {
    return true;
}

// Reconstructed from eboot.elf at 0x700310.
template <class T>
bool StateNodeSetValue<T>::_GetTargetRange(float* min, float* max) const {
    static_cast<void>(min);
    static_cast<void>(max);
    return false;
}

// Reconstructed from eboot.elf at 0x7240D0. An undriven value is described
// through a reference to the node's own "value".
template <class T>
const char* StateNodeSetValue<T>::_GetValueDescription() const {
    if (mValue.mDriven && mValue.mProp.IsComplete()) {
        return mValue.mProp.Description(_GetContextEntityPrototype());
    }
    const GameObjectId object = mObject->mId;
    const Symbol id = GetId();
    PropPath::Node member;
    PropPath::Node value;
    const PropPath path((member = Symbol("value")), (value = Symbol("value")));
    const PropRef self(object, id, path);
    return self.ValueToString(mObject->mEntity);
}

// Reconstructed from eboot.elf at 0x725EA0.
template <class T>
void StateNodeSetValue<T>::_LoadOldValue(const Component& saved) {
    static Symbol value;
    if (value == Symbol()) {
        value = Symbol("value");
    }
    static Symbol value_prop;
    if (value_prop == Symbol()) {
        value_prop = Symbol("value_prop");
    }
    mValue.mValue = GetPropValue<T>(saved, PropPath(value));
    mValue.mProp = GetPropValue<PropRef>(saved, PropPath(value_prop));
    mValue.mDriven = mValue.mProp.IsComplete();
}

// Reconstructed from eboot.elf at 0x728510. The value is zero clamped into
// the target's range.
template <class T>
ResourcePtr<WaveformResource<float>> StateNodeSetValue<T>::_MakeDefaultCurve() const {
    float range[2] = {-1e30F, 1e30F};
    _GetTargetRange(&range[0], &range[1]);
    const float lowest = range[0] > 0.0F ? range[0] : 0.0F;
    const float value = range[1] < 0.0F ? range[1] : lowest;
    ResourcePtr<WaveformResource<float>> curve(new WaveformResource<float>());
    curve->SetType(2);
    TKeyTrack<ValueTangent<float>> track;
    track.Reserve(2);
    track.AddKey(ValueTangent<float>{value, 0.0F, 0.0F, true}, 0.0F, false, false);
    track.AddKey(ValueTangent<float>{value, 0.0F, 0.0F, true}, 1.0F, false, false);
    curve->SetCurve(0, track);
    return curve;
}

// Reconstructed from eboot.elf at 0x726350.
template <class T>
void StateNodeSetValue<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeSetValue prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    GroupMetadata& value = prototype.mValue.Register(
        registry, Symbol("value"), offsetof(StateNodeSetValue, mValue), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    value.mHelp = String("This value will be applied to the target's value.");
    value.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeSetValue&>(args.mComponent)._HasValueToSet();
    });
    value.mReserved = true;

    BoolMetadata& persistent = TypeSpecificMetadata(
        registry.RegisterProp("persistent", offsetof(StateNodeSetValue, mPersistent), kPropertyBool, 1),
        prototype.mPersistent);
    persistent.mDefault = prototype.mPersistent;
    static_cast<void>(Symbol("prop"));
    persistent.mHelp = String("If checked, this node will continually enforce the specified value each frame.");
    persistent.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    // A change to the curve conversion replaces a default curve with one in
    // the target's range.
    IntMetadata<int>& conversion = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "conversion", offsetof(StateNodeSetValue, mConversion), kPropertyInt, 4).mMetadata.Get());
    conversion.mDefault = prototype.mConversion;
    static_cast<void>(Symbol("prop"));
    conversion.mHelp = String(
        "For numerical types, the value to set can be converted to a new range of values, either directly or "
        "according to a user-specified curve. This is useful when trying to set the target value according to "
        "some other input value which may not be expressed in the same range.");
    conversion.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kConversionNone, String("None"), String("Direct pass-through with no conversion applied.")},
        AllowedValue<int>{
            kConversionRange,
            String("Range-based"),
            String("Values are linearly remapped from the specified input range to the specified output range.")},
        AllowedValue<int>{
            kConversionCurve,
            String("Curve-based"),
            String("Values are remapped from the specified input range to an output value according to the "
                   "specified curve.")}};
    conversion.mOnChanged = [](const PropAccessorArgs& args) {
        if (*static_cast<const int*>(args.mValue) != kConversionCurve) {
            return;
        }
        StateNodeSetValue& node = *static_cast<StateNodeSetValue*>(args.mComponent);
        const Waveform<float> defaultCurve;
        bool isDefault = false;
        if (node.mOutputCurve.mResource.Get() == defaultCurve.mResource.Get() ||
            *node.mOutputCurve.mResource == *defaultCurve.mResource) {
            isDefault = node.mOutputCurve.mOverride.Get() == nullptr;
        }
        if (isDefault) {
            ResourcePtr<WaveformResource<float>> curve = node._MakeDefaultCurve();
            node.mOutputCurve.SetResource(curve.Get());
        }
    };
    conversion.mIsHidden = !IsNumber(DrivenPropType<T>::kType);

    // The input range defaults to and is bounded by the driving property's
    // range when the value is driven, otherwise by the target's.
    GroupMetadata& inputRange = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "input_range", offsetof(StateNodeSetValue, mInputRange), kPropertyStruct, sizeof(Vector2)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Vector2 zero = {0.0F, 0.0F};
    PropRegistry& inputMembers = inputRange.mRegistry;
    inputRange.mHelp = String("The range from which to convert the input value.");
    inputRange.mKind = 1;
    inputRange.mIsHidden._Set([](const TagArgs& args) {
        return !IsNumber(DrivenPropType<T>::kType) ||
               static_cast<const StateNodeSetValue&>(args.mComponent).mConversion == kConversionNone;
    });
    auto inputMin = [](const TagArgs& args) {
        const StateNodeSetValue& node = static_cast<const StateNodeSetValue&>(args.mComponent);
        float min = -1e30F;
        if (node.mValue.mDriven && node.mValue.mProp.IsComplete()) {
            GetPropRefRange(node._GetContextEntityPrototype(), node.mValue.mProp, &min, nullptr);
        } else {
            node._GetTargetRange(&min, nullptr);
        }
        return min;
    };
    auto inputMax = [](const TagArgs& args) {
        const StateNodeSetValue& node = static_cast<const StateNodeSetValue&>(args.mComponent);
        float max = 1e30F;
        if (node.mValue.mDriven && node.mValue.mProp.IsComplete()) {
            GetPropRefRange(node._GetContextEntityPrototype(), node.mValue.mProp, nullptr, &max);
        } else {
            node._GetTargetRange(nullptr, &max);
        }
        return max;
    };
    FloatMetadata& inputX =
        TypeSpecificMetadata(inputMembers.RegisterProp("x", offsetof(Vector2, x), kPropertyFloat, 4), zero.x);
    inputX.mDefault = zero.x;
    static_cast<void>(Symbol("prop"));
    inputX.mMin._Set(inputMin);
    inputX.mMax._Set(inputMax);
    inputX.mDefault._Set(inputMin);
    FloatMetadata& inputY =
        TypeSpecificMetadata(inputMembers.RegisterProp("y", offsetof(Vector2, y), kPropertyFloat, 4), zero.y);
    inputY.mDefault = zero.y;
    static_cast<void>(Symbol("prop"));
    inputY.mMin._Set(inputMin);
    inputY.mMax._Set(inputMax);
    inputY.mDefault._Set(inputMax);

    // The output range defaults to and is bounded by the target's range.
    GroupMetadata& outputRange = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "output_range", offsetof(StateNodeSetValue, mOutputRange), kPropertyStruct, sizeof(Vector2))
                                                                      .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& outputMembers = outputRange.mRegistry;
    outputRange.mHelp = String("The range to which to input value will be remapped.");
    outputRange.mKind = 1;
    outputRange.mIsHidden._Set([](const TagArgs& args) {
        return !IsNumber(DrivenPropType<T>::kType) ||
               static_cast<const StateNodeSetValue&>(args.mComponent).mConversion != kConversionRange;
    });
    auto targetMin = [](const TagArgs& args) {
        float min = -1e30F;
        static_cast<const StateNodeSetValue&>(args.mComponent)._GetTargetRange(&min, nullptr);
        return min;
    };
    auto targetMax = [](const TagArgs& args) {
        float max = 1e30F;
        static_cast<const StateNodeSetValue&>(args.mComponent)._GetTargetRange(nullptr, &max);
        return max;
    };
    FloatMetadata& outputX =
        TypeSpecificMetadata(outputMembers.RegisterProp("x", offsetof(Vector2, x), kPropertyFloat, 4), zero.x);
    outputX.mDefault = zero.x;
    static_cast<void>(Symbol("prop"));
    outputX.mMin._Set(targetMin);
    outputX.mMax._Set(targetMax);
    outputX.mDefault._Set(targetMin);
    FloatMetadata& outputY =
        TypeSpecificMetadata(outputMembers.RegisterProp("y", offsetof(Vector2, y), kPropertyFloat, 4), zero.y);
    outputY.mDefault = zero.y;
    static_cast<void>(Symbol("prop"));
    outputY.mMin._Set(targetMin);
    outputY.mMax._Set(targetMax);
    outputY.mDefault._Set(targetMax);

    // The curve is a sampled curve (type 2) in the target's range.
    FloatWaveformMetadata& outputCurve = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
        registry.RegisterProp(
            "output_curve", offsetof(StateNodeSetValue, mOutputCurve), kPropertyWaveformFloat, sizeof(Waveform<float>)),
        prototype.mOutputCurve));
    outputCurve.mDefault = Waveform<float>(prototype.mOutputCurve);
    static_cast<void>(Symbol("prop"));
    outputCurve.mHelp = String(
        "The curve used to remap the input value. The beginning of the curve corresponds to the first component "
        "of the input range and the end of the curve corresponds to the second component of the input range.");
    outputCurve.mAllowedTypes = eastl::vector<int>{2};
    outputCurve.mDefault._Set([](const TagArgs& args) {
        const ResourcePtr<WaveformResource<float>> curve =
            static_cast<const StateNodeSetValue&>(args.mComponent)._MakeDefaultCurve();
        return Waveform<float>(curve.Get());
    });
    outputCurve.mIsHidden._Set([](const TagArgs& args) {
        return !IsNumber(DrivenPropType<T>::kType) ||
               static_cast<const StateNodeSetValue&>(args.mComponent).mConversion != kConversionCurve;
    });
    outputCurve.mMin._Set(targetMin);
    outputCurve.mMax._Set(targetMax);

    StateBase::_FixDrivenProp<T>(registry, PropPath(Symbol("value")));
    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol("false");
}

// Reconstructed from eboot.elf at 0x723830.
template <class T>
void StateNodeSetValue<T>::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateNodeSetValue", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x7EFF90. The binary emits the factory
// in stategraph/StateNodeSetValue.o with the class's Init.
template <class T>
Component* StateNodeSetValue<T>::_Create() {
    return new StateNodeSetValue();
}

// Reconstructed from eboot.elf at 0x7E9390.
template <class T>
void StateNodeSetValue<T>::Init() {
    InitComponentClass<StateNodeSetValue>();
}

// Reconstructed from eboot.elf at 0x724300.
template <class T>
void StateNodeSetValue<T>::Instance::_Enter() {
    if (!mSetValueNode->mPersistent) {
        mTargetValue = _GetValueToApply();
    }
    if (Instance* const from = _GetEnteredBlendFrom()) {
        mValue = from->mValue;
    } else {
        _GetCurrentValue(mValue);
        mStartValue = mValue;
    }
}

// Reconstructed from eboot.elf at 0x724370.
template <class T>
void StateNodeSetValue<T>::Instance::_SetBlend(float blend) {
    mBlend = blend;
}

// Reconstructed from eboot.elf at 0x724380.
template <class T>
bool StateNodeSetValue<T>::Instance::_Poll() {
    if (mSetValueNode->mPersistent) {
        mTargetValue = _GetValueToApply();
    }
    bool blending;
    if (mSetValueNode->CanBlend() && mGraph->_IsBlendingIn()) {
        if (Instance* const from = _GetEnteredBlendFrom()) {
            mStartValue = from->mValue;
        }
        mValue = StateNodeValueOps<T>::Blend(*mGraph, mStartValue, mTargetValue, mBlend);
        blending = true;
    } else {
        mValue = mTargetValue;
        blending = false;
    }
    if (_GetEnteredBlendTo() == nullptr) {
        _ApplyValue(mValue);
    }
    if (mSetValueNode->mPersistent || blending) {
        return true;
    }
    return !_IsFinishedApplyingValue();
}

// Reconstructed from eboot.elf at 0x724470.
template <class T>
bool StateNodeSetValue<T>::Instance::_WantsBlendPartner() const {
    return mSetValueNode->CanBlend() && mBlendFrom == nullptr;
}

// Reconstructed from eboot.elf at 0x7244A0.
template <class T>
bool StateNodeSetValue<T>::Instance::_TakeBlendPartner(StateNodeInstance* previous) {
    Instance* const partner = static_cast<Instance*>(previous->FindBlendPartner(*this));
    if (partner == nullptr) {
        return false;
    }
    mBlendFrom = partner;
    partner->mBlendTo = this;
    return true;
}

// Reconstructed from eboot.elf at 0x7244D0.
template <class T>
bool StateNodeSetValue<T>::Instance::_IsBlendPartner(const StateNodeInstance& other) const {
    if (mBlendTo != nullptr) {
        return false;
    }
    return _HasSameTarget(other);
}

// Reconstructed from eboot.elf at 0x724530.
template <class T>
T StateNodeSetValue<T>::Instance::_GetValueToApply() const {
    const StateNodeSetValue& node = *mSetValueNode;
    Entity* const entity = _GetContextEntity();
    T value = node.mValue.mDriven && node.mValue.mProp.IsComplete() ? node.mValue.mProp.template Get<T>(entity)
                                                                    : node.mValue.mValue;
    _ConvertValue(value);
    return value;
}

// Reconstructed from eboot.elf at 0x7248E0.
template <class T>
bool StateNodeSetValue<T>::Instance::_IsFinishedApplyingValue() const {
    return true;
}

// Reconstructed from eboot.elf at 0x724C00.
template <class T>
void StateNodeSetValue<T>::Instance::_ConvertValue(T& value) const {
    StateNodeValueOps<T>::Convert(*mSetValueNode, value);
}

// Registers the StateNodeSetValue<T> types (stategraph/StateNodeSetValue.o,
// 0x7E9320-0x7F2307), calling each type's Init.
void StateNodeSetValueInit();  // 0x7E9320
