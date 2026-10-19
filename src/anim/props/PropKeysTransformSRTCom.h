#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "entity/waveform/RotationKey.h"
#include "utl/text/Symbol.h"

// The transform keys' property slots read and write the property's
// transform (kPropertyXfm) and keep the key's quaternion in step.
template <>
void PropKeysTypeCom<TransformSRTKey>::UpdateValueAtIndex(unsigned long index, Entity* entity);  // slot 63: 0x31F810
template <>
unsigned long PropKeysTypeCom<TransformSRTKey>::AddNewKey(float frame, Entity* entity, bool replace);  // slot 69: 0x31F450
template <>
bool PropKeysTypeCom<TransformSRTKey>::ApplyFrame(float frame, Entity* entity) const;  // slot 70: 0x31F8F0

// The keys of a transform property in a property animation
// (anim/PropKeysTransformSRTCom.o, 0x31E2E0-0x32233F; class id
// "PropKeysTransformSRTCom"): scale, rotation and translation kept with
// the rotation's quaternion (TransformSRTKey), which interpolate linearly
// by default, blending the angles one by one (the 0x40000000 flag); the
// driver takes the heaviest playback's transform. The vtable at 0x18F97D0
// has 76 slots and the key track's at 0x18F9A40 17. The object is 320
// bytes.
class PropKeysTransformSRTCom : public PropKeysTypeCom<TransformSRTKey> {
public:
    // Linear, blending the angles one by one.
    PropKeysTransformSRTCom();  // 0x31E570
    // Slots 0-1: 0x31F3C0, 0x31FF90.
    ~PropKeysTransformSRTCom() override {}

    Symbol GetId() const override;          // slot 4: 0x320020
    Symbol GetClassName() const override;   // slot 5: 0x320030
    int CurrentRev() const override;        // slot 7: 0x320040
    bool IsA(Symbol type) const override;   // slot 8: 0x320060
    Component* AsComponent() override;      // slot 9: 0x320090
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x3200A0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x3202D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x3202E0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x31E490
    PropertyType GetKeysType() const override;      // slot 55: 0x320540
    Symbol GetKeysSubType() const override;         // slot 56: 0x320550
    // Slots 57, 59 and 60: every interpolation; rotation keys with a
    // serialized value.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x3205F0
    bool IsRotationKeys() const override;      // 0x320600
    bool HasSerializedValue() const override;  // 0x320610
    // Slots 73-75: an array of the transform's nine floats (the angles
    // refreshed), the key of a node's nine floats, the identity.
    DataNode _ValueToDataNode(const TransformSRTKey& value, bool forEditor) const override;  // 0x3209E0
    TransformSRTKey _ValueFromDataNode(const DataNode& value) const override;                 // 0x321120
    TransformSRTKey _DefaultValue() const override;                                           // 0x321560

    // kPropertyXfm. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyXfm;
    }
    // The empty symbol, created on first use (local static at 0x19FB158).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x31E2E0
    // Registers the class as the keys of transform properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x31E3B0
    // "TKey<kTransformSRT>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x31E560

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AB800, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B3470.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x31E620

    // "PropKeysTransformSRTCom", constructed by the static initializer at 0x3221D0.
    static Symbol sId;  // 0x19FAF00
    // Also "PropKeysTransformSRTCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FAF08
    static PropRegistry sPropRegistry;  // 0x19FAF10
    static ComMetaData sMetaData;       // 0x19FAFB0
};

static_assert(offsetof(PropKeysTransformSRTCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysTransformSRTCom, mKeys) == 280);
static_assert(sizeof(PropKeysTransformSRTCom) == 320);

// The transform keys' driver uses the template's _Poll (0x321E10), whose
// write refreshes the chosen key's Euler angles and sets the transform.
template <>
void TPropDriver<PropKeysTransformSRTCom>::_SetDrivenProperty(
    Component* component,
    const PropPath& path,
    const void* value);  // 0x31FD70
