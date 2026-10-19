#pragma once

// The keys of a Vector3 property in a property animation (TKey<Vector3>)
// (anim/PropKeysVec3Com.o). Only the registration Anim::Init calls is
// declared; the class is not reconstructed.
class PropKeysVec3Com {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AAF10.
    static void Init();
};
