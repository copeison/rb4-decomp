#include "persistence/saveload/SaveLoadMgr.h"

#include "os/platform/PlatformMgr.h"
#include "os/system/System.h"
#include "os/threading/CallOnMainThread.h"
#include "persistence/options/SystemOptions.h"
#include "persistence/profiles/Profile.h"
#include "persistence/profiles/ProfileMgr.h"
#include "persistence/storage/StorageRef.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"

namespace {

// The room a profile save prompt asks for, and a system options save
// prompt. Names not in the reference map.
constexpr unsigned long kProfileSaveSize = 0x19000;
constexpr unsigned long kSystemOptionsSaveSize = 0x400;

// The pad of the system options' status messages. Name not in the
// reference map.
constexpr int kNoPad = -1;

// Records SaveSync's results. Its slots only store a result and its flag;
// the constructor leaves them unset. The vtable is at 0x18FCE10, with the
// destructors at 0x3AEE70 and 0x3AEE30 and the slots at 0x3AEE40-0x3AEE60.
// Name not in the reference map.
class SaveSyncCallback : public ProfileStorageClientCallback {
public:
    SaveSyncCallback() {}
    ~SaveSyncCallback() override {}

    void OnProfileSaveResult(SaveLoad::Error error) override {
        mSaveResult = error;
        mSaveDone = true;
    }
    void OnProfileLoadResult(SaveLoad::Error error) override {
        mLoadResult = error;
        mLoadDone = true;
    }
    void OnProfileDeleteResult(SaveLoad::Error error) override {
        mDeleteResult = error;
        mDeleteDone = true;
    }

    bool mSaveDone;
    bool mLoadDone;
    bool mDeleteDone;
    SaveLoad::Error mSaveResult;
    SaveLoad::Error mLoadResult;
    SaveLoad::Error mDeleteResult;
};

}  // namespace

SaveLoadMgr* theSaveLoadMgr;

// Reconstructed from eboot.elf at 0x3ABB80.
const char* SaveLoad::ErrorToString(int error) {
    switch (error) {
    case kNoError:
        return "kNoError";
    case kNoActiveProfile:
        return "kNoActiveProfile";
    case kNoDevice:
        return "kNoDevice";
    case kMultipleDevices:
        return "kMultipleDevices";
    case kNoSaveNeeded:
        return "kNoSaveNeeded";
    case kIncompatibleVersion:
        return "kIncompatibleVersion";
    case kDeviceFull:
        return "kDeviceFull";
    case kUnknownError:
        return "kUnknownError";
    default:
        return "(unknown error code)";
    }
}

// Reconstructed from eboot.elf at 0x3ABBA0.
void SaveLoadMgr::Init(ProfileMgr* profileMgr) {
    theSaveLoadMgr = new SaveLoadMgr(profileMgr);
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = static_cast<MsgSink*>(theSaveLoadMgr);
    DataSetGlobal(Symbol("saveload_mgr"), self);
}

// Reconstructed from eboot.elf at 0x3ABC70.
SaveLoadMgr::SaveLoadMgr(ProfileMgr* profileMgr)
    : SyncMsgSource(&mCritSec),
      mAutosave(true),
      mIsBusy(false),
      mPendingDevice(-1),
      mPendingMsg(),
      mProfileClient(this),
      mSysOptionsClient(this, profileMgr),
      mProfileMgr(profileMgr) {
    mStorageChangedSink.mSink = nullptr;
    mStorageChangedSink.mSource = nullptr;
    mProfileRemovedSink.mSink = nullptr;
    mProfileRemovedSink.mSource = nullptr;
    mPendingSaves.reserve(64);
    mPendingLoads.reserve(64);
    DataArrayPtr config(SystemConfig()->FindArray(Symbol("viper_autosave"), false));
    if (config != nullptr) {
        bool enable = true;
        config->FindData(Symbol("enable"), enable, false);
        mAutosave = enable;
    }
    ThePlatformMgr->AddSink(&mStorageChangedSink, this, StorageChangedMsg::Event(), Symbol("on_storage_changed"));
    static Symbol profileAboutToBeRemoved;
    if (profileAboutToBeRemoved == Symbol()) {
        profileAboutToBeRemoved = Symbol("profile_about_to_be_removed");
    }
    static Symbol onProfileAboutToBeRemoved;
    if (onProfileAboutToBeRemoved == Symbol()) {
        onProfileAboutToBeRemoved = Symbol("on_profile_about_to_be_removed");
    }
    mProfileMgr->AddSink(&mProfileRemovedSink, this, profileAboutToBeRemoved, onProfileAboutToBeRemoved);
}

