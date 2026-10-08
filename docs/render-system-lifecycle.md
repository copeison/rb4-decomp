# Render-system construction

`render_system_construct` at `0x3DD410` builds the shared base renderer. The
clean control-flow reconstruction is in
`src/render/core/system/render_system_lifecycle.cpp`; backend object layouts remain behind
adapters until their individual types are identified.

Construction proceeds in this order:

1. Initialize the core frame state and its recursive mutex.
2. Construct 13 fixed, 128-byte platform-configuration slots.
3. Construct the default resources and the remaining backend state.
4. Initialize the callback queue and its recursive mutex.
5. Publish `g_render_system`.
6. Read `platform_mgr.supported_platforms` and initialize the corresponding
   slots, including each platform's sorted resolution list.
7. Invoke a capability predicate for slot seven. The return value is ignored
   in this build, so its precise source-level purpose remains open.
8. Allocate the 232-byte renderer settings block and initialize it.

`render_system_destruct` at `0x3DD790` releases the settings block, callback
state, backend objects, default resources, platform configurations, core
vectors, and mutexes in reverse ownership order.

The frame prefix contains a typed 40-byte render-context array at offset
`0x48`, with
begin, end, capacity, and the allocator's two-word state. Shutdown walks this
array backward and deletes each remaining context. The primary render context
at `0x38` is separate from the active back-buffer frame owner at `0x70`.
The common context prefix now owns the deleting, initialize, and shutdown
dispatch slots at vtable offsets `0x08`, `0x10`, and `0x18`; runtime lifecycle
code no longer models those context calls as frame-owner operations.

`src/render/core/system/render_system_state.h` centralizes the verified
312-byte prefix shared by frame activation and lifetime code. The prefix now
includes the recursive mutex and lock bookkeeping at `0x08`-`0x1F`, the copied
16-byte startup options at `0x28`, active frame owner and target-state array at
`0x78`/`0x80`, primary and auxiliary frame epochs at `0xA0`/`0xA8`, frame
timing state at `0x100`-`0x127`, settings at `0x128`, and the render factory at
`0x130`. Shared lock helpers live in `src/render/core/synchronization` and
operate directly on this prefix. The submit-done worker uses the full frame
lock pair, including owner-thread tracking; polling uses the lighter recursive
mutex and depth-counter sequence found in its own function.

Ending a frame clears the active target-state array and owner directly. The
array is a 32-byte begin/end/capacity/allocator record, distinct from the
40-byte render-context array whose allocator carries two words of state.
The back-buffer frame-owner prefix now exposes its deleting, target-state,
poll, and begin dispatch slots directly. Output extents and frame attachment
therefore share the same typed target-state handle instead of separate opaque
adapter contracts.

The fixed platform array is separate from the supported-platform list. Every
slot receives its empty constructor, while only IDs named by configuration are
populated with capability flags and resolutions.
