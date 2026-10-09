#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The base of the scene's atmosphere effects, the fog (RndFogCom) and the
// volumetric scattering (RndVolumetricScatteringCom)
// (render/RndAtmosphereCom.o, 0x451450-0x451C6F). Its class id is
// "Atmosphere": "Base class for atmosphere components". The scene
// component's "atmosphere" property names the object that holds one, and
// an atmosphere created on the scene's root object becomes the scene's
// atmosphere when it has none. The distances are the range the effect
// covers. The vtable at 0x1902BD8 has 41 slots. The object is 32 bytes.
class RndAtmosphereCom : public Component {
public:
    RndAtmosphereCom();  // 0x451450
    // Inlined into the _Imprint of the class and its subclasses.
    RndAtmosphereCom(const RndAtmosphereCom& other)
        : Component(other),
          mEnabled(other.mEnabled),
          mStartDist(other.mStartDist),
          mEndDist(other.mEndDist) {}
    ~RndAtmosphereCom() override;  // slots 0-1: 0x4514A0, 0x4514B0

    Symbol GetId() const override;         // slot 4: 0x451990
    Symbol GetClassName() const override;  // slot 5: 0x4519A0
    int CurrentRev() const override;       // slot 7: 0x4519B0
    bool IsA(Symbol type) const override;  // slot 8: 0x4519D0
    Component* AsComponent() override;     // slot 9: 0x451A00
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x451A10
    // Slot 19: an atmosphere on the scene's root object becomes the scene's
    // atmosphere unless the scene has one. RndFogCom keeps it.
    void _PostCreate() override;                // 0x451900
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x451B40
    ComMetaData& _GetMetaData() override;       // slot 23: 0x451B50

    // Sets the start distance, pushing the end out to it. The volumetric
    // scattering's revision upgrade calls it. Name not in the reference
    // map.
    void SetStartDist(float dist);  // 0x4518C0
    // Pushes the end distance out to the start distance. No caller in this
    // build.
    void _SyncStartDist();  // 0x4518D0
    // Sets the end distance, pulling the start in to it. The volumetric
    // scattering's revision upgrade calls it. Name not in the reference
    // map.
    void SetEndDist(float dist);  // 0x4518E0
    // Pulls the start distance in to the end distance. No caller in this
    // build.
    void _SyncEndDist();  // 0x4518F0

    // Describes the class and registers "enabled", "start_dist" and
    // "end_dist" (in meters). Not reconstructed: the property metadata it
    // fills is not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x4514D0
    // Initializes a subclass's metadata as this class's subclass and runs
    // _Init in the metadata heap. Emitted in render/RndFogCom.o. Not
    // reconstructed, as RndOptionsCom's is not. Name not in the reference
    // map.
    static void _InitAsSuperclass(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x4522E0

    static Symbol sId;  // 0x1A75618, "Atmosphere"
    // Also "Atmosphere". Name not in the reference map.
    static Symbol sClassName;  // 0x1A75620
    static PropRegistry sPropRegistry;  // 0x1A75630
    static ComMetaData sMetaData;       // 0x1A756D0

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets. In Component's tail
    // padding: the scene drawer ignores a disabled atmosphere (0x41B98A).
    bool mEnabled;     // "enabled"
    float mStartDist;  // "start_dist"
    float mEndDist;    // "end_dist"
};

static_assert(offsetof(RndAtmosphereCom, mEnabled) == 22);
static_assert(offsetof(RndAtmosphereCom, mStartDist) == 0x18);
static_assert(offsetof(RndAtmosphereCom, mEndDist) == 0x1C);
static_assert(sizeof(RndAtmosphereCom) == 0x20);

// World units per meter; 1. The atmosphere's distances are registered in
// meters through it. Name not in the reference map; the evidence is weak.
extern float gUnitsPerMeter;  // 0x19B033C
