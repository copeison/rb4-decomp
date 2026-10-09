# Mogg and instrument generator managers

Three objects own the managers that play Mogg files and instrument racks.
Their managers are reconstructed, and so are the two instrument generators;
`MoggGenerator` is declared only. Every manager has the 18-slot `AudioGeneratorManager` vtable, keeps
its typed pool at `+0x40` and uses the shared pool sequence of
`GeneratorPool.h` (see [audio-generators.md](audio-generators.md)).

| Object | Range | Manager vtable | `kIdStr` | Generator | Size |
| --- | --- | ---: | ---: | --- | ---: |
| `audio/MoggGenerator.o` | `0x47A70`-`0x4BD1F` | `0x18E0BE0` | `0x19B00B0` | `MoggGenerator` | 464 |
| `audio/MultiInstrumentGenerator.o` | `0x4E780`-`0x5277F` | `0x18E1550` | `0x19B00C8` | `MultiFusionGenerator` | 840 |
| `audio/SynthRackGenerator.o` | `0x58130`-`0x5B29E` | `0x18E2598` | `0x19B00E0` | `SynthRackGenerator` | 528 |

The sources are `src/audio/core/generators/MoggGenerator.{h,cpp}` and
`src/audio/core/instruments/{MultiFusionGenerator,SynthRackGenerator}.{h,cpp}`.
The map's `MultiInstrumentGeneratorManager` registers itself in this build
as `"MultiFusionGeneratorManager"`, so the classes take that name and the
source file keeps the generator's name. SynthRack is newer than the map;
its names follow the other managers'.

## MoggGeneratorManager

`Init` (`0x47A80`) starts the Mogg decoding support before registering the
manager: `VorbisReader::Init` (`0xD2D80`, thirteen single-letter script
functions), `ByteGrinderInit` (`0xC3BE0`) and `StreamReaderThread::Init`
(`0xD0650`, the reader thread stored at `0x19C9900`). The first two names
are weak; see `src/audio/core/decoders`.

The static `mMoggFileMap` (`0x19C85D8`, guarded by the `CritSec` at
`0x19C85C8`) maps sound names to Mogg files. `RegisterMoggResource`
(`0x47AB0`) maps a `MoggResource`'s sound name (`+0x38`, the file name
without its directory) to its path (`+0x30`); `UnregisterMoggResource`
(`0x47BB0`) erases every name of that path; `AddResourceAlias` (`0x47C80`)
adds another name. `Play` (`0x4B170`) is `PlayWithCallback` (`0x47E80`)
without a callback: `_FindSound` (`0x47D80`, inlined there) looks the name
up, and `_AllocateAndSetUpGenerator` (`0x47FC0`) binds a voice and calls
`MoggGenerator::Setup` (`0x480D0`). A voice whose setup fails is not
released.

`mDefaultBufferSizeMs` (`0x19B00B8`, 3000) is the stream buffer length a
request without one gets; game code reads it too. The float after it
(`0x19B00BC`, 0.85) goes with it to the stream setup at `0xC9F80`, which
compares the channels' drift against that fraction of the buffer; the name
`mDefaultBufferLagRatio` is weak. `Id()` and `ResExt()` (`.mogg`) are the
map's inline helpers; the MoggMusic object (`0x4C0D8`) shares `Id()`'s
static.

`MoggGenerator` derives from `AudioGenerator` only and adds no virtual
functions, so its 32 slots (vtable `0x18E0AD0`) are declared in order. Its
constructor (`0x4B830`) and destructor (`0x48E60`) are out of line. The rest
of its layout stays opaque: at `+0x50` it embeds an object whose vtable
(`0x18E5B58`) is the one `StandardStream`'s constructor (`0xC9C40`)
installs, followed by the audio-bus and bus-generator vectors. The
generator stays declared until `StandardStream` and the stream readers it
drives are modelled.

## Instrument generators

`MultiFusionGenerator` and `SynthRackGenerator` share one shape:
`InstrumentGenerator` (a `VirtualInstrument` at `+0` and an
`AudioGenerator` at `+0x138`) and an `AudioBusCallable` at `+0x188`. The
pool constructors are inlined into `_InitGeneratorPool` (`0x51930`,
`0x5A9C0`): `VirtualInstrument(nullptr)` at `0x66DF0`, then the
`InstrumentGenerator` vtables at `0x18E15F0`/`0x18E1798` and the
`AudioBusCallable` vtable at `0x18E0A98`, then the final vtables (primary
`0x18E11A0` and `0x18E21E8`). The destructors are out of line (`0x4F790`,
`0x587F0`).

