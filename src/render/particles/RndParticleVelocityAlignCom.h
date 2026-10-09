#pragma once

#include "render/particles/RndParticleExtCom.h"
#include "utl/text/Symbol.h"

// Turns a particle system's sprites along their velocity
// (render/RndParticleVelocityAlignCom.o). Its class id is
// "PSysVelocityAlign". The vtable at 0x192DE80 has 42 slots; it shares
// slot 20 (0x6249E0) with RndParticleExtCom and overrides its slot 41, so
// in this build the class derives from it (the map's build has it derive
// from Component). RndParticleCom only checks whether the object has one.
// Its layout is not decoded.
class RndParticleVelocityAlignCom : public RndParticleExtCom {
public:
    ~RndParticleVelocityAlignCom() override;  // slots 0-1: 0x629D10, 0x62A390

    Symbol GetId() const override;         // slot 4: 0x62A3B0
    Symbol GetClassName() const override;  // slot 5: 0x62A3C0
    int CurrentRev() const override;       // slot 7: 0x62A3D0
    bool IsA(Symbol type) const override;  // slot 8: 0x62A3F0
    Component* AsComponent() override;     // slot 9: 0x62A420
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x62A430
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x62A580
    ComMetaData& _GetMetaData() override;       // slot 23: 0x62A590
    bool _OnResourcesLoaded() override;         // slot 29: 0x629D20
    // Slot 41: the map's _Poll(ObjPtr const&).
    void _PollExtension() override;  // 0x629D90

    // The class symbol, "PSysVelocityAlign", constructed by the static
    // initializer at 0x62AA0A.
    static Symbol sId;  // 0x1AAA488
    // The class's second symbol, also "PSysVelocityAlign". Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AAA490
};
