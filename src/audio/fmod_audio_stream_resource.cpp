#include "fmod_audio_stream_resource.h"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace rb4 {

namespace {

std::recursive_mutex g_stream_resource_mutex;
std::unordered_map<std::string, FmodAudioStreamResource*>
    g_stream_resources;

}  // namespace

// Reconstructed from eboot.elf at 0x271660.
FmodAudioStreamResource::FmodAudioStreamResource(std::string asset_path)
    : asset_path_(std::move(asset_path)) {}

// Reconstructed from eboot.elf at 0x271750.
FmodAudioStreamResource::~FmodAudioStreamResource() {
    unregister_resource();
}

// Reconstructed from eboot.elf at 0x272340.
bool FmodAudioStreamResource::load(
    FmodAudioState& audio_state,
    std::string resolved_path,
    bool path_exists) {
    unregister_resource();
    resolved_path_ = std::move(resolved_path);
    status_ = FmodAudioStreamResourceStatus::ready;

    if (!path_exists) {
        resolved_path_.clear();
        status_ = FmodAudioStreamResourceStatus::file_not_found;
        return false;
    }
    if (audio_state.core_system == nullptr) {
        resolved_path_.clear();
        status_ = FmodAudioStreamResourceStatus::audio_system_unavailable;
        return false;
    }

    FMOD::Sound* probe = nullptr;
    audio_state.core_system->createSound(
        resolved_path_.c_str(), FMOD_CREATESTREAM, nullptr, &probe);
    if (probe == nullptr) {
        resolved_path_.clear();
        status_ = FmodAudioStreamResourceStatus::audio_system_unavailable;
        return false;
    }

    FMOD_SOUND_TYPE type{};
    FMOD_SOUND_FORMAT format{};
    std::int32_t channels = 0;
    std::int32_t bits = 0;
    probe->getFormat(&type, &format, &channels, &bits);
    probe->release();

    if (format != FMOD_SOUND_FORMAT_PCM16 ||
        (channels != 1 && channels != 2) || bits != 16) {
        status_ = FmodAudioStreamResourceStatus::unsupported_format;
    }
    register_resource();
    return status_ == FmodAudioStreamResourceStatus::ready;
}

// Reconstructed from eboot.elf at 0x272DF0.
bool FmodAudioStreamResource::is_unavailable() const {
    return resolved_path_.empty();
}

const std::string& FmodAudioStreamResource::asset_path() const {
    return asset_path_;
}

const std::string& FmodAudioStreamResource::resolved_path() const {
    return resolved_path_;
}

FmodAudioStreamResourceStatus FmodAudioStreamResource::status() const {
    return status_;
}

bool FmodAudioStreamResource::decode_failed() const {
    return decode_failed_;
}

void FmodAudioStreamResource::mark_decode_failed() {
    decode_failed_ = true;
}

// Reconstructed from eboot.elf at 0x271C20.
FmodAudioStreamResource* FmodAudioStreamResource::find(
    std::string_view resolved_path) {
    std::lock_guard lock(g_stream_resource_mutex);
    const auto found = g_stream_resources.find(std::string(resolved_path));
    return found != g_stream_resources.end() ? found->second : nullptr;
}

// Reconstructed from eboot.elf at 0x271D00.
const std::array<std::string_view, 5>&
FmodAudioStreamResource::supported_extensions() {
    static constexpr std::array<std::string_view, 5> extensions = {
        "mp3", "wav", "aac", "ogg", "m4a"};
    return extensions;
}

std::string_view FmodAudioStreamResource::category_name() {
    return "Streaming Audio";
}

// Reconstructed from eboot.elf at 0x271B20.
void FmodAudioStreamResource::register_resource() {
    std::lock_guard lock(g_stream_resource_mutex);
    g_stream_resources[resolved_path_] = this;
    registered_ = true;
}

// Reconstructed from eboot.elf at 0x271830.
void FmodAudioStreamResource::unregister_resource() {
    if (!registered_) {
        return;
    }

    std::lock_guard lock(g_stream_resource_mutex);
    const auto found = g_stream_resources.find(resolved_path_);
    if (found != g_stream_resources.end() && found->second == this) {
        g_stream_resources.erase(found);
    }
    registered_ = false;
}

}  // namespace rb4
