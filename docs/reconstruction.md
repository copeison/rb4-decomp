# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0xA0` | `game_initialize` | `src/game/startup/initialize.cpp` | Complete top-level initialization order and arguments recovered; two option-field meanings remain unknown. |
| `0x190` | `game_run_frame` | `src/game/frame/frame.cpp` | Update order and render/skip/exit control flow recovered; several owning class names remain unknown. |
| `0x3C0` | `game_main` | `src/game/application/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x402C30`, `0x402D30` | game-system renderer lifecycle | `src/game/systems/systems.cpp` | Orbis renderer creation, platform warmup, shared and default resource initialization, nine dependent initializers, cleanup registration, guarded shutdown, virtual deletion, and singleton clearing recovered. |
| `0x997590` | `stage_presence_id_to_symbol` | `src/game/stage/stage_presence.cpp` | All 22 enum values and their interned symbol strings recovered. |
| `0xBB06A0`, `0xBB5D40` | `ui_layout_id_to_symbol`, layout asset map | `src/ui/layout/ui_layout_id.cpp` | All 108 ordered IDs and their 92 direct layout paths recovered. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line/command_line.cpp` | Complete behavior and observed container layout reconstructed. |
| `0x8D28A0`, `0x8D2EA0`, `0x8D3000`, `0x8D3060` | special pad reader and calibration sample path | `src/input/devices/special_pad_reader.cpp` | Hardware probing, sensor-mode report, rolling sample collection, and transfer recovered. |
| `0x2773C0`, `0x278270` | `fmod_audio_initialize`, custom DSP registration | `src/audio/fmod/system/fmod_audio_system.cpp` | FMOD 1.10.04 startup, advanced settings, file callbacks, driver format, DSP buffers, and all 12 custom plugins recovered. |
| `0x258C60`, `0x2590B0`, `0x261F60` | PS4 FMOD module and thread-affinity setup | `src/audio/fmod/platform/orbis/fmod_orbis_platform.cpp` | PRX loading, engine affinity-group lookup, CPU-mask construction, and the 11-entry private FMOD affinity table recovered. |
| `0x6BEF40`-`0x6C0160` | default render lighting and fallback | `src/render/resources/lighting/default_lighting.cpp` | `RndSceneResource` loading, authored light/probe discovery, object-ID lists, mode switching, scale-dependent probe/spot setup, and fallback directional-light creation recovered. |
| `0x6BEC50` | default render materials | `src/render/resources/materials/default_materials.cpp` | Six named material objects, shader-graph paths, unique sharing, and explicit additive/source blend modes recovered. |
| `0x6BEBC0` | default render camera | `src/render/resources/camera/default_camera.cpp` | Creation of `default_cam` and attachment of its `RndCameraCom` recovered. |
| `0x6BDE60`, `0x6C00F0` | default fallback textures | `src/render/resources/textures/default_textures.cpp` | Seven named color/error families, exact extents, all seven texture shapes, and indexed lookup recovered. |
| `0x6BDCA0`, `0x6BF860`, `0x6BFA00` | default render-resource lifecycle | `src/render/resources/system/default_render_resources.cpp` | Feature gate, initialization order, per-frame scene polling, and complete texture/compute/scene teardown recovered. |
| `0x3DD410`, `0x3DD790` | render-system construction and destruction | `src/render/core/system/render_system_lifecycle.cpp`, `src/render/core/system/render_system_globals.cpp` | Core synchronization, 13 platform slots, default and backend resources, typed singleton publication and clearing, settings ownership, and reverse teardown recovered. |
| `0x1AE4D0`, `0x363030`, `0x4414A0`, `0x8D5DE0` | renderer platform and API identities | `src/render/core/platform/render_platform.cpp` | Thirteen platform slots, seven exact API names, configured platform-to-API lookup, and the fixed Orbis API recovered. |
| `0x3641B0`, `0x6B9940`, `0x6B99B0` | renderer platform configuration | `src/render/core/platform/render_platform_config.cpp` | Supported-platform selection, fixed capability profiles, validated resolution lists, 1,920 × 1,080 fallback, and sorting recovered. |
| `0x3DDAE0`-`0x3DDFE4`, `0x3DEAA0`, `0x3DED70`, `0x3DEDC0` | render-system runtime lifecycle | `src/render/core/system/render_system_runtime.cpp`, `src/render/core/system/render_system_globals.cpp`, `src/render/core/system/render_epoch.cpp` | Backend startup, primary render context at `0x38`, additional owner vector at `0x48`, active back buffer at `0x70`, shared epoch at `0xA0`, skipped-frame advancement, deferred-release drain, and shutdown ordering recovered. |
| `0x8D5DF0`, `0x8D77F0`, `0x8D79B0` | Orbis render-system object lifetime | `src/render/platform/orbis/system/orbis_render_system.cpp`, `src/render/platform/orbis/system/orbis_render_system_globals.cpp` | 4,352-byte allocation, offset-zero common base, video and worker state, recursive submission locks, deferred-command list, typed platform singleton, and reverse destruction recovered. |
| `0x8D7340`-`0x8D80BB` | Orbis video output and submit worker | `src/render/platform/orbis/video/orbis_video_output.cpp` | Typed video handle at `0xEDC` and event queue at `0xEE0`, direct video/queue lifecycle calls, flip events, default vertex buffers, submit-thread state, runtime vsync application, and shutdown recovered. |
| `0x8D8100`-`0x8D857A` | Orbis GPU synchronization | `src/render/platform/orbis/synchronization/orbis_gpu_sync.cpp` | Typed context and active-frame fields, ten-counter idle barrier, active-frame flushing, exact local/global epoch selection, and the complete mutex-protected deferred-allocation queue lifecycle recovered. |
| `0x8D8300` | Orbis frame submission | `src/render/platform/orbis/synchronization/orbis_frame_submit.cpp` | Typed submit-token consumption, condition wait, active-command flush, platform-context submission, and collected back-buffer advancement recovered. |
| `0x8D5E30`, `0x8E24A0`-`0x8E289C` | Orbis back buffer | `src/render/platform/orbis/video/orbis_back_buffer.cpp` | Two displayable Gnm textures, 64-KiB minimum alignment, engine texture wrapping, video-output registration, active-buffer toggling, and object lifetime recovered. |
| `0x5C2700`-`0x5C2A75`, `0x5C2DF0`-`0x5C2EE3` | Common render mesh | `src/render/core/meshes/render_mesh.cpp` | Exact 128-byte base layout, update-listener link, 12-byte triangle vector, geometry finalization and CPU-storage release, residency predicates, shared backend update dispatch, render-epoch stamp, primary and secondary pending-update callbacks, and complete lifetime recovered. |
| `0x11B2CD0`-`0x11B2E09`, state use at `0x448730`-`0x448815` | Common render target | `src/render/core/targets/render_target.cpp`, `src/render/core/frame/render_frame_owner.cpp` | Exact 32-byte target layout, typed state header, optional 1,552-byte state ownership, base lifetime, active-buffer default, active-state spans, dimensions, draw mode, and debug view recovered. |
| `0x8E72B0`-`0x8EA29D`, `0x8EC310`-`0x8EC8B7` | Orbis render context | `src/render/platform/orbis/context/orbis_render_context.cpp`, `src/render/platform/orbis/context/orbis_render_context_state.cpp` | Graphics/compute allocation and lifetime, compute-queue gate at offset `9`, two-bank submission counters at `0x22880`, active frame at `0x40D90`, frame submission/reset, target binding, and cached pipeline state recovered. |
| `0x8EA2D0`-`0x8EA552`, `0x8EC8C0`-`0x8EC992` | Orbis transient drawing | `src/render/platform/orbis/context/orbis_transient_draw.cpp` | Active-bank format selection, vertex append/cursor advance, eight-stream binding with defaults, sequential 16-bit embedded indices, and indexed draw submission recovered. |
| `0x8E9810`-`0x8E99DF`, `0x8EA830`-`0x8EAA0F`, `0x8EC6A0`-`0x8EC791` | Orbis shader state | `src/render/platform/orbis/shaders/orbis_shader_state.cpp` | Exact sampler construction, vertex/pixel/compute routing, four-stage shader unbinding, and masked clearing of 128 read/write and read-only texture/buffer slots recovered. |
| `0x8E1EE0`-`0x8E246F` | Orbis texture binding | `src/render/platform/orbis/shaders/orbis_texture_binding.cpp` | Six engine shader stages mapped to Gnm stages, 16-slot sampler gating, writable pixel/compute bindings, and graphics-versus-compute context routing recovered. |
| `0x8E99E0`-`0x8E9F5D`, `0x8EBDA0`-`0x8EBF80` | Orbis depth-stencil clear | `src/render/platform/orbis/context/orbis_depth_clear.cpp` | HTILE and stencil compute clears, 64-dword dispatch sizing, raster fallback state, built-in clear draw, and accelerated-clear completion reporting recovered. |
| `0x8D85C0`, `0x8E1570`-`0x8E1715`, `0x8EB730`-`0x8EB868` | Orbis GPU fence | `src/render/platform/orbis/synchronization/orbis_fence.cpp` | Exact 24-byte object layout, four-byte GPU label lifetime, wrap-safe sequence replacement, graphics/compute release-memory signaling, and address waits recovered. |
| `0x8EB3E0`-`0x8EB728` | Orbis resource synchronization | `src/render/platform/orbis/synchronization/orbis_resource_sync.cpp` | Shared release labels, graphics/compute completion events, per-resource epoch tracking, command-buffer address waits, and grouped record removal recovered. |
| `0x8EAA10`-`0x8EB3D6` | Orbis resource barriers | `src/render/platform/orbis/synchronization/orbis_resource_barriers.cpp` | 32-byte transition/UAV records, immediate and split phases, batched cache actions, color/depth metadata resolution, and cross-queue handoff recovered. |
| `0x8EB870`-`0x8EBA1D` | Orbis dispatch and debug markers | `src/render/platform/orbis/context/orbis_render_commands.cpp` | Three-axis direct dispatch, graphics state transitions, standalone compute CUE flushing, and active-command-stream debug markers recovered. |
| `0x8EBA20`-`0x8EBD92` | Orbis GPU timing | `src/render/platform/orbis/synchronization/orbis_gpu_timing.cpp` | 512 paired GPU-clock timestamps, keyed begin/end recording, 800 MHz conversion to seconds, record release, and active-map removal recovered. |
| `0x442930`, `0x8D85F0`, `0x5C2700` | Mesh format parser and Orbis factory | `src/render/platform/orbis/meshes/orbis_mesh.cpp` | Eight exact format names, seven 472-byte Orbis variants, format-specific vtables, and the dedicated particle-format exclusion recovered. |
| `0x8D8C80`-`0x8D9E2F` | Orbis position-only mesh | `src/render/platform/orbis/meshes/orbis_position_mesh.cpp` | Missing virtual-method boundaries, 12-byte CPU vertices, accessors, backend finalization, double-buffered updates, and 16/32-bit index-buffer selection recovered. |
| `0x8D9E30`-`0x8DB18F` | Orbis color mesh | `src/render/platform/orbis/meshes/orbis_color_mesh.cpp` | Complete method boundaries, 28-byte position-plus-float4-color vertices, default alpha, accessors, backend finalization, and double-buffered updates recovered. |
| `0x8DB190`-`0x8DC36F` | Orbis color-texture mesh | `src/render/platform/orbis/meshes/orbis_color_texture_mesh.cpp` | Complete method boundaries, 36-byte position/color/UV vertices, default values, accessors, backend finalization, and double-buffered updates recovered. |
| `0x8DC370`-`0x8DD78F` | Orbis unskinned mesh | `src/render/platform/orbis/meshes/orbis_unskinned_mesh.cpp` | Complete method boundaries, 80-byte seven-attribute vertex layout, default color alpha, accessors, backend finalization, and double-buffered updates recovered. |
| `0x8DD790`-`0x8DEBDF` | Orbis skinned mesh | `src/render/platform/orbis/meshes/orbis_skinned_mesh.cpp` | Complete method boundaries, 100-byte vertex layout with weights and packed bone indices, accessors, backend finalization, and double-buffered updates recovered. |
| `0x8DEBE0`-`0x8E007F` | Orbis compressed unskinned mesh | `src/render/platform/orbis/meshes/orbis_unskinned_compressed_mesh.cpp` | Complete method boundaries, 52-byte packed vertex layout, zero defaults, accessors, backend finalization, and double-buffered updates recovered. |
| `0x8E0080`-`0x8E154F` | Orbis compressed skinned mesh | `src/render/platform/orbis/meshes/orbis_skinned_compressed_mesh.cpp` | Complete method boundaries, 64-byte packed vertex layout with packed weights and indices, zero defaults, accessors, backend finalization, and double-buffered updates recovered. |
| `0x442A80`, `0x443720`-`0x44397F`, `0x8E1840`-`0x8E1ED9` | Orbis vertex descriptors | `src/render/platform/orbis/meshes/orbis_vertex_descriptors.cpp` | Ten-to-eight mesh stream mapping, merged attribute layouts, exact Gnm data-format selection, read-only memory policy, active masks, and the nine-stream 120-byte instance layout recovered. |
| `0x8D8E00`, `0x8D9FB0`, `0x8DB310`, `0x8DC4F0`, `0x8DD910`, `0x8DED60`, `0x8E0200` | Orbis mesh draw | `src/render/platform/orbis/meshes/orbis_mesh_draw.cpp` | Common mesh/default stream binding, embedded instance upload, nine instance streams, triangle-list setup, 16/32-bit indexed range drawing, nonindexed fallback, and frame stamping recovered. |
| `0x8D8AD0`, `0x8E3800`-`0x8E3CF5` | Orbis constant buffer | `src/render/platform/orbis/buffers/orbis_constant_buffer.cpp` | Inline CPU payload, persistent GPU copy, 16-byte range updates, frame-local command-memory upload, Gnm descriptor construction, five-stage binding, and destruction recovered. |
| `0x639F30`-`0x63A045` | Common render constant buffer | `src/render/core/buffers/render_constant_buffer.cpp` | Exact 64-byte layout, descriptor-default element count, lazy/eager initial upload, construction state, and base lifetime recovered. |
| `0x8D8B30` | Orbis shader factory | `src/render/platform/orbis/shaders/orbis_shader.cpp` | Six-stage enum dispatch, four supported Orbis shader classes, exact object sizes, and stage-specific allocation names recovered. |
| `0x642250`-`0x642332` | Common render shader lifecycle | `src/render/core/shaders/render_shader.cpp` | 40-byte base layout, render-system factory dispatch, construction defaults, backend replacement, initialized-state tracking, release dispatch, and deletion recovered. |
| `0x69B6E0`-`0x69B79C` | Common render texture | `src/render/core/textures/render_texture.cpp` | Exact 168-byte base layout, sampler and resource defaults, frame sentinel, dimension state, allocation name, and base lifetime recovered. |
| `0x6900B0`-`0x69025F` | Common render 2D texture | `src/render/core/textures/render_texture_2d.cpp` | Exact 408-byte layout, descriptor normalization, 80-byte mip-chain state, linked-resource fields, and complete common lifetime recovered. |
| `0x6A0DA0`-`0x6A0F86` | Common render cube texture | `src/render/core/textures/render_texture_cube.cpp` | Exact 792-byte layout, normalized descriptor state, six typed 80-byte face mip chains, and complete common lifetime recovered. |
| `0x8E3D20`-`0x8E409F`, `0x8E43D0`-`0x8E46DF` | Orbis compute and pixel shaders | `src/render/platform/orbis/shaders/orbis_compute_shader.cpp`, `src/render/platform/orbis/shaders/orbis_pixel_shader.cpp` | Binary parsing, aligned header/code allocation, GPU-address patching, graphics/compute CUE binding, deferred release, and complete virtual lifecycles recovered. |
| `0x8E40A0`-`0x8E43CF`, `0x8E46E0`-`0x8E4F5F` | Orbis geometry and vertex shaders | `src/render/platform/orbis/shaders/orbis_geometry_shader.cpp`, `src/render/platform/orbis/shaders/orbis_vertex_shader.cpp` | Paired geometry code, dual vertex headers, semantic mapping, fetch-shader generation, context binding variants, deferred release, and complete virtual lifecycles recovered. |
| `0x8D8980`, `0x8E4F60`-`0x8E529F` | Orbis 1D texture | `src/render/platform/orbis/textures/orbis_texture_1d.cpp` | 408-byte factory and construction, Gnm descriptor setup, aligned tiled storage, complete mip upload, deferred release, and destruction recovered. |
| `0x6F5870`-`0x6F59E9` | Common render 1D texture | `src/render/core/textures/render_texture_1d.cpp` | Exact 392-byte layout, 168-byte descriptor input, descriptor normalization, 80-byte mip-chain state, and complete common lifetime recovered. |
| `0x8D62C0`-`0x8D7247`, `0x8D89B0` | Orbis 2D texture | `src/render/platform/orbis/textures/orbis_texture_2d.cpp` | 520-byte factory and construction, shared color allocation, double-buffered mip uploads, depth/stencil/HTILE storage, active-frame target selection through the typed frame-owner field, backend accessors, and destruction recovered. |
| `0x8D89E0`, `0x8E53C0`-`0x8E573F` | Orbis 3D texture | `src/render/platform/orbis/textures/orbis_texture_3d.cpp` | 408-byte factory and construction, compatible allocation reuse, tiled mip uploads, Gnm texture setup, deferred release, and destruction recovered. |
| `0x6F5CB0`-`0x6F5E29` | Common render 3D texture | `src/render/core/textures/render_texture_3d.cpp` | Exact 392-byte layout, 168-byte descriptor input, descriptor normalization, 80-byte mip-chain state, and complete common lifetime recovered. |
| `0x8D8A10`, `0x8E6BA0`-`0x8E7287` | Orbis cube texture | `src/render/platform/orbis/textures/orbis_texture_cube.cpp` | 832-byte factory and construction, typed color/depth usage dispatch, six-face color uploads, depth/stencil storage, optional render-target view, backend accessors, deferred release, and destruction recovered. |
| `0x8D8A40`, `0x8E5870`-`0x8E5C1F` | Orbis 1D texture array | `src/render/platform/orbis/textures/orbis_texture_array_1d.cpp` | 360-byte factory and construction, Gnm array descriptor, aligned tiled storage, per-layer mip uploads, deferred release, and destruction recovered. |
| `0x696C00`-`0x697120` | Common render 1D texture array | `src/render/core/textures/render_texture_array_1d.cpp` | Exact 344-byte layout, 168-byte descriptor, 80-byte per-layer mip chains, descriptor normalization, vector lifetime, and common teardown recovered. |
| `0x8D8A70`, `0x8E5D40`-`0x8E648F` | Orbis 2D texture array | `src/render/platform/orbis/textures/orbis_texture_array_2d.cpp` | 392-byte factory and construction, color and depth array paths, per-layer mip uploads, render/depth views, metadata release, and destruction recovered. |
| `0x698160`-`0x6985C0` | Common render 2D texture array | `src/render/core/textures/render_texture_array_2d.cpp` | Exact 344-byte layout, descriptor resolution, 80-byte per-layer mip chains, vector lifetime, resource-index default, and common teardown recovered. |
| `0x8D8AA0`, `0x8E6640`-`0x8E6A7F` | Orbis cube texture array | `src/render/platform/orbis/textures/orbis_texture_array_cube.cpp` | 360-byte factory and construction, Gnm cube-array descriptor, six-face per-cube mip uploads, deferred release, and destruction recovered. |
| `0x69AAC0`-`0x69AD49` | Common render cube texture array | `src/render/core/textures/render_texture_array_cube.cpp` | Exact 344-byte layout, typed six-face 480-byte cube records, descriptor normalization, vector lifetime, and common teardown recovered. |
| `0x8D8BC0`, `0x8E3250`-`0x8E37D0`, `0x8EA740` | Orbis compute buffer | `src/render/platform/orbis/buffers/orbis_compute_buffer.cpp`, `src/render/platform/orbis/buffers/orbis_compute_buffer_commands.cpp` | 136-byte factory, one/two-bank GPU storage, typed active descriptor and allocation selection, CPU uploads, six-stage read/read-write binding, blocking GDS counter copy, deferred release, and destruction recovered. |
| `0x636C70`-`0x636DB6` | Common render compute buffer | `src/render/core/buffers/render_compute_buffer.cpp` | Exact 80-byte layout, 48-byte descriptor copy, CPU staging allocation, backend initialization dispatch, staging lifetime, and type sentinel recovered. |
| `0x8EA740`-`0x8EA7C8` | Orbis compute-buffer count copy | `src/render/platform/orbis/buffers/orbis_compute_buffer_commands.cpp` | Active source RW binding, blocking four-byte GDS-to-memory DMA, destination-buffer selection, and binding cleanup recovered. |
| `0x8D8BF0`, `0x8E2AA0`-`0x8E322B` | Orbis particle buffer | `src/render/platform/orbis/buffers/orbis_particle_buffer.cpp` | 360-byte object, double-buffered 208-byte-per-particle vertex storage, quad index generation, vertex and instance stream binding, indexed drawing, deferred release, and destruction recovered. |
| `0x6EAFD0`-`0x6EB042`, `0x6ECB80`-`0x6ECB95` | Common render particle buffer | `src/render/core/buffers/render_particle_buffer.cpp` | Exact 64-byte layout, factory dispatch, capacity and draw-state defaults, context ownership, and base lifetime recovered. |
| `0x5F7D30`-`0x5F7DF6` | Common render occlusion query | `src/render/core/synchronization/render_occlusion_query.cpp` | Exact 64-byte layout, factory dispatch, state defaults, self-linked intrusive node, unlinking, and base lifetime recovered. |
| `0x8D8C30`, `0x8E28C0`-`0x8E2A78` | Orbis occlusion query | `src/render/platform/orbis/synchronization/orbis_occlusion_query.cpp` | 72-byte object, intrusive-list lifetime, aligned per-frame result allocation, begin/end collection, conditional rendering, and five missing IDA function boundaries recovered. |
| `0x27A1C0`-`0x27A850` | FMOD file callbacks and asynchronous reader | `src/audio/fmod/io/fmod_file_io.cpp` | Open, close, read, seek, priority queue, worker, cancellation, and shutdown behavior recovered. |
| `0x262300`, `0x27ACB0` | listener update and engine-to-FMOD transform conversion | `src/audio/fmod/system/fmod_listener.cpp` | Primary listener gating, 48-byte transform layout, handedness conversion, and zero velocity recovered. |
| `0x2763F0`-`0x276520`, `0x2786D0` | `HMX.BufferedOutput` callbacks and custom-output initialization | `src/audio/fmod/io/fmod_buffered_output.cpp`, `src/audio/fmod/system/fmod_audio_system.cpp` | Output descriptor, 128 virtual drivers, format negotiation, update dispatch, and update-driven FMOD flags recovered. |
| `0x2781C0`, `0x2783E0`, `0x278880`-`0x2789CD` | mix-buffer dispatch and FMOD pre/post-mix callback | `src/audio/fmod/mixing/fmod_mix_callback.cpp` | Semaphore ownership, mix sequence, source reset, observer dispatch, and cumulative/rolling timing recovered. |
| `0x1127880` | output-block listener dispatch | `src/audio/core/output/audio_output_dispatcher.cpp` | Pending-list promotion, two-phase listener calls, atomic guards, and 128-sample subdivision recovered. |
| `0x277840`, `0x277A80` | attach or detach an external FMOD Studio system | `src/audio/fmod/system/fmod_audio_system.cpp` | User-data binding, format discovery, DSP registration, callback enablement, and synchronized detach recovered. |
| `0xD3BA0`, `0xD3BC0` | global audio mix-format accessors | `src/audio/core/format/audio_mix_format.cpp` | Sample rate, reciprocal, buffer cadence, and milliseconds-per-buffer calculations recovered. |
| `0x1128380`, `0x278A00`, `0x278DF0` | double-buffered deferred FMOD release queue | `src/audio/fmod/system/fmod_deferred_release.cpp` | Mix-consumer registration, enqueue, buffer swap, channel stop, DSP release, and detach-time clearing recovered. |
| `0x2783A0` | engine speaker configuration mapping | `src/audio/fmod/system/fmod_audio_system.cpp` | Mono, stereo, 5.1, and 7.1 FMOD modes and raw channel counts recovered. |
| `0x277BA0`, `0x278C80`-`0x278D72` | `fmod_audio_consume_timing_report` and percentage getters | `src/audio/fmod/system/fmod_timing_report.cpp` | Atomic snapshot/reset, buffer-duration normalization, rolling-window normalization, and per-source aggregation recovered. |
| `0x267E50`-`0x26837A` | `AudioClipFmod` release lifecycle | `src/audio/fmod/playback/audio_clip_fmod.cpp` | Deferred low-level channel release, Studio event teardown, invalid-handle handling, and blocking synchronous-update drain recovered. |
| `0x266CB0`-`0x26759C` | `AudioClipFmod` DSP, playback startup, and Studio event callback | `src/audio/fmod/playback/audio_clip_fmod.cpp` | `HMXRawAudioBus`, event/low-level fallback, HMX plugin binding, head-DSP insertion, parent routing, sample-rate base frequency, and callback readiness recovered. |
| `0x267BB0`-`0x268479` | `AudioClipFmod` runtime controls | `src/audio/fmod/playback/audio_clip_fmod.cpp` | Pause/resume state transitions, millisecond playback position, 3D attribute updates, and named Studio parameters recovered. |
| `0x268380`, `0x268510`-`0x26943F` | `FmodAudioBusGenerator` pool and creation | `src/audio/fmod/playback/fmod_audio_bus_generator.cpp` | Fixed pool, stale-handle rejection, sound creation, and Studio-bus route registration recovered. |
| `0x2692B0`, `0x2693B0`-`0x26AA9F` | `FmodAudioStreamGenerator` pool and playback | `src/audio/fmod/playback/fmod_audio_stream_generator.cpp` | MP3 type, 288-byte pool, handles, asynchronous channel startup, loop points, seeking, pause control, volume, 3D updates, and teardown recovered. |
| `0x26AB60`-`0x26E419` | `FmodBufferedStreamGenerator` pool and rendering | `src/audio/fmod/playback/fmod_buffered_stream_generator.cpp` | 568-byte pool, resource gate, decoder blocks, seeking, gain, ring refill, normal PCM16 stereo interpolation, synchronized-render controls, and the Optimal 32x six-point, fifth-order kernel recovered. |
| `0x26E990`-`0x2715FA` | `FmodDialogGenerator` and `FmodStudioSoundGenerator` | `src/audio/fmod/playback/fmod_dialog_generator.cpp`, `src/audio/fmod/playback/fmod_studio_sound_generator.cpp` | Dialog and generic pools, `.bank` routing, `event:/` and `snapshot:/` normalization, programmer sounds, Studio event lifecycle, timeline and parameter controls, 3D updates, and two-stage volume fading recovered. |
| `0x271660`-`0x272E02` | `FmodAudioStreamResource` | `src/audio/fmod/resources/fmod_audio_stream_resource.cpp` | Streaming-audio extensions, platform-path load, FMOD PCM16 mono/stereo probing, status codes, normalized-path registry, lookup, and teardown recovered. |
| `0x272E60`-`0x27315C` | `FMODSoundToPCMCallback` | `src/audio/fmod/resources/fmod_sound_to_pcm_callback.cpp` | Nonblocking-open wait, format and length query, PCM16 buffer sizing, block decode loop, cancellation, completion, rewind, and teardown recovered. |
| `0x273740`-`0x275180` | `FModBankResource` | `src/audio/fmod/resources/fmod_bank_resource.cpp` | PS4 and localized path handling, per-Studio-system bank and sample-data load, bus locking, unload waits, event and bus path enumeration, and paired master-bank routing recovered. |
| `0x275490`-`0x275C00`, `0x27B3E0`-`0x27BA9B` | FMOD audio input manager and record devices | `src/audio/fmod/input/fmod_audio_input_manager.cpp` | Studio bus binding, authored-volume scaling, mute and channel-group access, fixed device slots, `GENERAL` driver filtering, duplicate suppression, and connection reconciliation recovered. |
| `0x275E20`-`0x2763C0` | FMOD recording audio render target | `src/audio/fmod/input/fmod_recording_audio_render_target.cpp` | Embedded FMOD state delegation, buffered-output mixer reads, dual-layer locking, mix-consumer dispatch, and asynchronous recording-thread lifecycle recovered. |

## Game initialization

`game_initialize` performs a fixed startup sequence: core services, UI and
stage-presence identifiers, `config/rockband.dta`, sound, engine type
registrations, primary game systems, UI resources, Dingo backend networking,
time-stretch audio, and player assignments. It then loads
`kLayoutGameStartup` and marks remaining command-line switches handled.

The game-system option block is exactly 16 bytes. Its first three bytes are set
to true and its final eight bytes are zero. Rendering initialization reads the
second flag directly; the meanings of the first and third flags are retained as
unknown fields until their virtual consumer is recovered.

## Command-line argument layout

IDA shows 16-byte entries containing a string pointer at offset 0 and a handled
flag at offset 8. The owning object begins with the usual three pointers for a
contiguous container: first element, one-past-last element, and capacity. The
reconstructed function walks the first two pointers and marks every unhandled
argument whose first character is `-`.

The structure names are descriptive because original symbols are unavailable.
Offset and size assertions preserve the observed binary layout.

## Special pad calibration reader

The special pad reader probes devices `0738:8261` and `0E6F:0173` through the
private `scePadOpenExt` API and assigns internal hardware IDs `0x1F` and
`0x20`. It controls a calibration sensor with feature report `0x30`, collects
up to 64 samples from `ScePadData::deviceUniqueData`, and transfers each batch
to calibration code. Numeric mode names are retained until their physical
sensor meanings are proven. See `docs/special-pad-reader.md` for the layouts
and call-site evidence.

## Stage presence IDs

The function at `0x997590` initializes a guarded table of 22 eight-byte engine
symbols and indexes it directly with the requested ID. Every symbol string is
embedded beside the function, which makes the enum order exact. The cleaned
source uses a function-local static array to express the same one-time
initialization without reproducing compiler guard internals.

## UI layout IDs

The function at `0xBB06A0` contains a guarded table for IDs 0 through 107 and a
separate `kLayoutInvalid` result for ID -1. The ordered list lives in
`src/ui/layout/ui_layout_list.inc` so the enum and symbol table share one readable
source of truth. ID `0x2C` is `kLayoutGameStartup`, matching the startup layout
loaded by `game_initialize`.

The loader at `0xBB5D40` supplies direct `.layout` paths for 92 of those IDs.
The same include now drives `ui_layout_primary_path`, keeping every recovered
path aligned with its enum value. Sixteen deprecated, fallback, or unsupported
IDs have no direct path. SOMP IDs 101 and 102 also fall through in the original
switch to preload the subsequent session layouts; the helper reports the first
path associated with each requested ID.
