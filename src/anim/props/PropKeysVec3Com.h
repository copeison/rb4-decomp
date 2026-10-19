#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/vector/Vector3.h"
#include "utl/text/Symbol.h"

// The keys of a 3D vector property in a property animation
// (anim/PropKeysVec3Com.o, 0x322340-0x32531F; class id "PropKeysVec3Com"):
// vectors, which interpolate linearly by default and may take any
// interpolation; the driver takes the heaviest playback's value. The vtable
// at 0x18F9BB8 has 76 slots and the key track's at 0x18F9E28 17. The
// object is 320 bytes.
class PropKeysVec3Com : public PropKeysTypeCom<Vector3> {
public:
    PropKeysVec3Com() {
        mInterpolation = KeyTrackBase::kInterpolationLinear;
    }
    // Slots 0-1: 0x3230D0, 0x323160.
    ~PropKeysVec3Com() override {}

    Symbol GetId() const override;          // slot 4: 0x3231F0
    Symbol GetClassName() const override;   // slot 5: 0x323200
    int CurrentRev() const override;        // slot 7: 0x323210
    bool IsA(Symbol type) const override;   // slot 8: 0x323230
    Component* AsComponent() override;      // slot 9: 0x323260
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x323270
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x3234A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x3234B0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x3224F0
    PropertyType GetKeysType() const override;      // slot 55: 0x323710
    Symbol GetKeysSubType() const override;         // slot 56: 0x323720
    // Slot 57: every interpolation, the spline included.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x3237C0
    // Slots 73-75: an array of the three floats, the vector of a node's
    // array, the zero vector.
    DataNode _ValueToDataNode(const Vector3& value, bool forEditor) const override;  // 0x324150
    Vector3 _ValueFromDataNode(const DataNode& value) const override;                 // 0x3242C0
    Vector3 _DefaultValue() const override;                                           // 0x324440

    // kPropertyVector3. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyVector3;
    }
    // The empty symbol, created on first use (local static at 0x19FB3D8).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x322340
    // Registers the class as the keys of vector properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x322410
    // "TKey<kVector3>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x3225C0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AAF10, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B2B00.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x3225D0

    // "PropKeysVec3Com", constructed by the static initializer at 0x325260.
    static Symbol sId;  // 0x19FB180
    // Also "PropKeysVec3Com". Name not in the reference map.
    static Symbol sClassName;           // 0x19FB188
    static PropRegistry sPropRegistry;  // 0x19FB190
    static ComMetaData sMetaData;       // 0x19FB230
};

static_assert(offsetof(PropKeysVec3Com, mInterpolation) == 272);
static_assert(offsetof(PropKeysVec3Com, mKeys) == 280);
static_assert(sizeof(PropKeysVec3Com) == 320);

// The driver's _Poll is the template's (0x324B60), which sets the heaviest
// playback's vector.
