# Orbis fence

The Orbis resource-factory vtable creates a fence through
`orbis_create_fence` at `0x8D85C0`. The factory allocates a 24-byte object and
invokes `orbis_fence_construct` at `0x8E1570`.

The constructor allocates one four-byte GPU-visible value named `PS4Fence`
with alignment value 4 and initializes it to zero. The object retains that
allocation as its fence value storage. The recovered object layout contains
the virtual dispatch pointer at offset `+0`, the GPU value pointer at `+8`, and
the 32-bit sequence at `+16`; the final four bytes are unused by the recovered
methods. `static_assert` keeps the typed reconstruction at exactly 24 bytes.

The complete destructor at `0x8E15F0` and base destructor at `0x8E1640`
release that value through the renderer's deferred-allocation queue while an
Orbis render system exists. If renderer construction has not completed or its
shutdown has already cleared the global instance, they release the allocation
immediately instead. Both clear the stored pointer after release.

The deleting destructor at `0x8E1680` follows the same conditional release and
then frees the 24-byte fence object. Its two virtual-table entries are the
complete and deleting destructors, which confirms that the fence has no other
virtual operations in this class.

Defining this factory also exposed ten neighboring Orbis resource-factory
boundaries that IDA had previously treated as unowned code. They remain with
address-based names until their resource types are supported by stronger
strings or call-site evidence.

`orbis_fence_next_value` at `0x8E16D0` advances the 32-bit sequence associated
with the label. When the sequence reaches `FFFFFFFF`, it retires the old label,
allocates and zeroes a replacement, and restarts at one. Replacing the label
avoids making an in-flight wrap indistinguishable from an already completed
fence value.

The render-context methods at `0x8EB730` and `0x8EB7F0`
(`PS4Context::_SignalFenceImpl` and `_WaitFenceImpl`) signal and wait for the
current sequence on the active pipe:
- **Graphics** signals with `GfxContext::writeAtEndOfPipe(kEopCbDbReadsDone,
  ...)`, writing the next value as a 32-bit immediate.
- **Compute** signals with `ComputeContext::writeReleaseMemEvent(
  kReleaseMemEventCsDone, ...)`.

Both waits use `waitOnAddress` with a full 32-bit mask and
`kWaitCompareFuncGreaterEqual`, so they wait until the label reaches the
fence's sequence.

Combined IDA evidence is preserved in `analysis/exports/orbis-fence.asm` and
`analysis/exports/orbis-fence.c`.
