#pragma once

// The inverse-kinematics component that moves a bone toward weighted bone
// targets (BoneTarget); the map names the class CharIkBoneInterp
// (anim/CharIkBoneInterpCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class CharIkBoneInterp {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AF460.
    static void Init();
};
