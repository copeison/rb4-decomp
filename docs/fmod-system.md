# FModSystem

`FModSystem` owns the engine's FMOD Studio and low-level systems and runs the
engine's work around each FMOD mix. It derives from `AudioRenderTarget` (see
[audio-render-target.md](audio-render-target.md)). The vtable is at
`0x18F0E40` and the object is `0x340` bytes. The map places the class in the
`FmodPlatform` object; the source is in `src/audio/fmod/system/FmodPlatform.cpp`.
The primary system is created by the platform audio object at `0x262110`
under the name `fmod_primary` and stored at `0x19F29D8` (`FModSystem::Get`).

## Layout

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x118` | `mStudioSystem` | `FMOD::Studio::System`. |
| `+0x120` | `mLowLevelSystem` | `FMOD::System`. |
| `+0x128` | `mDspBufferLength` | FMOD's DSP buffer length. |
| `+0x12C` | `mSpeakerMode` | Custom-output speaker mode. |
| `+0x130` | `mMaxChannels` | Studio channel limit. |
| `+0x134` | `mShuttingDown` | Blocks the mix callback. |
| `+0x140` | `mBufferedOutputCallback` | `std::function` run by `HMX.BufferedOutput`. |
| `+0x178` | `mMixCount` | Number of FMOD mixes. |
| `+0x180` | `mHmxTimer` | CPU timer named `hmx`. |
| `+0x1C8` | `mFmodTimer` | CPU timer named `fmod`. |
| `+0x210` | `mBufferSetTimer` | Rolling CPU timer named `buffer_set`. |
| `+0x278` | `mSourceTimers` | Per-source timers (`mogg`, `vibe`, `fusion`). |
| `+0x2A0` | `mInMix` | Set between premix and postmix. |
| `+0x2A8` | `mMixSemaphore` | Held across each mix. |
| `+0x2C0` | `mDeferredCritSec` | Guards the deferred releases. |
| `+0x2D0` | `mDeferredReleases` | Two EASTL vectors of channel and DSP pairs. |
| `+0x310` | `mDeferredBuffer` | Index of the active vector. |
| `+0x318` | `mDeferredReleaser` | Premix callback that drains them. |

## Initialization

`Init` at `0x276F30` names the target, creates the semaphore, stores the
buffer length, buffer count and channel limit, creates the timers
(`_InitTimers` at `0x276FD0`), optionally runs `InitFmod`, and registers the
deferred releaser as a premix callback. The map gives `Init(bool)`; this build
passes the name, output type, buffer settings and channel counts.

`InitFmod` at `0x2773C0` creates the Studio system with header version
`0x00011004` (FMOD 1.10.04). It stores the system as user data on both FMOD
objects and requests the output type, falling back to autodetect. It then
raises three settings: the mixer stack grows by 16 KiB and both command
queues are multiplied by four. After `initialize` succeeds it installs the
`FmodFileWrapper` file callbacks, reads the driver's sample rate and the DSP
buffer length, sets the engine's audio properties
(`Audio::SetAudioSystemProperties`), registers the custom DSPs and enables the
premix and postmix callbacks.

`RegisterPlugins` at `0x278270` registers twelve DSP descriptions in this
order: `HMX.Analysis`, `BitCrusherPlugin`, `DelayPlugin`, `FilterPlugin`,
`FmodGainPlugin`, `HMX.SignalTap`, `SmbPitchShift`, `Hmx.Stutter`,
`Hmx.SoundClashSlot`, the tremolo, `HmxVibePlugin` and `HmxWahPlugin`. Each
getter is declared on its plug-in class in `src/audio/fmod/mixing`.

`InitWithStudioSystem` at `0x277760` adopts a Studio system created
elsewhere. `SetStudioSystem` at `0x277840` reads its format, rebuilds the
mixer sample rate and plugins, and enables the callbacks; a null system takes
the `Terminate` path at `0x277A80`, which waits for the mix semaphore, marks
the system shutting down, empties both deferred-release vectors and drops
the FMOD pointers.

`InitBufferedOutput` at `0x2786D0` creates a Studio system that renders
through the `HMX.BufferedOutput` plug-in instead of the console output. It
applies the stored buffer size, channel limit and speaker format, and
initializes with synchronous Studio updates plus the stream-from-update,
mix-from-update and right-handed flags.

`SetSpeakerConfig` at `0x2783A0` maps the engine's four configurations
through the tables at `0x125D1F0` and `0x125D200`:

| Configuration | Speaker mode | Raw speakers |
| --- | --- | ---: |
| 0, mono | `FMOD_SPEAKERMODE_MONO` | 1 |
| 1, stereo | `FMOD_SPEAKERMODE_STEREO` | 2 |
| 2, 5.1 | `FMOD_SPEAKERMODE_5POINT1` | 6 |
| 3, 7.1 | `FMOD_SPEAKERMODE_7POINT1` | 8 |

## HMX.BufferedOutput

The output plug-in's callbacks are at `0x2763F0` through `0x276520`. It
reports 128 drivers named `null_output_%d`, each at the engine sample rate in
stereo. Initialization stores the `FModSystem` passed as extra driver data,
selects float output and copies the speaker mode and raw speaker count. Its
update callback runs `mBufferedOutputCallback`; the recording target sets it
to read one mixer block into its buffer.

## Mixing

`_SystemCallback` at `0x2783E0` ignores calls while the system is shutting
down or has no Studio system.

- On premix it waits for the mix semaphore, increments the mix count, starts
  the `hmx`, `fmod` and `buffer_set` timers, resets the per-source timers and
  runs `ExecutePremixCallbacks`. The `hmx` timer stops when they return.
- On postmix, if a mix is in progress, it stops the `fmod` and `buffer_set`
  timers and every source timer, clears the flag and releases the semaphore.

`ExecutePremixCallbacks` at `0x2781C0` calls `ExecutePremix` on every
registered `FmodPremixCallback` under the premix lock, then has the mixer
render the buffer in 128-sample blocks.

The CPU timers (`AudioCpuTimer`, vtable `0x18F0EF0`) accumulate milliseconds
under a spin lock. Their averages and maxima are percentages of one audio
buffer (`Audio::sMsPerBuffer`). The rolling timer (vtable `0x18F0F28`) records
the sum of the last few buffers and divides by the window size.
`GetCPUPercent` at `0x277BA0` snapshots and resets every timer; it fills
EASTL maps and is not reconstructed.

## Deferred releases

FMOD may still be mixing a channel when a generator stops it.
`DeferRelease` at `0x278A00` appends the channel and DSP to the active vector
while a Studio system exists. The `DeferredReleaser` premix callback at
`0x278DF0` swaps the vectors under the lock, then stops each channel and
releases its DSP.

## Coordinates

`Convert` at `0x27ACB0` builds `FMOD_3D_ATTRIBUTES` from an engine
`Transform`: position from the translation, forward and up from the second and
third matrix rows, each with X negated, and zero velocity. The platform object
applies it to Studio listener 0 at `0x262300`.
