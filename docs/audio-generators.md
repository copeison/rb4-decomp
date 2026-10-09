# Audio generators

Every playing sound is an `AudioGenerator` taken from a fixed pool owned by an
`AudioGeneratorManager`. The base classes live in the engine's audio module
and are declared in `src/audio/core/generators`; only the members the FMOD
generators use are reconstructed.

## AudioGenerator

The 80-byte base has its vtable at `0x18DCD58`:

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x08` | `mName` | Sound name, stored by the sound manager's play. |
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
`GetGain`, `SetMute`, `GetMute`, `IsMusic`, `IsInstrument`, an unnamed
dialog query, `Init`, the destructor pair, `Poll`,
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
`Play(Symbol, AudioEmitterCom*, bool)`, `Prepare`, `Init`, `Destroy`, the
empty `Poll`, `GetIndex`, `GetId`, `GetResourceExt`, `LockIfOwned`,
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

## Emitter interface

`AudioEmitterCom` in the source is the emitter interface that the audio
emitter component keeps at `+0x228` (vtable `0x18DF628`). Its 29 slots
forward to the component, and their names come from the map's
`AudioEmitterCom` members: `GetCompositeGenerator`, `RegisterTempoListener`,
`UnregisterTempoListener`, the `PlaySound`, `PrepareSound`, `PlayMusic` and
`PrepareMusic` overloads, `StopAllSounds`, `KillAllSounds`,
`PauseAllSounds`, `ContinueAllSounds`, `GetMasterMusic`, `GetMixGroup`,
`GetWorldXfm`, two `PlayDialog` overloads, the dialog queries and setters,
`Is2D`, `Is3D`, `Set2D`, `Set3D` and `GetComponent`. The names the map lacks
are marked in the header.

`PlayArgs` (104 bytes) carries the request. `mGlobalSoundHandle` (`+0x48`)
registers the new handle with the state graph's global sound handles;
`mReservedName` (`+0x50`) is never read.

## Reconstructed core

The platform-neutral generator code is reconstructed in
`src/audio/core/generators`:

- `AudioGenerator.cpp` is the map's `audio/AudioGenerator.o`: the manager's
  `Init` (`0x40500`), `Destroy` (`0x40540`), `Play` (`0x40570`), `Prepare`
  (`0x406C0`) and destructor (`0xE780`, reached from `0x40AC0`),
  `KillLocked`, `GetNewHandle`, the handle release check at `0x40770`,
  `RegisterTempoListener` (`0x407C0`) and `UnregisterTempoListener`
  (`0x407E0`), which forward to the emitter, four event-parameter
  queries forwarded to the FMOD platform (`0x40800` to `0x40890`), and the
  `PlayArgs::Route` descriptions at `0x408C0`. The `AudioGenerator`
  defaults at `0xE3E0` to `0xE5F0` and its destructor are emitted with the
  sound manager in the map; they are kept in the same file.
- `AudioBusGenerator.cpp` holds the bus generator (`0xE0490` to `0xE1510`).
  Its block buffer is an `AudioBuffer<float>`; a rendered block sets a flag
  in the buffer's tail padding. Gain and mute use a 48-byte per-block ramp
  (`BlockRamp`) that differs from the FMOD generators' `GainRamp`.
  `_ResetGainAndMute` at `0xE0900` is on `FmodAudioBusGenerator` in the map.
- `DialogGenerator.cpp` holds the dialog base (`0x1127330` to `0x1127730`).
  `PlayArgs` ends at `0x68` bytes; dialog requests are a `DialogPlayArgs`
  marked by format 4, whose sink is a `std::function` at `+0x70`.

`PlayArgs` gained its inlined constructor and destructor: the route path is a
`Symbol`, and the parameter list is an owned `eastl::vector`.
`LinkedListSizeTracked` nodes and lists gained the inlined constructors and
destructors that every owner's destructor repeats.

The sound manager, which registers the managers and resolves handles, is
described in `docs/sound-manager.md`.

## CompositeGenerator

`CompositeGenerator.cpp` is `audio/CompositeGenerator.o` (`0x40AF0` to
`0x41E6B`, vtable `0x18DFFB8`, 160 bytes). Every emitter owns one as the
parent of its sounds, and `SoundManager::PlaySound` groups several results
under one. Its fields are an unused second list node (`+0x50`), the child
list (`+0x68`), the pending list (`+0x80`), the last gain (`+0x98`, one by
default) and the last mute flag (`+0x9C`). Children link through their pool
node.

`AddGenerator` only tries the kill lock: when another thread holds it, the
child goes to the pending list under `mCompositeGenChildListLock`
(`0x19C8498`). Every forwarding call takes the kill lock and first moves the
pending children (`_AddPendingChildren`). `Pause`, `Continue` and `Stop`
forward and set the state; `GetElapsedMs` and `GetTimelineMs` both return
the largest child `GetElapsedMs`; `SetParameter` reports whether any child
took the value, `GetParameter` the first child that has it. `GetGain` reads
the first child, or the stored gain without children. `Poll` releases the
children that finished and whose handle can be deactivated, and stops
once none is left. `Kill` kills each child; a child still locked by a
caller moves to the default emitter's composite generator unless this
generator belongs to that emitter. `Release` is the map's inline
`AudioGeneratorManager::FreeGenerator`. `kTypeId` is at `0x19C8480`.

`CompositeGeneratorManager` (`0xDD10` to `0xE3C0`, vtable `0x18DCCB8`) has
the shared pool code; `Play` returns null, the resource extension is
`.--none--`, and `kIdStr` (`0x19B0098`) is unreferenced. The map emits the
manager in `SoundManager.o`; the source keeps it with the generator.

Slots 25-27 are now `SetPlayScale`, `GetPlayScale` and
`GetPrimaryStreamValue`. Only the Fusion and music generators override
them: the scale defaults to one, a Fusion play request sets it and the music
generators forward it to their streams. These names are weak.
