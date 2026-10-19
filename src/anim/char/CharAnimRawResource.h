#pragma once

// The raw keys of a character animation, extension "charanimraw"
// (anim/CharAnimRawResource.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class CharAnimRawResource {
public:
    // Registers the class as a resource. Emitted in anim/AnimInit.o at
    // 0x2ADE80.
    static void Init();
};
