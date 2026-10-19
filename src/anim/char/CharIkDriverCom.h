#pragma once

// The component that runs a character's inverse-kinematics components in
// order ("ik", IkRef) (anim/CharIkDriverCom.o). Only the registration
// Anim::Init calls is declared; the class is not reconstructed.
class CharIkDriverCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AEFC0.
    static void Init();
};
