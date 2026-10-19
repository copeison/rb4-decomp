#pragma once

// The component that plays a property animation over a frame range
// ("frame_range_start", "frame_range_end", "time_units", "is_looping",
// "target_order") (anim/PropAnimCom.o). Only the registration Anim::Init
// calls is declared; the class is not reconstructed.
class PropAnimCom {
public:
    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2A7C70.
    static void Init();
};
