#include "persistence/storage/StorageRef_PS4.h"

#include <save_data.h>
#include <stdio.h>
#include <string.h>

#include "os/content/ContentMgr.h"
#include "os/files/File.h"
#include "os/platform/PlatformMgr.h"
#include "persistence/storage/StorageMgr.h"
#include "persistence/storage/StorageMgr_PS4.h"
#include "utl/streams/FileStream.h"
#include "utl/text/MakeString.h"

namespace {

// The title's save data directory is "slot017"; both the buffer size and
// the number are 17. Names not in the reference map.
constexpr unsigned long kSaveDirNameSize = 17;
constexpr int kSaveDirIndex = 17;

// The blocks InitMount creates the directory with, and the blocks the
// operations mount it with. Names not in the reference map.
constexpr SceSaveDataBlocks kCreateBlocks = 1280;
constexpr SceSaveDataBlocks kMountBlocks = 320;

// The mode InitMount creates the directory with. Name not in the reference
// map.
constexpr SceSaveDataMountMode kCreateMountMode =
    SCE_SAVE_DATA_MOUNT_MODE_RDWR | SCE_SAVE_DATA_MOUNT_MODE_CREATE | SCE_SAVE_DATA_MOUNT_MODE_COPY_ICON;

// The error types PlatformMgr::ShowSaveDataDialog shows. Names not in the
// reference map.
constexpr int kDialogErrorNoSpace = 4;
constexpr int kDialogErrorBroken = 6;

// FileStream's write mode: kWrite with the low bit set. Name not in the
// reference map.
constexpr FileMode kWriteSaveData = static_cast<FileMode>(5);

}  // namespace

// Reconstructed from eboot.elf at 0x3B1140.
StorageRef_PS4::StorageRef_PS4()
    : mState(kStateIdle),
      mOwner(-1),
      mSize(0),
      mMountInitialized(false),
      mContext(nullptr),
      mCallback(nullptr),
      mBuffer(nullptr),
      mBufferSize(0),
      mMountIndex(-1) {}

// Reconstructed from eboot.elf at 0x3B11D0.
StorageRef_PS4::StorageRef_PS4(const StorageRef_PS4& other)
    : StorageRef(),
      ThreadCallback(),
      mState(kStateIdle),
      mOwner(other.mOwner),
      mSize(other.mSize),
      mMountInitialized(false),
      mContext(nullptr),
      mCallback(nullptr),
      mBuffer(nullptr),
      mBufferSize(0),
      mMountIndex(-1) {
    SetName(other.mName.c_str(), other.mDisplayName.c_str());
}

// Reconstructed from eboot.elf at 0x3B1310.
StorageRef_PS4& StorageRef_PS4::operator=(const StorageRef_PS4& other) {
    mSize = other.mSize;
    mOwner = other.mOwner;
    mMountInitialized = other.mMountInitialized;
    SetName(other.mName.c_str(), "");
    return *this;
}

// Reconstructed from eboot.elf at 0x3B1350.
const char* StorageRef_PS4::Name() {
    return mName.c_str();
}

// Reconstructed from eboot.elf at 0x3B1360.
void StorageRef_PS4::SetName(const char* name, const char* displayName) {
    mName = name;
    mDisplayName = displayName;
}

// Reconstructed from eboot.elf at 0x3B1430.
unsigned long StorageRef_PS4::Size() {
    if (mSize == 0) {
        mSize = _DetermineSize();
    }
    return mSize;
}

// Reconstructed from eboot.elf at 0x3B1460.
unsigned long StorageRef_PS4::_DetermineSize() {
    mState = kStateReading;
    unsigned long size = 0;
    _WithMountedSaveDir([this, &size] {
        void* const file = FileOpen(mPath.c_str(), kRead);
        if (!FileFail(file)) {
            size = static_cast<unsigned long>(FileSize(file));
        }
        FileClose(file);
    });
    mState = kStateIdle;
    return size;
}

