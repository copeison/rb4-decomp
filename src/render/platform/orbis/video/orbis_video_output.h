#pragma once

#include <cstddef>
#include <cstdint>

class PS4Device;

namespace rb4 {

enum class OrbisSubmitEventType {
    kFlipComplete,
    kEndOfPipe,
};

struct OrbisSubmitEvent {
    OrbisSubmitEventType type;
};

void orbis_video_output_open(PS4Device& system);
void orbis_video_output_set_flip_rate(
    PS4Device& system,
    std::uint32_t rate);
void orbis_video_output_set_window_margins(
    PS4Device& system,
    std::uint32_t top,
    std::uint32_t bottom);
void orbis_create_event_queue(
    PS4Device& system,
    const char* name);
void orbis_register_gnm_event(
    PS4Device& system,
    std::uint32_t event_id);
void orbis_register_video_flip_event(PS4Device& system);
void orbis_unregister_gnm_event(
    PS4Device& system,
    std::uint32_t event_id);
void orbis_delete_event_queue(PS4Device& system);
void orbis_video_output_close(PS4Device& system);
void orbis_hide_system_splash_screen();
bool orbis_wait_for_submit_events(
    PS4Device& system,
    OrbisSubmitEvent* events,
    std::size_t capacity,
    std::size_t& event_count);
void orbis_process_flip_complete(PS4Device& system);
void orbis_create_render_context(PS4Device& system);
void orbis_wait_for_submit_thread(PS4Device& system);
std::int32_t orbis_submit_done_thread_entry(void* context);
void orbis_submit_done_thread_run(PS4Device& system);

}  // namespace rb4
