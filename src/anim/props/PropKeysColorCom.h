#pragma once

// The keys of a colour property in a property animation (TKey<Hmx::Color>)
// (anim/PropKeysColorCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysColorCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ACF00.
    static void Init();
};
