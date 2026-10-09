#pragma once

#include "entity/core/Component.h"

// The particle system component. Its methods are not reconstructed; only
// the types the particle buffers read are declared.
class RndParticleCom : public Component {
public:
    // How the quads turn, from the particle_alignment property. Enumerator
    // names are not in the reference map; they follow the property's
    // labels.
    enum ParticleAlignment : int {
        kCameraAligned = 0,    // Face the camera.
        kCameraXYAligned = 1,  // Face the camera in XY, keeping Z straight up.
        kXAxisAligned = 2,     // Face world +X.
        kYAxisAligned = 3,     // Face world -Y.
        kZAxisAligned = 4,     // Face world +Z.
    };
};
