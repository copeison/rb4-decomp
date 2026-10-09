# FMOD microphones

The microphone classes belong to the `mic` module and live in `src/mic`.
The engine's mic list (`MicHwManager`, at `0x19C8FC0`) and the `Mic` base
class are not reconstructed; `src/mic/core` declares only what the FMOD
classes use.

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

`_SetupMicArray` at `0x275830` empties the mic list and adds one `Mic_FMOD`
per slot.

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
| 15 | `StopPlayback` | `0x27BF80` |

`AttachToHardware` at `0x27B780` binds a driver only while FMOD still reports
the expected name, and takes its sample rate. `CheckDeviceStillConnected` at
`0x27B880` finds the driver by name; a missing or disconnected driver releases
the mic and clears its name. `Stop` stops recording, releases the sound and
any playback event.
