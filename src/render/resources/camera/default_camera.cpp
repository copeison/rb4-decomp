#include "render/resources/camera/default_camera.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x6BEBC0 and the equivalent inlined sequence
// in render_initialize_default_resources at 0x6BDCA0.
void render_create_default_camera(DefaultCameraState& state, RndScene& scene) {
    auto* object = rnd_scene_create_object(scene, "default_cam");
    state.camera = object == nullptr ? nullptr : rnd_object_add_camera(*object);
}

}  // namespace rb4
