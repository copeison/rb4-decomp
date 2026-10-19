#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The keys of a float property in a property animation
// (anim/PropKeysFloatCom.o, 0x305D40-0x308AAF; class id
// "PropKeysFloatCom"): float values with tangents, which interpolate
// linearly by default and may take any interpolation, the spline included.
// The driver blends the playbacks' values by weight. The vtable at
// 0x18F7E10 has 76 slots and the key track's at 0x18F8080 17. The object
// is 320 bytes.
class PropKeysFloatCom : public PropKeysTypeCom<ValueTangent<float>> {
public:
    PropKeysFloatCom() {
        mInterpolation = KeyTrackBase::kInterpolationLinear;
    }
    // Slots 0-1: 0x306BA0, 0x306F70.
    ~PropKeysFloatCom() override {}

    Symbol GetId() const override;          // slot 4: 0x307000
    Symbol GetClassName() const override;   // slot 5: 0x307010
    int CurrentRev() const override;        // slot 7: 0x307020
    bool IsA(Symbol type) const override;   // slot 8: 0x307040
    Component* AsComponent() override;      // slot 9: 0x307070
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x307080
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x3072B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x3072C0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x305EF0
    // Slot 42: the keys have tangents.
    bool HasTangents() const override;          // 0x3072D0
    PropertyType GetKeysType() const override;  // slot 55: 0x307520
    Symbol GetKeysSubType() const override;     // slot 56: 0x307530
    // Slot 57: every interpolation, the spline included.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x3075D0
    // Slots 73-75: a float node, the node's float with automatic tangents,
    // zero with automatic tangents.
    DataNode _ValueToDataNode(const ValueTangent<float>& value, bool forEditor) const override;  // 0x307E40
    ValueTangent<float> _ValueFromDataNode(const DataNode& value) const override;                 // 0x307E60
    ValueTangent<float> _DefaultValue() const override;                                           // 0x307E90

    // kPropertyFloat. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyFloat;
    }
    // The empty symbol, created on first use (local static at 0x19FA178).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x305D40
    // Registers the class as the keys of float properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x305E10
    // "TKey<kFloat>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x305FC0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AAA60, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B2990.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x305FD0

    // "PropKeysFloatCom", constructed by the static initializer at 0x3089F0.
    static Symbol sId;  // 0x19F9F18
    // Also "PropKeysFloatCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19F9F20
    static PropRegistry sPropRegistry;  // 0x19F9F30
    static ComMetaData sMetaData;       // 0x19F9FD0
};

static_assert(offsetof(PropKeysFloatCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysFloatCom, mKeys) == 280);
static_assert(sizeof(PropKeysFloatCom) == 320);

// The float keys' driver sets the playbacks' mean value, weighted by their
// weights, with automatic tangents; nothing when the weights are not
// positive.
template <>
void TPropDriver<PropKeysFloatCom>::_Poll(GameObject* root);  // 0x306C30
