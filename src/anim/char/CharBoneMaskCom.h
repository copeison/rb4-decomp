#pragma once

// The component that masks bone channels ("bone_channel_masks",
// "skeleton_file") (anim/CharBoneMaskCom.o). Only the registration Anim::Init
// calls is declared; the class is not reconstructed.
class CharBoneMaskCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AE800.
    static void Init();
};
