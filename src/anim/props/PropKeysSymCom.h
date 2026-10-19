#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The keys of a symbol property in a property animation
// (anim/PropKeysSymCom.o, 0x31B470-0x31E1F0; class id "PropKeysSymCom"): a
// key's value is the property's until the next key. The vtable at
// 0x18F93E8 has 76 slots. The object is 320 bytes.
class PropKeysSymCom : public PropKeysTypeCom<Symbol> {
public:
    PropKeysSymCom() {}
    // Slots 0-1: 0x31C200, 0x31C290.
    ~PropKeysSymCom() override {}

    Symbol GetId() const override;          // slot 4: 0x31C320
    Symbol GetClassName() const override;   // slot 5: 0x31C330
    int CurrentRev() const override;        // slot 7: 0x31C340
    bool IsA(Symbol type) const override;   // slot 8: 0x31C360
    Component* AsComponent() override;      // slot 9: 0x31C390
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x31C3A0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x31C5D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x31C5E0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x31B620
    PropertyType GetKeysType() const override;      // slot 55: 0x31C830
    Symbol GetKeysSubType() const override;         // slot 56: 0x31C840
    // Slots 73-75: a symbol node, the node's symbol, the empty symbol.
    DataNode _ValueToDataNode(const Symbol& value, bool forEditor) const override;  // 0x31D150
    Symbol _ValueFromDataNode(const DataNode& value) const override;                 // 0x31D170
    Symbol _DefaultValue() const override;                                           // 0x31D180

    // kPropertySymbol. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertySymbol;
    }
    // The empty symbol, created on first use. Inlined into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x31B470
    // Registers the class as the keys of symbol properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x31B540
    // "TKey<kSymbol>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x31B6F0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AC5A0, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B4860.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x31B700

    // "PropKeysSymCom", constructed by the static initializer at 0x31E170,
    // which first sets a shared header's null object id to -1 (0x19FAC68).
    static Symbol sId;  // 0x19FAC70
    // Also "PropKeysSymCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FAC78
    static PropRegistry sPropRegistry;  // 0x19FAC80
    static ComMetaData sMetaData;       // 0x19FAD20
};

static_assert(offsetof(PropKeysSymCom, mKeys) == 280);
static_assert(sizeof(PropKeysSymCom) == 320);
