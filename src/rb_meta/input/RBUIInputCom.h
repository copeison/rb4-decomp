#pragma once

#include <cstddef>

#include "ui/input/UIInputCom.h"
#include "utl/text/Symbol.h"

// The game's UI input component (rb_meta/RBUIInputCom.o; constructor
// 0xCB1A00, vtable 0x1977FD0, static initializer 0xCB3B90): UIInputCom with
// the band's focus and strum lock handling. Only what the dialogs use is
// declared; the class is not reconstructed.
class RBUIInputCom : public UIInputCom {
public:
    // "RBUIInputCom".
    static Symbol sId;  // 0x1B0F6B0

    // The run-time focus and strum lock state between UIInputCom's fields
    // and the properties, which the constructor sets up. Not modelled.
    unsigned char mRuntimeState[120];
    // "ignore_sticky_focus": "Turn this on if you want your scene to deal
    // with focus itself, rather than having RBUIInputCom attempt to handle
    // it generically."
    bool mIgnoreStickyFocus;
    // "auto_strum_lock": the receivers strum lock when focused.
    bool mAutoStrumLock;
    // "disable_strum_lock": no strum lock in the scene.
    bool mDisableStrumLock;
};

static_assert(offsetof(RBUIInputCom, mRuntimeState) == 408);
static_assert(offsetof(RBUIInputCom, mIgnoreStickyFocus) == 528);
static_assert(offsetof(RBUIInputCom, mDisableStrumLock) == 530);
