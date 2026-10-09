# FMOD stream generators

Two generators play `FmodAudioStreamResource` files. Their sources are in
`src/audio/fmod/playback`.

## FmodAudioStreamGenerator

The generator streams a file on a low-level FMOD channel. Its vtable is at
`0x18F0418`, the manager's at `0x18F0528`, and a pool entry is `0x120` bytes.

`FmodAudioStreamGeneratorManager::Play` at `0x26A270` calls
`PlayWithCallback` (`0x268CA0`) without a callback. That finds the stream
resource registered under the request name and rejects failed resources and
buffered requests (format 3 with streaming set). `_AllocateAndSetUpGenerator`
at `0x268D80` takes a voice and calls `Setup`.

`Setup` at `0x268EF0` resets the channel state, the play timer, the loop
points and the speed. It applies the initial gain, or a unity gain, and the
initial mute. It opens the file as a nonblocking, accurate-time stream; a
voice on an emitter other than the default 2D emitter adds `FMOD_3D`. A bus
route stores the Studio bus and checks its path with the engine bus
interface.

`Poll` at `0x2694D0` drives the voice:

- An invalid channel handle or failed `isPlaying` call moves the voice to
  stopping; a stopping voice stops its channel and returns false.
- A ready voice calls `_TryStartChannel` at `0x269980`. It waits for the
  sound to open and retries a Studio bus that reports
  `FMOD_ERR_STUDIO_NOT_LOADED` up to ten times before playing unrouted. It
  then reads the millisecond and PCM lengths, starts a paused channel with
  `FMOD_LOOP_NORMAL` and no repeats, and starts the play timer unless a pause
  was requested.
- Speed changes multiply the channel's base frequency.
- Loop changes loop between the start and end points, loop from the start
  to the end of the sound when only a start is set, or disable looping.
- The gain and mute ramps advance by the change in timeline position. A
  completed gain ramp marked "stop after fade" stops the voice.
- While playing, the channel volume is the product of the two ramps, and a
  pending seek is applied once the channel position differs.
- A change of the requested pause state pauses or resumes the channel and
  the play timer.
- The 3D attributes follow the emitter.

`GetElapsedMs` reports the play timer and `GetTimelineMs` the last channel
position. `Pause` and `Continue` only record the request. `Kill` at
`0x269D00` releases the sound and marks the voice stopped. `Release` returns
the voice and checks its bus path with the engine bus interface again.

## FmodBufferedStreamGenerator

This generator decodes a stream into its own buffers and renders them as an
`AudioBus` through a nested `FmodAudioBusGenerator`, which keeps buffered
music sample-accurate with the song clock. The primary vtable is at
`0x18F0668`, its `AudioBus` vtable at `0x18F0780`, the manager's at
`0x18F05C8`, and a pool entry is `0x238` bytes. The map has no object for
this class; its name comes from the type string at `0x125CEB5`.

The manager accepts only format-3 requests with streaming set and owns a
`StreamReaderThread` at `+0x48`, started by `Init` and stopped by the
destructor.

The voice controls forward to the nested bus generator: pause, resume, stop,
gain and mute. Speed is stored at `+0x1D8` under the bus lock. `Init` at
`0x26AFF0` prepares the bus for stereo 128-sample blocks at the engine sample
rate.

A request is a `BufferedStreamPlayArgs`: the `PlayArgs` followed by the
block size in frames (`+0x68`), the block count (`+0x6C`) and the number of
blocks kept behind the play position (`+0x70`).

`Setup` at `0x26ADA0` opens the stream without blocking, sizes an
interleaved 16-bit `AudioBuffer<short>` for every block, gives each
`StreamReader` its block (reader *i* starts at frame *i* times the block
size) and takes a voice from the bus generator pool, which renders this
generator's `AudioBus`. `_FinishOpen` at `0x26BAD0`, run by `Poll` until FMOD
reports the stream ready, reads the format, marks mono readers and queues
every reader on the manager's reader thread.

`Process` at `0x26C720` stays silent until the start block has read, then
resamples from the current block. Unsynced playback interpolates linearly
at the speed times the stream-to-output rate ratio. While synced, it uses
the six-point kernel at `0x26D940` (`_InterpolateOptimal`: Olli Niemitalo's
optimal 32x, six-point, fifth-order z-form interpolator) and advances by
`_GetSyncSpeed` (`0x26D8D0`): the smoothed target rate over the output rate,
or zero once the target is reached. Reads wrap in the ring; at the end of
the stream the last frame repeats. Crossing a block boundary moves to the
neighbouring reader, and `_RefillBuffers` (`0x26C480`) then requeues the
readers so up to half the ring is ahead of the current block and the rest
behind it. A failed read or the end of the stream stops the bus generator.

`SeekToMs` at `0x26BDC0` takes every queued reader off the thread, rebuilds
the ring around the target and waits for the block at the target. Loop
points (`0x26C0A0`, `0x26C0F0`) take effect at the next `Process`; only
`_SetReaderPosition` (`0x26B650`) can split a block at the loop end, and no
caller asks it to.

`EnableSync` (`0x26DAB0`) starts following a target 30 ms (`0x19B4930`)
ahead of the timeline; `SetSyncTargetMs` (`0x26DBA0`) moves it and
`DisableSync` (`0x26DBD0`) stops. `_UpdateSync` (`0x26BC70`, inlined into
`Poll`) measures the target's rate against the time manager's clock, clamps
it to 20 times the stream rate and feeds a `DoubleExponentialSmoother`.
None of these controls has a caller in this build.

`Release` at `0x26C1B0` releases the bus generator, cancels the readers,
frees the buffer and the sound and returns the voice to the pool. The empty
functions at `0x26BD30`, `0x26D8B0` and `0x26D8C0` have no references and
are not modelled.
