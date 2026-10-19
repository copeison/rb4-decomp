#pragma once

#include "ui/input/UIInputCom.h"
#include "utl/text/Symbol.h"

// The game's UI input component (rb_meta/RBUIInputCom.o), built on
// UIInputCom. Only its class symbol is declared; the class is not
// reconstructed.
class RBUIInputCom : public UIInputCom {
public:
    // "RBUIInputCom".
    static Symbol sId;  // 0x1B0F6B0
};
