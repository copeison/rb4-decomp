#pragma once

#include <cstddef>

#include "fmod_api.h"

namespace rb4 {

struct Vector3 {
    float x;
    float y;
    float z;
};

struct EngineTransform {
    Vector3 right;
    Vector3 forward;
    Vector3 up;
    Vector3 position;
};

static_assert(sizeof(Vector3) == 12);
static_assert(sizeof(EngineTransform) == 48);
static_assert(offsetof(EngineTransform, forward) == 12);
static_assert(offsetof(EngineTransform, up) == 24);
static_assert(offsetof(EngineTransform, position) == 36);

FMOD_3D_ATTRIBUTES audio_build_fmod_3d_attributes(
    const EngineTransform& transform);
void audio_update_listener_attributes(
    bool listener_enabled,
    const EngineTransform& transform);

}  // namespace rb4
