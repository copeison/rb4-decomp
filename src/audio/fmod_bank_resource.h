#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "fmod_api.h"

namespace rb4 {

class FmodBankResource {
public:
    explicit FmodBankResource(std::string asset_path);
    ~FmodBankResource();

    bool load(
        const std::vector<FMOD::Studio::System*>& studio_systems,
        std::string resolved_path);
    void unload_all();

    std::vector<std::string> event_paths() const;
    std::vector<std::string> bus_paths() const;
    bool is_unavailable() const;

    const std::string& asset_path() const;
    const std::string& resolved_path() const;

    static std::string resolve_platform_path(std::string path);
    static std::string strings_bank_path(std::string_view master_bank_path);
    static std::string master_bank_path(std::string_view strings_bank_path);
    static std::string_view category_name();
    static std::string_view supported_extension();

private:
    bool load_into_system(FMOD::Studio::System& system);

    std::string asset_path_;
    std::string resolved_path_;
    std::unordered_map<FMOD::Studio::System*, FMOD::Studio::Bank*> banks_;
};

}  // namespace rb4
