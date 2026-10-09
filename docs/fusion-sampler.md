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

The effects are declared, not reconstructed: `Delay` (a `TempoListener`),
`BitCrusher`, `DistortionEffect` and `AmpSimulator` (`0x1090650`). The amp
simulator is newer than the map, so its name is inferred. `FIRFilter` is
header-only. Its vtable (`0x18E5040`) and `DistortionEffect`'s destructor
(`0x9D010`) are emitted in this object, as in the map's build.

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
