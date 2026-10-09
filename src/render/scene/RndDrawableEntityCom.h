#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

class RndTexRendererMgr;

// The base of the components on the root object of a drawable entity
// (0x6C05D0-0x6C0B7B), such as RndSceneCom. Its class id is
// "DrawableEntity", and its metadata names it "RndDrawableEntityCom"; the
// reference map's older build calls the class RndEntityCom (object
// render/RndEntityCom.o), whose RuntimeData and ObtainPollMgr correspond to
// the members here. The vtable at 0x19378B0 has 41 slots. The object is 32
// bytes.
class RndDrawableEntityCom : public Component {
public:
    // The per-component state.
    struct RuntimeData {
        RuntimeData();  // 0x6C0600
        // An imprint's state starts afresh, so the copy is a default
        // construction; RndSceneCom's _Imprint calls the constructor.
        RuntimeData(const RuntimeData&) : RuntimeData() {}

        // Created on demand by ObtainTexRendererMgr and owned. Name not in
        // the reference map.
        RndTexRendererMgr* mTexRendererMgr;
    };

    RndDrawableEntityCom();            // 0x6C05D0
    // Copies the component for an imprint; the texture renderers start
    // afresh. Inlined into the _Imprint of the class (0x6C0960) and of
    // RndSceneCom (0x411CE0). Not in the reference map.
    RndDrawableEntityCom(const RndDrawableEntityCom& other)
        : Component(other), mRuntimeData(other.mRuntimeData) {}
    ~RndDrawableEntityCom() override;  // slots 0-1: 0x6C0610, 0x6C0660

    Symbol GetId() const override;         // slot 4: 0x6C08E0
    Symbol GetClassName() const override;  // slot 5: 0x6C08F0
    int CurrentRev() const override;       // slot 7: 0x6C0900
    bool IsA(Symbol type) const override;  // slot 8: 0x6C0920
    Component* AsComponent() override;     // slot 9: 0x6C0950
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6C0960
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6C0A90
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6C0AA0

    // The entity's texture renderers, created on first use. The map's
    // ObtainPollMgr() sits here in the older build. Name not in the
    // reference map.
    RndTexRendererMgr* ObtainTexRendererMgr();  // 0x6C06B0

    // Describes the class: a rendering class allowed only in drawable
    // entity resources, with no editor restrictions, found by its own
    // interface. It has no properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6C06F0
    // Makes RndDrawableEntityCom the superclass of a subclass's metadata,
    // runs _Init on it in the "metadata" heap and combines the editor
    // restrictions. Emitted in render/RndSceneCom.o, and called by the
    // other subclasses' _Init too (0x6CA5C8, 0x8A9E58). Name not in the
    // reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x4105A0

    static Symbol sId;  // 0x1AB00A0, "DrawableEntity"
    // Also "DrawableEntity"; the metadata's interface. Name not in the
    // reference map.
    static Symbol sClassName;           // 0x1AB00A8
    static PropRegistry sPropRegistry;  // 0x1AB00B0
    static ComMetaData sMetaData;       // 0x1AB0150

    RuntimeData mRuntimeData;
};

static_assert(offsetof(RndDrawableEntityCom, mRuntimeData) == 0x18);
static_assert(sizeof(RndDrawableEntityCom) == 0x20);
