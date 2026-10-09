#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "utl/text/Symbol.h"

// The component that borrows the material of another object in the entity
// (render/RndMaterialReferenceCom.o; vtable 0x1911258, 41 slots; destructor
// 0x4F6BD0). Its methods are not reconstructed; only the members the
// draw-instance components read are declared.
class RndMaterialReferenceCom : public Component {
public:
    // The class symbol, "MaterialReference", constructed by the static
    // initializer at 0x4F715A.
    static Symbol sId;  // 0x1A8BE28
    // The class's second symbol, also "MaterialReference", which the
    // component-order lists take. Name not in the reference map.
    static Symbol sClassName;  // 0x1A8BE30

    // The object whose RndMaterialCom is used; the null id when unset.
    // Name not in the reference map.
    GameObjectId mMaterial;
};

static_assert(offsetof(RndMaterialReferenceCom, mMaterial) == 24);
