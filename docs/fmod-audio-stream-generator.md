# FmodAudioStreamGenerator pool

The embedded names at `0x125CD08` and `0x125CD2D` identify
`FmodAudioStreamGeneratorManager` and `FmodAudioStreamGenerator`. The manager
vtable begins at `0x18F0528`, and its extension method returns `.mp3`.

This manager uses the same handle scheme and synchronized free-list design as
the bus-generator manager. Its pool entries are `0x120` bytes rather than
`0x1C0` bytes. Initialization at `0x26A690` constructs the fixed array, assigns
each generator its manager and index, and links every entry into the free list.
Shutdown at `0x26A850` succeeds only when the free-list count equals the pool
capacity.

`fmod_audio_stream_manager_retain` at `0x26A3D0` checks the complete encoded
handle before incrementing the entry reference count. This gives stream and bus
handles the same stale-generation protection. The manager can prepare all
streams for an audio reset at `0x26A450`, synchronously stop them at `0x26A4C0`,
and collect active handles at `0x26A530`.

The creation path at `0x26AB60` accepts only the stream-specific option form
whose format field is 3 and whose stream flag is set. `0x26AC30` then removes a
free entry, selects the requested or default audio state, assigns the sound
source, creates a fresh handle, and continues into stream initialization at
`0x26ADA0`.
