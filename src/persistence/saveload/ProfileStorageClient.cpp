#include "persistence/saveload/ProfileStorageClient.h"

#include <_pthread.h>

#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/threading/CallOnMainThread.h"
#include "persistence/profiles/Profile.h"
#include "persistence/storage/StorageRef_PS4.h"
#include "utl/streams/RevisionStream.h"
#include "utl/text/Symbol.h"

namespace {

// Pushes the "profile" heap, when it exists, for the scope. Each caller
// keeps its own cached heap number. Name not in the reference map.
class ProfileHeapScope {
public:
    explicit ProfileHeapScope(int heap) : mHeap(heap) {
        if (mHeap >= 0) {
            MemPushHeap(mHeap);
        }
    }
    ~ProfileHeapScope() {
        if (mHeap >= 0) {
            MemPopHeap();
        }
    }

private:
    int mHeap;
};

// Finds the "profile" heap once. Name not in the reference map.
inline int FindProfileHeap(int& heap) {
    if (heap == static_cast<int>(kNoHeap)) {
        heap = static_cast<int>(MemTryFindHeap("profile"));
    }
    return heap;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3AEEE0.
ProfileStorageClient::ProfileStorageClient(ProfileStorageClientCallback* callback)
    : mScratch(false), mLoaded(false) {
    __atomic_store_n(&mPendingFinds, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&mPendingReads, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&mPendingSaves, 0, __ATOMIC_SEQ_CST);
    mProfile = nullptr;
    mCallback = callback;
}

// Reconstructed from eboot.elf at 0x3AEF40.
void ProfileStorageClient::SetDevice(int device) {
    _GetStore()->SetDevice(device);
}

// Reconstructed from eboot.elf at 0x3AEF70.
StorageRef_PS4* ProfileStorageClient::_GetStore() const {
    Profile* const profile = mProfile;
    StorageRef_PS4* const store = &profile->mStorage;
    if (!store->IsBusy(false) && !store->IsValid()) {
        static Symbol saveFileName;
        if (saveFileName == Symbol()) {
            saveFileName = Symbol("save_file_name");
        }
        store->SetName(profile->GetSaveFileName(), Localize(saveFileName, nullptr, 0, TheLocale));
    }
    return store;
}

// Reconstructed from eboot.elf at 0x3AF080.
void ProfileStorageClient::ClearProfile(Profile* profile) {
    if (mProfile == profile) {
        mProfile = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x3AF090.
SaveLoad::Error ProfileStorageClient::Save(bool force) {
    Profile* const profile = mProfile;
    if (!profile->IsDirty() && !force) {
        return SaveLoad::kNoSaveNeeded;
    }
    if (profile->mPadNum == -1 || !ThePlatformMgr->IsSignedIn(profile->mPadNum)) {
        return SaveLoad::kNoActiveProfile;
    }
    StorageRef_PS4* const store = _GetStore();
    if (!store->IsValid()) {
        return SaveLoad::kNoSaveNeeded;
    }
    if (store->IsBusy(false)) {
        return SaveLoad::kNoSaveNeeded;
    }
    __atomic_add_fetch(&mPendingSaves, 1, __ATOMIC_SEQ_CST);
    Hmx::CallMemberFunctionOnMain<ProfileStorageClient, int>(this, &ProfileStorageClient::_PerformSave, 0);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3AF1D0. The thread query that opens it
// is the remains of the map's main-thread check.
void ProfileStorageClient::_PerformSave(const int& unused) {
    static_cast<void>(unused);
    static_cast<void>(scePthreadSelf());
    static int sHeap = static_cast<int>(kNoHeap);
    ProfileHeapScope heap(FindProfileHeap(sHeap));
    _SaveProfile();
    __atomic_sub_fetch(&mPendingSaves, 1, __ATOMIC_SEQ_CST);
}

// Reconstructed from eboot.elf at 0x3AF370.
SaveLoad::Error ProfileStorageClient::Load(bool find) {
    Profile* const profile = mProfile;
    if (profile->mPadNum == -1 || !ThePlatformMgr->IsSignedIn(profile->mPadNum)) {
        return SaveLoad::kNoActiveProfile;
    }
    StorageRef_PS4* const store = _GetStore();
    if (find) {
        __atomic_add_fetch(&mPendingFinds, 1, __ATOMIC_SEQ_CST);
        const int padNum = profile->mPadNum;
        theStorageMgr->Find(padNum, this, -1, true, profile->GetSaveFileName(), nullptr);
        return SaveLoad::kNoError;
    }
    if (!store->Exists()) {
        return SaveLoad::kNoDevice;
    }
    if (store->IsBusy(false)) {
        return SaveLoad::kNoSaveNeeded;
    }
    mScratch.Seek(0, kSeekBegin);
    mScratch.Resize(store->Size());
    store->Read(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.capacity(), this, nullptr);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3AF4A0.
void ProfileStorageClient::Restore() {
    Profile* const profile = mProfile;
    if (profile->mPadNum != -1 && mLoaded && profile->IsDirty()) {
        profile->EnableDirtying(false);
        _LoadProfileFromScratch();
        profile->EnableDirtying(true);
        profile->MakeClean();
    }
}

// Reconstructed from eboot.elf at 0x3AF500.
SaveLoad::Error ProfileStorageClient::_LoadProfileFromScratch() {
    if (mProfile == nullptr) {
        return SaveLoad::kUnknownError;
    }
    SaveLoad::Error error;
    RevisionStream revisionStream(mScratch, mProfile->GetSaveVersion(), 0);
    revisionStream.Seek(0, kSeekBegin);
    if (revisionStream.BeginRead()) {
        Profile* const profile = mProfile;
        revisionStream.EnableReadEncryption();
        profile->Load(revisionStream);
        revisionStream.DisableEncryption();
    } else if (revisionStream.mError != RevisionStream::kReadTooOld
               && revisionStream.mError != RevisionStream::kReadNoHeader) {
        return SaveLoad::kIncompatibleVersion;
    }
    error = SaveLoad::kNoError;
    mLoaded = true;
    return error;
}

// Reconstructed from eboot.elf at 0x3AF5D0.
SaveLoad::Error ProfileStorageClient::Delete() {
    if (mProfile->mPadNum == -1 || !ThePlatformMgr->IsSignedIn(mProfile->mPadNum)) {
        return SaveLoad::kNoActiveProfile;
    }
    StorageRef_PS4* const store = _GetStore();
    if (store->IsBusy(false) || !store->Exists()) {
        return SaveLoad::kNoSaveNeeded;
    }
    store->Delete(this, nullptr);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3AF650.
bool ProfileStorageClient::IsBusy() const {
    if (mProfile != nullptr && _GetStore()->IsBusy(false)) {
        return true;
    }
    return mPendingFinds > 0 || mPendingSaves > 0 || mPendingReads > 0;
}

// Reconstructed from eboot.elf at 0x3AF6A0.
void ProfileStorageClient::_SaveProfileToScratch() {
    Profile* const profile = mProfile;
    RevisionStream revisionStream(mScratch, profile->GetSaveVersion(), 0);
    revisionStream.Seek(0, kSeekBegin);
    revisionStream.BeginWrite();
    revisionStream.EnableWriteEncryption();
    profile->Save(revisionStream);
    revisionStream.DisableEncryption();
    profile->MakeClean();
    mLoaded = true;
}

// Reconstructed from eboot.elf at 0x3AF750.
void ProfileStorageClient::OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) {
    __atomic_sub_fetch(&mPendingFinds, 1, __ATOMIC_SEQ_CST);
    SaveLoad::Error error;
    if (result == StorageRef::kResultAccessDenied) {
        error = SaveLoad::kDeviceFull;
    } else if (result != StorageRef::kResultSuccess) {
        error = SaveLoad::kUnknownError;
    } else if (refs.size() == 1) {
        StorageRef_PS4* const store = _GetStore();
        *store = *refs.front();
        mScratch.Seek(0, kSeekBegin);
        mScratch.Resize(store->Size());
        store->Read(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.capacity(), this, context);
        return;
    } else if (refs.size() == 0) {
        error = SaveLoad::kNoDevice;
        if (mProfile != nullptr) {
            mProfile->mReadyToSave = true;
        }
    } else {
        error = SaveLoad::kMultipleDevices;
    }
    mCallback->OnProfileLoadResult(error);
}

// Reconstructed from eboot.elf at 0x3AF830.
void ProfileStorageClient::OnReadResult(void* context, int result, unsigned char* data, unsigned long size, unsigned long fileSize) {
    static_cast<void>(context);
    static_cast<void>(data);
    static_cast<void>(size);
    static_cast<void>(fileSize);
    __atomic_add_fetch(&mPendingReads, 1, __ATOMIC_SEQ_CST);
    Hmx::CallMemberFunctionOnMain<ProfileStorageClient, int>(this, &ProfileStorageClient::_PerformLoad, result);
}

// Reconstructed from eboot.elf at 0x3AF8F0. The thread query that opens it
// is the remains of the map's main-thread check.
void ProfileStorageClient::_PerformLoad(const int& result) {
    static_cast<void>(scePthreadSelf());
    SaveLoad::Error error = SaveLoad::kIncompatibleVersion;
    if (result == StorageRef::kResultSuccess) {
        static int sHeap = static_cast<int>(kNoHeap);
        ProfileHeapScope heap(FindProfileHeap(sHeap));
        error = _LoadProfile();
    }
    mCallback->OnProfileLoadResult(error);
    __atomic_sub_fetch(&mPendingReads, 1, __ATOMIC_SEQ_CST);
}

// Reconstructed from eboot.elf at 0x3AF980.
void ProfileStorageClient::OnWriteResult(void* context, int result, unsigned char* data, unsigned long size) {
    static_cast<void>(context);
    static_cast<void>(data);
    static_cast<void>(size);
    SaveLoad::Error error;
    switch (result) {
    case StorageRef::kResultDeviceFull:
        error = SaveLoad::kDeviceFull;
        break;
    case StorageRef::kResultInvalidDevice:
        error = SaveLoad::kNoDevice;
        break;
    case StorageRef::kResultAccessDenied:
        error = SaveLoad::kNoActiveProfile;
        break;
    case StorageRef::kResultGeneralError:
        error = SaveLoad::kUnknownError;
        break;
    default:
        error = SaveLoad::kNoError;
        break;
    }
    mCallback->OnProfileSaveResult(error);
}

// Reconstructed from eboot.elf at 0x3AF9B0.
void ProfileStorageClient::OnDeleteResult(void* context, int result) {
    static_cast<void>(context);
    mCallback->OnProfileDeleteResult(result == StorageRef::kResultSuccess ? SaveLoad::kNoError : SaveLoad::kUnknownError);
}

// Reconstructed from eboot.elf at 0x3AF9D0.
SaveLoad::Error ProfileStorageClient::_LoadProfile() {
    return _LoadProfileFromScratch();
}

// Reconstructed from eboot.elf at 0x3AF9E0.
void ProfileStorageClient::_SaveProfile() {
    _SaveProfileToScratch();
    _GetStore()->Write(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.size(), this, nullptr);
}
