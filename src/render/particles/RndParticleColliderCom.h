#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class RndParticleCollection;
class Transform;

// A surface the particles of a particle system collide with. The class name
// comes from the string at 0x12A4A5C ("RndEditorDrawCom counterpart to
// RndParticleColliderCom"); it has no object in the reference map, whose
// build predates it. Its class id is "PSysCollider". The vtable at
// 0x192B7E8 has 41 slots. Its layout is not decoded.
class RndParticleColliderCom : public Component {
public:
    ~RndParticleColliderCom() override;  // slots 0-1: 0x5FDEF0, 0x5FF7C0

    Symbol GetId() const override;         // slot 4: 0x5FF7E0
    Symbol GetClassName() const override;  // slot 5: 0x5FF7F0
    int CurrentRev() const override;       // slot 7: 0x5FF800
    bool IsA(Symbol type) const override;  // slot 8: 0x5FF820
    Component* AsComponent() override;     // slot 9: 0x5FF850
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x5FF860
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x5FF9B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x5FF9C0
    bool _OnResourcesLoaded() override;         // slot 29: 0x5FEB40
    void _Poll() override;                      // slot 33: 0x5FEB90
    void _EditPoll() override;                  // slot 38: 0x5FF280

    // Bounces the particles off the collider: `shape` is the system's
    // "per_particle_collision_shape", `xfm` the transform into the
    // particles' space, `elasticity` the system's and `dt` the time step.
    // RndParticleCom::_UpdatePositions calls it for each collider. Name not
    // in the reference map.
    void ResolveCollisions(
        int shape,
        RndParticleCollection& particles,
        const Transform& xfm,
        float elasticity,
        float dt);  // 0x5FE100

    // The class symbol, "PSysCollider", constructed by the static
    // initializer at 0x60008A.
    static Symbol sId;  // 0x1AA8688
    // The class's second symbol, also "PSysCollider". Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AA8690
};
