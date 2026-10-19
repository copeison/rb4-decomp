// anim/AnimInstID.o (0x32C6C0 to 0x32C6E8).
#include "anim/core/AnimInstID.h"

AnimInstID AnimInstID::sNullID;

// Reconstructed from eboot.elf at 0x32C6C0.
AnimInstID::AnimInstID() : mDriver(nullptr), mId(static_cast<unsigned long>(-1)) {}

// Reconstructed from eboot.elf at 0x32C6D0.
AnimInstID::AnimInstID(AnimDriver* driver, unsigned long id) : mDriver(driver), mId(id) {}

// Reconstructed from eboot.elf at 0x32C6E0.
const AnimInstID& AnimInstID::NullID() {
    return sNullID;
}
