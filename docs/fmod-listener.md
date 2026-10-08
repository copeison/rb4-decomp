# FMOD listener transform bridge

The helper at `0x27ACB0` converts a 48-byte engine transform into FMOD's
48-byte `FMOD_3D_ATTRIBUTES` structure. The cleaned implementation is in
`src/audio/fmod/system/fmod_listener.cpp`.

The engine transform stores four consecutive three-float vectors: right,
forward, up, and position. FMOD receives position, velocity, forward, and up.
The right vector is not copied, and the listener velocity is cleared to zero.

The engine and FMOD use opposite handedness on the X axis. Position, forward,
and up therefore copy Y and Z directly while negating X. The same converter is
also used when creating Studio event instances and when updating low-level
channel attributes.

`audio_update_listener_attributes` at `0x262300` applies the converted block to
Studio listener index 0 only when listener updates are enabled and the global
FMOD audio state has a valid Studio system. The original listener-enable flag
is at byte offset 14 in its owning object; the cleaned function accepts that
state as a named Boolean until the rest of the owner is reconstructed.
