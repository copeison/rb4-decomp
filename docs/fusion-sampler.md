# Fusion sampler

`audio/FusionSampler.o` spans `0x95C40` to `0x9D90F` and is reconstructed in
`src/audio/core/fusion/FusionSampler.{h,cpp}`. `FusionSampler` is an
`InstrumentGenerator`: a `VirtualInstrument` with an `AudioGenerator` at
`+312`. The map uses `InstrumentGenerator` for slaves, the scheduler and
`MultiInstrumentGenerator`, but lists no members for it. The source gives it
the virtuals after `VirtualInstrument`'s (slots 38 to 54). The sampler is
abstract, and `FusionGenerator` completes it. The object is `0x5470` bytes.

## Vtables

The primary vtable is at `0x18E4D68`, and the `AudioGenerator` vtable is at
`0x18E4F30`. The emitted vtable matches the binary slot for slot. These slots
differ from the earlier partial declaration:

| Slot | Address | Member |
| ---: | --- | --- |
| 10 | `0x43C40` | `IsInstrument`. It also overrides `AudioGenerator`'s slot 17 through the thunk at `0x43FC0`, which is why `AudioBus` slot 10 now has that name. |
| 11 | `0x51D70` | `CallPreProcessCallbacks(int, float, int, int, bool)`, which the map names on `FusionGenerator`. |
| 13 | `0x9A200` | `ResetInstrumentState`. It releases the slaves and resets the channel, speed, tempo, transpose and track gains. |
| 14 | `0x9A3F0` | `ResetMidiState`, an inferred name. It turns all notes off and resets the pitch bend. |
| 16 | `0x97530` | `IsNotePlaying`, an inferred name. It checks for a pending note-on or a playing voice. |
| 17 | `0x975D0` | `NoteOff`. `HandleMidiMessage` sends note-offs here. |
| 31 | `0x43D00` | `GetNumVoicesInUse`, the name the map uses. |
| 47-48 | `0x43D70` | A transpose that each note-on adds, stored at `+0x5430`. |

`ResetPatchRelatedState` (`0x96610`) is not virtual. The constructor and
`LoadPatch` call it.

## Notes

`NoteOn` and `NoteOff` only queue a `NoteAction` for each note under
`sNoteActionCritSec` (`0x19C8EB0`). The queued note keeps its loudest
velocity, and its float is a start offset in milliseconds.
`_ProcessNoteActions` drains the queue at the start of each block, from note
127 down. Note-ons stop once `mMaxNumVoices` is spent. A note-off releases
the note's voices and plays the note-off keyzones at the held velocity.

`_ProcessNoteOn` matches the note, after the transpose, and the velocity
against the patch's keyzones. The keyzone select mode then decides what
plays:

- Layers play every match.
- Random picks one match by `random_weight` and never repeats the zone that
  played last.
- Random with repetition picks by weight and may repeat the last zone.
- Cycle steps through the matches in turn.

Both random modes skip zones whose weight is zero. Random draws come from
`gRand` (`math/Rand.o`, `0x19E6628`), a 64-bit xorshift.

`_TryKeyOnZone` takes a voice from the pool. It runs the two random
modulators with a value in `[0, 1)` and the two velocity modulators with the
velocity. These modulators feed the start-point and pitch `ModulatorTarget`s
at `+0x190` and `+0x1B8`.

## Presets

`LoadPatch` takes the patch's `FusionPatchCom` and its current preset.
`_LoadPreset` (`0x987E0`, inferred name) applies the 392-byte
`FusionPatchCom::PresetSettings`, which covers:

- trim, pan, pitch-bend ranges, start point and fine tune;
- the voice limit and the keyzone select mode;
- portamento and the filter;
- the envelope, LFO and modulator arrays;
- the delay, distortion, bit crusher and amp simulation.

A bank select (controller 0) selects another preset.

Each control value is an `SPL::Parameter` clamped to a spec. The specs live
at `0x19C8EC0` to `0x19C8F28`. The volume spec's default lies above its
maximum, so a reset clamps it to 0 dB. Four `ExpInterpolator`s
(`math/Interp.o`) map controller values onto the following settings:

- the filter frequency and Q;
- the LFO frequency;
- the delay time.

## Processing

`Process` (`0x9ACC0`) runs the following steps:

1. It advances the control ramps and drains the note queue.
2. It mixes the sampler's voices through two 2048-sample scratch channels.
3. It mixes the "before effects" slaves into the block.
4. It runs the effects: distortion, then the bit crusher, then the amp
   simulator with its three-band EQ, then the delay. The amp and EQ work on
   the left channel, which is then copied to the right.
