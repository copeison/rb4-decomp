#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "fmod_audio_system.h"

namespace rb4 {

enum class FmodAudioStreamResourceStatus : std::int32_t {
    ready = 0,
    file_not_found = 1,
    audio_system_unavailable = 2,
    unsupported_format = 3,
};

class FmodAudioStreamResource {
public:
    explicit FmodAudioStreamResource(std::string asset_path);
    ~FmodAudioStreamResource();

    bool load(
        FmodAudioState& audio_state,
        std::string resolved_path,
        bool path_exists);
    bool is_unavailable() const;

    const std::string& asset_path() const;
    const std::string& resolved_path() const;
    FmodAudioStreamResourceStatus status() const;

    static FmodAudioStreamResource* find(std::string_view resolved_path);
    static const std::array<std::string_view, 5>& supported_extensions();
    static std::string_view category_name();

private:
    void register_resource();
    void unregister_resource();

    std::string asset_path_;
    std::string resolved_path_;
    FmodAudioStreamResourceStatus status_ =
        FmodAudioStreamResourceStatus::ready;
    bool registered_ = false;
};

}  // namespace rb4
