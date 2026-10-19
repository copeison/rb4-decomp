#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "utl/text/Symbol.h"

// The keys of a colour property in a property animation
// (anim/PropKeysColorCom.o, 0x302B50-0x30534F; class id
// "PropKeysColorCom"): linear colours, which interpolate linearly by
// default, in gamma space, and have no spline. The driver blends the
// playbacks' colours by weight. The vtable at 0x18F77D0 has 76 slots and
// the key track's at 0x18F7A40 17. The object is 320 bytes.
class PropKeysColorCom : public PropKeysTypeCom<Hmx::Color> {
public:
    PropKeysColorCom() {
        mInterpolation = KeyTrackBase::kInterpolationLinear;
    }
    // Slots 0-1: 0x303720, 0x303900.
    ~PropKeysColorCom() override {}

    Symbol GetId() const override;          // slot 4: 0x303990
    Symbol GetClassName() const override;   // slot 5: 0x3039A0
    int CurrentRev() const override;        // slot 7: 0x3039B0
    bool IsA(Symbol type) const override;   // slot 8: 0x3039D0
    Component* AsComponent() override;      // slot 9: 0x303A00
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x303A10
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x303C40
    ComMetaData& _GetMetaData() override;       // slot 23: 0x303C50
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x302D00
    PropertyType GetKeysType() const override;      // slot 55: 0x303EA0
    Symbol GetKeysSubType() const override;         // slot 56: 0x303EB0
    // Slot 57: anything but Spline.
    bool IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const override;  // 0x303F50
    // Slot 58: these are colour keys.
    bool IsColorKeys() const override;  // 0x303F60
    // Slots 73-75: a colour node without its space, the node's linear
    // colour, transparent black.
    DataNode _ValueToDataNode(const Hmx::Color& value, bool forEditor) const override;  // 0x304730
    Hmx::Color _ValueFromDataNode(const DataNode& value) const override;                 // 0x304750
    Hmx::Color _DefaultValue() const override;                                           // 0x304760

    // kPropertyColor. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyColor;
    }
    // The empty symbol, created on first use (local static at 0x19F9EA8).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x302B50
    // Registers the class as the keys of colour properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x302C20
    // "TKey<kColor>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x302DD0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ACF00, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B5C90.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x302DE0

    // "PropKeysColorCom", constructed by the static initializer at 0x305280.
    static Symbol sId;  // 0x19F9C50
    // Also "PropKeysColorCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19F9C58
    static PropRegistry sPropRegistry;  // 0x19F9C60
    static ComMetaData sMetaData;       // 0x19F9D00
};

static_assert(offsetof(PropKeysColorCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysColorCom, mKeys) == 280);
static_assert(sizeof(PropKeysColorCom) == 320);

// The colour keys' driver sets the playbacks' mean colour, weighted by
// their weights, through the template's _SetDrivenProperty (0x305080);
// nothing when the weights are not positive.
template <>
void TPropDriver<PropKeysColorCom>::_Poll(GameObject* root);  // 0x3037B0
