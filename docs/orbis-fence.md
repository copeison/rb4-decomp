# Orbis fence

The Orbis resource-factory vtable creates a fence through
`orbis_create_fence` at `0x8D85C0`. The factory allocates a 24-byte object and
invokes `orbis_fence_construct` at `0x8E1570`.

The constructor allocates one four-byte GPU-visible value named `PS4Fence`
with alignment value 4 and initializes it to zero. The object retains that
allocation as its fence value storage.

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

The render-context methods at `0x8EB730` and `0x8EB7F0` signal and wait for the
current sequence. Graphics recording uses a release-memory packet and a
`WAIT_REG_MEM` address comparison. Standalone compute recording emits the
corresponding compute release-memory packet and `sceGnmComputeWaitOnAddress`.
Both waits compare all 32 bits and use the equality comparison mode.