// Reconstructed from eboot.elf at 0x3B1540.
void StorageRef_PS4::Read(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) {
    mContext = context;
    mCallback = callback;
    mBuffer = data;
    mBufferSize = size;
    mState = kStateReading;
    ThreadCall(this);
}

// Reconstructed from eboot.elf at 0x3B1560.
void StorageRef_PS4::Write(unsigned char* data, unsigned long size, StorageCallback* callback, void* context) {
    mContext = context;
    mCallback = callback;
    mBuffer = data;
    mBufferSize = size;
    mState = kStateWriting;
    ThreadCall(this);
}

// Reconstructed from eboot.elf at 0x3B1580.
void StorageRef_PS4::Delete(StorageCallback* callback, void* context) {
    static_cast<void>(callback);
    static_cast<void>(context);
    mState = kStateWriting;
    _WithMountedSaveDir([this] { FileDelete(mPath.c_str()); });
    mState = kStateIdle;
}

// Reconstructed from eboot.elf at 0x3B1640.
void StorageRef_PS4::_WithMountedSaveDir(const std::function<void()>& operation) {
    if (InitMount()) {
        const unsigned int mountMode =
            mState == kStateReading ? SCE_SAVE_DATA_MOUNT_MODE_RDONLY : SCE_SAVE_DATA_MOUNT_MODE_RDWR;
        if (_MountSaveDir(mountMode)) {
            operation();
        }
        _UnmountSaveDir();
    }
}

// Reconstructed from eboot.elf at 0x3B16A0.
bool StorageRef_PS4::Exists() {
    mState = kStateReading;
    bool exists = false;
    _WithMountedSaveDir([&exists, this] { exists = FileExists(mPath.c_str(), kReadNoBuffer); });
    mState = kStateIdle;
    return exists;
}

// Reconstructed from eboot.elf at 0x3B1770.
bool StorageRef_PS4::IsValid() const {
    return mOwner != -1 && mName.c_str()[0] != '\0';
}

// Reconstructed from eboot.elf at 0x3B1790.
bool StorageRef_PS4::IsBusy(bool wait) const {
    static_cast<void>(wait);
    return mState != kStateIdle;
}

// Reconstructed from eboot.elf at 0x3B17A0.
int StorageRef_PS4::ThreadStart() {
    switch (mState) {
    case kStateWriting:
        return _ThreadWrite();
    case kStateReading:
        return _ThreadRead();
    default:
        return kResultGeneralError;
    }
}

// Reconstructed from eboot.elf at 0x3B17C0.
int StorageRef_PS4::_ThreadRead() {
    int result = kResultGeneralError;
    _WithMountedSaveDir([this, &result] {
        FileStream stream(mPath.c_str(), kRead, false);
        if (!stream.Fail()) {
            stream.Read(mBuffer, mBufferSize);
            result = kResultSuccess;
        }
    });
    return result;
}

// Reconstructed from eboot.elf at 0x3B1890.
int StorageRef_PS4::_ThreadWrite() {
    int result = kResultGeneralError;
    _WithMountedSaveDir([this, &result] {
        FileMkDir(mMountPoint.c_str());
        FileStream stream(mPath.c_str(), kWriteSaveData, false);
        if (!stream.Fail()) {
            stream.Write(mBuffer, mBufferSize);
            stream.Flush();
            if (!stream.Fail()) {
                result = kResultSuccess;
            }
        }
    });
    return result;
}

// Reconstructed from eboot.elf at 0x3B1990.
void StorageRef_PS4::ThreadDone(int result) {
    if (mState == kStateWriting) {
        mCallback->OnWriteResult(mContext, result, mBuffer, mBufferSize);
    } else if (mState == kStateReading) {
        mCallback->OnReadResult(mContext, result, mBuffer, mBufferSize, result == kResultSuccess ? mSize : 0);
    }
    mState = kStateIdle;
}

