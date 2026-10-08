#include "game/startup/startup.h"

#include <cstdint>
#include <limits>

#include "game/systems/systems.h"
#include "core/command_line/command_line.h"
#include "core/resources/resource_mode.h"
#include "ui/layout/ui_layout_id.h"

namespace rb4 {

struct DingoService;
struct UiLayoutController;

void core_initialize();
void ui_register_layout_ids();
void stage_presence_register_ids();
void system_config_initialize(const char* config_path);
void sound_manager_initialize(
    void* config,
    std::uint32_t device_index,
    bool option2,
    void* option3,
    void* option4);
void engine_register_types();
void animation_register_types();
void physics_register_types();
void game_audio_register_types();
void ui_register_types();
void dingo_initialize(DingoService& service);
void audio_configure_time_stretch();
void input_refresh_player_assignments();
[[noreturn]] void runtime_terminate(int status);
void ui_load_layout_by_id(
    UiLayoutController& controller,
    UiLayoutId id,
    bool force_reload);

extern DingoService g_dingo_service;
extern UiLayoutController g_ui_layout_controller;

// Reconstructed from eboot.elf at 0xA0.
bool game_initialize() {
    core_initialize();
    ui_register_layout_ids();
    stage_presence_register_ids();
    system_config_initialize("config/rockband.dta");

    sound_manager_initialize(
        nullptr,
        std::numeric_limits<std::uint32_t>::max(),
        false,
        nullptr,
        nullptr);

    engine_register_types();
    animation_register_types();
    physics_register_types();
    game_audio_register_types();

    GameSystemInitOptions options{};
    options.option0 = true;
    options.initialize_rendering = true;
    options.option2 = true;
    game_systems_initialize(options);

    ui_register_types();
    dingo_initialize(g_dingo_service);
    audio_configure_time_stretch();
    input_refresh_player_assignments();

    if (g_resource_precache_mode) {
        runtime_terminate(0);
    }

    ui_load_layout_by_id(
        g_ui_layout_controller, UiLayoutId::kLayoutGameStartup, false);
    command_line_mark_switches_handled(arguments);
    return true;
}

}  // namespace rb4
