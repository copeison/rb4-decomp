#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "entity/waveform/RotationKey.h"
#include "utl/text/Symbol.h"

// The rotation keys' property slots read and write the property's Euler
// angles (kPropertyEuler) and keep the key's quaternion in step.
template <>
void PropKeysTypeCom<RotationKey>::UpdateValueAtIndex(unsigned long index, Entity* entity);  // slot 63: 0x315CC0
template <>
unsigned long PropKeysTypeCom<RotationKey>::AddNewKey(float frame, Entity* entity, bool replace);  // slot 69: 0x315B70
template <>
bool PropKeysTypeCom<RotationKey>::ApplyFrame(float frame, Entity* entity) const;  // slot 70: 0x315E00

// The keys of a rotation property in a property animation
// (anim/PropKeysRotationCom.o, 0x314990-0x318DAF; class id
// "PropKeysRotationCom"): Euler angles kept with their quaternion
// (RotationKey), which interpolate linearly by default, blending the
// angles one by one (the 0x40000000 flag) unless the interpolation is
// changed; the driver takes the heaviest playback's rotation. The vtable
// at 0x18F8BC8 has 76 slots and the key track's at 0x18F8E38 17. The
// object is 320 bytes.
class PropKeysRotationCom : public PropKeysTypeCom<RotationKey> {
public:
    // Linear, blending the angles one by one.
    PropKeysRotationCom();  // 0x314C20
    // Slots 0-1: 0x315AE0, 0x316470.
    ~PropKeysRotationCom() override {}

    Symbol GetId() const override;          // slot 4: 0x316500
    Symbol GetClassName() const override;   // slot 5: 0x316510
    int CurrentRev() const override;        // slot 7: 0x316520
    bool IsA(Symbol type) const override;   // slot 8: 0x316540
    Component* AsComponent() override;      // slot 9: 0x316570
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x316580
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x3167B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x3167C0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x314B40
    PropertyType GetKeysType() const override;      // slot 55: 0x316A10
    Symbol GetKeysSubType() const override;         // slot 56: 0x316A20
    // Slots 57, 59 and 60: every interpolation; rotation keys with a
    // serialized value.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x316AC0
    bool IsRotationKeys() const override;      // 0x316AD0
    bool HasSerializedValue() const override;  // 0x316AE0
    // Slots 73-75: an array of the Euler angles (refreshed), the key of a
    // node's three angles, no rotation.
    DataNode _ValueToDataNode(const RotationKey& value, bool forEditor) const override;  // 0x316E90
    RotationKey _ValueFromDataNode(const DataNode& value) const override;                 // 0x317020
    RotationKey _DefaultValue() const override;                                           // 0x3171E0

    // kPropertyEuler. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyEuler;
    }
    // The empty symbol, created on first use (local static at 0x19FA988).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x314990
    // Registers the class as the keys of rotation properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x314A60
    // "TKey<kRotation>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x314C10

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AB3C0, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B2C70.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x314CD0

    // "PropKeysRotationCom", constructed by the static initializer at 0x318C40.
    static Symbol sId;  // 0x19FA730
    // Also "PropKeysRotationCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FA738
    static PropRegistry sPropRegistry;  // 0x19FA740
    static ComMetaData sMetaData;       // 0x19FA7E0
};

static_assert(offsetof(PropKeysRotationCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysRotationCom, mKeys) == 280);
static_assert(sizeof(PropKeysRotationCom) == 320);

// The rotation keys' driver uses the template's _Poll (0x317A70), whose
// write refreshes the chosen key's Euler angles and sets them.
template <>
void TPropDriver<PropKeysRotationCom>::_SetDrivenProperty(
    Component* component,
    const PropPath& path,
    const void* value);  // 0x316250
