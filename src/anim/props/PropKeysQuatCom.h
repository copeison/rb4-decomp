#pragma once

// The keys of a quaternion property in a property animation (TKey<Quat>)
// (anim/PropKeysQuatCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysQuatCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ABC40.
    static void Init();
};
