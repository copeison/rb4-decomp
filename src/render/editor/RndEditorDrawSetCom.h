#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The root component that collects an entity's editor-drawn components
// (render/RndEditorDrawSetCom.o, class id "EditorDrawSet"). Every
// RndEditorDrawCom requires it. Only the class symbols are declared; the
// class is not reconstructed.
class RndEditorDrawSetCom : public Component {
public:
    static Symbol sId;  // 0x1A86028
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1A86030
};
