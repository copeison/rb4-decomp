#pragma once

// The keys of a waveform property in a property animation
// (anim/PropKeysWaveformCom.o): one class per waveform type, registered as
// "PropKeysWaveformCom_<type>" (TKey<Waveform<T>>). Only the registration
// Anim::Init calls is declared; the class is not reconstructed.
template <class T>
class PropKeysWaveformCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AD3B0 (float) and 0x2AD860 (Hmx::Color).
    static void Init();
};
