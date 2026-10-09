#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

class RndLightMgrCom;

// The scene settings component on a scene entity's root object. Its methods
// are not reconstructed; only the lookups the renderer's defaults use are
// declared.
class RndSceneCom : public Component {
public:
    // The light manager component of the object the scene names, or null.
    // The map's signature is GetLightMgr(EntityPtr const&); this build reads
    // the entity through the component's owner.
    RndLightMgrCom* GetLightMgr();  // 0x408F60

    // The class symbol, "Scene", constructed by the object's static
    // initializer at 0x4190B4.
    static Symbol sId;  // 0x1A722A8
};
