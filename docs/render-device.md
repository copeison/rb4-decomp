# Render device

`RndDevice` (`src/render/system/RndDevice.{h,cpp}`) is the engine's render
device. `PS4Device` (`src/renderps4/system/PS4Device.{h,cpp}`) is its PS4
implementation. The game creates the device with `Rnd::PlatformCreateDevice`
at 0x8D5DF0 (`renderps4/system/PS4Init.cpp`), which allocates 4,352 bytes and
runs the `PS4Device` constructor.

The class and method names come from the reference map's `RndDevice.o` and
`PS4Device.o`. Field names, and the names of slots and helpers the map does
not list, are reconstructions and carry a `// Name not in the reference map.`
comment.

## Virtual slots

The base vtable's address point is 0x18FF528 (`_ZTV9RndDevice` at
0x18FF518). The PS4 vtable's address point is 0x195ED70 (`_ZTV9PS4Device` at
0x195ED60).

| Slot | Method | `RndDevice` | `PS4Device` |
| --- | --- | --- | --- |
| 0, 1 | destructors | 0x3DD790, 0x3DDAC0 | 0x8D79B0, 0x8D7B00 |
| 2 | `_ProcessDeferredDeletion()` | 0x3DEF50 (empty) | 0x8D84B0 |
| 3 | `_InitImpl(const RndInitParams*)` | pure | 0x8D7B20 |
| 4 | `_PostInitImpl()` | pure | 0x8D8580 (empty) |
| 5 | `_TerminateImpl()` | pure | 0x8D8040 |
| 6 | `_BeginFrameImpl(bool)` | pure | 0x8D8100 |
| 7 | `_EndFrameImpl(FixedVector<RndWindow*, 6>&, bool)` | pure | 0x8D8300 |
| 8 | `_AcquireDeferredContextImpl(RndContext*)` | 0x3DEF60 (empty) | inherited |
| 9 | unknown, empty | 0x3DEF70 | inherited |
| 10 | `_ReleaseDeferredContextImpl(RndContext*)` | 0x3DEF80 (empty) | inherited |
| 11 | `_ExecuteDeferredContextImpl(RndContext*)` | 0x3DEF90 (empty) | inherited |
| 12 | `_SetConsoleStateImpl(ConsoleState)` | 0x3DEFA0 (empty) | inherited |
| 13 | `_GetGpuBlockingBehaviorImpl() const` | 0x3DEFB0 (returns 0) | 0x8D8590 (returns 0) |
| 14 | unknown, returns 0 | 0x3DEFC0 | 0x8D83E0 |
| 15 | unknown, returns a zeroed 24-byte value | 0x3DEFD0 | inherited |
| 16 | unknown, empty | 0x3DEFF0 | inherited |

These slot assignments are evidence-based:

- `Init` calls slot 3 between the resource manager's initialize and finalize
  steps, and calls slot 4 after the contexts are initialized.
- `Terminate` tail-calls slot 5.
- `_DoBeginFrame` calls slot 6 and then slot 13.
- `_DoEndFrame` calls slot 7 with the frame windows.
- `AcquireDeferredContext`, `ReleaseDeferredContext` and
  `ExecuteDeferredContext` call slots 8, 10 and 11.
- `SetConsoleState` tail-calls slot 12.

## Layout

`RndDevice`'s data ends at 3,804 bytes, so the first `PS4Device` field sits
in the base class's tail padding. Each field is checked by `static_assert`.

| Offset | Field |
| --- | --- |
| 8 | `CritSec mCritSec` (`Lock`, `Unlock`, `PollMainWindow`) |
| 24 | lock-owner thread |
| 40 | `RndInitParams mInitParams`, copied by `Init` |
| 56 | immediate context (`_InstallImmediateContext`) |
| 64, 68 | pending begin-frame flag and flags |
| 72 | deferred contexts (`eastl::vector<RndContext*>`) |
| 104 | HDR output mode |
| 112, 120 | main window and current window |
| 128 | current target states (`eastl::vector`) |
| 160, 168 | frame count and offscreen frame count |
| 176, 177 | in-frame and terminating flags |
| 184 | `FixedVector<RndWindow*, 6>` of windows drawn this frame |
| 256-292 | frame timing and the 59:1 smoothed frame rate |
| 296, 304 | settings and factory |
| 312 | 13 platform configurations, 128 bytes each |
| 1976 | default resources |
| 2544 | resource manager |
| 3256 | lighting resources |
| 3560-3576 | FogDeferred shader, primitive meshes, audio-analysis textures |
| 3584 | GPU statistics block |
| 3712 | four built-in constant buffers |
| 3744 | pending-free `CritSec` and `eastl::vector<RndMaterialRuntimeData*>` |
| 3800 | console state (`GetConsoleState`) |
| 3804 | `PS4Device`: video-out handle, event queue |
| 3816 | `Condition` for the submit token at 3840 |
| 3852, 3992 | default vertex and identity-instance buffer descriptors |
| 4144 | submit-done `NamedThread` |
| 4280, 4296 | submission and deferred-delete `CritSec`s |
| 4312 | deferred-delete list (`PS4DeferredDelete` nodes) |
| 4344 | cached flip rate |

