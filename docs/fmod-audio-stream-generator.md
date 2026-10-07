# FmodAudioStreamGenerator lifecycle

The embedded names at `0x125CD08` and `0x125CD2D` identify
`FmodAudioStreamGeneratorManager` and `FmodAudioStreamGenerator`. The generator
vtable begins at `0x18F0418`, the manager vtable begins at `0x18F0528`, and the
manager's extension method returns `.mp3`.

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

The generator methods occupy `0x2692B0` through `0x26A26A`. Startup at
`0x269980` polls `FMOD::Sound::getOpenState`, retries Studio-bus result 76 up to
ten times, and falls back to the default channel group if the bus stays
unavailable. Once the sound is ready, it reads both millisecond and PCM
lengths, starts a paused channel, enables looping, captures the base frequency,
applies volume, and enters the requested playing or paused state.

The runtime update at `0x2694D0` handles invalid FMOD channel handles, pending
startup, stopping, volume transitions, playback-rate changes, loop-point
changes, millisecond seeks, pause synchronization, and 3D attributes. The
small virtual methods at `0x269BF0` through `0x269D37` expose pause/resume,
position, playback rate, loop points, audio-reset preparation, and stop
behavior. Returning a generator to its pool at `0x269FB0` also unregisters its
Studio-bus path.

The adjacent block beginning at `0x26AB60` is the larger
`FmodBufferedStreamGenerator`, rather than this 288-byte pool. Its initialized
object uses fields through offset `0x234`, creates decoder buffers, and acquires
a nested `FmodAudioBusGenerator`. It is reconstructed separately in
`src/audio/fmod_buffered_stream_generator.cpp`.