5. It mixes the remaining slaves.

A silent sampler with no slaves only renders the delay's tail. The block is
timed with the render target's `"fusion"` timer and
`VirtualInstrument::mProcessTimer`. The voice count peak goes to
`VirtualInstrument::mVoiceMeter` (`Meter`, lock `0x19C9910`).

`Delay` (a `TempoListener`) is reconstructed in `Delay.cpp`; see
`audio-dsp.md`. So are `BitCrusher.cpp`, `DistortionEffect.cpp` and
`AmpSimulator.cpp`. The amp simulator (`0x1090650`) is newer than the map, so
its name is inferred, and its five amp models are only declared.
`FIRFilter` is header-only. Its vtable (`0x18E5040`) and
`DistortionEffect`'s destructor (`0x9D010`) are emitted in this object, as in
the map's build.

## Controllers

`SetController` and `GetController` read and write only the MSB. Besides the
standard controllers (bank select, portamento time, volume, pan, expression,
portamento, resonance, release, attack and brightness), the sampler handles
these:

| Controllers | Setting |
| --- | --- |
| 14 to 16 | Bit crusher. |
| 22 to 25 | LFO frequency and depth. |
| 26 to 29 | Delay time and gains. |
| 30 | Pitch bend. |
| 31 | Start point. |
| 52 to 59 | The eight track gains at `+0x5450`. |

`GetController` scales the volume and expression values by 127 twice,
which wraps them in 8 bits.

## FusionGenerator

`audio/FusionGenerator.o` (`0x41F80` to `0x44430`) is reconstructed in
`src/audio/core/fusion/FusionGenerator.{h,cpp}`. `FusionGenerator`
completes the sampler (`0x5498` bytes). Its primary vtable is at `0x18E00C8`
(75 slots), and its `AudioGenerator` vtable is at `0x18E0330`. Both match the
emitted vtables slot for slot.

The generator overrides slots 0-1, 11 (`CallPreProcessCallbacks`), 38-39
(the audio-thread client list, guarded by `mClientListLock` at
`0x19C84F8`), 41 (`SetPatch`) and 44-46. Slots 44-46 clear, store and test
the master handle at `+0x5470`; the map names slot 44 `DetachedFromMaster`.
Slots 55-74 are the `AudioGenerator` overrides.

`FusionGeneratorManager::Play` (`0x423A0`) plays only patches registered in
`sPatchMap` (`0x19C84C0`, lock `0x19C84B0`). A `FusionPlayArgs` request is
format 2. When its slot type (`+0x68`) is nonzero, the generator becomes a
slave of the instrument at `+0x6C` through `AddSlave`. Any other request
takes a bus generator (`+0x5478`) from `gAudioBusGeneratorManager`
(`0x19E26A8`). The playback calls forward to that bus generator. A slave
stops at once and leaves its master. `InstrumentHandleLock` is the map's
lock on a master's handle; this build inlines it.

The other names are weak: `FusionPlayArgs`, `mBusGenerator`,
`GetPatchNames` (`0x42090`) and `sPatchMapCritSec`.

## FusionPatchResource

`audio/FusionPatchResource.o` (`0x5B590` to `0x5C282`) is reconstructed in
`src/audio/core/resources/FusionPatchResource.{h,cpp}`. The class is an
`EntityResource` of `0xD0` bytes, with its vtable at `0x18E2660`. The root
object, named `"fusion_patch"`, holds the `FusionPatchCom`.

The resource loads in one of three ways:

- `.sxt` files load directly through `_LoadFromSXTFile`.
- `.fusion` files load directly through `_LoadFromDTAFile`.
- Any other file loads through the cache. A `.fusion` cache holds the text
  format. `Save` writes that format and `_LoadEntity` reads it back, unless
  `Component::sRegressionTesting` (`0x19E2970`) is set.

A patch that loads registers with `FusionGeneratorManager::AddPatch` under
its file name, and its destructor removes it. `NeedsReload` (slot 12)
returns true in two cases: there is no patch, or a keyzone's sample changed
on disk or must itself be reloaded.

`HasDependencies` (slot 8, `0x5C280`) returns true, and `GetDependencies`
(slot 9, `0x5B890`) appends each keyzone's sample, after that sample's own
dependencies, to an `eastl::vector<ResourcePtr<Resource>>`. The emitted
vtable matches `0x18E2660` slot for slot.
`IsModified` (`0x5BFF0`) is declared only. The function at `0x5C170`
returns zero and is not identified.
