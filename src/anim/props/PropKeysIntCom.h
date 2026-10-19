#pragma once

// The keys of an integer property in a property animation
// (anim/PropKeysIntCom.o): one class per integer type, registered as
// "PropKeysIntCom_<type>" (TKey<kInt8> to TKey<kUint64>). Only the
// registration Anim::Init calls is declared; the class is not
// reconstructed.
template <class T>
class PropKeysIntCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A84E0 (signed char), 0x2A8990 (short), 0x2A8E40 (int),
    // 0x2A92F0 (unsigned char), 0x2A97A0 (unsigned short), 0x2A9C50
    // (unsigned int), 0x2AA100 (long) and 0x2AA5B0 (unsigned long).
    static void Init();
};
