#include "audio/fmod/input/fmod_audio_input_manager.h"

#include <algorithm>
#include <array>

namespace rb4 {

namespace {

constexpr std::size_t kRecordDriverNameCapacity = 256;
constexpr char kGeneralDriverSuffix[] = "GENERAL";
constexpr std::size_t kGeneralDriverSuffixLength =
    sizeof(kGeneralDriverSuffix) - 1;

struct RecordDriverInfo {
    std::array<char, kRecordDriverNameCapacity> name{};
    FMOD_GUID guid{};
    std::int32_t sample_rate = 0;
    FMOD_SPEAKERMODE speaker_mode = FMOD_SPEAKERMODE_DEFAULT;
    std::int32_t speaker_channels = 0;
    FMOD_DRIVER_STATE state = 0;
};

bool query_record_driver(
    FMOD::System& system,
    std::int32_t driver,
    RecordDriverInfo& info) {
    return system.getRecordDriverInfo(
               driver,
               info.name.data(),
               static_cast<std::int32_t>(info.name.size()),
               &info.guid,
               &info.sample_rate,
               &info.speaker_mode,
               &info.speaker_channels,
               &info.state) == FMOD_OK;
}

}  // namespace

// Reconstructed from eboot.elf at 0x27B3E0.
FmodAudioInputDevice::FmodAudioInputDevice(std::int32_t slot)
    : slot_(slot) {}

// Reconstructed from eboot.elf at 0x27B780.
bool FmodAudioInputDevice::bind_record_driver(
    FMOD::System& system,
    std::int32_t driver,
    const std::string& expected_name) {
    RecordDriverInfo info;
    if (!query_record_driver(system, driver, info) ||
        info.name.data() != expected_name) {
        return false;
    }

    driver_ = driver;
    name_ = expected_name;
    sample_rate_ = info.sample_rate;
    current_frequency_ = static_cast<float>(info.sample_rate);
    return true;
}

// Reconstructed from eboot.elf at 0x27B880.
bool FmodAudioInputDevice::refresh_record_driver(FMOD::System& system) {
    if (!is_active()) {
        return false;
    }

    std::int32_t driver_count = 0;
    std::int32_t connected_count = 0;
    system.getRecordNumDrivers(&driver_count, &connected_count);
    (void)connected_count;

    for (std::int32_t driver = 0; driver < driver_count; ++driver) {
        RecordDriverInfo info;
        if (!query_record_driver(system, driver, info) ||
            info.name.data() != name_) {
            continue;
        }
        if ((info.state & FMOD_DRIVER_STATE_CONNECTED) != 0) {
            return true;
        }
        break;
    }

    disconnect();
    return false;
}

void FmodAudioInputDevice::disconnect() {
    driver_ = -1;
    name_.clear();
}

bool FmodAudioInputDevice::is_active() const {
    return driver_ >= 0 && !name_.empty();
}

std::int32_t FmodAudioInputDevice::slot() const {
    return slot_;
}

std::int32_t FmodAudioInputDevice::driver() const {
    return driver_;
}

std::int32_t FmodAudioInputDevice::sample_rate() const {
    return sample_rate_;
}

const std::string& FmodAudioInputDevice::name() const {
    return name_;
}

FmodAudioInputManager::FmodAudioInputManager(
    FMOD::Studio::System& studio_system,
    FMOD::System& core_system)
    : studio_system_(studio_system), core_system_(core_system) {}

// Reconstructed from eboot.elf at 0x275C00.
bool FmodAudioInputManager::is_supported() const {
    return true;
}

// Reconstructed from eboot.elf at 0x2754C0.
bool FmodAudioInputManager::set_bus_paths(
    const std::vector<std::string>& paths) {
    buses_.clear();
    buses_.reserve(paths.size());
    for (const auto& path : paths) {
        buses_.push_back({path, nullptr, 1.0F});
    }
    return bind_buses();
}

// Reconstructed from eboot.elf at 0x275630.
bool FmodAudioInputManager::bind_buses() {
    for (auto& route : buses_) {
        if (route.path.empty() ||
            studio_system_.getBus(route.path.c_str(), &route.bus) != FMOD_OK ||
            route.bus == nullptr ||
            route.bus->getVolume(&route.authored_volume, nullptr) != FMOD_OK) {
            for (auto& rollback : buses_) {
                rollback.bus = nullptr;
            }
            bus_binding_failed_ = true;
            return false;
        }
    }

    bus_binding_failed_ = false;
    return true;
}

// Reconstructed from eboot.elf at 0x2758A0.
bool FmodAudioInputManager::retry_bus_bindings() {
    return !bus_binding_failed_ || bind_buses();
}

// Reconstructed from eboot.elf at 0x2758B0.
void FmodAudioInputManager::clear_bus_paths() {
    buses_.clear();
    bus_binding_failed_ = false;
}

// Reconstructed from eboot.elf at 0x275740.
void FmodAudioInputManager::set_bus_volume(
    std::int32_t index,
    float gain) {
    if (index < 0 || static_cast<std::size_t>(index) >= buses_.size()) {
        return;
    }
    auto& route = buses_[index];
    if (route.bus == nullptr) {
        return;
    }
    route.bus->setVolume(gain * route.authored_volume);
}

// Reconstructed from eboot.elf at 0x275780.
void FmodAudioInputManager::set_bus_mute(
    std::int32_t index,
    bool mute) {
    if (index < 0 || static_cast<std::size_t>(index) >= buses_.size()) {
        return;
    }
    auto* bus = buses_[index].bus;
    if (bus != nullptr) {
        bus->setMute(mute);
    }
}

// Reconstructed from eboot.elf at 0x2757C0.
FMOD::ChannelGroup* FmodAudioInputManager::bus_channel_group(
    std::int32_t index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= buses_.size()) {
        return nullptr;
    }

