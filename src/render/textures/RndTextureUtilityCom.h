#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class RndTextureBase;

// The component that generates the lighting lookup textures
// (render/RndTextureUtilityCom.o; vtable 0x19363E8, 41 slots; destructor
// 0x6A8D30). Its methods are not reconstructed; only what
// RndLightGlobals::LoadResources calls is declared.
class RndTextureUtilityCom : public Component {
public:
    // The two hair reflectance textures, or null while their resources
    // are missing or failed.
    RndTextureBase* GetHairReflectanceTex0();  // 0x6A8DD0
    RndTextureBase* GetHairReflectanceTex1();  // 0x6A8E10

    // The class symbol, "TextureUtility", constructed by the static
    // initializer at 0x6AC67A.
    static Symbol sId;  // 0x1AAF590
    // The class's second symbol, also "TextureUtility". Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AAF598
};
