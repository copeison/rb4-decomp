#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// One slot of the overshell (rb_meta/RBOvershellSlotCom.o, about
// 0xC03000-0xC24B00; _Init 0xC05FD0, constructor 0xC0A530, static
// initializer 0xC24990): "Holds state for an overshell slot." Only the
// class id the overshell's "overshell_slots" property requires is
// declared; the class is not reconstructed.
class RBOvershellSlotCom : public Component {
public:
    // "RBOvershellSlotCom".
    static Symbol sId;  // 0x1B03ED0
};