    auto* bus = buses_[index].bus;
    if (bus == nullptr) {
        return nullptr;
    }

    FMOD::ChannelGroup* channel_group = nullptr;
    if (bus->getChannelGroup(&channel_group) != FMOD_OK) {
        return nullptr;
    }
    return channel_group;
}

// Reconstructed from eboot.elf at 0x275830.
void FmodAudioInputManager::initialize_device_slots(std::int32_t count) {
    devices_.clear();
    if (count <= 0) {
        return;
    }

    devices_.reserve(count);
    for (std::int32_t slot = 0; slot < count; ++slot) {
        devices_.emplace_back(slot);
    }
}

// Reconstructed from eboot.elf at 0x275950.
bool FmodAudioInputManager::refresh_record_devices() {
    bool changed = false;
    for (auto& device : devices_) {
        if (device.is_active() && !device.refresh_record_driver(core_system_)) {
            changed = true;
        }
    }

    std::int32_t driver_count = 0;
    std::int32_t connected_count = 0;
    core_system_.getRecordNumDrivers(&driver_count, &connected_count);
    (void)connected_count;

    for (std::int32_t driver = 0; driver < driver_count; ++driver) {
        RecordDriverInfo info;
        if (!query_record_driver(core_system_, driver, info) ||
            (info.state & FMOD_DRIVER_STATE_CONNECTED) == 0 ||
            !is_general_record_driver(info.name.data()) ||
            has_active_device(info.name.data())) {
            continue;
        }

        auto* device = first_available_device();
        if (device != nullptr &&
            device->bind_record_driver(core_system_, driver, info.name.data())) {
            changed = true;
        }
    }
    return changed;
}

// Reconstructed from eboot.elf at 0x275910.
bool FmodAudioInputManager::is_general_record_driver(
    const std::string& name) {
    return name.size() >= kGeneralDriverSuffixLength &&
        name.compare(
            name.size() - kGeneralDriverSuffixLength,
            kGeneralDriverSuffixLength,
            kGeneralDriverSuffix) == 0;
}

bool FmodAudioInputManager::has_active_device(const std::string& name) const {
    return std::any_of(
        devices_.begin(), devices_.end(), [name](const auto& device) {
            return device.is_active() && device.name() == name;
        });
}

FmodAudioInputDevice* FmodAudioInputManager::first_available_device() {
    const auto device = std::find_if(
        devices_.begin(), devices_.end(), [](const auto& candidate) {
            return !candidate.is_active();
        });
    return device == devices_.end() ? nullptr : &*device;
}

}  // namespace rb4