// Reconstructed from eboot.elf at 0x3B1A70. The dialog goes through the
// PlatformMgr_PS4.o copy of the manager pointer (0x19FE988).
bool StorageRef_PS4::InitMount() {
    if (mMountInitialized) {
        return true;
    }
    SceSaveDataDirName dirName;
    snprintf(dirName.data, kSaveDirNameSize, "%s%03d", "slot", kSaveDirIndex);
    SceSaveDataMount2 mount = {};
    SceSaveDataMountResult mountResult = {};
    mount.userId = mOwner;
    mount.dirName = &dirName;
    mount.mountMode = kCreateMountMode;
    mount.blocks = kCreateBlocks;
    const int error = sceSaveDataMount2(&mount, &mountResult);
    if (error == 0) {
        _SetSaveDataParams(&mountResult.mountPoint);
        sceSaveDataUmount(&mountResult.mountPoint);
        return true;
    }
    if ((error | 4) == SCE_SAVE_DATA_ERROR_EXISTS) {
        // The directory exists, or the library is busy with it.
        return true;
    }
    if (error == SCE_SAVE_DATA_ERROR_BROKEN) {
        ThePlatformMgr->ShowSaveDataDialog(1, mOwner, nullptr, &dirName, kDialogErrorBroken, mountResult.requiredBlocks);
        return false;
    }
    if (error == SCE_SAVE_DATA_ERROR_NO_SPACE_FS) {
        ThePlatformMgr->ShowSaveDataDialog(1, mOwner, nullptr, &dirName, kDialogErrorNoSpace, mountResult.requiredBlocks);
        return false;
    }
    return false;
}

// Reconstructed from eboot.elf at 0x3B1C30.
void StorageRef_PS4::_SetSaveDataParams(SceSaveDataMountPoint* mountPoint) {
    SceSaveDataParam param;
    memset(&param, 0, sizeof(param));
    const char* const title = mDisplayName.c_str();
    strncpy(param.title, title, strlen(title));
    sceSaveDataSetParam(mountPoint, SCE_SAVE_DATA_PARAM_TYPE_ALL, &param, sizeof(param));
}

// Reconstructed from eboot.elf at 0x3B1CC0. The cheat flag is read through
// the TheContentMgr reference at 0x18FB600.
bool StorageRef_PS4::_MountSaveDir(unsigned int mountMode) {
    if (TheContentMgr.AreAllMountsGoingToFailDueToCheat()) {
        mMountIndex = -1;
        return false;
    }
    mMountIndex = static_cast<StorageMgr_PS4*>(theStorageMgr)->AquireMountPoint();
    const char* const mountPoint = (FormatString("savedata%d:") << mMountIndex).Str();
    mMountPoint = mountPoint;
    const char* const name = Name();
    const char* const path = (FormatString("%s/%s") << mMountPoint << name).Str();
    mPath = path;

    SceSaveDataDirName dirName;
    snprintf(dirName.data, kSaveDirNameSize, "%s%03d", "slot", kSaveDirIndex);
    SceSaveDataMount2 mount = {};
    SceSaveDataMountResult mountResult = {};
    mount.userId = mOwner;
    mount.dirName = &dirName;
    mount.mountMode = mountMode;
    mount.blocks = kMountBlocks;
    return sceSaveDataMount2(&mount, &mountResult) == 0;
}

// Reconstructed from eboot.elf at 0x3B1EF0.
void StorageRef_PS4::_UnmountSaveDir() {
    if (mMountIndex != -1) {
        SceSaveDataMountPoint mountPoint;
        strcpy(mountPoint.data, (FormatString("/savedata%d") << mMountIndex).Str());
        sceSaveDataUmount(&mountPoint);
        static_cast<StorageMgr_PS4*>(theStorageMgr)->ReleaseMountPoint(mMountIndex);
        mMountIndex = -1;
        mMountPoint = "";
        mPath = "";
    }
}
