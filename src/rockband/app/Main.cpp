#include "rockband/app/Main.h"

#include <cstdint>
#include <limits>

#include "os/files/File.h"
#include "render/debug/screenshot_capture.h"
#include "render/system/RndDevice.h"
#include "render/system/RndInit.h"
#include "ui/layout/UILayoutId.h"
#include "utl/options/Option.h"

// The callees below are not identified yet; they stand in for free functions
// and member calls on global subsystem objects whose classes have not been
// reconstructed. Names not in the reference map.
struct DingoService;
struct UILayoutController;

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
    UILayoutController& controller,
    UILayoutId id,
    bool force_reload);

extern DingoService g_dingo_service;
extern UILayoutController g_ui_layout_controller;

void system_update();                         // 0x369940
void sound_manager_update();                  // 0x7560
void optional_service_update();               // 0x8ECAC0
void dingo_update();                          // 0x33B510
void async_callback_queue_update();           // 0xD20C10
void somp_session_manager_update();           // 0xD2C440
void song_readiness_update();                 // 0xF2CE50
void platform_state_update();                 // 0xAE3880
void somp_ui_state_update();                  // 0xD1E2C0
void song_loading_update();                   // 0x8F3220
void profile_manager_update();                // 0xD5C230
void ui_event_queue_update();                 // 0xADE120
void resource_manager_update();               // 0x92F4A0
void player_state_update();                   // 0xB0A070
void ui_manager_update();                     // 0x8C8900
void optional_voting_update();                // 0xA47EC0
void ui_layout_controller_update();           // 0xBB58C0
void deferred_system_update();                // 0x3AE960
void deferred_game_action_update();           // 0xC60720
void overshell_update();                      // 0xB0CE80
void network_connection_monitor_update();     // 0xBD9430
void resource_request_queue_update();         // 0x928FC0

bool ui_layout_consume_skip_frame();          // 0x8B1840
void ui_manager_render();                     // 0x8C9A80
bool exit_requested();

namespace {

// Inlined into RunOneFrame. Name not in the reference map.
void UpdateFrameSubsystems() {
    system_update();
    sound_manager_update();
    optional_service_update();
    dingo_update();
    async_callback_queue_update();
    somp_session_manager_update();
    song_readiness_update();
    platform_state_update();
    somp_ui_state_update();
    song_loading_update();
    profile_manager_update();
    ui_event_queue_update();
    resource_manager_update();
    player_state_update();
    ui_manager_update();
    optional_voting_update();
    ui_layout_controller_update();
    deferred_system_update();
    deferred_game_action_update();
    overshell_update();
    network_connection_monitor_update();
    resource_request_queue_update();
}

}  // namespace

// Reconstructed from eboot.elf at 0xA0.
bool App::Initialize(int argc, char** argv) {
    (void)argc;
    (void)argv;

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

    RndInitParams options{};
    options.mUnknown0 = true;
    options.mInitRendering = true;
    options.mUnknown2 = true;
    Rnd::Init(options);

    ui_register_types();
    dingo_initialize(g_dingo_service);
    audio_configure_time_stretch();
    input_refresh_player_assignments();

    if (gResourcePrecacheMode) {
        runtime_terminate(0);
    }

    ui_load_layout_by_id(g_ui_layout_controller, kLayoutGameStartup, false);
    OptionCheck(gOptionArgs);
    return true;
}

// Reconstructed from eboot.elf at 0x190.
bool App::RunOneFrame() {
    UpdateFrameSubsystems();

    if (TheRndDevice() != nullptr) {
        TheRndDevice()->PollMainWindow();
    }
    if (TheRndDevice() == nullptr) {
        return false;
    }

    if (ui_layout_consume_skip_frame()) {
        TheRndDevice()->ForceIncrementFrameCount();
    } else {
        if (rb4::screenshot_capture_pending()) {
            rb4::screenshot_capture_current_frame();
        }

        if (TheRndDevice()->BeginMainWindowFrame()) {
            ui_manager_render();
            TheRndDevice()->EndMainWindowFrame();
        }
    }

    return !exit_requested() && TheRndDevice() != nullptr;
}

// Reconstructed from eboot.elf at 0x3C0. The start routine passes a third,
// null argument that main does not read.
int main(int argc, char** argv) {
    App::Run(argc, argv);
    return 0;
}
