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

The manager serializes pool mutations with a recursive mutex. Before an audio
reset, `0x2686D0` dispatches the per-voice preparation method across the entire
array. The default bus-generator mode marks live voices as stopping; a separate
mode `1` survives and recalculates its timing step from source metadata. The
manager can stop every voice synchronously at `0x268740` and collect all
handles whose active bit is set at `0x2687B0`. The small accessors at
`0x268510` and `0x268900` read and write the field at offset `0x38`.

The higher-level creation paths at `0x268CA0`, `0x268D80`, and `0x268EF0`
resolve sound data, take a free generator, initialize its playback controls,
and create an asynchronous FMOD sound. The creation mode is `0x14080` for a
normal 2D source and `0x14090` for a spatial source. A Studio-bus route stores
the bus immediately and registers its resolved path with the engine.

`fmod_audio_bus_generator_try_start_sound` at `0x269980` polls the asynchronous
open state. It also asks a selected Studio bus for its channel group, retrying
result 76 up to ten times before falling back to the default group. Once the
sound reports ready, the generator reads its millisecond and PCM lengths,
starts a paused low-level channel, enables normal looping with loop count zero,
captures the base frequency, applies the initial volume, and finally enters
the requested playing or paused state.

Several option fields used for gain conversion and transition envelopes in
`0x268EF0` still lack stable engine names. The cleaned source exposes the
confirmed start-paused, start-silent, route, and route-path fields while the
remaining controls stay in the focused decompilation export.

The runtime update at `0x2694D0` treats `FMOD_ERR_INVALID_HANDLE` as a lost
channel and moves the generator toward stopped state. A ready generator polls
the asynchronous startup helper, while a stopping generator stops and clears
its channel. Active playback refreshes volume and millisecond position, applies
a queued seek, synchronizes the requested paused state with FMOD, and updates
3D attributes from the engine transform. The binary also advances two generic
transition envelopes in this method; their callback payload types remain
unnamed and are retained in the focused export.
