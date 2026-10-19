#pragma once

// The keys of a scale-rotation-translation property in a property animation
// (TKey<TransformSRTKey>). Newer than the reference map; the class id string
// "PropKeysTransformSRTCom" gives the name (anim/PropKeysTransformSRTCom.o).
// Only the registration Anim::Init calls is declared; the class is not
// reconstructed.
class PropKeysTransformSRTCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AB800.
    static void Init();
};
