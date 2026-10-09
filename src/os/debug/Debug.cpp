#include "os/debug/Debug.h"

// Reconstructed from eboot.elf at 0x35CC30.
void HmxFail(const char* msg) {
    TheDebug.Fail(msg);
}
