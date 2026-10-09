# Main frame loop

`App::RunOneFrame` at `0x190` updates the runtime and game subsystems in a fixed
order, then either skips or renders one frame. Its boolean return value controls
the loop in `main`.

## Update order

| Order | Address | Name | Evidence |
| ---: | --- | --- | --- |
| 1 | `0x369940` | `SystemPoll` | Fans out to core runtime, input assignment, platform, timing, and file-system updates. |
| 2 | `0x7560` | `SoundManager::Poll` | Polls the default and joypad emitter entities, every generator manager, the default render target, the FMOD platform and the mics (`docs/sound-manager.md`). |
| 3 | `0x8ECAC0` | unresolved optional service | Invokes three virtual update methods when its global object exists. |
| 4 | `0x33B510` | `dingo_update` | Dispatches the Dingo backend's per-frame virtual method. |
| 5 | `0xD20C10` | `async_callback_queue_update` | Drains queued asynchronous callbacks on the current thread. |
| 6 | `0xD2C440` | `somp_session_manager_update` | Handles SOMP jobs, invitations, session transitions, and connection errors. |
| 7-9 | `0xF2CE50`, `0xAE3880`, `0xD1E2C0` | partially classified | Updates readiness, platform state, and SOMP UI state. |
| 10 | `0x8F3220` | `song_loading_update` | Advances song loading and reports missing song data. |
| 11 | `0xD5C230` | `RBProfileMgr::Poll` | Updates profile state against current UI state. |
| 12 | `0xADE120` | UI event queue update | Dispatches pending UI work. |
| 13 | `0x92F4A0` | `resource_manager_update` | Resolves pending resources when their load state changes. |
| 14 | `0xB0A070` | player-state update | Updates player/UI state; class identity is still unknown. |
| 15 | `0x8C8900` | `UIMgr::Poll` | Advances layout transitions and emits transition events. |
| 16 | `0xA47EC0` | optional voting update | Applies pending state on an optional object in the voting/UI region. |
| 17 | `0xBB58C0` | `ui_layout_controller_update` | Updates the active layout and shell audio resources. |
| 18-20 | `0x3AE960`, `0xC60720`, `0xB0CE80` | partially classified | Processes deferred system/game work and overshell state. |
| 21 | `0xBD9430` | `network_connection_monitor_update` | Raises the lost-connection UI error when online service drops. |
| 22 | `0x928FC0` | resource request queue update | Final resource-related work before rendering. |

`SystemPoll`, `SoundManager::Poll`, `RBProfileMgr::Poll` and `UIMgr::Poll`
are the reference map's names, matched by behaviour, strings and the map's
globals (`theSoundManager`, `theRBProfileMgr`, `theUI`). The other names are
descriptive placeholders. Their call order and behavior are confirmed, while the owning
class names are still unknown.

## Render decision

After updates, the render system polls its frame owner. The loop stops
immediately if that poll destroys the global render system. When `UIMgr::GetCurrentLayout` returns a layout, it
can consume a pending skip-frame counter; this records a skipped render frame.
Otherwise the game services an optional platform callback, begins a render
frame, submits the active UI layout, and ends the frame.

The function requests another iteration only while the exit flag is clear and
the render system still exists. The cleaned control flow is in
`src/rockband/app/Main.cpp`; the direct decompilation remains in
`analysis/exports/game-run-frame.c`.
