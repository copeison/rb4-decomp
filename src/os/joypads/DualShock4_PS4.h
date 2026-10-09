#pragma once

// DualShock 4 controller (os/DualShock4_PS4.o), the base of the PS4 special
// controllers. The class has not been reconstructed: only the members its
// subclasses use are declared, and the rest of the layout is opaque.
class DualShock4Controller {
public:
    // Virtual in the original; the vtable has not been reconstructed and
    // subclasses call it directly.
    void Activate(int platformUserId, bool special);  // 0x8D02F0

protected:
    // The vtable and PhysicalController state. Name not in the reference map.
    unsigned char mUnknown0[0x8];
    // Set by Activate (PhysicalController::GetPlatformUserId). Name not in
    // the reference map.
    int mPlatformUserId;
    // Name not in the reference map.
    unsigned char mUnknownC[0x2004 - 0xC];
};

static_assert(sizeof(DualShock4Controller) == 0x2004);
