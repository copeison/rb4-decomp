#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that instances a child entity resource
// (entity/InstanceCom.o). Only the class symbols the entity resources use
// are declared; the class is not reconstructed.
class InstanceCom : public Component {
public:
    static Symbol sId;  // 0x19E4A80
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x19E4A88
};
