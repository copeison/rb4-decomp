#include "math/random/Rand.h"

Rand gRand;

// Reconstructed from eboot.elf at 0x215980.
void Rand::Init() {}

// Reconstructed from eboot.elf at 0x215990.
void Rand::Terminate() {}

// Reconstructed from eboot.elf at 0x2159A0.
Rand::Rand() : mState(kDefaultSeed) {}

// Reconstructed from eboot.elf at 0x2159B0.
Rand::Rand(unsigned long seed) : mState(seed != 0 ? seed : kDefaultSeed) {}

// Reconstructed from eboot.elf at 0x2159D0.
void Rand::Seed(unsigned long seed) {
    mState = seed != 0 ? seed : kDefaultSeed;
}
