#include "persistence/saveload/SystemOptionsStorageClient.h"

#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/threading/CallOnMainThread.h"
#include "persistence/options/SystemOptions.h"
#include "persistence/profiles/ProfileMgr.h"
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

// Reconstructed from eboot.elf at 0x3AFBE0.
SystemOptionsStorageClient::SystemOptionsStorageClient(SystemOptionsStorageClientCallback* callback, ProfileMgr* profileMgr)
    : mScratch(false), mCallback(callback), mProfileMgr(profileMgr) {}

// Reconstructed from eboot.elf at 0x3AFC20.
bool SystemOptionsStorageClient::IsBusy() const {
    return GetStore()->IsBusy(false);
}

// Reconstructed from eboot.elf at 0x3AFC40.
StorageRef_PS4* SystemOptionsStorageClient::GetStore() const {
    StorageRef_PS4* const store = &mProfileMgr->GetSystemOptions()->mStorage;
    if (!store->IsBusy(false)) {
        static Symbol systemOptionsName;
        if (systemOptionsName == Symbol()) {
            systemOptionsName = Symbol("system_options_name");
        }
        static const char* const sDisplayName = Localize(systemOptionsName, nullptr, 0, TheLocale);
        store->SetName(mProfileMgr->GetSystemSaveFileName(), sDisplayName);
    }
    return store;
}

// Reconstructed from eboot.elf at 0x3AFD70.
void SystemOptionsStorageClient::SetDevice(int device) {
    GetStore()->SetDevice(device);
}

// Reconstructed from eboot.elf at 0x3AFDA0.
SaveLoad::Error SystemOptionsStorageClient::Save(bool force) {
    if (!force && !mProfileMgr->GetSystemOptions()->IsDirty()) {
        return SaveLoad::kNoSaveNeeded;
    }
    const int device = mProfileMgr->GetBestStorageDevice();
    StorageRef_PS4* const store = GetStore();
    if (!store->IsValid()) {
        store->SetDevice(device);
    }
    if (!store->IsValid()) {
        return SaveLoad::kNoDevice;
    }
    if (store->IsBusy(false)) {
        return SaveLoad::kNoSaveNeeded;
    }
    Hmx::CallMemberFunctionOnMain<SystemOptionsStorageClient, int>(this, &SystemOptionsStorageClient::PerformSave, 0);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3AFFF0.
void SystemOptionsStorageClient::PerformSave(const int& unused) {
    static_cast<void>(unused);
    static int sHeap = static_cast<int>(kNoHeap);
    ProfileHeapScope heap(FindProfileHeap(sHeap));
    SaveOptions();
}

// Reconstructed from eboot.elf at 0x3B0150.
SaveLoad::Error SystemOptionsStorageClient::Load(bool find) {
    StorageRef_PS4* const store = GetStore();
    if (find) {
        const int owner = mProfileMgr->GetSystemOptions()->mStorage.Owner();
        theStorageMgr->Find(owner, this, -1, false, mProfileMgr->GetSystemSaveFileName(), nullptr);
        return SaveLoad::kNoError;
    }
    if (!store->Exists()) {
        return SaveLoad::kNoDevice;
    }
    if (store->IsBusy(false)) {
        return SaveLoad::kNoSaveNeeded;
    }
    mScratch.Resize(store->Size());
    store->Read(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.capacity(), this, nullptr);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3B0240.
SaveLoad::Error SystemOptionsStorageClient::Delete() {
    StorageRef_PS4* const store = GetStore();
    if (store->IsBusy(false) || !store->Exists()) {
        return SaveLoad::kNoSaveNeeded;
    }
    store->Delete(this, nullptr);
    return SaveLoad::kNoError;
}

// Reconstructed from eboot.elf at 0x3B02A0.
void SystemOptionsStorageClient::SaveOptionsToScratch() {
    SystemOptions* const options = mProfileMgr->GetSystemOptions();
    RevisionStream revisionStream(mScratch, 0, 0);
    revisionStream.Seek(0, kSeekBegin);
    revisionStream.BeginWrite();
    options->Save(revisionStream);
    options->MakeClean();
}

// Reconstructed from eboot.elf at 0x3B0330.
SaveLoad::Error SystemOptionsStorageClient::LoadOptionsFromScratch() {
    SaveLoad::Error error = SaveLoad::kNoError;
    RevisionStream revisionStream(mScratch, 0, 0);
    revisionStream.Seek(0, kSeekBegin);
    if (revisionStream.BeginRead()) {
        SystemOptions* const options = mProfileMgr->GetSystemOptions();
        options->Load(revisionStream);
        options->MakeClean();
    } else if (revisionStream.mError != RevisionStream::kReadTooOld
               && revisionStream.mError != RevisionStream::kReadNoHeader) {
        error = SaveLoad::kIncompatibleVersion;
    }
    return error;
}

// Reconstructed from eboot.elf at 0x3B03F0.
void SystemOptionsStorageClient::OnFindResult(void* context, int result, const eastl::list<StorageRef_PS4*>& refs) {
    SaveLoad::Error error = SaveLoad::kUnknownError;
    if (result == StorageRef::kResultSuccess) {
        if (refs.size() == 0) {
            error = SaveLoad::kNoDevice;
        } else if (refs.size() == 1) {
            StorageRef_PS4* const store = GetStore();
            *store = *refs.front();
            mScratch.Resize(store->Size());
            store->Read(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.capacity(), this, context);
            return;
        } else {
            error = SaveLoad::kMultipleDevices;
        }
    }
    mCallback->OnSysOptionsLoadResult(error);
}

// Reconstructed from eboot.elf at 0x3B04A0.
void SystemOptionsStorageClient::OnReadResult(void* context, int result, unsigned char* data, unsigned long size, unsigned long fileSize) {
    static_cast<void>(context);
    static_cast<void>(data);
    static_cast<void>(size);
    static_cast<void>(fileSize);
    Hmx::CallMemberFunctionOnMain<SystemOptionsStorageClient, int>(this, &SystemOptionsStorageClient::PerformLoad, result);
}

// Reconstructed from eboot.elf at 0x3B0660.
void SystemOptionsStorageClient::PerformLoad(const int& result) {
    SaveLoad::Error error = SaveLoad::kIncompatibleVersion;
    if (result == StorageRef::kResultSuccess) {
        static int sHeap = static_cast<int>(kNoHeap);
        ProfileHeapScope heap(FindProfileHeap(sHeap));
        LoadOptionsFromScratch();
        error = SaveLoad::kNoError;
    }
    mCallback->OnSysOptionsLoadResult(error);
}

// Reconstructed from eboot.elf at 0x3B07A0.
void SystemOptionsStorageClient::OnWriteResult(void* context, int result, unsigned char* data, unsigned long size) {
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
    mCallback->OnSysOptionsSaveResult(error);
}

// Reconstructed from eboot.elf at 0x3B07D0.
void SystemOptionsStorageClient::OnDeleteResult(void* context, int result) {
    static_cast<void>(context);
    mCallback->OnSysOptionsDeleteResult(result == StorageRef::kResultSuccess ? SaveLoad::kNoError : SaveLoad::kUnknownError);
}

// Reconstructed from eboot.elf at 0x3B07F0.
SaveLoad::Error SystemOptionsStorageClient::LoadOptions() {
    return LoadOptionsFromScratch();
}

// Reconstructed from eboot.elf at 0x3B08B0.
void SystemOptionsStorageClient::SaveOptions() {
    SaveOptionsToScratch();
    GetStore()->Write(reinterpret_cast<unsigned char*>(mScratch.mBuffer.begin()), mScratch.mBuffer.size(), this, nullptr);
}
