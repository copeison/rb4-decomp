# Render backend resource

The render system owns one optional 344-byte backend resource through the
pointer at offset `0xDE8`. Creation at `0x451C90` allocates that exact typed
size, constructs its base state, and registers its intrusive link with the
resource manager before publishing the pointer.

The base construction and registration algorithms remain one focused adapter
because they depend on the still-partial internal backend-resource layout.
Allocation ownership is direct. Release at `0x451CC0` invokes dynamic vtable
slot `0x08` and clears the owning render-system pointer immediately.
