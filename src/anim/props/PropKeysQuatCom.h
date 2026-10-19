#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/rotation/Quat.h"
#include "utl/text/Symbol.h"

// The quaternion slots of the property animation keys, which store
// their rotation in a 3x3 matrix property of a TransCom (read and written
// through PropRef::Find rather than _GetProp and _SetProp), are
// specialized: a key reads the matrix's rotation, and applying a frame
// writes the rotation matrix and marks the local transform dirty.
template <>
void PropKeysTypeCom<Hmx::Quat>::UpdateValueAtIndex(unsigned long index, Entity* entity);  // slot 63: 0x312890
template <>
unsigned long PropKeysTypeCom<Hmx::Quat>::AddNewKey(float frame, Entity* entity, bool replace);  // slot 69: 0x3127D0
template <>
bool PropKeysTypeCom<Hmx::Quat>::ApplyFrame(float frame, Entity* entity) const;  // slot 70: 0x312910

// The keys of a rotation stored as a "quaternion" struct in a property
// animation (anim/PropKeysQuatCom.o, 0x311080-0x31498F; class id
// "PropKeysQuatCom"): quaternions, which interpolate linearly by default
// and may take any interpolation; the editor shows them as Euler angles in
// degrees. The driver blends the playbacks' rotations by weight. The vtable
// at 0x18F87E0 has 76 slots and the key track's at 0x18F8A50 17. The
// object is 320 bytes.
class PropKeysQuatCom : public PropKeysTypeCom<Hmx::Quat> {
public:
    PropKeysQuatCom() {
        mInterpolation = KeyTrackBase::kInterpolationLinear;
    }
    // Slots 0-1: 0x3121E0, 0x312A10.
    ~PropKeysQuatCom() override {}

    Symbol GetId() const override;          // slot 4: 0x312AA0
    Symbol GetClassName() const override;   // slot 5: 0x312AB0
    int CurrentRev() const override;        // slot 7: 0x312AC0
    bool IsA(Symbol type) const override;   // slot 8: 0x312AE0
    Component* AsComponent() override;      // slot 9: 0x312B10
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x312B20
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x312D50
    ComMetaData& _GetMetaData() override;       // slot 23: 0x312D60
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x311230
    PropertyType GetKeysType() const override;      // slot 55: 0x312FB0
    Symbol GetKeysSubType() const override;         // slot 56: 0x312FC0
    // Slot 57: every interpolation, the spline included.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x313060
    // Slots 73-75: for the editor an array of the Euler angles in degrees,
    // otherwise of x, y, z and w; the quaternion of three angles in degrees
    // (normalized) or of four components; the zero quaternion.
    DataNode _ValueToDataNode(const Hmx::Quat& value, bool forEditor) const override;  // 0x3133B0
    Hmx::Quat _ValueFromDataNode(const DataNode& value) const override;                 // 0x313740
    Hmx::Quat _DefaultValue() const override;                                           // 0x3138B0

    // kPropertyStruct. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyStruct;
    }
    // "quaternion", created on first use (local static at 0x19FA6F8). Inlined
    // into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("quaternion");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x311080
    // Registers the class as the keys of "quaternion" structs, under the
    // "PropKeys" interface.
    static void Register();  // 0x311150
    // "TKey<kStruct>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x311300

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ABC40, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B3CC0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x311310

    // "PropKeysQuatCom", constructed by the static initializer at 0x314820.
    static Symbol sId;  // 0x19FA4A0
    // Also "PropKeysQuatCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FA4A8
    static PropRegistry sPropRegistry;  // 0x19FA4B0
    static ComMetaData sMetaData;       // 0x19FA550
};

static_assert(offsetof(PropKeysQuatCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysQuatCom, mKeys) == 280);
static_assert(sizeof(PropKeysQuatCom) == 320);

// The quaternion keys' driver blends the playbacks' rotations by weight
// (each on the running sum's hemisphere), normalizes the sum and writes its
// rotation matrix, as _SetDrivenProperty does; nothing when the weights are
// not positive or the sum is zero. The keys' SampleValueAtFrame is emitted
// at 0x3125F0.
template <>
void TPropDriver<PropKeysQuatCom>::_Poll(GameObject* root);  // 0x312270
// Writes the quaternion's rotation matrix into the property's storage and
// marks the TransCom's local transform dirty. No caller was found; the
// driver inlines the same code. The binary takes the keys' PropRef and
// finds the storage through PropRef::Find.
template <>
void TPropDriver<PropKeysQuatCom>::_SetDrivenProperty(
    Component* component,
    const PropPath& path,
    const void* value);  // 0x312720
