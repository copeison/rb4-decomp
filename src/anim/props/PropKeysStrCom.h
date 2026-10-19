#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The keys of a string property in a property animation
// (anim/PropKeysStrCom.o, 0x318DB0-0x31B3B1; class id "PropKeysStrCom"):
// strings are keyed as "string" structs. In this build the keys only hold
// values: new keys do not read the property, applying a frame does nothing
// and the driver picks the heaviest playback's string without setting the
// property. The vtable at 0x18F9000 has 76 slots. The object is 320 bytes.
class PropKeysStrCom : public PropKeysTypeCom<String> {
public:
    PropKeysStrCom() {}
    // Slots 0-1: 0x319AD0, 0x319BA0.
    ~PropKeysStrCom() override {}

    Symbol GetId() const override;          // slot 4: 0x319C30
    Symbol GetClassName() const override;   // slot 5: 0x319C40
    int CurrentRev() const override;        // slot 7: 0x319C50
    bool IsA(Symbol type) const override;   // slot 8: 0x319C70
    Component* AsComponent() override;      // slot 9: 0x319CA0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x319CB0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x319EE0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x319EF0
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x318F60
    PropertyType GetKeysType() const override;      // slot 55: 0x31A170
    Symbol GetKeysSubType() const override;         // slot 56: 0x31A180
    // Slots 73-75: a string node, the node's text, the empty string.
    DataNode _ValueToDataNode(const String& value, bool forEditor) const override;  // 0x31A5E0
    String _ValueFromDataNode(const DataNode& value) const override;                 // 0x31A600
    String _DefaultValue() const override;                                           // 0x31A630

    // kPropertyStruct. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyStruct;
    }
    // "string", created on first use (local static at 0x19FAC48). Inlined
    // into its users.
    static const Symbol& StaticKeysSubType() {
        static Symbol subType;
        if (subType == Symbol()) {
            subType = Symbol("string");
        }
        return subType;
    }
    // A driver for the property.
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x318DB0
    // Registers the class as the keys of "string" structs, under the
    // "PropKeys" interface.
    static void Register();  // 0x318E80
    // "TKey<kStruct>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x319030

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ACA50, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B5180.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x319040

    // "PropKeysStrCom", constructed by the static initializer at 0x31B300,
    // which first sets a shared header's null object id to -1 (0x19FA9E8).
    static Symbol sId;  // 0x19FA9F0
    // Also "PropKeysStrCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FA9F8
    static PropRegistry sPropRegistry;  // 0x19FAA00
    static ComMetaData sMetaData;       // 0x19FAAA0
};

static_assert(offsetof(PropKeysStrCom, mKeys) == 280);
static_assert(sizeof(PropKeysStrCom) == 320);

// Slots 63, 69 and 70 of the string keys do nothing: the key keeps its value,
// no key is added (index 0) and no property is set.
template <>
inline void PropKeysTypeCom<String>::UpdateValueAtIndex(unsigned long index, Entity* entity) {
    static_cast<void>(index);
    static_cast<void>(entity);
}  // 0x319B80
template <>
inline unsigned long PropKeysTypeCom<String>::AddNewKey(float frame, Entity* entity, bool replace) {
    static_cast<void>(frame);
    static_cast<void>(entity);
    static_cast<void>(replace);
    return 0;
}  // 0x319B70
template <>
inline bool PropKeysTypeCom<String>::ApplyFrame(float frame, Entity* entity) const {
    static_cast<void>(frame);
    static_cast<void>(entity);
    return false;
}  // 0x319B90

// The heaviest playback's string, which the driver does not apply.
template <>
void TPropDriver<PropKeysStrCom>::_Poll(Entity* entity);  // 0x31AD80
