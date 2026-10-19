#pragma once

// The keys of a string property in a property animation (TKey<String>)
// (anim/PropKeysStrCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysStrCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2ACA50.
    static void Init();
};
