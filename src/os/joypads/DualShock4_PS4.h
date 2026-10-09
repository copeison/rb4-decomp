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
    // The vtable pointer. Name not in the reference map.
    unsigned char mVtable[0x8];
    // Set by Activate (PhysicalController::GetPlatformUserId). Name not in
    // the reference map.
    int mPlatformUserId;
    // PhysicalController and DualShock4Controller state that is not
    // modelled. Name not in the reference map.
    unsigned char mControllerState[0x1B4 - 0xC];
    // The scePadOpen handle that Activate (0x8D02F0) stores. Name not in
    // the reference map.
    int mPadHandle;
    // The pad readings and the rest of the controller state, not modelled.
    // Name not in the reference map.
    unsigned char mPadState[0x2004 - 0x1B8];
};

static_assert(sizeof(DualShock4Controller) == 0x2004);
