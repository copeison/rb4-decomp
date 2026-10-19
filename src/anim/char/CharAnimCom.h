#pragma once

// The component that plays a character animation, absolutely or relative to
// the bind pose or another animation ("relative", "relative_anim")
// (anim/CharAnimCom.o). Only the registration Anim::Init calls is declared;
// the class is not reconstructed.
class CharAnimCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AE440.
    static void Init();
};
