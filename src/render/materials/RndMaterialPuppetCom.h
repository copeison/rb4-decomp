#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that makes the materials of an instanced entity follow the
// instancing object (render/RndMaterialPuppetCom.o; vtable 0x1911100, 41
// slots; constructor 0x4F6630). Its methods are not reconstructed; only the
// class symbol the material component looks up is declared.
class RndMaterialPuppetCom : public Component {
public:
    // The class symbol, "MaterialPuppet", constructed by the static
    // initializer at 0x4F6B1A.
    static Symbol sId;  // 0x1A8BBB8
};
