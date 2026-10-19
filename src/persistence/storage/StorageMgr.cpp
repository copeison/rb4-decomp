#include "persistence/storage/StorageMgr.h"

#include "persistence/storage/StorageMgr_PS4.h"
#include "persistence/storage/StorageRef.h"

StorageMgr* theStorageMgr;

// Reconstructed from eboot.elf at 0x11B2C80.
void StorageMgr::Init() {
    theStorageMgr = new StorageMgr_PS4();
}

// Reconstructed from eboot.elf at 0x11B2CB0.
const char* StorageRef::ResultToString(int result) {
    switch (result) {
    case kResultDeviceFull:
        return "kResultDeviceFull";
    case kResultInvalidDevice:
        return "kResultInvalidDevice";
    case kResultAccessDenied:
        return "kResultAccessDenied";
    case kResultGeneralError:
        return "kResultGeneralError";
    case kResultSuccess:
        return "kResultSuccess";
    default:
        return "(unknown error code)";
    }
}
