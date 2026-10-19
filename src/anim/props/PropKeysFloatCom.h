#pragma once

// The keys of a float property in a property animation (TKey<float>, or
// TKey<ValueTangentFloat> with tangents) (anim/PropKeysFloatCom.o). Only the
// registration Anim::Init calls is declared; the class is not reconstructed.
class PropKeysFloatCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AAA60.
    static void Init();
};
