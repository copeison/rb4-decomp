#include "audio/fmod/resources/fmod_bank_resource.h"

#include <algorithm>
#include <array>
#include <thread>
#include <utility>

namespace rb4 {

namespace {

constexpr std::size_t kMaximumBankObjects = 2048;
constexpr std::size_t kMaximumBankPath = 512;

}  // namespace

// Reconstructed from eboot.elf at 0x273740.
FmodBankResource::FmodBankResource(std::string asset_path)
    : asset_path_(std::move(asset_path)) {}

// Reconstructed from eboot.elf at 0x2738A0.
FmodBankResource::~FmodBankResource() {
    unload_all();
}

// Reconstructed from eboot.elf at 0x274200 and 0x274A80.
bool FmodBankResource::load(
    const std::vector<FMOD::Studio::System*>& studio_systems,
    std::string resolved_path) {
    unload_all();
    resolved_path_ = resolve_platform_path(std::move(resolved_path));
    if (resolved_path_.empty()) {
        return false;
    }

    bool loaded = false;
    for (auto* system : studio_systems) {
        if (system != nullptr) {
            loaded |= load_into_system(*system);
        }
    }
    return loaded;
}

bool FmodBankResource::load_into_system(FMOD::Studio::System& system) {
    FMOD::Studio::Bank* bank = nullptr;
    if (system.loadBankFile(resolved_path_.c_str(), 0, &bank) != FMOD_OK ||
        bank == nullptr) {
        unload_all();
        return false;
    }

    bank->loadSampleData();
    FMOD_STUDIO_LOADING_STATE state = FMOD_STUDIO_LOADING_STATE_LOADING;
    while (bank->getSampleLoadingState(&state) == FMOD_OK &&
           state == FMOD_STUDIO_LOADING_STATE_LOADING) {
        std::this_thread::yield();
        system.update();
    }

    std::array<FMOD::Studio::Bus*, kMaximumBankObjects> buses{};
    std::int32_t bus_count = 0;
    if (bank->getBusList(
            buses.data(),
            static_cast<std::int32_t>(buses.size()),
            &bus_count) == FMOD_OK) {
        for (std::int32_t index = 0; index < bus_count; ++index) {
            buses[index]->lockChannelGroup();
        }
        if (bus_count > 0) {
            system.flushCommands();
        }
    }

    banks_[&system] = bank;
    return true;
}

// Reconstructed from eboot.elf at 0x273980.
void FmodBankResource::unload_all() {
    for (const auto& [system, bank] : banks_) {
        bank->unload();
        FMOD_STUDIO_LOADING_STATE state = FMOD_STUDIO_LOADING_STATE_UNLOADING;
        while (bank->getLoadingState(&state) == FMOD_OK &&
               state == FMOD_STUDIO_LOADING_STATE_UNLOADING) {
            std::this_thread::yield();
            system->update();
        }
    }
    banks_.clear();
}

// Reconstructed from eboot.elf at 0x273C10.
std::vector<std::string> FmodBankResource::event_paths() const {
    std::vector<std::string> paths;
    if (banks_.empty()) {
        return paths;
    }

    auto* bank = banks_.begin()->second;
    std::int32_t event_count = 0;
    if (bank->getEventCount(&event_count) != FMOD_OK || event_count <= 0) {
        return paths;
    }

    std::array<FMOD::Studio::EventDescription*, kMaximumBankObjects> events{};
    event_count = std::min(
        event_count, static_cast<std::int32_t>(events.size()));
    if (bank->getEventList(
            events.data(), event_count, &event_count) != FMOD_OK) {
        return {};
    }

    std::array<char, kMaximumBankPath> path{};
    paths.reserve(event_count);
    for (std::int32_t index = 0; index < event_count; ++index) {
        std::int32_t retrieved = 0;
        if (events[index]->getPath(
                path.data(),
                static_cast<std::int32_t>(path.size()),
                &retrieved) != FMOD_OK) {
            return {};
        }
        paths.emplace_back(path.data());
    }
    return paths;
}

// Reconstructed from eboot.elf at 0x273F00.
std::vector<std::string> FmodBankResource::bus_paths() const {
    std::vector<std::string> paths;
    if (banks_.empty()) {
        return paths;
    }

    auto* bank = banks_.begin()->second;
    std::int32_t bus_count = 0;
    if (bank->getBusCount(&bus_count) != FMOD_OK || bus_count <= 0) {
        return paths;
    }

    std::array<FMOD::Studio::Bus*, kMaximumBankObjects> buses{};
    bus_count = std::min(
        bus_count, static_cast<std::int32_t>(buses.size()));
    if (bank->getBusList(buses.data(), bus_count, &bus_count) != FMOD_OK) {
        return {};
    }

    std::array<char, kMaximumBankPath> path{};
    paths.reserve(bus_count);
    for (std::int32_t index = 0; index < bus_count; ++index) {
        std::int32_t retrieved = 0;
        if (buses[index]->getPath(
                path.data(),
                static_cast<std::int32_t>(path.size()),
                &retrieved) != FMOD_OK) {
            return {};
        }
        paths.emplace_back(path.data());
    }
    return paths;
}

// Reconstructed from eboot.elf at 0x275120.
bool FmodBankResource::is_unavailable() const {
    return banks_.empty();
}

const std::string& FmodBankResource::asset_path() const {
    return asset_path_;
}

const std::string& FmodBankResource::resolved_path() const {
    return resolved_path_;
}

// Reconstructed from eboot.elf at 0x274710.
std::string FmodBankResource::resolve_platform_path(std::string path) {
    for (const auto* desktop : {"desktop", "Desktop"}) {
        const auto position = path.find(desktop);
        if (position != std::string::npos) {
            path.replace(position, 7, "PS4");
            break;
        }
    }
    return path;
}

// Reconstructed from eboot.elf at 0x274C40.
std::string FmodBankResource::strings_bank_path(
    const std::string& master_bank_path) {
    const auto extension = master_bank_path.rfind('.');
    const auto base = extension == std::string::npos
        ? master_bank_path
        : master_bank_path.substr(0, extension);
    return base + ".strings.bank";
}

// Reconstructed from eboot.elf at 0x274CF0.
std::string FmodBankResource::master_bank_path(
    const std::string& strings_bank_path) {
    const auto suffix = strings_bank_path.rfind(".strings.bank");
    if (suffix == std::string::npos) {
        return strings_bank_path;
    }
    return strings_bank_path.substr(0, suffix) + ".bank";
}

const char* FmodBankResource::category_name() {
    return "FMod Banks";
}

const char* FmodBankResource::supported_extension() {
    return "bank";
}

}  // namespace rb4
