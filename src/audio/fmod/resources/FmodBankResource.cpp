#include "audio/fmod/resources/FmodBankResource.h"

#include <cstring>

#include "audio/core/system/SoundManager.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"

// The directory part of a path, written to the buffer and returned; "." when
// the path has none. At 0x245300; declared here until utl/FilePath declares
// it.
char* FileGetPath(const char* path, char* buffer);

CritSec FModBankResource::sLoadedBanksCritSec;
eastl::list<FModBankResource*> FModBankResource::sLoadedBanks;
ResourcePtr<FModBankResource> FModBankResource::sMasterBank;
ResourcePtr<FModBankResource> FModBankResource::sMasterStringsBank;
ResourceMetaData FModBankResource::sMetaData;
bool FModBankResource::sLoadingMasterStringsBank;
bool FModBankResource::sLoadingMasterBank;

namespace {

// Lists of up to 2,048 events or buses, and 512-byte paths.
constexpr int kMaxBankListSize = 2048;
constexpr int kMaxPathLength = 512;
// Length of "desktop" and of "eng.bank".
constexpr unsigned long kDesktopLength = 7;
constexpr unsigned long kEnglishBankLength = 8;
// FMOD_STUDIO_LOAD_BANK_NORMAL.
constexpr unsigned int kLoadBankNormal = 0;

// Whether the lower-cased path ends with the suffix. Each test works on a
// copy of the path.
bool PathEndsWith(const String& path, const char* suffix) {
    String lowered(path);
    return lowered.ToLower().endswith(suffix);
}

bool IsMasterBank(const String& path) {
    return PathEndsWith(path, "master bank.bank");
}

bool IsMasterStringsBank(const String& path) {
    return PathEndsWith(path, "master bank.strings.bank");
}

// Loads the bank at the path as a resource.
ResourcePtr<FModBankResource> LoadBank(const String& path) {
    return Resource::GetOrLoad<FModBankResource>(ResourcePath(path.c_str()), false);
}

}  // namespace

