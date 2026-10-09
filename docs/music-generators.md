# Music generators

The music generators play songs that follow a timeline. Their managers are
reconstructed in `src/audio/core/music`; the generators themselves stay
declared. Every manager uses the shared pool sequence of
`audio/core/generators/GeneratorPool.h` (see
[audio-generators.md](audio-generators.md)).

## Objects

| Object | Manager code | Manager vtable | Pool entry | Generator vtables |
| --- | --- | ---: | ---: | --- |
| `MidiMusicGenerator.o` | `0x44430` to `0x44A9C`, `0x463C0` to `0x46A6C` | `0x18E09F8` | `0x768` | `0x18E04E0`, `0x18E0950` (+0x158), `0x18E09C0` (+0x3B0) |
| `MoggMusicGenerator.o` | `0x4BD30` to `0x4C384`, `0x4D750` to `0x4DDEC` | `0x18E1100` | `0x300` | `0x18E0C80`, `0x18E10C8` (+0x158) |
| `MusicTimelineGenerator.o` | `0x566F0` to `0x56878`, `0x57740` to `0x57F3C` | `0x18E2148` | `0x300` | `0x18E1CC8`, `0x18E2110` (+0x158) |

Each manager vtable has the 18 `AudioGeneratorManager` slots. Slot 3 is a
jump to `AudioGeneratorManager::Init` for the MidiMusic and MoggMusic
managers; the MusicTimeline manager keeps the base `Init` (`0x40500`).
`GetId` and `GetResourceExt` inline the map's `Id()` and `ResExt()`, whose
local statics this build places after each object's statics. The
extensions are `.midisong`, `.moggsong` and `.musictimeline`.

## Statics

| Static | MidiMusic | MoggMusic | MusicTimeline |
| --- | ---: | ---: | ---: |
| `kIdStr` (unreferenced) | `0x19B00A8` | `0x19B00C0` | `0x19B00D8` |
| Generator `kTypeId` | `0x19C8538` | `0x19C8648` | `0x19C8758` |
| Resource map lock | `0x19C8540` | `0x19C8650` | |
| Resource map | `0x19C8550` | `0x19C8660` | |
| `Id()::id` | `0x19C8588` | `0x19C86A8` | `0x19C8770` |
| `ResExt()::resExt` | `0x19C8598` | `0x19C86B8` | `0x19C8760` |

`kMusicTimelineSoundId` (`0x19B00D0`, `".musictimeline"`) is also
unreferenced. Each object starts its static initializer by setting an int
to -1 (`0x19C8530`, `0x19C8640`, `0x19C8750`); other objects repeat it, so
it comes from a shared header and is not modelled here.

## Resource registration

`MidiMusicResource` and `MoggMusicResource` register under the sound name
their slot 13 returns (`GetSoundId`, a weak name): the MIDI resource's
constructor and the mogg resource's `LoadFile` and `Load` call
`Register*MusicResource`, and their destructors call
`Unregister*MusicResource`, which erases every entry of the resource. The
maps are `eastl::map<Symbol, *MusicResource*>` guarded by a static
`CritSec`. `MidiMusicGeneratorManager::FindMidiMusicResource` (`0x44600`) is
an unreferenced lookup; `Play` inlines its own.

## Play

- MidiMusic (`0x44700`): finds the resource, takes a voice, then a voice of
  the SynthRack manager (`_GetManager` by the inline id at `0x19C85A8`, the
  map's `MultiInstrumentGeneratorManager::Id()`), cast to
  `InstrumentGenerator`. Without the instrument the voice is freed
  (`AudioGeneratorManager::FreeGenerator`, inlined). `Setup` is at
  `0x44AA0`.
- MoggMusic (`0x4BF00`): finds the resource, takes a voice and clears its
  Mogg voice and resource, then asks the Mogg manager (inline id at
  `0x19C8620`) for a paused voice of the resource's mogg through
  `MoggGeneratorManager::PlayWithCallback` (`0x47E80`), with the generator as
  its audio-thread client. The request is a whole copy of the caller's with
  format 0. `Setup` is at `0x4C390`.
- MusicTimeline (`0x566F0`): plays only a request named `.musictimeline`
  and does not substitute the default emitter. `Setup` is at `0x56880`.

## Declared, not reconstructed

The three generators depend on `MusicGenerator`'s 130-slot interface and on
the song maps (tempo, measure and beat maps) and MIDI cursors it drives,
none of which is modelled, so their objects stay declared.

The generators are large and are only declared, with their bases, exact
sizes, the `AudioGenerator` slots they override and the members their
managers use:

- `MusicGenerator` (`MusicGenerator.h`, 344 bytes, vtable `0x18E18A8`
  with 130 slots, constructor `0x52780`, destructor `0x52C00`) implements
  `Pause`, `Continue`, `Stop`, `GetLengthMs`, `SetSpeed`, `GetSpeed`,
  `IsMusic` and `Kill`. Its own slots 32-43 are declared: the pure
  `IsReady` (a weak name; the Mogg music generator asks its Mogg voice),
  the content and song ticks and positions, the smoothed position, the
  section name and index and `GetCurrentBPM` (`0x55EF0`), which the emitter
  component reads. The slots from 44 on are not declared.
- `PlayMusicArgs` (160 bytes, format 1) adds `MusicPlayOptions` (`+0x68`,
  32 bytes: the sync option, the master handle, the timeline mapping, the
  unmute point and a velocity-like 64), which `MusicGenerator`'s setup
  (`0x52AF0`) copies, and two scales of one. `MusicSyncOptions` has
  `kMaster` (1) and `kSlave` (2) from `SymbolToMusicSyncOptions`
  (`0x559D0`).
- `MidiPlayCursor` (`MidiPlayCursor.h`, 600 bytes, vtable `0x18E5460`,
  constructor `0xAA200`, destructor `0xAA270`).
- `MidiMusicGenerator` is `MusicGenerator`, `MidiPlayCursor` and
  `AudioBusCallable`; `MoggMusicGenerator` and `MusicTimelineGenerator` are
  `MusicGenerator` and `AudioBusCallable`. Their constructors are inlined
  into `_InitGeneratorPool`; the destructors are out of line (`0x45060`,
  `0x4C510`, `0x56C90`).
- `MidiMusicResource.h` and `MoggMusicResource.h` (in
  `src/audio/core/resources`) declare `Init` and slot
  13; the mogg resource also declares the fields `Play` reads.
