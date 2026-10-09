#pragma once

#include "entity/core/Component.h"

class RndMesh;

// A 2D quad whose size typically follows a reference texture. Only the
// startup and shutdown entry points that Rnd::Init and Rnd::Terminate call
// and the shared mesh accessor are declared.
class RndTexturedQuadCom : public Component {
public:
    // Builds the shared quad mesh.
    static void PostInit();  // 0x44C8B0
    static void Terminate();  // 0x44C930
    static RndMesh* GetMesh();  // 0x44C960
};
