#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The base of the renderer's options components (render/RndOptionsCom.o,
// 0x46A870-0x46ACC6): the overlay, camera, light, mesh and other "*Options"
// classes, which live on the root object of the options entity
// (RndOptions.o). Its class id is "Options". The vtable at 0x1904728 has 41
// slots. The object is 24 bytes; its only member sits in Component's tail
// padding.
class RndOptionsCom : public Component {
public:
    // The per-component state. Constructor at 0x46A8A0, which the copies
    // call out of line.
    struct RuntimeData {
        RuntimeData() : mSuppressed(false) {}

        // Copied from RndOptions::SetSuppressed (0x469CB0) into every
        // options component of the options entity. Name not in the
        // reference map.
        bool mSuppressed;
    };

    RndOptionsCom();            // 0x46A870
    // Copies the component for an imprint; the run-time state starts
    // afresh. Inlined into every _Imprint of the options classes.
    RndOptionsCom(const RndOptionsCom& other) : Component(other) {}
    ~RndOptionsCom() override;  // slots 0-1: 0x46A8B0, 0x46A8C0

    Symbol GetId() const override;         // slot 4: 0x46AA40
    Symbol GetClassName() const override;  // slot 5: 0x46AA50
    int CurrentRev() const override;       // slot 7: 0x46AA60
    bool IsA(Symbol type) const override;  // slot 8: 0x46AA80
    Component* AsComponent() override;     // slot 9: 0x46AAB0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x46AAC0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x46ABE0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x46ABF0

    // Describes the class: a lightweight rendering class allowed only in
    // the options resource, with editor restrictions 12. It has no
    // properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x46A8E0
    // Makes RndOptionsCom the superclass of a subclass's metadata, runs _Init
    // on it in the "metadata" heap, and combines the editor restrictions
    // _Init sets with those the metadata had (InheritEditorRestrictions).
    // Each subclass's _Init calls it first. Emitted outside the object. Name
    // not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x3BF4C0

    static Symbol sId;                  // 0x1A869D0, "Options"
    // Also "Options". Name not in the reference map.
    static Symbol sClassName;           // 0x1A869D8
    static PropRegistry sPropRegistry;  // 0x1A869E0
    static ComMetaData sMetaData;       // 0x1A86A80

    RuntimeData mRuntimeData;
};

static_assert(offsetof(RndOptionsCom, mRuntimeData) == 22);
static_assert(sizeof(RndOptionsCom) == 24);
