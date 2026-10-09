# Sound manager

`audio/SoundManager.o` occupies `0x980` to `0x8E53` in this build, with its
static initializer at `0xF410`. It is reconstructed in
`src/audio/core/system/SoundManager.{h,cpp}`. The object is much smaller than
the map's: the VCA and master faders (`FadersControl`, `MasterFader`,
`VcaFader`), the CPU queries and the PS4 music-output routing are gone.

## Layout

`theSoundManager` is at `0x19C5638`. It is a `MsgSink` with its vtable at
`0x18DC250`: the destructor pair (`0xB60`, `0xBD0`), `Handle` (`0x8E00`) and
`MsgSink::GetSinkObject`. The object is 288 bytes:

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x08` | `mStandardGenManagersInitialized` | Set by `InitStandardGenManagers`. |
| `+0x10` | `mManagersByExt` | Registered managers by resource extension. |
| `+0x48` | `mManagers` | Registered managers; handle bits 24-30 index them. |
| `+0x68` | `mCompositeGenMgr` | Pool of 256 `CompositeGenerator`s. |
| `+0x70` | `mDefaultEmitterResource` | Entity of the default 2D emitter. |
| `+0x78` | `mDefault2DEmitter` | Emitter used when a request names none. |
| `+0x80` | `mListenerXfm` | Listener transform from `UpdateActiveListener`. |
| `+0xB0` | `mLanguage` | Language of localized banks, `"eng"` by default. |
| `+0xB8` | `mJoypadEmitters` | `JoypadEmitterAndListenerEntry` list. |
| `+0xD8` | `mPaused` | Last state set through the pause bus. |
| `+0xDC` | `mGenManagerConfig` | Pool sizes and voice settings. |

The object's statics follow it: the threaded-poll flag, a `Semaphore`, the
stop flag and the `NamedThread` of the default emitter poll, the
`AddSoundHandle` hook (`0x19C5800`), the default `GenManagerConfig`
(`0x19C5810`), the managers created by name (`0x19C5850`), the Fusion
manager (`0x19C5888`) and three 512-byte paths (`0x19C5890`).

## Startup and shutdown

`App::Initialize` calls the static `SoundManager::Init` at `0xD50`. It reads
`-fmod_live_update`, registers the script functions, starts the mics and
the FMOD platform, runs the resource and component types' inline `Init`
functions, creates the default emitter and the composite manager, publishes
the `theSoundManager` script variable, registers the
`sound_manager game_wide_emitter_names` and adds `Terminate` (`0x14D0`) as
an exit callback. An overload at `0xC40` reads the same settings from a
configuration array; nothing calls it.

`InitStandardGenManagers` (`0x6000`, called by the time-stretch setup)
creates the FMOD managers through the platform and the Mogg, MoggMusic,
Fusion, MultiFusion, SynthRack, MidiMusic and MusicTimeline managers through
`InitGeneratorManager<T>`. Every manager's `Init` registers it back through
`_RegisterGeneratorManager`, which returns its handle index.

`_DoTerminate` kills the emitters' sounds, drains the composite pool while
polling, stops the poll thread and deletes the emitter entities.

## Per-frame poll

`Poll` (`0x7560`) releases the poll thread's semaphore, or polls the default
emitter's entity itself, then polls each joypad emitter's entity, every
generator manager, the default render target (`Update`), the FMOD platform
(slot 22) and `gMicHwManager`.

## Playing sounds

`PlaySound(PlayArgs const&)` (`0x7A00`) offers the request to every manager
after the composite one. One result is returned as is; several are grouped
under a composite generator that starts paused or playing. The result is
parented to the request's emitter or the default one, named, and its handle
is passed to the `AddSoundHandle` hook when the request names a global
handle. The script functions `play_sound`, `sound_valid`, `sound_stop`,
`sound_pause`, `sound_continue`, `sound_get_elapsed_ms` and
`sound_set_param` hold the generator through `SoundHandleLock`.

`_GetLoadedEvents` (`0x80E0`) evaluates the variable the message's third
node names, takes its object (`DataNode::LiteralSink`, `0x237C00`) and asks
the FMOD platform for its loaded events. It returns an array with one
`(symbol path path)` entry per event; the strings drop a leading `/`.

## Not reconstructed

- The component and resource `Init` bodies (among them
  `AudioEmitterCom::Init` at `0x42F0`, `AudioListenerCom::Init` at
  `0x4B70` and `FusionPatchCom::Init` at `0x5350`, which `_InitComponents`
  calls in the binary's order), which the map emits in this
  object, are declared only; `FusionPatchResource::Init` is inline in its
  header. `CompositeGeneratorManager`'s members
  (`0xDD10` to `0xE3C0`) are reconstructed with the composite generator.
- The seven managers `InitStandardGenManagers` creates live in their
  generators' objects, with their `kIdStr` (`0x19B00A0` to `0x19B00E0`) and
  vtables: see [fusion-sampler.md](fusion-sampler.md),
  [music-generators.md](music-generators.md) and
  [instrument-generators.md](instrument-generators.md). The pool code they
  share is `src/audio/core/generators/GeneratorPool.h`.
