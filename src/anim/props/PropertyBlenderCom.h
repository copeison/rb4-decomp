#pragma once

// The component that blends between frames of property animations by name
// ("Used to blend between arbitrary PropAnim frames by name.")
// (anim/PropertyBlenderCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropertyBlenderCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AF8E0.
    static void Init();
};
