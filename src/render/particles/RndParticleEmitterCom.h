#pragma once

#include "entity/core/Component.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Rand;
class Transform;

// The base of the components that give a particle system's new particles
// their emission shape. The class name comes from the string at 0x12A75CE;
// it has no object in the reference map, whose build predates it. Its class
// id is "PSysEmitter". The vtable at 0x192CDF8 has 43 slots. Its layout is
// not decoded.
class RndParticleEmitterCom : public Component {
public:
    ~RndParticleEmitterCom() override;  // slots 0-1: 0x61FC00, 0x61FC10

    Symbol GetId() const override;         // slot 4: 0x61FED0
    Symbol GetClassName() const override;  // slot 5: 0x61FEE0
    int CurrentRev() const override;       // slot 7: 0x61FEF0
    bool IsA(Symbol type) const override;  // slot 8: 0x61FF10
    Component* AsComponent() override;     // slot 9: 0x61FF40
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x61FF50
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x620070
    ComMetaData& _GetMetaData() override;       // slot 23: 0x620080

    // Slot 41 at 0x620090: moves the queued birth transforms of a particle
    // system onto the emitter's shape, drawing from the system's generator.
    // Empty here. Name not in the reference map.
    virtual void _PlaceNewParticles(Rand* rand, eastl::vector<Transform>& births) {
        static_cast<void>(rand);
        static_cast<void>(births);
    }

    // The class symbol, "PSysEmitter", constructed by the static initializer
    // at 0x62010A.
    static Symbol sId;  // 0x1AA90C8
    // The class's second symbol, also "PSysEmitter"; the particle system
    // finds its emitter by it as a base class. Name not in the reference
    // map.
    static Symbol sClassName;  // 0x1AA90D0
};
