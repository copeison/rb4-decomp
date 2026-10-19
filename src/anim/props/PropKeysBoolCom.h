#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The keys of a bool property in a property animation (anim/PropKeysBoolCom.o,
// 0x2FFD60-0x302A91 with its template instances; class id
// "PropKeysBoolCom"): a key's value is the property's until the next key.
// The vtable at 0x18F73A8 has 76 slots and the key track's at 0x18F7618 17.
// The object is 320 bytes.
class PropKeysBoolCom : public PropKeysTypeCom<bool> {
public:
    PropKeysBoolCom() {}
    // Slots 0-1: 0x300AE0, 0x300B70.
    ~PropKeysBoolCom() override {}

    Symbol GetId() const override;          // slot 4: 0x300C00
    Symbol GetClassName() const override;   // slot 5: 0x300C10
    int CurrentRev() const override;        // slot 7: 0x300C20
    bool IsA(Symbol type) const override;   // slot 8: 0x300C40
    Component* AsComponent() override;      // slot 9: 0x300C70
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x300C80
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x300EB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x300EC0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x2FFF10
    PropertyType GetKeysType() const override;      // slot 55: 0x3010F0
    Symbol GetKeysSubType() const override;         // slot 56: 0x301100
    // Slots 73-75: an integer node, any non-zero integer, false.
    DataNode _ValueToDataNode(const bool& value, bool forEditor) const override;  // 0x3019E0
    bool _ValueFromDataNode(const DataNode& value) const override;                 // 0x301A00
    bool _DefaultValue() const override;                                           // 0x301A20

    // kPropertyBool. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyBool;
    }
    // The empty symbol, created on first use (local static at 0x19F9C38).
    // Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x2FFD60
    // Registers the class as the keys of bool properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x2FFE30
    // "TKey<kBool>"; no caller was found (the registration names the struct
    // "TKey<bool>"). Name not in the reference map.
    static const char* StaticKeyTypeName();  // 0x2FFFE0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A8030, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B1E60.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2FFFF0

    // "PropKeysBoolCom", constructed by the static initializer at 0x3029E0,
    // which first sets a shared header's null object id to -1 (0x19F99D8).
    static Symbol sId;  // 0x19F99E0
    // Also "PropKeysBoolCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19F99E8
    static PropRegistry sPropRegistry;  // 0x19F99F0
    static ComMetaData sMetaData;       // 0x19F9A90
};

static_assert(offsetof(PropKeysBoolCom, mInterpolation) == 272);
static_assert(offsetof(PropKeysBoolCom, mKeys) == 280);
static_assert(sizeof(PropKeysBoolCom) == 320);
