# FmodAudioBusGenerator pool

`FmodAudioBusGeneratorManager` is named by the embedded type string at
`0x125CCA5`. Its vtable begins at `0x18F0370`, and the associated generator
type string at `0x125CCCC` confirms the pooled object as
`FmodAudioBusGenerator`.

The manager owns a fixed array of `0x1C0`-byte generator objects. Initialization
at `0x268910` constructs every slot, gives it the manager and its pool index,
and places its intrusive link on the manager's free list. Shutdown at
`0x268A60` destroys the array only when every slot has returned to that list.
The check prevents active voices from being destroyed with the pool.

Allocation at `0x268B40` removes the first free slot, stores the sound source
and selected audio state, and creates a new handle. A null audio-state argument
selects the global default. Returning a voice at `0x268380` clears the handle's
active bit, drops the sound-source pointer, and appends the slot to the free
list if it is not already present.

Handles are signed 32-bit values. Bit 31 marks an active handle, bits 14 through
23 hold the pool index, and bits 0 through 13 are a generation counter. The
retain path at `0x268660` checks both the supplied index and complete handle
before incrementing the voice reference count. This prevents a stale handle
from retaining a slot that has since been reused.

The manager serializes pool mutations with a recursive mutex. It can dispatch
the per-voice preparation method across the entire array at `0x2686D0`, stop
every voice synchronously at `0x268740`, and collect all handles whose active
bit is set at `0x2687B0`. The small accessors at `0x268510` and `0x268900`
read and write the manager field at offset `0x38`.

The higher-level creation paths at `0x268CA0`, `0x268D80`, and `0x268EF0`
resolve sound data, take a free generator, initialize its playback controls,
create the FMOD sound, and optionally route a Studio bus. Their detailed sound
and option layouts remain the next reconstruction step.
