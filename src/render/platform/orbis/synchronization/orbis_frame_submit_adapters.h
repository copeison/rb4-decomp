#pragma once

namespace rb4 {

struct OrbisRenderContext;
struct OrbisRenderSystem;

void orbis_lock_submission(OrbisRenderSystem& system);
void orbis_unlock_submission(OrbisRenderSystem& system);
void orbis_submit_scope_begin(OrbisRenderSystem& system);
void orbis_submit_scope_end(OrbisRenderSystem& system);
void orbis_flush_active_frame(OrbisRenderSystem& system);

}  // namespace rb4