`InstrumentGenerator`'s own vtable (`0x18E15F0`) has 51 slots:
`VirtualInstrument`'s 38, then `AddAudioThreadClient`,
`RemoveAudioThreadClient`, `SupportsAudioThreadClients` (true),
`SetPatch(patch, channel)`, `AddSlave` and `RemoveSlave` (false),
`DetachedFromMaster` (the map's name, slot 44), `AttachedToMaster`,
`IsAttachedToMaster`, and the transpose and time-stretch pairs. Slots 38,
39 and 41 are pure; the others have empty defaults emitted in the
MultiInstrumentGenerator object. `FusionSampler` (`0x18E4D68`, 55 slots)
adds its `SetSpeed`, `GetSpeed`, `SetPlayScale` and `GetPlayScale`
overrides as slots 51-54, and `FusionGenerator` its 20 overrides as slots
55-74. `VirtualInstrument::IsNotePlaying` takes the channel as a second
argument, which the MultiFusion generator uses.

The primary vtables of the two generators have 75 slots: the 51 and 24
entries for the overridden `AudioGenerator` and `AudioBusCallable`
functions, in the order `GetGeneratorOfType`, `GetTypeId`, `_InitTypeId`,
`Init`, `Pause`, `Continue`, `Stop`, `GetElapsedMs`, `GetTimelineMs`,
`SeekToMs`, `SetGain`, `GetGain`, `SetMute`, `GetMute`, `SetParameter`,
`GetParameter`, `SetSpeed`, `GetSpeed`, `Poll`, `Release`,
`_PrepareToMakeSamples`, `SetPlayScale`, `GetPlayScale` and `Kill`. The
emitted vtables match the binary's slot for slot, so the pools call `Init`
(slot 54) and `Stop` (slot 57) directly.

Both play through a bus voice of `gAudioBusGeneratorManager`
(`0x19E26A8`), the `FmodAudioBusGeneratorManager` that the FMOD platform's
slot 20 (`0x261210`) stores; the Fusion and Mogg generators use it too.
Their setup resets the transpose and time-stretch fields, takes a voice
with `_GetGenerator`, sets the bus's sample rate to the render target's,
calls the voice's `Setup` with the instrument as source and callable, and
sets the state to playing or paused.

### MultiFusionGeneratorManager

`mResourceMap` (`0x19C86E0`, guarded at `0x19C86D0`) maps sound names to
`MultiFusionResource`s; the resource's loaders call
`RegisterMultiFusionResource` (`0x4E7A0`) and
`UnregisterMultiFusionResource` (`0x4E8A0`) erases a resource's names. `Play`
(`0x4E970`) accepts a registered name or the bare name `.multifusion` (its
own static at `0x19C8718`), binds a voice and calls `Setup(args,
resource)` (`0x4EC90`) or `Setup(args)` (`0x4EDA0`, inlined there); a
failed setup releases the voice. The resource extension is `.multifusion`.

The generator keeps 16 channels: instrument generators (`+0x1B0`), names
(`+0x230`), volumes in decibels (`+0x2B0`) and mutes (`+0x2F0`), then the
transpose, the time-stretch mode, the bus voice (`+0x310`), the
audio-thread clients and their lock, and the `MultiFusionResource`
(`+0x340`). `SetResource` (`0x4F230`, an inferred name) takes a Fusion
generator for each channel's patch from `FusionGeneratorManager` until none
is free; when the resource's flag at `+0x138` is set, the other channels
become slaves of channel 0. The MIDI overrides pass each call to the
channel's instrument; `Process` mixes the instruments that are not slaves.
`MultiFusionResource` (320 bytes, vtable `0x18E2B20`) is declared in
`src/audio/core/resources`. `kTypeId` is at `0x19C8728`.

### SynthRackGeneratorManager

`Play` (`0x58140`) answers only the name `.synth_rack` and inlines
`SynthRackGenerator::Setup` (`0x58370`), which looks the render target up
again. `GetResourceExt` reports `.multi_inst`, apparently left from the
multi-instrument manager. `Id()`'s static (`0x19C85A8`) is emitted with the
Fusion generator object, which also reaches this manager through it.

The rack is a vector of 24-byte slots (`+0x1B0`): an instrument, a name, a
channel volume and a mute that `SetInstrument` (`0x58FF0`) applies. The
transpose (`+0x1D0`), the beat (`+0x1D4`), the time-stretch mode and the
bus voice (`+0x1E0`) follow, then the audio-thread clients and their lock.
The indexed members at `0x592A0`-`0x59B10` reach one slot's instrument;
their names are inferred, and `GetPitchBend` and `GetController` index the
rack by the channel, as the binary does. `Process` renders each instrument
at the rack's beat and mixes the ones that are not silent. `kTypeId` is at
`0x19C8788`.

## Unidentified statics

Each object's static initializer first stores -1 in an int it never reads
(`0x19C85B8`, `0x19C86C8`, `0x19C8780`); neighbouring objects do the same
(for example `0x19C8750`). They are not defined in the source.
