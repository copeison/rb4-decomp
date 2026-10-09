#include <libsysmodule.h>

#include "os/joypads/Keyboard.h"

// Reconstructed from eboot.elf at 0x36D990.
void KeyboardInit() {
    KeyboardInitCommon();
    sceSysmoduleLoadModule(SCE_SYSMODULE_LIBIME);
}

// Reconstructed from eboot.elf at 0x36D9B0.
void KeyboardTerminate() {
    KeyboardTerminateCommon();
}
