#pragma once

// A character's lip-sync data, extension "lipsync". Newer than the reference
// map; the class id string gives the name (anim/CharLipsyncResource.o). Only
// the registration Anim::Init calls is declared; the class is not
// reconstructed.
class CharLipsyncResource {
public:
    // Registers the class as a resource. Emitted in anim/AnimInit.o at
    // 0x2AE2D0.
    static void Init();
};
