#include "render/core/system/render_system_frame.h"

#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x3DE0E0.
void render_system_poll() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    render_system_lock(*system);
    render_system_enter_locked_call(*system);

    auto* frame_owner = render_system_frame_owner(*system);
    if (frame_owner != nullptr) {
        render_frame_owner_poll(*frame_owner);
    }

    render_system_leave_locked_call(*system);
    render_system_unlock(*system);
}

bool render_system_is_alive() {
    return render_system_instance() != nullptr;
}

// Reconstructed from eboot.elf at 0x3DE130.
bool render_system_begin_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return false;
    }

    render_system_prepare_frame(*system, false);
    auto* frame_owner = render_system_active_frame_owner(*system);
    if (frame_owner != nullptr &&
        render_system_attach_frame_owner(*system, *frame_owner)) {
        return true;
    }

    render_system_finish_frame(*system, false);
    return false;
}

// Reconstructed from eboot.elf at 0x3DE7C0.
void render_system_end_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    render_system_clear_active_frame(*system);
    render_system_finish_frame(*system, false);
}

// Reconstructed from eboot.elf at 0x3DEAA0.
void render_system_skip_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    render_system_lock(*system);
    render_system_increment_skipped_frame_count(*system);
    render_system_unlock(*system);
}

}  // namespace rb4
