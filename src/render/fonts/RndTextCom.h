#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The text component (render/RndTextCom.o; its static initializer is at
// 0x67A3F0, class id "Text"). Only the class symbols the UI combo boxes
// name as their text objects' required component are declared; the class
// is not reconstructed.
class RndTextCom : public Component {
public:
    static Symbol sId;  // 0x1AAD878, "Text"
    // The class symbol GameObject::CreateComponent takes; also "Text". Name
    // not in the reference map.
    static Symbol sClassName;  // 0x1AAD880
};
