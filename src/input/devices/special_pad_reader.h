#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace rb4 {

constexpr std::size_t kCalibrationSampleCapacity = 64;

enum class SpecialPadHardware : std::uint32_t {
    kUnknown = 0,
    kVendor0738Product8261 = 0x1F,
    kVendor0E6FProduct0173 = 0x20,
};

enum class CalibrationSensorMode : std::uint32_t {
    kDisabled = 0,
    kMode1 = 1,
    kMode2 = 2,
};

struct CalibrationSamples {
    std::array<float, kCalibrationSampleCapacity> values{};
    std::uint32_t count = 0;
};

// Semantic state used by the cleaned reconstruction. The original class also
// contains its base pad-reader state before these fields.
struct SpecialPadReaderState {
    std::int32_t user_id = -1;
    CalibrationSensorMode calibration_mode = CalibrationSensorMode::kDisabled;
    SpecialPadHardware hardware = SpecialPadHardware::kUnknown;
    CalibrationSamples samples;
};

void special_pad_reader_open(SpecialPadReaderState& reader, std::int32_t user_id);

bool special_pad_reader_set_calibration_mode(
    SpecialPadReaderState& reader,
    CalibrationSensorMode mode);

void special_pad_reader_append_calibration_samples(
    SpecialPadReaderState& reader,
    const void* pad_data,
    std::int32_t count);

bool special_pad_reader_take_calibration_samples(
    SpecialPadReaderState& reader,
    CalibrationSamples& output);

}  // namespace rb4
