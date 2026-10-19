#include "os/users/UserGuid.h"

#include "utl/text/Str.h"

// Reconstructed from eboot.elf at 0x11B15D0. The binary places the object in
// the second net block.
UserGuid& UserGuid::operator=(String str) {
    HxGuid::operator=(str);
    return *this;
}
