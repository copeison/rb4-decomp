#include "mic/fmod/MicHwManager_FMOD.h"

#include <cstring>

#include "audio/fmod/system/FmodPlatform.h"
#include "mic/core/MicHwManager.h"
#include "mic/fmod/Mic_FMOD.h"
#include "utl/containers/Std.h"

MicHwManager_FMOD gMicHwManagerFMOD;

namespace {

// Mic type reported by FMOD microphones. Name not in the reference map.
constexpr int kMicTypeFmod = 1;
constexpr char kGeneralSuffix[] = "GENERAL";
constexpr std::size_t kGeneralSuffixLength = sizeof(kGeneralSuffix) - 1;

// Grows the route vector to hold `count` routes; new routes start unbound at
// unity volume. Reconstructed from the EASTL resize at 0x275C10.
void ResizeRoutes(MicHwManager_FMOD::BusRouteList& routes, long count) {
    const long size = routes.mEnd - routes.mBegin;
    if (count <= size) {
        routes.mEnd = routes.mBegin + count;
        return;
    }
    if (count > routes.mCapacity - routes.mBegin) {
        const long grown = size != 0 ? size * 2 : 1;
        const long capacity = grown > count ? grown : count;
        auto* storage = static_cast<MicHwManager_FMOD::BusRoute*>(
            HmxAllocator::gStlAllocator.allocate(capacity * sizeof(MicHwManager_FMOD::BusRoute)));
        std::memmove(storage, routes.mBegin, size * sizeof(MicHwManager_FMOD::BusRoute));
        if (routes.mBegin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                routes.mBegin,
                (routes.mCapacity - routes.mBegin) * sizeof(MicHwManager_FMOD::BusRoute));
        }
        routes.mBegin = storage;
        routes.mEnd = storage + size;
        routes.mCapacity = storage + capacity;
    }
    for (auto* route = routes.mEnd; route != routes.mBegin + count; ++route) {
        route->mPath = Symbol("");
        route->mBus = nullptr;
        route->mAuthoredVolume = 1.0F;
    }
    routes.mEnd = routes.mBegin + count;
}

}  // namespace

MicHwManager_FMOD::MicHwManager_FMOD()
    : mUnknown8(false), mRoutes(), mBindFailed(false) {}

// Reconstructed from eboot.elf at 0x275490.
MicHwManager_FMOD::~MicHwManager_FMOD() {
    if (mRoutes.mBegin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            mRoutes.mBegin, (mRoutes.mCapacity - mRoutes.mBegin) * sizeof(BusRoute));
    }
}

// Reconstructed from eboot.elf at 0x275C00.
bool MicHwManager_FMOD::_Init() {
    return true;
}

// Inlined into SetBusPaths and ClearBusPaths at 0x2754C0 and 0x2758B0.
void MicHwManager_FMOD::_ReleaseRoutes() {
    for (auto* route = mRoutes.mBegin; route != mRoutes.mEnd; ++route) {
        gFmodBusInterface->IsBusKnown(route->mPath);
    }
    mRoutes.mEnd = mRoutes.mBegin;
}

// Reconstructed from eboot.elf at 0x2754C0.
void MicHwManager_FMOD::SetBusPaths(const PathList& paths) {
    _ReleaseRoutes();
    const long count = paths.mEnd - paths.mBegin;
    ResizeRoutes(mRoutes, count);
    for (long index = 0; index < count; ++index) {
        mRoutes.mBegin[index].mPath = paths.mBegin[index];
    }
    _BindBuses();
}

// Reconstructed from eboot.elf at 0x275630.
void MicHwManager_FMOD::_BindBuses() {
    const long count = mRoutes.mEnd - mRoutes.mBegin;
    bool failed = false;
    if (count > 0) {
        FMOD::Studio::System* studio = FModSystem::Get()->mStudioSystem;
        long bound = 0;
        for (; bound < count; ++bound) {
            BusRoute& route = mRoutes.mBegin[bound];
            if (studio->getBus(route.mPath.Str(), &route.mBus) != FMOD_OK ||
                !gFmodBusInterface->IsBusLoaded(route.mPath)) {
                break;
            }
            route.mBus->getVolume(&route.mAuthoredVolume, nullptr);
        }
        if (bound < count) {
            failed = true;
            for (long index = 0; index < bound; ++index) {
                gFmodBusInterface->IsBusKnown(mRoutes.mBegin[index].mPath);
            }
        }
    }
    mBindFailed = failed;
}

