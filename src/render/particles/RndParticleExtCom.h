#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The base of the components that extend a particle system on the same
// object: they add themselves to the system's extension list once their
// resources load and leave it before they are destroyed, and the system
// polls them every frame. The class name comes from the string at 0x12A830A;
// it has no object in the reference map, whose build predates it. Its class
// id is "PSysExtension". The vtable at 0x192D4B8 has 42 slots. Its layout is
// not decoded.
class RndParticleExtCom : public Component {
public:
    ~RndParticleExtCom() override;  // slots 0-1: 0x624A60, 0x624A70

    Symbol GetId() const override;         // slot 4: 0x624A90
    Symbol GetClassName() const override;  // slot 5: 0x624AA0
    int CurrentRev() const override;       // slot 7: 0x624AB0
    bool IsA(Symbol type) const override;  // slot 8: 0x624AD0
    Component* AsComponent() override;     // slot 9: 0x624B00
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x624B10
    // Slot 20 at 0x6249E0: leaves the particle system's extension list
    // (through the list's erase at 0x601F60).
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x624C40
    ComMetaData& _GetMetaData() override;       // slot 23: 0x624C50
    // Slot 29 at 0x624A00: joins the extension list of the object's
    // RndParticleCom (through the list's insert at 0x601E50).
    bool _OnResourcesLoaded() override;

    // Slot 41 at 0x624C60: called by RndParticleCom::_Poll once the
    // particles are updated. Empty here. Name not in the reference map.
    virtual void _PollExtension() {}

    // The class symbol, "PSysExtension", constructed by the static
    // initializer at 0x624CBA.
    static Symbol sId;  // 0x1AA9A98
    // The class's second symbol, also "PSysExtension". Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AA9AA0
};
