# Orbis fence

The Orbis resource-factory vtable creates a fence through
`orbis_create_fence` at `0x8D85C0`. The factory allocates a 24-byte object and
invokes `orbis_fence_construct` at `0x8E1570`.

The constructor allocates one four-byte GPU-visible value named `PS4Fence`
with alignment value 4 and initializes it to zero. The object retains that
allocation as its fence value storage.

Defining this factory also exposed ten neighboring Orbis resource-factory
boundaries that IDA had previously treated as unowned code. They remain with
address-based names until their resource types are supported by stronger
strings or call-site evidence.
