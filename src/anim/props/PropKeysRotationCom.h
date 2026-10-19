#pragma once

// The keys of a rotation property in a property animation
// (TKey<RotationKey>). Newer than the reference map; the class id string
// "PropKeysRotationCom" gives the name (anim/PropKeysRotationCom.o). Only the
// registration Anim::Init calls is declared; the class is not reconstructed.
class PropKeysRotationCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AB3C0.
    static void Init();
};
