#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "audio/fmod/api/fmod_api.h"

namespace rb4 {

class FmodAudioInputDevice {
public:
    explicit FmodAudioInputDevice(std::int32_t slot);

    bool bind_record_driver(
        FMOD::System& system,
        std::int32_t driver,
        const std::string& expected_name);
    bool refresh_record_driver(FMOD::System& system);
    void disconnect();

    bool is_active() const;
    std::int32_t slot() const;
    std::int32_t driver() const;
    std::int32_t sample_rate() const;
    const std::string& name() const;

private:
    std::int32_t slot_ = -1;
    std::int32_t driver_ = -1;
    std::string name_;
    std::int32_t sample_rate_ = 48'000;
    float current_frequency_ = 48'000.0F;
};

class FmodAudioInputManager {
public:
    FmodAudioInputManager(
        FMOD::Studio::System& studio_system,
        FMOD::System& core_system);

    bool is_supported() const;
    bool set_bus_paths(const std::vector<std::string>& paths);
    bool retry_bus_bindings();
    void clear_bus_paths();

    void set_bus_volume(std::int32_t index, float gain);
    void set_bus_mute(std::int32_t index, bool mute);
    FMOD::ChannelGroup* bus_channel_group(std::int32_t index) const;

    void initialize_device_slots(std::int32_t count);
    bool refresh_record_devices();

    static bool is_general_record_driver(const std::string& name);

private:
    struct BusRoute {
        std::string path;
        FMOD::Studio::Bus* bus = nullptr;
        float authored_volume = 1.0F;
    };

    bool bind_buses();
    bool has_active_device(const std::string& name) const;
    FmodAudioInputDevice* first_available_device();

    FMOD::Studio::System& studio_system_;
    FMOD::System& core_system_;
    std::vector<BusRoute> buses_;
    std::vector<FmodAudioInputDevice> devices_;
    bool bus_binding_failed_ = false;
};

}  // namespace rb4
