# Orbis render context

The Orbis render system allocates a `0x44890`-byte platform context during
startup and constructs it at `0x8E72B0`. The context owns two primary graphics
frame slots, two compute queues, eighteen compute-context slots, sixteen
transient vertex buffers, GPU timestamps, labels, and command-state caches.

`PS4Context::_CreateGfxContext` (`0x8E7AF0`) sets up both graphics frame
slots. For each slot it:
1. allocates a CUE heap for 64 ring entries
   (`ConstantUpdateEngine::computeHeapSize`, from the `"gpu"` heap), a 32 MiB
   draw command buffer and a 2 MiB constant command buffer;
2. clears the slot's ten submission flags;
3. calls `GfxContext::init` with a CUE `RingSetup` of 128 resource, 16
   read-write, 16 sampler and 32 vertex-buffer slots (the SDK 5.500 overload
   matches the binary's packed argument);
4. allocates a 192-byte global resource table and two 4 MiB GS rings, and
   installs them with `setGlobalResourceTableAddr`, `setEsGsRingBuffer` (100
   dwords per vertex) and `setGsVsRingBuffers` (100 dwords per stream, at most
   `0x100000` vertices).

The table and the rings live in single members at `0x22868`-`0x22878`, so the
second slot's allocations replace the first's pointers. The per-slot pointers
are at `0x22838`-`0x22860`.

The transient vertex buffers form two banks of all eight mesh formats. Every
buffer reserves space for `0x40000` vertices and uses the shared descriptor
builder at `0x8E1840`. Their constructor and initializer are now named at
`0x8EC7C0` and `0x8EC7E0`.

When compute queues are enabled, the context sets up the two
`sce::Gnmx::ComputeQueue`s at `0x22920`. Each gets a zeroed 4 KiB ring
(256-byte aligned) and a 4-byte read pointer, then:
1. `initialize(pipe, 0)`;
2. `Gnm::mapComputeQueueWithPriority`;
3. the root dispatch command buffer is initialized over the ring.

This is SDK 2.500's inline map with a priority. Queue zero maps pipe 1 at
`kPipePriorityMedium` and queue one maps pipe 0 at `kPipePriorityLow`.

The context then initializes the eighteen compute contexts with the SDK
5.500 LCUE overload of `ComputeContext::init`. Each gets a `0x3FFFFC`-byte
command buffer and a `"gpu"`-heap resource buffer sized like a 64-entry CUE
heap.

The timestamp pool at `0x8E7DF0` allocates 8 KiB and divides it into 512 pairs
of 64-bit timestamps. The label allocator begins with 32 records. The
destructor at `0x8E8070` frees only the timestamps and the labels before the
members are destroyed. The deleting destructor at `0x8E82B0` frees the context
allocation.

`PS4Context::SubmitFrame` (`0x8E82D0`) works through the frame:
1. It triggers `kEventTypeCacheFlushAndInvEvent` on the graphics context.
2. For each of the active bank's nine compute contexts, it sets the context's
   pending flag. It then writes 0 to the flag with a `kReleaseMemEventCsDone`
   release-memory event and submits the context. Contexts zero to two go to
   queue zero and the rest to queue one.
3. It sets the graphics flag and clears it with
   `writeAtEndOfPipeWithInterrupt` (`kEopFlushCbDbCaches`).
4. It submits the graphics context and toggles the active frame.

`PS4Context::_ResetFrame` (`0x8E8450`) prepares the active frame for
recording:
1. It resets the graphics context and calls its
   `initializeDefaultHardwareState`, which also invalidates the CUE's shader
   contexts.
2. When compute is enabled, it resets the bank's nine compute contexts and
   their default dispatch state.
3. It forgets `SetupDraw`'s state, turns the GS mode off, and enables color
   writes, as `_BeginFrameImpl` does.
4. It rewinds the eight transient buffers and sets DX clip space in
   `ClipControl`.