// Reconstructed from eboot.elf at 0x3AC160. A linked subscription leaves its
// source before the link unlinks itself; the binary does this for each
// subscription as it is destroyed, after the lock, the later one first.
SaveLoadMgr::~SaveLoadMgr() {
    for (MsgSource::EventSinkElem* sink : {&mProfileRemovedSink, &mStorageChangedSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x3AC480.
void SaveLoadMgr::SaveAll(bool force) {
    for (Profile* const profile : mProfileMgr->mProfiles) {
        if (profile->IsDirty() || force) {
            Save(profile, force);
        }
    }
}

// Reconstructed from eboot.elf at 0x3AC510.
void SaveLoadMgr::Save(Profile* profile, bool force) {
    static Symbol saveStart;
    if (saveStart == Symbol()) {
        saveStart = Symbol("save_start");
    }
    static Symbol saveEnd;
    if (saveEnd == Symbol()) {
        saveEnd = Symbol("save_end");
    }
    if (!mAutosave && !force) {
        ExportStatus(saveEnd, profile->mPadNum, SaveLoad::kNoSaveNeeded);
        return;
    }
    if (mProfileClient.mProfile != nullptr && mProfileClient.IsBusy()) {
        for (const SaveProfile& pending : mPendingSaves) {
            if (pending.mProfile == profile) {
                return;
            }
        }
        SaveProfile pending;
        pending.mProfile = profile;
        pending.mForce = force;
        mPendingSaves.push_back(pending);
        return;
    }
    mProfileClient.mProfile = profile;
    ExecuteSave(force);
}

// Reconstructed from eboot.elf at 0x3AC830.
void SaveLoadMgr::ExportStatus(const Symbol& msg, int padNum, SaveLoad::Error error) {
    MsgParams params;
    params.mMsg = msg;
    params.mError = error;
    params.mPadNum = padNum;
    Hmx::CallMemberFunctionOnMain<SaveLoadMgr, MsgParams>(this, &SaveLoadMgr::ExportMsg, params);
}

// Reconstructed from eboot.elf at 0x3AC8E0.
void SaveLoadMgr::ExecuteSave(bool force) {
    static Symbol saveStart;
    if (saveStart == Symbol()) {
        saveStart = Symbol("save_start");
    }
    static Symbol saveEnd;
    if (saveEnd == Symbol()) {
        saveEnd = Symbol("save_end");
    }
    Profile* const profile = mProfileClient.mProfile;
    if (!profile->mReadyToSave) {
        ExportStatus(saveEnd, profile->mPadNum, SaveLoad::kNoSaveNeeded);
    } else {
        const SaveLoad::Error error = mProfileClient.Save(force);
        if (error == SaveLoad::kNoSaveNeeded || error == SaveLoad::kNoActiveProfile) {
            ExportStatus(saveEnd, profile->mPadNum, error);
        } else if (error == SaveLoad::kNoError) {
            ExportStatus(saveStart, profile->mPadNum, SaveLoad::kNoError);
        } else {
            ExportStatus(saveStart, profile->mPadNum, SaveLoad::kNoError);
            ExportStatus(saveEnd, profile->mPadNum, error);
        }
    }
    UpdateIsBusy();
}

// Reconstructed from eboot.elf at 0x3ACBA0. The wait spins on the flag the
// client's write result sets.
SaveLoad::Error SaveLoadMgr::SaveSync(Profile* profile, bool force) {
    if (!mAutosave && !force) {
        return SaveLoad::kNoSaveNeeded;
    }
    SaveSyncCallback callback;
    ProfileStorageClient client(&callback);
    client.mProfile = profile;
    SaveLoad::Error error = client.Save(force);
    if (error == SaveLoad::kNoError) {
        while (!callback.mSaveDone) {
        }
        error = callback.mSaveResult;
    }
    return error;
}

// Reconstructed from eboot.elf at 0x3ACCB0.
void SaveLoadMgr::UpdateIsBusy() {
    mIsBusy = mProfileClient.IsBusy() || mSysOptionsClient.IsBusy();
}

// Reconstructed from eboot.elf at 0x3ACCE0.
void SaveLoadMgr::SaveNewDevice(Profile* profile) {
    mPendingDevice = StorageRef::kAnyDevice;
    if (profile->mPadNum != -1) {
        theStorageMgr->PromptForDevice(
            profile->mPadNum, kProfileSaveSize, this, reinterpret_cast<void*>(kPromptSaveNewDevice));
    }
}

// Reconstructed from eboot.elf at 0x3ACD30.
void SaveLoadMgr::ConfirmSaveNewDevice(Profile* profile) {
    const int anyDevice = StorageRef::kAnyDevice;
    if (mPendingDevice != anyDevice) {
        mProfileClient.SetDevice(mPendingDevice);
        Save(profile, true);
        mPendingDevice = anyDevice;
    }
}

// Reconstructed from eboot.elf at 0x3ACD80.
void SaveLoadMgr::ConfirmOptionsOverwrite() {
    const int anyDevice = StorageRef::kAnyDevice;
    if (mPendingDevice != anyDevice) {
        mSysOptionsClient.SetDevice(mPendingDevice);
        mProfileMgr->GetSystemOptions()->MakeDirty();
        SaveSystemOptions(false);
        mPendingDevice = anyDevice;
    }
}

// Reconstructed from eboot.elf at 0x3ACDE0.
void SaveLoadMgr::SaveSystemOptions(bool force) {
    static Symbol saveoptStart;
    if (saveoptStart == Symbol()) {
        saveoptStart = Symbol("saveopt_start");
    }
    static Symbol saveoptEnd;
    if (saveoptEnd == Symbol()) {
        saveoptEnd = Symbol("saveopt_end");
    }
    const SaveLoad::Error error = mSysOptionsClient.Save(force);
    if (error != SaveLoad::kNoSaveNeeded) {
        if (error == SaveLoad::kNoError) {
            ExportStatus(saveoptStart, kNoPad, SaveLoad::kNoError);
        } else {
            ExportStatus(saveoptStart, kNoPad, SaveLoad::kNoError);
            ExportStatus(saveoptEnd, kNoPad, error);
        }
    }
    UpdateIsBusy();
}

// Reconstructed from eboot.elf at 0x3AD080.
void SaveLoadMgr::Load(Profile* profile, bool find) {
    static Symbol loadStart;
    if (loadStart == Symbol()) {
        loadStart = Symbol("load_start");
    }
    static Symbol loadEnd;
    if (loadEnd == Symbol()) {
        loadEnd = Symbol("load_end");
    }
    if (mProfileClient.mProfile != nullptr && mProfileClient.IsBusy()) {
        for (const LoadProfile& pending : mPendingLoads) {
            if (pending.mProfile == profile) {
                return;
            }
        }
        LoadProfile pending;
        pending.mProfile = profile;
        pending.mFind = find;
        mPendingLoads.push_back(pending);
        return;
    }
    mProfileClient.mProfile = profile;
    const SaveLoad::Error error = mProfileClient.Load(find);
    if (error != SaveLoad::kNoSaveNeeded) {
        if (error == SaveLoad::kNoError) {
            ExportStatus(loadStart, profile->mPadNum, SaveLoad::kNoError);
        } else {
            ExportStatus(loadStart, profile->mPadNum, SaveLoad::kNoError);
            ExportStatus(loadEnd, profile->mPadNum, error);
        }
    }
    UpdateIsBusy();
}

// Reconstructed from eboot.elf at 0x3AD470.
void SaveLoadMgr::RestoreFromLastSave(Profile* profile) {
    mProfileClient.mProfile = profile;
    mProfileClient.Restore();
}

// Reconstructed from eboot.elf at 0x3AD480.
void SaveLoadMgr::SaveSystemOptionsPrompt() {
    theStorageMgr->PromptForDevice(
        StorageRef::kAnyUser, kSystemOptionsSaveSize, this, reinterpret_cast<void*>(kPromptSaveSystemOptions));
}

// Reconstructed from eboot.elf at 0x3AD4B0.
void SaveLoadMgr::AutoloadSystemOptions() {
    static Symbol loadoptStart;
    if (loadoptStart == Symbol()) {
        loadoptStart = Symbol("loadopt_start");
    }
    static Symbol loadoptEnd;
    if (loadoptEnd == Symbol()) {
        loadoptEnd = Symbol("loadopt_end");
    }
    const SaveLoad::Error error = mSysOptionsClient.Load(true);
    if (error != SaveLoad::kNoSaveNeeded) {
        if (error == SaveLoad::kNoError) {
            ExportStatus(loadoptStart, kNoPad, SaveLoad::kNoError);
        } else {
            ExportStatus(loadoptStart, kNoPad, SaveLoad::kNoError);
            ExportStatus(loadoptEnd, kNoPad, error);
        }
    }
}

// Reconstructed from eboot.elf at 0x3AD700.
void SaveLoadMgr::LoadSystemOptionsPrompt() {
    theStorageMgr->PromptForDevice(StorageRef::kAnyUser, 0, this, reinterpret_cast<void*>(kPromptLoadSystemOptions));
}

// Reconstructed from eboot.elf at 0x3AD730.
void SaveLoadMgr::DeleteSystemOptions() {
    mSysOptionsClient.Delete();
}

// Reconstructed from eboot.elf at 0x3AD740.
void SaveLoadMgr::DeleteSave(Profile* profile) {
    static Symbol deleteStart;
    if (deleteStart == Symbol()) {
        deleteStart = Symbol("delete_start");
    }
    static Symbol deleteEnd;
    if (deleteEnd == Symbol()) {
        deleteEnd = Symbol("delete_end");
    }
    mProfileClient.mProfile = profile;
    const SaveLoad::Error error = mProfileClient.Delete();
    if (error != SaveLoad::kNoSaveNeeded) {
        if (error == SaveLoad::kNoError) {
            ExportStatus(deleteStart, profile->mPadNum, SaveLoad::kNoError);
        } else {
            ExportStatus(deleteStart, profile->mPadNum, SaveLoad::kNoError);
            ExportStatus(deleteEnd, profile->mPadNum, error);
        }
    }
    UpdateIsBusy();
}

// Reconstructed from eboot.elf at 0x3ADA30.
void SaveLoadMgr::ChooseDevice(Profile* profile) {
    if (profile->mPadNum != -1) {
        static Symbol choosedeviceStart;
        if (choosedeviceStart == Symbol()) {
            choosedeviceStart = Symbol("choosedevice_start");
        }
        {
            Message<0> msg(choosedeviceStart);
            Export(msg.mData);
        }
        mProfileClient.mProfile = profile;
        theStorageMgr->PromptForDevice(
            profile->mPadNum, kProfileSaveSize, this, reinterpret_cast<void*>(kPromptChooseDevice));
    }
}

// Reconstructed from eboot.elf at 0x3ADB40.
void SaveLoadMgr::ToggleAutosave() {
    mAutosave = !mAutosave;
}

// Reconstructed from eboot.elf at 0x3ADB50.
void SaveLoadMgr::ExportMsg(const MsgParams& params) {
    Message<2> msg(params.mMsg);
    msg.mData->Node(2) = DataNode(static_cast<int>(params.mError));
    msg.mData->Node(3) = DataNode(params.mPadNum);
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0x3ADD50. The other's existing save asks
// for a confirmation ("confirm_overwrite") before ConfirmSaveNewDevice or
// ConfirmOptionsOverwrite writes over it.
void SaveLoadMgr::OnDevicePromptResult(void* context, int result, int device) {
    static Symbol choosedeviceEnd;
    if (choosedeviceEnd == Symbol()) {
        choosedeviceEnd = Symbol("choosedevice_end");
    }
    if (result != StorageRef::kResultSuccess || device == -1) {
        ExportStatus(choosedeviceEnd, kNoPad, SaveLoad::kNoDevice);
        return;
    }
    switch (static_cast<DevicePrompt>(reinterpret_cast<long>(context))) {
    case kPromptChooseDevice:
        mProfileClient.SetDevice(device);
        ExportStatus(choosedeviceEnd, kNoPad, SaveLoad::kNoError);
        break;
    case kPromptSaveNewDevice: {
        Profile* const profile = mProfileClient.mProfile;
        StorageRef_PS4 store(profile->mStorage);
        if (!store.Exists()) {
            mProfileClient.SetDevice(device);
            Save(profile, true);
        } else {
            mPendingDevice = device;
            static Symbol confirmOverwrite;
            if (confirmOverwrite == Symbol()) {
                confirmOverwrite = Symbol("confirm_overwrite");
            }
            ExportStatus(confirmOverwrite, kNoPad, SaveLoad::kNoError);
        }
        break;
    }
    case kPromptSaveSystemOptions: {
        SystemOptions* const options = mProfileMgr->GetSystemOptions();
        StorageRef_PS4 store(options->mStorage);
        if (store.Exists()) {
            mPendingDevice = device;
            static Symbol confirmOverwrite;
            if (confirmOverwrite == Symbol()) {
                confirmOverwrite = Symbol("confirm_overwrite");
            }
            ExportStatus(confirmOverwrite, kNoPad, SaveLoad::kNoError);
        } else {
            mSysOptionsClient.SetDevice(device);
            options->MakeDirty();
            SaveSystemOptions(false);
        }
        break;
    }
    case kPromptLoadSystemOptions: {
        static Symbol loadoptStart;
        if (loadoptStart == Symbol()) {
            loadoptStart = Symbol("loadopt_start");
        }
        mSysOptionsClient.SetDevice(device);
        const SaveLoad::Error error = mSysOptionsClient.Load(false);
        if (error != SaveLoad::kNoError) {
            static Symbol loadoptEnd;
            if (loadoptEnd == Symbol()) {
                loadoptEnd = Symbol("loadopt_end");
            }
            ExportStatus(loadoptEnd, kNoPad, error);
        } else {
            ExportStatus(loadoptStart, kNoPad, SaveLoad::kNoError);
        }
        break;
    }
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x3AE260.
void SaveLoadMgr::OnProfileSaveResult(SaveLoad::Error error) {
    Profile* const profile = mProfileClient.mProfile;
    static Symbol saveEnd;
    if (saveEnd == Symbol()) {
        saveEnd = Symbol("save_end");
    }
    ExportStatus(saveEnd, profile != nullptr ? profile->mPadNum : -1, error);
}

// Reconstructed from eboot.elf at 0x3AE3B0.
void SaveLoadMgr::OnProfileLoadResult(SaveLoad::Error error) {
    Profile* const profile = mProfileClient.mProfile;
    static Symbol loadEnd;
    if (loadEnd == Symbol()) {
        loadEnd = Symbol("load_end");
    }
    ExportStatus(loadEnd, profile != nullptr ? profile->mPadNum : -1, error);
}

// Reconstructed from eboot.elf at 0x3AE500.
void SaveLoadMgr::OnProfileDeleteResult(SaveLoad::Error error) {
    Profile* const profile = mProfileClient.mProfile;
    static Symbol deleteEnd;
    if (deleteEnd == Symbol()) {
        deleteEnd = Symbol("delete_end");
    }
    ExportStatus(deleteEnd, profile != nullptr ? profile->mPadNum : -1, error);
}

// Reconstructed from eboot.elf at 0x3AE650.
void SaveLoadMgr::OnSysOptionsSaveResult(SaveLoad::Error error) {
    static Symbol saveoptEnd;
    if (saveoptEnd == Symbol()) {
        saveoptEnd = Symbol("saveopt_end");
    }
    ExportStatus(saveoptEnd, kNoPad, error);
}

// Reconstructed from eboot.elf at 0x3AE780.
void SaveLoadMgr::OnSysOptionsLoadResult(SaveLoad::Error error) {
    static Symbol loadoptEnd;
    if (loadoptEnd == Symbol()) {
        loadoptEnd = Symbol("loadopt_end");
    }
    ExportStatus(loadoptEnd, kNoPad, error);
}

// Reconstructed from eboot.elf at 0x3AE8B0.
void SaveLoadMgr::OnSysOptionsDeleteResult(SaveLoad::Error error) {
    static_cast<void>(error);
}

// Reconstructed from eboot.elf at 0x3AE8D0.
void SaveLoadMgr::DumpFiles(Profile* profile) {
    int user = profile->mPadNum;
    if (user == -1) {
        user = StorageRef::kAnyUser;
    }
    static FileDumper dumper;
    theStorageMgr->Find(user, &dumper, StorageRef::kAnyDevice, false, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x3AE960.
void SaveLoadMgr::Poll() {
    UpdateIsBusy();
    if (mIsBusy) {
        return;
    }
    if (!mPendingLoads.empty()) {
        const LoadProfile pending = mPendingLoads.back();
        mPendingLoads.pop_back();
        Load(pending.mProfile, pending.mFind);
    } else if (!mPendingSaves.empty()) {
        const SaveProfile pending = mPendingSaves.back();
        mPendingSaves.pop_back();
        Save(pending.mProfile, pending.mForce);
    }
}

// Reconstructed from eboot.elf at 0x3AEAD0. The removed profile arrives as
// its MsgSink; the binary evaluates the node through a copy of the map's
// DataNode::GetSink at 0xF9450.
DataNode SaveLoadMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onProfileAboutToBeRemoved;
    if (onProfileAboutToBeRemoved == Symbol()) {
        onProfileAboutToBeRemoved = Symbol("on_profile_about_to_be_removed");
    }
    if (type == onProfileAboutToBeRemoved) {
        MsgSink* const sink = static_cast<MsgSink*>(msg->Evaluate(2).LiteralSink(msg));
        _OnProfileAboutToBeRemoved(static_cast<Profile*>(sink));
        return DataNode(0);
    }
    static Symbol saveAll;
    if (saveAll == Symbol()) {
        saveAll = Symbol("save_all");
    }
    if (type == saveAll) {
        SaveAll(msg->Size() > 2 ? msg->Int(2) > 0 : false);
        return DataNode(0);
    }
    return DataNode();
}

// Inlined into Handle (0x3AEAD0).
void SaveLoadMgr::_OnProfileAboutToBeRemoved(Profile* profile) {
    mProfileClient.ClearProfile(profile);
    for (SaveProfile* pending = mPendingSaves.begin(); pending != mPendingSaves.end();) {
        if (pending->mProfile == profile) {
            pending = mPendingSaves.erase(pending);
        } else {
            ++pending;
        }
    }
    for (LoadProfile* pending = mPendingLoads.begin(); pending != mPendingLoads.end();) {
        if (pending->mProfile == profile) {
            pending = mPendingLoads.erase(pending);
        } else {
            ++pending;
        }
    }
}
