#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The component that lists the overlays in the debug options
// (render/RndOverlayOptionsCom.o, 0x5F9880-0x5FA690). Its class id is
// "OverlayOptions". It has one boolean property per registered overlay, and
// an "<overlay>_options" group for overlays with options of their own; the
// registry is rebuilt whenever the overlay list changes. The vtable at
// 0x192B128 has 41 slots. The object is 24 bytes.
class RndOverlayOptionsCom : public RndOptionsCom {
public:
    RndOverlayOptionsCom();            // 0x5F98A0
    ~RndOverlayOptionsCom() override;  // slots 0-1: 0x5F98D0, 0x5F98E0

    Symbol GetId() const override;         // slot 4: 0x5FA2A0
    Symbol GetClassName() const override;  // slot 5: 0x5FA2B0
    int CurrentRev() const override;       // slot 7: 0x5FA2C0
    bool IsA(Symbol type) const override;  // slot 8: 0x5FA2E0
    Component* AsComponent() override;     // slot 9: 0x5FA310
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x5FA320
    // Slot 20: stops being theRndOverlayOpts.
    void _PreDestroy(DestroyType type) override;  // 0x5F9880
    // Slot 22: rebuilds the registry first when the overlays changed.
    PropRegistry& _GetPropRegistry() override;  // 0x5F9AB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x5FA460
    // Slot 29: becomes theRndOverlayOpts.
    bool _OnResourcesLoaded() override;  // 0x5F9890

    // Clears mOverlaysChanged and rebuilds sPropRegistry from RndOptionsCom's
    // registry and one "show" property per overlay, whose getter and setter
    // (0x5FA470, 0x5FA4A0) read and set the overlay's showing state. Name
    // not in the reference map. Not reconstructed: the property metadata it
    // fills is not modelled.
    void _RebuildRegistry();  // 0x5F9AD0

    // Describes the class and inherits RndOptionsCom's registry. The map's
    // signature; the call through RndOptionsCom's superclass helper
    // (0x3BF4C0) precedes it in the class's Init.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x5F9900

    static Symbol sId;                  // 0x1AA7CB8, "OverlayOptions"
    // Also "OverlayOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AA7CC0
    static PropRegistry sPropRegistry;  // 0x1AA7CD0
    static ComMetaData sMetaData;       // 0x1AA7D70

    // Set when an overlay registers or unregisters (RndOverlayMgr), and
    // cleared by _RebuildRegistry. Name not in the reference map.
    bool mOverlaysChanged;
};

static_assert(offsetof(RndOverlayOptionsCom, mOverlaysChanged) == 23);
static_assert(sizeof(RndOverlayOptionsCom) == 24);

// The live component, or null while none has loaded its resources.
extern RndOverlayOptionsCom* theRndOverlayOpts;  // 0x1AA7F18
