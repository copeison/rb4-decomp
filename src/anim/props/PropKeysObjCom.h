#pragma once

#include <cstddef>

#include "anim/props/PropKeysCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The keys of an object-id property in a property animation
// (anim/PropKeysObjCom.o, 0x30E2A0-0x310FAF; class id "PropKeysObjCom"): a
// key's value is the property's until the next key. The vtable at
// 0x18F83F8 has 76 slots. The object is 320 bytes.
class PropKeysObjCom : public PropKeysTypeCom<GameObjectId> {
public:
    PropKeysObjCom() {}
    // Slots 0-1: 0x30F020, 0x30F0B0.
    ~PropKeysObjCom() override {}

    Symbol GetId() const override;          // slot 4: 0x30F140
    Symbol GetClassName() const override;   // slot 5: 0x30F150
    int CurrentRev() const override;        // slot 7: 0x30F160
    bool IsA(Symbol type) const override;   // slot 8: 0x30F180
    Component* AsComponent() override;      // slot 9: 0x30F1B0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x30F1C0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x30F3F0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x30F400
    PropDriver* CreatePropDriver() const override;  // slot 41: 0x30E450
    PropertyType GetKeysType() const override;      // slot 55: 0x30F630
    Symbol GetKeysSubType() const override;         // slot 56: 0x30F640
    // Slots 73-75: an object-id node, the id the node holds, the null id.
    DataNode _ValueToDataNode(const GameObjectId& value, bool forEditor) const override;  // 0x30FF30
    GameObjectId _ValueFromDataNode(const DataNode& value) const override;                 // 0x30FF40
    GameObjectId _DefaultValue() const override;                                           // 0x30FF50

    // kPropertyGameObjectId. Inlined into its users.
    static PropertyType StaticKeysType() {
        return kPropertyGameObjectId;
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
    static PropDriver* StaticCreatePropDriver(const PropRef& prop);  // 0x30E2A0
    // Registers the class as the keys of object-id properties, under the
    // "PropKeys" interface.
    static void Register();  // 0x30E370
    // "TKey<kGameObjectId>"; no caller was found. Name not in the reference
    // map.
    static const char* StaticKeyTypeName();  // 0x30E520

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AC0F0, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B3E30.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x30E530

    // "PropKeysObjCom", constructed by the static initializer at 0x310F10,
    // which first sets a shared header's null object id to -1 (0x19FA218).
    static Symbol sId;  // 0x19FA220
    // Also "PropKeysObjCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19FA228
    static PropRegistry sPropRegistry;  // 0x19FA230
    static ComMetaData sMetaData;       // 0x19FA2D0
};

static_assert(offsetof(PropKeysObjCom, mKeys) == 280);
static_assert(sizeof(PropKeysObjCom) == 320);
