#include "special_pad_reader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace rb4 {

void pad_reader_open_special(
    SpecialPadReaderState& reader,
    std::int32_t user_id);
void input_state_lock();
void input_state_unlock();

namespace {

constexpr std::int32_t kSpecialPadPortType = 2;
constexpr std::uint32_t kCalibrationFeatureReportId = 0x30;

#pragma pack(push, 1)
struct SpecialPadOpenParameters {
    std::uint16_t vendor_id;
    std::uint16_t product_id;
    std::uint16_t product_id_copy;
};

struct CalibrationFeatureReport {
    std::uint8_t report_id;
    std::uint8_t command;
    std::uint8_t parameter;
    std::uint8_t reserved;
    std::uint8_t mode;
};
#pragma pack(pop)

struct ExtendedControllerInformation {
    float touchpad_density;
    std::uint16_t touchpad_width;
    std::uint16_t touchpad_height;
    std::uint8_t left_stick_deadzone;
    std::uint8_t right_stick_deadzone;
    std::uint8_t connection_type;
    std::uint8_t connected_count;
    std::int32_t connected;
    std::int32_t device_class;
    std::array<std::uint8_t, 8> unknown;
};

struct PadData {
    std::array<std::uint8_t, 107> prefix;
    std::uint8_t device_unique_data_length;
    std::array<std::uint8_t, 12> device_unique_data;
};

static_assert(sizeof(SpecialPadOpenParameters) == 6);
static_assert(sizeof(CalibrationFeatureReport) == 5);
static_assert(offsetof(ExtendedControllerInformation, connected_count) == 11);
static_assert(sizeof(PadData) == 0x78);
static_assert(sizeof(CalibrationSamples) == 0x104);

extern "C" {
int scePadOpenExt(
    std::int32_t user_id,
    std::int32_t port_type,
    std::int32_t index,
    const SpecialPadOpenParameters* parameters);
int scePadGetExtControllerInformation(
    std::int32_t handle,
    ExtendedControllerInformation* information);
int scePadSetFeatureReport(
    std::int32_t handle,
    std::uint32_t report_id,
    const void* data,
    std::uint32_t size);
int scePadClose(std::int32_t handle);
int usleep(std::uint32_t microseconds);
}

class InputStateGuard {
public:
    InputStateGuard() { input_state_lock(); }
    ~InputStateGuard() { input_state_unlock(); }

    InputStateGuard(const InputStateGuard&) = delete;
    InputStateGuard& operator=(const InputStateGuard&) = delete;
};

constexpr SpecialPadOpenParameters kHardwareVariant1{
    0x0738,
    0x8261,
    0x8261,
};

constexpr SpecialPadOpenParameters kHardwareVariant2{
    0x0E6F,
    0x0173,
    0x0173,
};

bool is_hardware_connected(
    std::int32_t user_id,
    const SpecialPadOpenParameters& parameters) {
    const auto handle =
        scePadOpenExt(user_id, kSpecialPadPortType, 0, &parameters);

    bool connected = false;
    if (handle >= 0) {
        ExtendedControllerInformation information{};
        scePadGetExtControllerInformation(handle, &information);
        connected = information.connected_count != 0;
    }

    // The original closes the returned value even when opening failed.
    scePadClose(handle);
    return connected;
}

std::uint8_t feature_mode_value(CalibrationSensorMode mode) {
    switch (mode) {
        case CalibrationSensorMode::kDisabled:
            return 0x01;
        case CalibrationSensorMode::kMode1:
            return 0xFF;
        case CalibrationSensorMode::kMode2:
            return 0x43;
    }

    return 0x01;
}

float read_calibration_sample(
    CalibrationSensorMode mode,
    const PadData& data) {
    if (mode == CalibrationSensorMode::kMode1) {
        return static_cast<float>(data.device_unique_data[9]);
    }

    std::uint16_t raw_sample = 0;
    std::memcpy(&raw_sample, data.device_unique_data.data() + 7, sizeof(raw_sample));
    return std::sqrt(static_cast<float>(raw_sample));
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D28A0.
void special_pad_reader_open(
    SpecialPadReaderState& reader,
    std::int32_t user_id) {
    pad_reader_open_special(reader, user_id);
    reader.user_id = user_id;

    if (is_hardware_connected(user_id, kHardwareVariant1)) {
        reader.hardware = SpecialPadHardware::kVendor0738Product8261;
    } else if (is_hardware_connected(user_id, kHardwareVariant2)) {
        reader.hardware = SpecialPadHardware::kVendor0E6FProduct0173;
    }
}

// Reconstructed from eboot.elf at 0x8D3060.
bool special_pad_reader_set_calibration_mode(
    SpecialPadReaderState& reader,
    CalibrationSensorMode mode) {
    const InputStateGuard lock;

    reader.calibration_mode = mode;
    reader.samples.count = 0;

    const auto& parameters =
        reader.hardware == SpecialPadHardware::kVendor0E6FProduct0173
            ? kHardwareVariant2
            : kHardwareVariant1;
    const auto handle =
        scePadOpenExt(reader.user_id, kSpecialPadPortType, 0, &parameters);
    if (handle < 0) {
        return false;
    }

    const CalibrationFeatureReport report{
        0x30,
        0x01,
        0x08,
        0x00,
        feature_mode_value(mode),
    };

    auto result = scePadSetFeatureReport(
        handle, kCalibrationFeatureReportId, &report, sizeof(report));
    if (result != 0) {
        usleep(1000);
        result = scePadSetFeatureReport(
            handle, kCalibrationFeatureReportId, &report, sizeof(report));
    }

    scePadClose(handle);
    return result == 0;
}

// Reconstructed from eboot.elf at 0x8D2EA0.
void special_pad_reader_append_calibration_samples(
    SpecialPadReaderState& reader,
    const void* pad_data,
    std::int32_t count) {
    if (reader.calibration_mode == CalibrationSensorMode::kDisabled || count <= 0) {
        return;
    }

    const auto* records = static_cast<const PadData*>(pad_data);
    for (std::int32_t index = 0; index < count; ++index) {
        if (reader.samples.count >= kCalibrationSampleCapacity) {
            std::move(
                reader.samples.values.begin() + 1,
                reader.samples.values.end(),
                reader.samples.values.begin());
            reader.samples.count = kCalibrationSampleCapacity - 1;
        }

        reader.samples.values[reader.samples.count++] =
            read_calibration_sample(reader.calibration_mode, records[index]);
    }
}

// Reconstructed from eboot.elf at 0x8D3000.
bool special_pad_reader_take_calibration_samples(
    SpecialPadReaderState& reader,
    CalibrationSamples& output) {
    const InputStateGuard lock;

    output = reader.samples;
    reader.samples.count = 0;
    return true;
}

}  // namespace rb4
