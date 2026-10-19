#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The keys of an integer property in a property animation
// (anim/PropKeysIntCom.o, 0x308AB0-0x30A4AF): one class per integer type,
// registered as "PropKeysIntCom_<type>" ("PropKeysIntCom_kInt8" to
// "PropKeysIntCom_kUint64"); a key's value is the property's until the next
// key. Each instance's StaticKeysType, StaticKeysSubType,
// StaticCreatePropDriver, Register, CreatePropDriver and StaticKeyTypeName
// lie at 0x308AB0, +0x10, +0xB0, +0x180, +0x260 and +0x330 for signed
// char, and 0x340 further on for each later type (short, int, unsigned
// char, unsigned short, unsigned int, long, unsigned long). The other
// methods are emitted with the instances in anim/AnimInit.o; the addresses
// given are signed char's, whose vtable is 0x18F1CA0 (76 slots; 0x18F2130,
// 0x18F25C0, 0x18F2A50, 0x18F2EE0, 0x18F3370, 0x18F3800 and 0x18F3C90 for
// the others). The objects are 320 bytes.
template <class T>
class PropKeysIntCom : public PropKeysTypeCom<T> {
public:
    PropKeysIntCom() {}
    // Slots 0-1: 0x2B7990, 0x2B8310.
    ~PropKeysIntCom() override {}

    Symbol GetId() const override;          // slot 4: 0x2B83A0
    Symbol GetClassName() const override;   // slot 5: 0x2B83B0
    int CurrentRev() const override;        // slot 7: 0x2B83C0
    bool IsA(Symbol type) const override;   // slot 8: 0x2B83E0
    Component* AsComponent() override;      // slot 9: 0x2B8410
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2B8420
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2B8650
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2B8660
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x308D10
    PropertyType GetKeysType() const override;      // slot 55: 0x2B8890
    Symbol GetKeysSubType() const override;         // slot 56: 0x2B88A0
    // Slots 73-75: an integer node, the node's integer, zero.
    DataNode _ValueToDataNode(const T& value, bool forEditor) const override;  // 0x2B90F0
    T _ValueFromDataNode(const DataNode& value) const override;                 // 0x2B9110
    T _DefaultValue() const override;                                           // 0x2B9130

    // The integer type.
    static PropertyType StaticKeysType();  // 0x308AB0
    // The empty symbol, created on first use.
    static const Symbol& StaticKeysSubType();  // 0x308AC0
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x308B60
    // Registers the class as the keys of the type's properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x308C30
    // The key struct's type name, "TKey<kInt8>" to "TKey<kUint64>". Name
    // not in the reference map.
    static const char* StaticKeyTypeName();  // 0x308DE0

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A84E0 (signed char), 0x2A8990 (short), 0x2A8E40 (int), 0x2A92F0
    // (unsigned char), 0x2A97A0 (unsigned short), 0x2A9C50 (unsigned int),
    // 0x2AA100 (long) and 0x2AA5B0 (unsigned long), which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B7820 for signed
    // char.
    static Component* _Create();
    // The registry builder. Emitted in anim/AnimInit.o at 0x2B73F0 for
    // signed char.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);

    // "PropKeysIntCom_<type>", formatted by the guarded template
    // initializers (0x2B0950 and 0x2B07B0 for signed char) into a buffer
    // per type. Names not in the reference map but sId's.
    static Symbol sId;          // 0x19F6368 for signed char
    static Symbol sClassName;   // 0x19F60D0
    static PropRegistry sPropRegistry;  // 0x19F62B0
    static ComMetaData sMetaData;       // 0x19F60E0

private:
    // Formats the class id. Inlined into the initializers. Name not in the
    // reference map.
    static const char* _MakeClassName();
};

static_assert(sizeof(PropKeysIntCom<signed char>) == 320);
static_assert(sizeof(PropKeysIntCom<unsigned long>) == 320);
