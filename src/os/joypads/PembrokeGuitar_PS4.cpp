#include "os/joypads/PembrokeGuitar_PS4.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <pad.h>

#include "os/threading/CritSec.h"

// Returns the lock that guards controller state shared with the pad-reading
// thread. Name not in the reference map.
CritSec* JoypadControllerCritSec();  // 0x8CF3A0

namespace {

// Special-controller pad API entry points missing from the SDK headers.
// Names come from the binary's import table.
#pragma pack(push, 1)
struct SpecialPadOpenParam {
    std::uint16_t mVendorId;
    std::uint16_t mProductId;
    std::uint16_t mProductIdCopy;
};

struct CalbertFeatureReport {
    std::uint8_t mReportId;
    std::uint8_t mCommand;
    std::uint8_t mParameter;
    std::uint8_t mReserved;
    std::uint8_t mMode;
};
#pragma pack(pop)

struct ExtControllerInformation {
    float mTouchpadDensity;
    std::uint16_t mTouchpadWidth;
    std::uint16_t mTouchpadHeight;
    std::uint8_t mLeftStickDeadzone;
    std::uint8_t mRightStickDeadzone;
    std::uint8_t mConnectionType;
    std::uint8_t mConnectedCount;
    std::int32_t mConnected;
    std::int32_t mDeviceClass;
    std::uint8_t mUnknown[8];
};

static_assert(sizeof(SpecialPadOpenParam) == 6);
static_assert(sizeof(CalbertFeatureReport) == 5);
static_assert(offsetof(ExtControllerInformation, mConnectedCount) == 11);
static_assert(sizeof(ScePadData) == 0x78);
static_assert(offsetof(ScePadData, deviceUniqueData) == 0x6C);

extern "C" {
int scePadOpenExt(
    std::int32_t userId,
    std::int32_t type,
    std::int32_t index,
    const SpecialPadOpenParam* param);
int scePadGetExtControllerInformation(
    std::int32_t handle,
    ExtControllerInformation* info);
int scePadSetFeatureReport(
    std::int32_t handle,
    std::uint32_t reportId,
    const void* data,
    std::uint32_t size);
int usleep(std::uint32_t microseconds);
}

constexpr std::int32_t kSpecialPadPortType = 2;
constexpr std::uint32_t kCalbertFeatureReportId = 0x30;

constexpr SpecialPadOpenParam kMadCatzGuitar{0x0738, 0x8261, 0x8261};
constexpr SpecialPadOpenParam kPdpGuitar{0x0E6F, 0x0173, 0x0173};

bool IsGuitarConnected(std::int32_t userId, const SpecialPadOpenParam& param) {
    const int handle =
        scePadOpenExt(userId, kSpecialPadPortType, 0, &param);

    bool connected = false;
    if (handle >= 0) {
        ExtControllerInformation info{};
        scePadGetExtControllerInformation(handle, &info);
        connected = info.mConnectedCount != 0;
    }

    // The original closes the returned value even when opening failed.
    scePadClose(handle);
    return connected;
}

std::uint8_t CalbertFeatureMode(JoypadCalbertMode mode) {
    switch (mode) {
        case kJoypadCalbertDisabled:
            return 0x01;
        case kJoypadCalbertMode1:
            return 0xFF;
        case kJoypadCalbertMode2:
            return 0x43;
    }

    return 0x01;
}

float ReadCalbertSample(JoypadCalbertMode mode, const ScePadData& data) {
    if (mode == kJoypadCalbertMode1) {
        return static_cast<float>(data.deviceUniqueData[9]);
    }

    std::uint16_t rawSample = 0;
    std::memcpy(&rawSample, data.deviceUniqueData + 7, sizeof(rawSample));
    return std::sqrt(static_cast<float>(rawSample));
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D28A0.
void PembrokeGuitarController::Activate(int platformUserId) {
    DualShock4Controller::Activate(platformUserId, true);

    if (IsGuitarConnected(mPlatformUserId, kMadCatzGuitar)) {
        mType = kJoypadPembrokeGuitarMadCatz;
    } else if (IsGuitarConnected(mPlatformUserId, kPdpGuitar)) {
        mType = kJoypadPembrokeGuitarPdp;
    }
}

// Reconstructed from eboot.elf at 0x8D3060.
bool PembrokeGuitarController::SetCalbertMode(JoypadCalbertMode mode) {
    ScopedCritSecPtr lock(JoypadControllerCritSec());

    mCalbertMode = mode;
    mUnknown2008 = 0;
    mCalbertValues.mNumValues = 0;

    const SpecialPadOpenParam& param =
        mType == kJoypadPembrokeGuitarPdp ? kPdpGuitar : kMadCatzGuitar;
    const int handle =
        scePadOpenExt(mPlatformUserId, kSpecialPadPortType, 0, &param);
    if (handle < 0) {
        return false;
    }

    const CalbertFeatureReport report{
        0x30,
        0x01,
        0x08,
        0x00,
        CalbertFeatureMode(mode),
    };

    int result = scePadSetFeatureReport(
        handle, kCalbertFeatureReportId, &report, sizeof(report));
    if (result != 0) {
        usleep(1000);
        result = scePadSetFeatureReport(
            handle, kCalbertFeatureReportId, &report, sizeof(report));
    }

    scePadClose(handle);
    return result == 0;
}

// Reconstructed from eboot.elf at 0x8D2EA0.
void PembrokeGuitarController::_ProcessScePadData(ScePadData* data, int count) {
    if (mCalbertMode == kJoypadCalbertDisabled || count <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        if (mCalbertValues.mNumValues >= CalbertValues::kMaxValues) {
            std::memmove(
                mCalbertValues.mValues,
                mCalbertValues.mValues + 1,
                sizeof(float) * (CalbertValues::kMaxValues - 1));
            mCalbertValues.mNumValues = CalbertValues::kMaxValues - 1;
        }

        mCalbertValues.mValues[mCalbertValues.mNumValues++] =
            ReadCalbertSample(mCalbertMode, data[i]);
    }
}

// Reconstructed from eboot.elf at 0x8D3000.
bool PembrokeGuitarController::GetCalbertValues(CalbertValues& values) {
    ScopedCritSecPtr lock(JoypadControllerCritSec());

    values = mCalbertValues;
    mCalbertValues.mNumValues = 0;
    return true;
}
