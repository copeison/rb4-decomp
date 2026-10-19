#pragma once

// A set of bone channel masks, extension "bonemask"
// (anim/CharBoneMaskResource.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class CharBoneMaskResource {
public:
    // Registers the class as a resource. Emitted in anim/AnimInit.o at
    // 0x2AE160.
    static void Init();
};
