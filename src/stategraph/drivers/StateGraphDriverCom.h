#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that runs the StateGraph instances of an entity
// (stategraph/StateGraphDriverCom.o; its registration is at 0x701620, class
// id "StateGraphDriver"). Only the class symbols that the scene resource
// uses are declared; the class is not reconstructed.
class StateGraphDriverCom : public Component {
public:
    static Symbol sId;  // 0x1AB29D0
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB29D8
};
