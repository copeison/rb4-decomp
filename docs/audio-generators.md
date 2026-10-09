# Audio generators

Every playing sound is an `AudioGenerator` taken from a fixed pool owned by an
`AudioGeneratorManager`. The base classes live in the engine's audio module
and are declared in `src/audio/core/generators`; only the members the FMOD
generators use are reconstructed.

## AudioGenerator

The 80-byte base has its vtable at `0x18DCD58`:

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x10` | `mManager` | Owning manager. |
| `+0x18` | `mIndex` | Pool index. |
| `+0x1C` | `mState` | 0 init, 2 ready, 3 playing, 4 paused, 5 stopped, 6 stopping. |
| `+0x20` | `mRefCount` | Atomic reference count taken by `LockIfOwned`. |
| `+0x24` | `mHandle` | Encoded handle; `-1` when unused. |
| `+0x28` | `mPoolNode` | Free-list node (`LinkedListSizeTracked`). |
| `+0x40` | `mEmitter` | `AudioEmitterCom` that owns the sound. |
| `+0x48` | `mRenderTarget` | `AudioRenderTarget` that mixes it. |

The 32 virtual slots are, in order: `Pause`, `Continue`, `Stop`, a state
getter, `GetElapsedMs`, `GetTimelineMs`, `GetLengthMs`, `SeekToMs`,
`SetSpeed`, `GetSpeed`, `SetParameter`, `GetParameter`, `SetGain`,
`GetGain`, `SetMute`, `GetMute`, three unnamed queries (the third returns
true only for dialog generators), `Init`, the destructor pair, `Poll`,
`Release`, a plugin-data lookup, three unnamed defaults, `_InitTypeId`,
`Kill`, `GetGeneratorOfType` and a type getter. The map gives
`SetMute(bool)`; this build passes an extra "immediate" flag.

`Stop` asks for a graceful stop that the next `Poll` completes. `Kill`
stops at once. `KillLocked` at `0x406D0` calls `Kill` under a global lock,
and every pool's `SendKillToAllGenerators` uses it.

`GetNewHandle` at `0x40720` builds an active handle from a 14-bit
generation counter, the manager index in bits 24 and up, the pool index in
bits 14 to 23, and the active bit 31. `LockIfOwned` compares the complete
handle, so a stale handle cannot retain a reused slot.

## AudioGeneratorManager

The manager's vtable at `0x18DFF18` has 18 slots: `Play(PlayArgs const&)`,
`Play(Symbol, AudioEmitterCom*, bool)`, `Prepare`, `Init`, `Destroy`, an
unnamed no-op, `GetIndex`, `GetId`, `GetResourceExt`, `LockIfOwned`,
`SendStopToAllGenerators`, `SendKillToAllGenerators`, an active-handle
collector, `_SetManagerIndex`, `_InitGeneratorPool`, `_DeleteGeneratorPool`
and the destructor pair. Its fields are the pool size at `+0x08`, a `CritSec`
at `+0x10`, the free list at `+0x20` and the manager index at `+0x38`. Each
concrete manager stores its typed pool at `+0x40`.

`Init` registers the manager with the sound manager to obtain its index, then
builds the pool. `Destroy` frees the pool only when every voice is idle.

## Shared pool sequence

The five FMOD managers each have their own copy of the same pool code.
`src/audio/fmod/playback/FmodGeneratorPool.h` keeps one source for it:

- `_InitGeneratorPool` allocates the array with `new[]`, calls each voice's
  `Init`, and links it into the free list.
- `_DeleteGeneratorPool` deletes the array only when the free list holds
  every voice.
- The allocation inlined into each `Play` pops the first free voice and binds
  its emitter and render target. A null target selects the default target at
  `0x19C90E8`.
- `Release` clears the active bit and the emitter, then relinks the voice.

`PlayArgs` carries the request. The FMOD generators read its start-paused
and start-muted flags (`+0x10`, `+0x11`), the optional initial gain in
decibels with its fade and post-fade option (`+0x14` to `+0x20`), the route
(1 for a Studio event, 2 for a Studio bus) and route path (`+0x28`,
`+0x30`), the 3D spread (`+0x38`), the render-target name (`+0x40`), the
parameter list (`+0x58`), the format (`+0x60`) and the streaming flag
(`+0x65`).

## Gain ramps

Each FMOD generator keeps two 48-byte linear ramps: one for gain and one for
mute. They are inlined at every use; `GainRamp` names the layout. A ramp
starts at 1000 ms and complete. `SetGain` applies at least 25 ms (the
constant at `0x124D444`) and, for an immediate change outside playback,
snaps to the target. `Poll` advances both ramps by the change in timeline
position. A gain ramp marked "stop after fade" stops the voice once it
completes.

## Generator classes

| Class | Pool entry | Generator vtable | Manager vtable | Extension |
| --- | ---: | ---: | ---: | --- |
| `FmodAudioBusGenerator` | `0x1C0` | `0x18F0208` | `0x18F0370` | `.fmod_bus` |
| `FmodAudioStreamGenerator` | `0x120` | `0x18F0418` | `0x18F0528` | `.mp3` |
| `FmodBufferedStreamGenerator` | `0x238` | `0x18F0668` | `0x18F05C8` | `.mp3` |
| `FmodDialogGenerator` | `0x1A0` | `0x18F08D8` | `0x18F0838` | `.bank` |
| `FmodStudioSoundGenerator` | `0xD0` | `0x18F09F0` | `0x18F0B00` | `.bank` |

See [fmod-audio-bus-generator.md](fmod-audio-bus-generator.md),
[fmod-audio-stream-generator.md](fmod-audio-stream-generator.md) and
[fmod-studio-sound-generator.md](fmod-studio-sound-generator.md).
