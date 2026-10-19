#pragma once

// The base of the inverse-kinematics components ("Base class for all CharIk
// components"; class id "CharIk") (anim/CharIk.o). Only the registration
// Anim::Init calls is declared; the class is not reconstructed.
class CharIkCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AEBE0.
    static void Init();
};
