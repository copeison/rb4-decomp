#pragma once

#include "utl/text/Symbol.h"

// A property animation (anim/PropAnimResource.o). Only the class id the UI
// scrollbar's "position_anim" property names is declared; the class is not
// reconstructed.
class PropAnimResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as UIScrollbarCom::_Init (0x8CD270). The local static is at
    // 0x19F7BC8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("PropAnimResource");
        }
        return id;
    }
};
