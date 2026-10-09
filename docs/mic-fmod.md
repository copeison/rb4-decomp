# FMOD microphones

The microphone classes belong to the `mic` module and live in `src/mic`.
This build's mic module differs from the map's: the mic list is a
platform-neutral `MicHwManager` that owns the mics, and a single reader
thread polls every mic.

## MicHwManager

`gMicHwManager` (`0x19C8FC0`, vtable `0x18E5070`, 128 bytes) is built by the
static initializer at `0xA28D0`. It holds the mics, change listeners and
platforms in three EASTL vectors under one lock (`0xA1C20` to `0xA2800`).
`Poll` notifies the listeners after a change, polls the platforms and runs
each mic's `Poll`. `Terminate` stops the reader thread, shuts the platforms
down and deletes the mics. `CaptureMic` and `ReleaseMic` claim a mic through
its in-use flag; their index check accepts the mic count itself.

`MicReaderThread` (`0xA29E0` to `0xA2D00`) is a single "mic_reader" thread
started by the first platform. Every 11 ms it runs each mic's
`MicThreadPoll`, and every 100 passes the platforms' connection checks.

`MicHwPlatform` is the base of `MicHwManager_FMOD`; its non-virtual wrappers
at `0xA2D70` to `0xA2DE0` initialize, poll, shut down and check a platform.
The name is not in the map.

## Mic

`Mic` (vtable `0x18E63E0`, 16,600 bytes, `0xE1530` to `0xE1B00`) keeps a
recent ring of 8,192 samples for analysis and a continuous ring of 16,384
for readers (`RingBuffer`, inline in this build). `Poll` (`0xE1810`) runs the
platform's `_Poll`, which stores new samples, and, while analysis is on,
runs the `PitchDetector` over the latest 8,192 samples and smooths its
energy into a level that rises faster than it falls. The detector's outputs
are the MIDI pitch, the energy, the window level and the input envelope's
peak, which `Mic` stores as `mPitch`, `mLevel`, `mWindowLevel` and `mInputPeak` (see
[audio-dsp.md](audio-dsp.md)).

## MicHwManager_FMOD

The FMOD platform object is a 56-byte singleton at `0x19F2EE0` with its
vtable at `0x18F0CF0`. It registers with the engine mic list and owns the
Studio buses that carry the mic signal. Earlier reconstructions called it
`FmodAudioInputManager`.

Each bus route is 24 bytes: the bus path, the `FMOD::Studio::Bus*` and the
volume authored in the bank. `SetBusPaths` at `0x2754C0` replaces the routes
and `_BindBuses` at `0x275630` resolves each path and reads its volume. A
failure unbinds the routes bound so far and sets a retry flag that `_Poll` at
`0x2758A0` services. `SetBusVolume` multiplies the requested volume by the
authored one; `SetBusMute` and `GetBusChannelGroup` complete the controls.

`_SetupMicArray` at `0x275830` starts the mic reader thread and adds one
`Mic_FMOD` per slot. Earlier reconstructions read the first call as
emptying the mic list.

`_CheckConnectsAndDisconnects` at `0x275950` first asks every bound FMOD mic
whether its driver is still connected. It then enumerates the connected
record drivers whose names pass `_IsValidMicName` (they end in `GENERAL`),
skips names already bound, and attaches each new driver to a free FMOD mic.
Every change flags the mic list as changed.

## Mic_FMOD

A 16,720-byte microphone on one FMOD record driver; the vtable is at
`0x18F1208`. Earlier reconstructions called it `FmodAudioInputDevice`. The
constructor at `0x27B3E0` records the slot index, sets no driver, and starts
at 48 kHz.

| Slot | Method | Address |
| ---: | --- | --- |
| 2 | Attachment status (2 when bound) | `0x27C7C0` |
| 3 | `GetType` (1 for FMOD) | `0x27C7D0` |
| 4 | `IsRunning` | `0x27C7E0` |
| 7 | `MicThreadPoll` | `0x27C300` |
| 9 | `GetName` | `0x27C7F0` |
| 10 | `Start` | `0x27BC50` |
| 11 | `StartPlayback` | `0x27BB20` |
| 12 | `Stop` | `0x27BE70` |
| 13 | Apply volume | `0x27BF40` |
| 14 | Apply mute | `0x27BF60` |
| 15 | `_Poll` | `0x27BF80` |

`Start` (`0x27BC50`) records into a looping half-second stereo user sound.
`StartPlayback` (`0x27BB20`) also plays the mic through a Studio bus or a
new event instance. `_Poll` copies the left channel of newly recorded frames
into the rings. `MicThreadPoll` starts playback of the record buffer once
the target latency is recorded, then measures how far playback trails
recording, smooths it (0.97/0.03) and changes the playback frequency by 1%
to keep it near the target; past the maximum latency it speeds up in
proportion to the excess. A failed FMOD call detaches the mic.

`AttachToHardware` at `0x27B780` binds a driver only while FMOD still reports
the expected name, and takes its sample rate. `CheckDeviceStillConnected` at
`0x27B880` finds the driver by name; a missing or disconnected driver releases
the mic and clears its name. `Stop` stops recording, releases the sound and
any playback event.
