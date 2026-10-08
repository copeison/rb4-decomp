# Render-system construction

`render_system_construct` at `0x3DD410` builds the shared base renderer. The
clean control-flow reconstruction is in
`src/render/core/system/render_system_lifecycle.cpp`; backend object layouts remain behind
adapters until their individual types are identified.

Construction proceeds in this order:

1. Initialize the core frame state and its recursive mutex.
2. Construct 13 fixed, 128-byte platform-configuration slots.
3. Construct the default resources, resource manager, lighting state, owned
   backend slots, GPU-stat block, and built-in buffer slots.
4. Initialize the deferred-release queue, its recursive mutex, and the adjacent
   frame-phase state.
5. Publish `g_render_system`.
6. Read `platform_mgr.supported_platforms` and initialize the corresponding
   slots, including each platform's sorted resolution list.
7. Invoke a capability predicate for slot seven. The return value is ignored
   in this build, so its precise source-level purpose remains open.
8. Allocate the 232-byte renderer settings block and initialize it.

The common 312-byte prefix construction is now source-owned. It installs the
base vtable, creates the recursive frame mutex, applies the three true startup
option defaults, initializes both dynamic pointer arrays, points the submitted
owner list at its six inline slots, sets the GPU query sentinel to `-1`, and
clears the timing, settings, and factory fields. The 64-byte deferred-release
and frame-phase tail at `0xEA0` is also source-owned. Destruction frees the active
target-state and render-context arrays by their recorded capacities, drains
the recursive lock depth, and destroys the frame mutex.

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

Frame submission receives a 16-byte owner list at `0xB8`, consisting of an
item pointer and element count. The Orbis backend iterates that count after
submitting the render context and advances each collected back buffer. This is
the observed binary contract; it is not a standard-library vector.

Frame preparation at `0x3DE170` now owns its common control flow. It acquires
the owner-tracked frame lock, marks the frame active, invokes platform slot
`0x30`, updates the performance-counter snapshot for primary frames, computes
instantaneous frames per second, and applies the original 59-to-1 rolling
average. It then marks the primary context active, consumes pending activation,
and begins GPU frame tracking. Only the phase-metric side effect and GPU
resource preparation remain behind narrow adapters; the typed common GPU-stat
block owns the begin/end query dispatch directly.

Frame finish at `0x3DE4A0` now mirrors that ownership: it resolves pending
activation, ends GPU tracking, dispatches primary or auxiliary submission,
advances the matching epoch, clears the submitted-owner count for primary
frames, drains deferred releases, resets context/frame-active state, releases
the owner-tracked lock, and polls default resources. The auxiliary entry and
exit functions at `0x3DE8F0`/`0x3DE9E0` also own their exact optional
single-target-state behavior. Construction of primary GPU submission records
remains isolated behind one adapter.

The default-resource block is the exact 568-byte range at render-system offset
`0x7B8` (`1976`) through `0x9EF`. Frame finish polls this typed block directly,
and shutdown releases it directly, removing the previous system-level wrapper
for both operations.

The deferred-release state at `0xEA0` contains a typed 48-byte recursive-lock
queue followed by a reserved pointer and the frame-phase value at `0xED8`.
Construction now initializes that complete 64-byte tail directly. Normal
enqueue doubles capacity, frame finish and shutdown drain the queue under the
lock, and enqueue during shutdown releases the object immediately. Teardown
frees the queue storage by capacity, unwinds the recorded recursive lock depth,
and destroys the mutex. Only the object-specific destruction routine remains
an adapter.

The fixed platform array is separate from the supported-platform list. Every
slot receives its empty constructor, while only IDs named by configuration are
populated with capability flags and resolutions.

Runtime initialization now writes the initialized flag and copies the exact
16-byte `GameSystemInitOptions` block directly before invoking platform vtable
slot `0x18`. Slot `0x20` completes initialization. The runtime epoch increments
the timing-state counter and captures the first performance counter when that
counter transitions from zero. Shutdown marks the shared state at `0xB1`
before resource release and invokes platform slot `0x28` last.

The four built-in constant buffers initialized at `0x3DDC20` are now direct
typed runtime state. Their descriptors and element indices occupy the verified
range beginning at system offset `0xA98`; their owned buffer pointers occupy
`0xE80` through `0xE98`. Initialization creates each buffer with deferred
upload, writes the two-zero, four-zero-vector, negative/zero sentinel, and
`{0.0, 1.0}` defaults, then invokes the backend upload dispatch at vtable slot
`0x10`. Shutdown destroys, releases, and clears all four owned pointers.

Runtime resource startup and shutdown now expose the original ownership order.
The resource manager begins at `0x9F0`, the 304-byte lighting-resource block at
`0xCB8`, an owned backend resource pointer at `0xDE8`, the 40-byte primitive
mesh set pointer at `0xDF0`, the 72-byte audio-analysis set pointer at `0xDF8`,
and the inline GPU-stat block at `0xE00`. Startup allocates the primitive and
audio sets explicitly before initializing GPU statistics. The primitive set is
an exact 40-byte, two-slot inline owner containing the default box and cylinder
meshes; its teardown dispatches dynamic release for both slots. Shutdown releases
the backend, lighting, and resource-manager state first, then destroys and
frees the primitive and audio sets, clearing both owning pointers before the
built-in constant buffers are released.

The lifecycle constructor now owns the complete contiguous backend-state setup
from the resource manager at `0x9F0` through the four null built-in buffer
slots at `0xE80`. The resource-manager and lighting internals retain narrow
constructor adapters, while their placement and ordering are direct. The
128-byte GPU-stat block at `0xE00` initializes both pointer arrays, the total
statistic pointer, query counters, four-frame history slot, backend pointer,
recursive lock depth, and mutex. Its destructor dynamically releases every
ordinary statistic, destroys and frees each root statistic, unwinds the mutex,
and frees both pointer arrays by their recorded capacities.
