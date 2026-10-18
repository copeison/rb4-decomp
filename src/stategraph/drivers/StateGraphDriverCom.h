#pragma once

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// The component that runs the StateGraph instances of an entity
// (stategraph/StateGraphDriverCom.o; its registration is at 0x701620, class
// id "StateGraphDriver"). Only the class symbols that the scene resource
// uses and the members the UI input component calls are declared; the
// class is not reconstructed.
class StateGraphDriverCom : public Component {
public:
    // Starts the state graph unless it runs already for the same object.
    // The map gives no return type; the binary's callers ignore it.
    void Activate(ResourcePath path, GameObjectId object);  // 0x701B20
    // Whether an instance of the state graph runs.
    bool IsActive(ResourcePath path) const;  // 0x701D70

    static Symbol sId;  // 0x1AB29D0
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB29D8
};
