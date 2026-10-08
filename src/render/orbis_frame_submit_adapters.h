#pragma once

namespace rb4 {

struct OrbisRenderObject;
struct OrbisRenderSystem;

void orbis_lock_submission(OrbisRenderSystem& system);
void orbis_unlock_submission(OrbisRenderSystem& system);
void orbis_submit_scope_begin(OrbisRenderSystem& system);
void orbis_submit_scope_end(OrbisRenderSystem& system);
bool orbis_submit_token_available(const OrbisRenderSystem& system);
void orbis_wait_for_submit_token(OrbisRenderSystem& system);
void orbis_consume_submit_token(OrbisRenderSystem& system);
bool orbis_frame_is_active(const OrbisRenderSystem& system);
void orbis_flush_active_frame(OrbisRenderSystem& system);
void orbis_submit_primary_frame_owner(OrbisRenderSystem& system);
void orbis_finalize_render_object(OrbisRenderObject& object);

}  // namespace rb4
