# Audio render targets

## Audio properties

`Audio::SetAudioSystemProperties` at `0xD3BC0` caches the engine's sample
rate, seconds per sample, buffer size, buffers per second and milliseconds
per buffer in globals. `GetSamplesPerSecond` (`0xD3BA0`) and
`GetSecondsPerSample` (`0xD3BB0`) read them. The defaults are 48 kHz with
1,024-sample buffers. The source is `src/audio/core/system/Audio.cpp`, which
also holds the engine's recursive `CritSec`.

## AudioRenderTarget

A render target is a named output that mixes generators. `FModSystem` and the
recording targets derive from it. The vtable is at `0x19A0108`, the
constructor at `0x1127FC0`, and the object is `0x118` bytes. The class name is
not in the map. The engine registry at `0x19C90B0` finds targets by name for
playback requests.

Its fields are the name at `+0x08`, the type at `+0x10` (1 for an
`FModSystem`, 2 for a recording target), the `AudioMixer` at `+0x18`, the
voice pool at `+0x98`, the premix callback list at `+0xB0` under the lock at
`+0xA0`, the sample rate at `+0xC8`, the buffer size and count at `+0xCC` and
`+0xD0`, the raw speaker count at `+0xD8`, and a map of CPU timers at `+0xE0`.

FMOD generators resolve the `FModSystem` behind a target from its type: the
target itself for type 1, or the system embedded at `+0x1E0` for type 2.

## AudioMixer

The mixer at `+0x18` feeds its `AudioBusCallable` clients in 128-sample
blocks. Its vtable is at `0x19A00E0`. `Mix` at `0x1127880` first promotes the
callables queued since the last mix, then, for each block, calls
`_PrepareToMakeSamples` on every callable and `_MakeSamples` on each one once.
An atomic guard in each callable makes the second pass run at most once per
block. The original assumes the buffer length is a multiple of 128.

## AudioBus

`AudioBus` is the source a bus generator renders (vtable `0x18E5A78`,
`0xC0` bytes). The map names its `Prepare` and `Process` members and five
inline lock helpers. Slot 2 is `Prepare`, slot 4 `Process`, and slots 5 and 7
lock and unlock the bus.

## Recording targets

`RecordingAudioRenderTarget` (vtable `0x19A01A0`, `0x1E0` bytes) records its
mix offline on a worker thread. `FmodRecordingAudioRenderTarget` at
`0x275E20` adds an `FModSystem` at `+0x1E0` that renders through
`HMX.BufferedOutput`, and a worker thread at `+0x520`. Neither name is in the
map; the worker thread is named `RecordingAudioRenderTarget`, and no caller
constructs the FMOD target in this build.

Construction initializes the embedded system with output type 1 and two
buffers, sets the speaker configuration, initializes buffered output, and
binds the output callback that calls `readfrommixer` with the recording
buffer and buffer size. Most virtual methods forward to the embedded system;
`Lock` and `Unlock` take both objects' locks. `StartAsyncRecording` at
`0x276080` runs the generic recording loop on a thread using the
`async_audio_record` task settings.
