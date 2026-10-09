#pragma once

#include "entity/core/Component.h"
#include "math/geometry/Rect.h"
#include "utl/text/Symbol.h"

class Vector2i;

// The component through which a parent entity instances a scene
// (render/RndSceneInstanceCom.o; vtable 0x1901170, 50 slots). Its methods
// are not reconstructed; only what the camera component uses is declared.
class RndSceneInstanceCom : public Component {
public:
    // The rectangle of the parent's viewport the instanced scene projects
    // into, for a target of the given size.
    Hmx::Rect CalcProjectionRect(const Vector2i& size) const;  // 0x4365F0

    static Symbol sId;  // 0x1A72AD8, "SceneInstance"
    // Also "SceneInstance". Name not in the reference map.
    static Symbol sClassName;  // 0x1A72AE0
};
