#pragma once

// The keys of an object-id property in a property animation
// (TKey<GameObjectId>) (anim/PropKeysObjCom.o). Only the registration
// Anim::Init calls is declared; the class is not reconstructed.
class PropKeysObjCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AC0F0.
    static void Init();
};
