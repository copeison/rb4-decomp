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

The decoder itself is not reconstructed yet. `Setup` at `0x26ADA0` opens the
sound, builds a vector of `0xC0`-byte stream blocks and acquires the bus
generator; `0x26BAD0` completes the open and submits every block to the
stream reader. `SeekToMs` at `0x26BDC0` waits for idle blocks before
resubmitting. `Process` at `0x26C720` renders the decoded PCM: its normal
path interpolates linearly between interleaved 16-bit frames, and the
synchronized path uses the six-point kernel at `0x26D940`. That kernel is
reconstructed as `_InterpolateOptimal`: Olli Niemitalo's optimal 32x,
six-point, fifth-order z-form interpolator, with float-rounded coefficients.
The controls at `0x26DAB0`, `0x26DBA0` and `0x26DBD0` enable synchronization,
set its target and disable it.
