#include "startup.h"

namespace rb4 {

// These adapters stand in for member calls on global subsystem objects whose
// class layouts have not been reconstructed yet. Address comments tie each
// call back to the executable while keeping the frame schedule readable.
void system_update();                         // 0x369940
void sound_manager_update();                  // 0x7560
void optional_service_update();               // 0x8ECAC0
void dingo_update();                          // 0x33B510
void async_callback_queue_update();            // 0xD20C10
void somp_session_manager_update();            // 0xD2C440
void song_readiness_update();                  // 0xF2CE50
void platform_state_update();                  // 0xAE3880
void somp_ui_state_update();                   // 0xD1E2C0
void song_loading_update();                    // 0x8F3220
void profile_manager_update();                 // 0xD5C230
void ui_event_queue_update();                  // 0xADE120
void resource_manager_update();                // 0x92F4A0
void player_state_update();                    // 0xB0A070
void ui_manager_update();                      // 0x8C8900
void optional_voting_update();                 // 0xA47EC0
void ui_layout_controller_update();            // 0xBB58C0
void deferred_system_update();                 // 0x3AE960
void deferred_game_action_update();            // 0xC60720
void overshell_update();                       // 0xB0CE80
void network_connection_monitor_update();      // 0xBD9430
void resource_request_queue_update();          // 0x928FC0

void render_system_poll();                     // 0x3DE0E0
bool render_system_is_alive();
bool ui_layout_consume_skip_frame();           // 0x8B1840
void render_system_skip_frame();               // 0x3DEAA0
bool platform_frame_callback_pending();         // 0x43B130
void run_platform_frame_callback();             // 0x43B140
bool render_system_begin_frame();              // 0x3DE130
void ui_manager_render();                      // 0x8C9A80
void render_system_end_frame();                // 0x3DE7C0
bool exit_requested();

namespace {

void update_frame_subsystems() {
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

// Reconstructed from eboot.elf at 0x190.
bool game_run_frame() {
    update_frame_subsystems();

    render_system_poll();
    if (!render_system_is_alive()) {
        return false;
    }

    if (ui_layout_consume_skip_frame()) {
        render_system_skip_frame();
    } else {
        if (platform_frame_callback_pending()) {
            run_platform_frame_callback();
        }

        if (render_system_begin_frame()) {
            ui_manager_render();
            render_system_end_frame();
        }
    }

    return !exit_requested() && render_system_is_alive();
}

}  // namespace rb4
