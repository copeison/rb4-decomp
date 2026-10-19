#pragma once

// The keys of a bool property in a property animation (TKey<bool>)
// (anim/PropKeysBoolCom.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysBoolCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A8030.
    static void Init();
};
