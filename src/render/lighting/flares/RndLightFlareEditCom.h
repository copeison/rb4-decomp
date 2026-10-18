#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/editor/RndEditorDrawCom.h"
#include "utl/text/Symbol.h"

// The editor companion of RndLightFlareCom (render/RndLightFlareEditCom.o,
// 0x47E780-0x47EEDF; the static initializer at 0x47EE10). Its class id is
// "LightFlareEditorData". It frames the object by its draw node's world
// sphere and outlines that sphere for a selected flare when the light
// options show selected flare light sources. The vtable at 0x1906298 has 59
// slots. The object is 48 bytes.
class RndLightFlareEditCom : public RndEditorDrawCom {
public:
    RndLightFlareEditCom();            // 0x47E780
    ~RndLightFlareEditCom() override;  // slots 0-1: 0x47E7B0, 0x47E7C0

    Symbol GetId() const override;         // slot 4: 0x47EC30
    Symbol GetClassName() const override;  // slot 5: 0x47EC40
    int CurrentRev() const override;       // slot 7: 0x47EC50
    bool IsA(Symbol type) const override;  // slot 8: 0x47EC70
    Component* AsComponent() override;     // slot 9: 0x47ECA0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x47ECB0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x47EDF0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x47EE00
    // Slot 41: 0x42, the draw node's sphere among them.
    unsigned int _GetCapabilityFlagsImpl() const override;  // 0x47EA30
    // Slot 42: only a selected object, while the light options show
    // selected flare light sources and are not suppressed.
    bool _FilterShowingImpl(
        const RndCameraContext* camera,
        bool selected) const override;  // 0x47EA40
    // Slot 46: the draw node's world sphere as a white wireframe.
    void _EditorDrawImpl(
        RndContext& context,
        bool selected,
        const VectorAdapter<unsigned long>& subEditables) override;  // 0x47EA70

    // Registers the class. Emitted in render/RndInit.o at 0x3FE980. Not
    // reconstructed: the registration helpers it inlines are not
    // modelled.
    static void Init();
    // The class factory, emitted with the other render factories. The
    // placement factory beside it (0x406120) is not modelled.
    static Component* _Create();  // 0x4060F0
    // Describes the class after RndEditorDrawCom::_Init: it requires a
    // light flare and is created with one.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x47E7E0

    static Symbol sId;                  // 0x1A88068, "LightFlareEditorData"
    // Also "LightFlareEditorData". Name not in the reference map.
    static Symbol sClassName;           // 0x1A88070
    static PropRegistry sPropRegistry;  // 0x1A88080
    static ComMetaData sMetaData;       // 0x1A88120
};

static_assert(sizeof(RndLightFlareEditCom) == 48);