// Reconstructed from eboot.elf at 0x275740.
void MicHwManager_FMOD::SetBusVolume(int index, float volume) {
    if (index >= 0 && index < mRoutes.mEnd - mRoutes.mBegin) {
        BusRoute& route = mRoutes.mBegin[index];
        route.mBus->setVolume(volume * route.mAuthoredVolume);
    }
}

// Reconstructed from eboot.elf at 0x275780.
void MicHwManager_FMOD::SetBusMute(int index, bool mute) {
    if (index >= 0 && index < mRoutes.mEnd - mRoutes.mBegin) {
        mRoutes.mBegin[index].mBus->setMute(mute);
    }
}

// Reconstructed from eboot.elf at 0x2757C0.
FMOD::ChannelGroup* MicHwManager_FMOD::GetBusChannelGroup(int index) {
    if (index < 0 || index >= mRoutes.mEnd - mRoutes.mBegin) {
        return nullptr;
    }
    FMOD::ChannelGroup* channelGroup = nullptr;
    if (mRoutes.mBegin[index].mBus->getChannelGroup(&channelGroup) != FMOD_OK) {
        return nullptr;
    }
    return channelGroup;
}

// Reconstructed from eboot.elf at 0x275830.
void MicHwManager_FMOD::_SetupMicArray(int numMics) {
    gMicHwManager.ClearMics();
    for (int index = 0; index < numMics; ++index) {
        gMicHwManager.AddMic(new Mic_FMOD(index));
    }
}

// Reconstructed from eboot.elf at 0x2758A0.
void MicHwManager_FMOD::_Poll() {
    if (mBindFailed) {
        _BindBuses();
    }
}

// Reconstructed from eboot.elf at 0x2758B0.
void MicHwManager_FMOD::ClearBusPaths() {
    _ReleaseRoutes();
}

// Reconstructed from eboot.elf at 0x275910.
bool MicHwManager_FMOD::_IsValidMicName(const char* name) {
    const std::size_t length = std::strlen(name);
    return length >= kGeneralSuffixLength &&
        std::strncmp(name + length - kGeneralSuffixLength, kGeneralSuffix, kGeneralSuffixLength) == 0;
}

// Reconstructed from eboot.elf at 0x275950. Bound mics whose driver vanished
// are released, and each connected GENERAL driver without a mic is attached
// to a free one.
void MicHwManager_FMOD::_CheckConnectsAndDisconnects() {
    for (Mic** mic = gMicHwManager.mMicsBegin; mic != gMicHwManager.mMicsEnd; ++mic) {
        if ((*mic)->GetType() != kMicTypeFmod) {
            continue;
        }
        auto* fmodMic = static_cast<Mic_FMOD*>(*mic);
        if (fmodMic->mDriverName != Symbol("") && !fmodMic->CheckDeviceStillConnected()) {
            gMicHwManager.MarkMicsChanged();
        }
    }

    FMOD::System* lowLevel = FModSystem::Get()->mLowLevelSystem;
    int numDrivers = 0;
    int numConnected = 0;
    lowLevel->getRecordNumDrivers(&numDrivers, &numConnected);
    for (int driver = 0; driver < numDrivers; ++driver) {
        char name[256];
        FMOD_GUID guid;
        int sampleRate = 0;
        FMOD_SPEAKERMODE speakerMode;
        int channels = 0;
        FMOD_DRIVER_STATE state = 0;
        if (lowLevel->getRecordDriverInfo(
                driver, name, sizeof(name), &guid, &sampleRate, &speakerMode, &channels,
                &state) != FMOD_OK ||
            (state & FMOD_DRIVER_STATE_CONNECTED) == 0 || !_IsValidMicName(name)) {
            continue;
        }
        bool attached = false;
        for (Mic** mic = gMicHwManager.mMicsBegin; mic != gMicHwManager.mMicsEnd; ++mic) {
            if ((*mic)->GetType() == kMicTypeFmod &&
                std::strcmp(static_cast<Mic_FMOD*>(*mic)->mDriverName.Str(), name) == 0) {
                attached = true;
                break;
            }
        }
        if (attached) {
            continue;
        }
        auto* mic = static_cast<Mic_FMOD*>(gMicHwManager.GetFreeMic(kMicTypeFmod));
        if (mic != nullptr && mic->AttachToHardware(driver, Symbol(name))) {
            gMicHwManager.MarkMicsChanged();
        }
    }
}
