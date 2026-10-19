#pragma once

// The keys of a symbol property in a property animation (TKey<Symbol>)
// (anim/PropKeysSymCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysSymCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AC5A0.
    static void Init();
};
