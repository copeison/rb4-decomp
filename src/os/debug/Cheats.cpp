#include "os/debug/Cheats.h"

#include "os/system/System.h"
#include "utl/data/DataArray.h"

CheatsManager* theCheatsManager;

// Reconstructed from eboot.elf at 0x391D90.
void CheatsInit() {
    DataArray* disable = SystemConfig(Symbol("cheats"), Symbol("disable_cheats"));
    if (disable->Int(1) == 0) {
        theCheatsManager = new CheatsManager;
    }
}

// Reconstructed from eboot.elf at 0x391E20.
void CheatsTerminate() {
    delete theCheatsManager;
    theCheatsManager = nullptr;
}
