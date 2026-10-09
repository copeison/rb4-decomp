#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class GameObject;

// The component that holds an object's editor capabilities
// (entity/EditorCom.o). Only the class symbols and the capabilities that
// EntityResource uses are declared; the class is not reconstructed.
class EditorCom : public Component {
public:
    // The capability bits. Enumerator names not in the reference map: the
    // registration at 0xEA230 lists Change Name, Change Layer, Delete,
    // Change Properties, Add Component and Remove Component in that order,
    // so the root's revoked 4 and 8 are taken to be Delete and Change
    // Properties; the evidence is weak.
    enum EditorCapability {
        kCanChangeName = 1,
        kCanChangeLayer = 2,
        kCanDelete = 4,
        kCanChangeProperties = 8,
    };

    // Sets the capability; the object is ignored. Name not in the reference
    // map.
    void GrantEditorCapability(GameObject* object, EditorCapability capability);  // 0xEABC0
    // Clears the capability. The map's signature is
    // RevokeEditorCapability(EditorCom::EditorCapability); this build
    // passes the object too and ignores it.
    void RevokeEditorCapability(GameObject* object, EditorCapability capability);  // 0xEABD0

    static Symbol sId;  // 0x19E29C0
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x19E29C8

    // The EditorCapability bits. Name not in the reference map.
    unsigned int mCapabilities;
};

static_assert(offsetof(EditorCom, mCapabilities) == 24);