## Corrections made during the conversion

- **Pending frees.** The queue at 3760 holds `RndMaterialRuntimeData`
  objects, queued by `SyncFreeMaterialData` (0x3DEC20). They are destroyed
  with a plain `delete` (0x4F7270 calls the destructor at 0x4F7580), not
  through a virtual call.
- **Out-of-line flush.** `_ProcessPendingFrees` exists out of line at
  0x3DDFF0. `Terminate` and `_DoEndFrame` inline it.
- **Built-in constant buffers.** `Terminate` releases them through
  `RndShaderCBuffer::SafeDelete`, and frees the primitive-mesh and
  audio-texture sets with `operator delete`. The buffers are created from the
  resource manager's render-target, clip-plane, misc-draw-state and
  occlusion-query blocks.
- **Main window.** `BeginMainWindowFrame` does not null-check the main window.
  The callers in the frame loop check the global device.
- **Owning members.** `RenderPlatformConfig` and `DefaultRenderResources` own
  vectors, so they now have real constructors and destructors. The three plain
  structures (resource manager, lighting, GPU statistics) keep their explicit
  construct and destruct helpers.

## Earlier names

| Earlier reconstruction | Original |
| --- | --- |
| `render_system_construct`, `render_system_destruct` | `RndDevice::RndDevice`, `~RndDevice` |
| `render_system_initialize` | `RndDevice::Init` |
| `render_system_initialize_builtin_buffers` | `RndDevice::_InitBuiltinCBuffers` |
| `render_system_shutdown` | `RndDevice::Terminate` |
| `render_system_acquire_frame_lock`, `render_system_release_frame_lock` | `RndDevice::Lock`, `Unlock` |
| `render_system_poll` | `RndDevice::PollMainWindow` |
| `render_system_begin_frame`, `render_system_end_frame` | `RndDevice::BeginMainWindowFrame`, `EndMainWindowFrame` |
| `render_system_prepare_frame`, `render_system_finish_frame` | `RndDevice::_DoBeginFrame`, `_DoEndFrame` |
| `render_system_attach_frame_owner` | `RndDevice::_DoBeginDrawingWindow` |
| `render_system_begin_auxiliary_frame`, `render_system_finish_auxiliary_frame` | `RndDevice::BeginOffscreenFrame`, `EndOffscreenFrame` |
| `render_system_skip_frame` | `RndDevice::ForceIncrementFrameCount` |
| `render_system_enqueue_deferred_release` | `RndDevice::SyncFreeMaterialData` |
| `render_system_flush_deferred_releases` | `RndDevice::_ProcessPendingFrees` |
| `render_system_activate_pending_frame` | `RndDevice::_FlushPendingBeginFrame` |
| `render_system_set_back_buffer`, `render_system_release_back_buffer` | `RndDevice::_InstallMainWindow`, `_DestroyMainWindow` |
| `render_system_set_render_context`, `render_system_release_render_contexts` | `RndDevice::_InstallImmediateContext`, `_DestroyContexts` |
| `render_system_set_factory` | `RndDevice::_InstallFactory` |
| `orbis_render_system_create` | `Rnd::PlatformCreateDevice` |
| `orbis_render_system_construct`, `orbis_render_system_destruct` | `PS4Device::PS4Device`, `~PS4Device` |
| `orbis_render_system_initialize`, `orbis_render_system_shutdown` | `PS4Device::_InitImpl`, `_TerminateImpl` |
| `orbis_render_system_wait_idle`, `orbis_wait_for_gpu_idle` | `PS4Device::_BeginFrameImpl`, `WaitForIdle` |
| `orbis_release_retired_allocations` | `PS4Device::_ReleaseRetiredAllocations` |
| `orbis_defer_allocation_release` | `PS4Device::DeferredDelete` |
| `orbis_release_all_retired_allocations` | `PS4Device::_ProcessDeferredDeletion` |
| `orbis_render_system_submit_frame` | `PS4Device::_EndFrameImpl` |
| `orbis_create_default_vertex_buffer`, `orbis_create_identity_instance_buffer` | `PS4Device::_InitDefaultVertexBuffers`, `_InitIdentityInstanceBuffers` |
| `g_render_system`, `g_orbis_render_system` | `gRndDevice`, `gPS4Device` |
| `GameSystemInitOptions` | `RndInitParams` |
