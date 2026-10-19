#include "persistence/storage/StorageMgr_PS4.h"

#include <save_data.h>
#include <user_service.h>

#include "os/locale/Locale.h"
#include "os/threading/CritSec.h"
#include "persistence/storage/StorageRef_PS4.h"
#include "utl/text/Symbol.h"

const int StorageRef::kAnyUser = -1;
const int StorageRef::kAnyDevice = -1;

namespace {

// Guards the mount points. At 0x1A6E2B8; the object's static initializer
// (0x3B10A0) constructs it. Name not in the reference map.
CritSec gMountPointCritSec;

}  // namespace

// Reconstructed from eboot.elf at 0x3B0AA0.
StorageMgr_PS4::StorageMgr_PS4() {
    sceUserServiceInitialize(nullptr);
    sceSaveDataInitialize3(nullptr);
    mMountPointsUsed.resize(kNumMountPoints);
    for (int i = 0; i < kNumMountPoints; ++i) {
        mMountPointsUsed[i] = false;
    }
}

// Reconstructed from eboot.elf at 0x3B0C20.
StorageMgr_PS4::~StorageMgr_PS4() {}

// Reconstructed from eboot.elf at 0x3B0C90.
void StorageMgr_PS4::Find(int user, StorageCallback* callback, int device, bool prompt, const char* name, void* context) {
    static_cast<void>(device);
    static_cast<void>(prompt);
    eastl::list<StorageRef_PS4*> refs;
    StorageRef_PS4* const ref = new StorageRef_PS4();
    ref->SetOwner(user);
    static Symbol saveFileName;
    if (saveFileName == Symbol()) {
        saveFileName = Symbol("save_file_name");
    }
    ref->SetName(name, Localize(saveFileName, nullptr, 0, TheLocale));
    int result = StorageRef::kResultAccessDenied;
    if (ref->InitMount()) {
        result = StorageRef::kResultSuccess;
        if (ref->Size() != 0) {
            refs.push_back(ref);
        }
    }
    callback->OnFindResult(context, result, refs);
    for (auto it = refs.begin(); it != refs.end(); ++it) {
        delete *it;
    }
}

// Reconstructed from eboot.elf at 0x3B0E80.
void StorageMgr_PS4::PromptForDevice(int user, unsigned long size, StorageCallback* callback, void* context) {
    static_cast<void>(user);
    static_cast<void>(size);
    static_cast<void>(callback);
    static_cast<void>(context);
}

// Reconstructed from eboot.elf at 0x3B0E90.
int StorageMgr_PS4::FindInDirectory(int user, const char* directory, eastl::list<StorageRef_PS4*>& refs) {
    static_cast<void>(user);
    static_cast<void>(directory);
    static_cast<void>(refs);
    return 0;
}

// Reconstructed from eboot.elf at 0x3B0EA0.
int StorageMgr_PS4::AquireMountPoint() {
    ScopedCritSec lock(gMountPointCritSec);
    for (int i = 0; i < kNumMountPoints; ++i) {
        if (!mMountPointsUsed[i]) {
            mMountPointsUsed[i] = true;
            return i;
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x3B1040.
void StorageMgr_PS4::ReleaseMountPoint(int index) {
    ScopedCritSec lock(gMountPointCritSec);
    mMountPointsUsed[index] = false;
}
