#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The camera component. Its methods are not reconstructed; only the class
// symbol that the renderer's defaults create it by is declared.
class RndCameraCom : public Component {
public:
    // The class's second symbol, "Camera", stored by the static initializer
    // at 0x6B8804 beside sId (0x1AAF8F0); the component factory is given it.
    // Name not in the reference map.
    static Symbol sClassName;  // 0x1AAF8F8
};
