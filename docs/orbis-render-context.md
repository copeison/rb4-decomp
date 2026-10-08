# Orbis render context

The Orbis render system allocates a `0x44890`-byte platform context during
startup and constructs it at `0x8E72B0`. The context owns two primary graphics
frame slots, two compute queues, eighteen compute-context slots, sixteen
transient vertex buffers, GPU timestamps, labels, and command-state caches.

Each graphics frame slot receives a CUE heap sized for 64 slots, a 32 MiB draw
command buffer, a 2 MiB resource buffer, two 4 MiB constant-update/scratch
buffers, and 192 bytes of auxiliary state. `0x8E7AF0` performs this allocation
for both frame slots.

The transient vertex buffers form two banks of all eight mesh formats. Every
buffer reserves space for `0x40000` vertices and uses the shared descriptor
builder at `0x8E1840`. Their constructor and initializer are now named at
`0x8EC7C0` and `0x8EC7E0`.

When compute queues are enabled, the context creates two 4 KiB queue rings
with 256-byte alignment. Queue zero maps pipe 1 at priority 1; queue one maps
pipe 0 at priority 0. It then initializes eighteen compute contexts, each with
a `0x3FFFFC`-byte command buffer and a CUE allocation sized for 64 slots.

The timestamp pool at `0x8E7DF0` allocates 8 KiB and divides it into 512 pairs
of 64-bit timestamps. The label allocator begins with 32 records. The
destructor at `0x8E8070` releases these resources in reverse structural order,
and the deleting destructor at `0x8E82B0` frees the context allocation.

Frame submission at `0x8E82D0` emits event type 22 into the active graphics
command buffer, then processes the active bank's nine compute contexts. Each
context receives a release-memory write to its completion label before it is
submitted. Slots zero through two use compute queue zero; slots three through
eight use queue one. A final graphics release-memory packet writes the frame's
graphics completion label, the graphics context is submitted, and the active
frame index toggles between zero and one.

Active-frame reset at `0x8E8450` rewinds the selected Gnmx graphics context and
emits the default draw and optional dispatch hardware state. When compute is
enabled, it rewinds all nine compute contexts in the active bank and emits the
default dispatch state for each. It also invalidates cached command state,
clears all eight transient-buffer cursors, and emits the default control state
used at the start of the next frame.