// Reconstructed from eboot.elf at 0x273740. Every bank joins the global list.
FModBankResource::FModBankResource() : mBankData(nullptr), mLocalized(false) {
    sLoadedBanksCritSec.Enter();
    sLoadedBanks.push_back(this);
    sLoadedBanksCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x2738A0.
FModBankResource::~FModBankResource() {
    sLoadedBanksCritSec.Enter();
    for (auto it = sLoadedBanks.begin(); it != sLoadedBanks.end();) {
        if (*it == this) {
            it = sLoadedBanks.erase(it);
        } else {
            ++it;
        }
    }
    sLoadedBanksCritSec.Exit();
    _UnloadAll();
}

// Reconstructed from eboot.elf at 0x273980. Each Studio system is updated
// until its bank has finished unloading.
void FModBankResource::_UnloadAll() {
    while (mBanks.size() != 0) {
        const auto first = mBanks.begin();
        FMOD::Studio::System* system = first->first;
        FMOD::Studio::Bank* bank = first->second;
        mBanks.erase(first);
        bank->unload();
        FMOD_STUDIO_LOADING_STATE state;
        FMOD_RESULT result = bank->getLoadingState(&state);
        while (result == FMOD_OK && state == FMOD_STUDIO_LOADING_STATE_UNLOADING) {
            scePthreadYield();
            result = bank->getLoadingState(&state);
            system->update();
        }
    }
    if (mBankData != nullptr) {
        MemFree(mBankData);
        mBankData = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x273AB0.
eastl::list<FModBankResource*>& FModBankResource::LockLoadedBanks() {
    sLoadedBanksCritSec.Enter();
    return sLoadedBanks;
}

// Reconstructed from eboot.elf at 0x273AD0.
void FModBankResource::UnlockLoadedBanks() {
    sLoadedBanksCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x273AF0.
void FModBankResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("bank"));
    metaData.mCategory = Symbol("FMod Banks");
    metaData.mTypeFlags[6] = true;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x273C10. The list stops at the first path
// FMOD cannot report.
eastl::vector<Symbol> FModBankResource::GetEvents() const {
    eastl::vector<Symbol> paths;
    if (mBanks.size() == 0) {
        return paths;
    }
    FMOD::Studio::Bank* bank = static_cast<const decltype(mBanks)::node_type*>(
        mBanks.mAnchor.mpNodeLeft)->mValue.second;
    int eventCount;
    if (bank->getEventCount(&eventCount) != FMOD_OK) {
        return paths;
    }
    FMOD::Studio::EventDescription* events[kMaxBankListSize];
    int count;
    bank->getEventList(events, kMaxBankListSize, &count);
    paths.reserve(count);
    for (int index = 0; index < count; ++index) {
        char path[kMaxPathLength];
        int retrieved;
        if (events[index]->getPath(path, kMaxPathLength, &retrieved) != FMOD_OK) {
            break;
        }
        paths.push_back(Symbol(path));
    }
    return paths;
}

// Reconstructed from eboot.elf at 0x273F00.
eastl::vector<Symbol> FModBankResource::GetBuses() const {
    eastl::vector<Symbol> paths;
    if (mBanks.size() == 0) {
        return paths;
    }
    FMOD::Studio::Bank* bank = static_cast<const decltype(mBanks)::node_type*>(
        mBanks.mAnchor.mpNodeLeft)->mValue.second;
    int busCount;
    if (bank->getBusCount(&busCount) != FMOD_OK) {
        return paths;
    }
    FMOD::Studio::Bus* buses[kMaxBankListSize];
    int count;
    bank->getBusList(buses, kMaxBankListSize, &count);
    paths.reserve(count);
    for (int index = 0; index < count; ++index) {
        char path[kMaxPathLength];
        int retrieved;
        if (buses[index]->getPath(path, kMaxPathLength, &retrieved) != FMOD_OK) {
            break;
        }
        paths.push_back(Symbol(path));
    }
    return paths;
}

// Reconstructed from eboot.elf at 0x2741F0. Banks are never read from a
// stream.
bool FModBankResource::Load(BinStream&, bool) {
    return false;
}

// Reconstructed from eboot.elf at 0x274200. Nothing is loaded while the
// runtime builds its precache. A first load of an ordinary bank makes sure
// the localized master bank is loaded; loading the master bank or its
// strings bank drops both references and then loads the companion.
void FModBankResource::_Load(bool reload) {
    if (gResourcePrecacheMode) {
        return;
    }
    String path;
    path.reserve(kMaxPathLength);
    _ResolvePlatformPath(path);

    if (!reload) {
        if (!IsMasterBank(path) && !IsMasterStringsBank(path) &&
            (!sMasterBank || sMasterBank->Fail())) {
            LoadBank(_GetLanguageMasterBankPath(path));
        }
        if ((IsMasterBank(path) && !sLoadingMasterBank) ||
            (IsMasterStringsBank(path) && !sLoadingMasterStringsBank)) {
            sMasterBank = nullptr;
            sMasterStringsBank = nullptr;
        }
    }

    eastl::vector<FMOD::Studio::System*> systems;
    FModSystem::GetAllStudioSystems(systems);
    for (FMOD::Studio::System* system : systems) {
        _LoadIntoSystem(path, system);
    }

    if (reload) {
        return;
    }
    if (IsMasterBank(path)) {
        sMasterBank = this;
        if (!sLoadingMasterBank) {
            sLoadingMasterStringsBank = true;
            const String bankPath(mPath.mPath);
            sMasterStringsBank = LoadBank(_GetStringsBankPath(bankPath));
            sLoadingMasterStringsBank = false;
        }
    } else if (IsMasterStringsBank(path)) {
        sMasterStringsBank = this;
        if (!sLoadingMasterStringsBank) {
            sLoadingMasterBank = true;
            const String bankPath(mPath.mPath);
            sMasterBank = LoadBank(_GetMasterBankPath(bankPath));
            sLoadingMasterBank = false;
        }
    }
}

// Reconstructed from eboot.elf at 0x274710. The binary appends the
// language through TextStream's Symbol insertion (0x258900), which utl does
// not declare yet; the Symbol's text is appended instead.
void FModBankResource::_ResolvePlatformPath(String& path) {
    path = GetUncachedResourcePath(mPath);
    unsigned long desktop = path.find("desktop");
    if (desktop == FixedString::npos) {
        desktop = path.find("Desktop");
    }
    if (desktop != FixedString::npos) {
        path.replace(desktop, kDesktopLength, "PS4");
    }
    if (path.endswith("_eng.bank") || path.endswith("/eng.bank")) {
        mLocalized = true;
        path.resize(std::strlen(path.c_str()) - kEnglishBankLength);
        path << theSoundManager.GetLanguage().Str() << ".bank";
    }
}

// Reconstructed from eboot.elf at 0x274920.
String FModBankResource::_GetLanguageMasterBankPath(const String& bankPath) {
    String path;
    path.reserve(kMaxPathLength);
    char directory[kMaxPathLength] = {};
    path = FileGetPath(bankPath.c_str(), directory);
    path += "/master bank.bank";
    return path;
}

// Reconstructed from eboot.elf at 0x274A80. A failed load unloads every
// copy. The sample data is loaded synchronously, and every bus's channel
// group is locked so the buses exist before playback.
void FModBankResource::_LoadIntoSystem(const String& path, FMOD::Studio::System* system) {
    FMOD::Studio::Bank* bank;
    if (system->loadBankFile(path.c_str(), kLoadBankNormal, &bank) != FMOD_OK) {
        _UnloadAll();
        return;
    }
    bank->loadSampleData();
    FMOD_STUDIO_LOADING_STATE state;
    FMOD_RESULT result;
    do {
        scePthreadYield();
        result = bank->getSampleLoadingState(&state);
        system->update();
    } while (result == FMOD_OK && state == FMOD_STUDIO_LOADING_STATE_LOADING);

    FMOD::Studio::Bus* buses[kMaxBankListSize];
    int count = 0;
    if (bank->getBusList(buses, kMaxBankListSize, &count) == FMOD_OK && count > 0) {
        for (int index = 0; index < count; ++index) {
            buses[index]->lockChannelGroup();
        }
        system->flushCommands();
    }
    mBanks[system] = bank;
}

// Reconstructed from eboot.elf at 0x274C40.
String FModBankResource::_GetStringsBankPath(const String& bankPath) {
    String path;
    path.reserve(kMaxPathLength);
    path = bankPath.c_str();
    path.erase(path.find_last_of('.'));
    path += ".strings.bank";
    return path;
}

// Reconstructed from eboot.elf at 0x274CF0. The capitalization follows the
// strings bank's own path.
String FModBankResource::_GetMasterBankPath(const String& bankPath) {
    String path;
    path.reserve(kMaxPathLength);
    char directory[kMaxPathLength] = {};
    path = FileGetPath(bankPath.c_str(), directory);
    path += bankPath.contains("Master") ? "/Master Bank.bank" : "/master bank.bank";
    return path;
}

// Reconstructed from eboot.elf at 0x274E70.
void FModBankResource::Save(BinStream&, bool) {}

// Reconstructed from eboot.elf at 0x274F20.
eastl::vector<String> FModBankResource::GetAllBankPaths() {
    eastl::vector<String> paths;
    sLoadedBanksCritSec.Enter();
    for (FModBankResource* bank : sLoadedBanks) {
        paths.emplace_back(String(bank->mPath.mPath));
    }
    sLoadedBanksCritSec.Exit();
    return paths;
}

// Reconstructed from eboot.elf at 0x275030.
ResourceMetaData* FModBankResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x275040.
Symbol FModBankResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x2750E0.
bool FModBankResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x275110.
void FModBankResource::LoadFile() {
    _Load(false);
}

// Reconstructed from eboot.elf at 0x275120.
bool FModBankResource::Fail() const {
    return mBanks.size() == 0;
}
