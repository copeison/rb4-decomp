#include "fmod_listener.h"

#include "fmod_audio_system.h"

namespace rb4 {

extern FmodAudioState* g_fmod_audio_state;

namespace {

FMOD_VECTOR to_fmod_coordinates(const Vector3& vector) {
    return {-vector.x, vector.y, vector.z};
}

}  // namespace

// Reconstructed from eboot.elf at 0x27ACB0.
FMOD_3D_ATTRIBUTES audio_build_fmod_3d_attributes(
    const EngineTransform& transform) {
    return {
        to_fmod_coordinates(transform.position),
        {0.0f, 0.0f, 0.0f},
        to_fmod_coordinates(transform.forward),
        to_fmod_coordinates(transform.up),
    };
}

// Reconstructed from eboot.elf at 0x262300.
void audio_update_listener_attributes(
    bool listener_enabled,
    const EngineTransform& transform) {
    if (!listener_enabled || g_fmod_audio_state == nullptr ||
        g_fmod_audio_state->studio_system == nullptr) {
        return;
    }

    constexpr std::int32_t kPrimaryListener = 0;
    const auto attributes = audio_build_fmod_3d_attributes(transform);
    g_fmod_audio_state->studio_system->setListenerAttributes(
        kPrimaryListener, &attributes);
}

}  // namespace rb4
