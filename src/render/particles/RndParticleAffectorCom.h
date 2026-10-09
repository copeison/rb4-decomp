#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class RndParticleCollection;
class RndParticleCom;
class Transform;

// The base of the components that attract or repel a particle system's
// in-flight particles. The class name comes from the string at 0x12A4449;
// it has no object in the reference map, whose build predates it. Its class
// id is "PSysAffector". The vtable at 0x192B2D0 has 43 slots. Only the
// members RndParticleCom reads are declared; the rest of the layout is not
// decoded.
class RndParticleAffectorCom : public Component {
public:
    ~RndParticleAffectorCom() override;  // slots 0-1: 0x5FA6F0, 0x5FA700

    Symbol GetId() const override;         // slot 4: 0x5FAAF0
    Symbol GetClassName() const override;  // slot 5: 0x5FAB00
    int CurrentRev() const override;       // slot 7: 0x5FAB10
    bool IsA(Symbol type) const override;  // slot 8: 0x5FAB30
    Component* AsComponent() override;     // slot 9: 0x5FAB60
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x5FAB70
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x5FACA0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x5FACB0

    // Slot 41 at 0x5FACC0: moves the particles of a system for the frame;
    // RndParticleCom::_UpdateForce passes the transform into the
    // particles' space and the time step. Empty here. Name not in the
    // reference map.
    virtual void _AffectParticles(
        RndParticleCollection& particles,
        const Transform& xfm,
        float dt) {
        static_cast<void>(particles);
        static_cast<void>(xfm);
        static_cast<void>(dt);
    }
    // Slot 42 at 0x5FACD0: runs once the particles moved and collided;
    // RndParticleCom::_UpdateParticles passes the system and the same
    // transform. Empty here. Name not in the reference map.
    virtual void _PostAffectParticles(RndParticleCom& system, const Transform& xfm) {
        static_cast<void>(system);
        static_cast<void>(xfm);
    }

    // The class symbol, "PSysAffector", constructed by the static
    // initializer at 0x5FAD2A.
    static Symbol sId;  // 0x1AA7F30
    // The class's second symbol, also "PSysAffector"; the particle system
    // finds its affectors by it as their base class. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AA7F38

    // Field names are not in the reference map; they follow the properties
    // "enabled", "distance_scale" and "force_strength" that the registry
    // builder binds at these offsets (0x5FA8CD-0x5FA9BB). "enabled" sits in
    // Component's tail padding.
    bool mEnabled;
    float mDistanceScale;
    // Positive attracts, negative repels; RndParticleCom skips an affector
    // whose strength is within 1e-4 of zero.
    float mForceStrength;
};

static_assert(offsetof(RndParticleAffectorCom, mEnabled) == 22);
static_assert(offsetof(RndParticleAffectorCom, mDistanceScale) == 24);
static_assert(offsetof(RndParticleAffectorCom, mForceStrength) == 28);
