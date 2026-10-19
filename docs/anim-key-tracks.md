# Key tracks

The templated key tracks the property animations and waveforms keep
their keys in (the map's `entity/Key.o` and its template instances).
Names follow [naming.md](naming.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `TKeyTrack<ValueTangent<float>>` instance | `0x1C9870`-`0x1CA2FD`, vtable `0x18EA4A0` | `src/entity/waveform/Key.h` |
| `TKeyTrack<Hmx::Color>` instance | `0x1CB730`-`0x1CBCD5`, vtable `0x18EA600` | `src/entity/waveform/Key.h` |
| `SampleValueAtFrame` (`ValueTangent<float>`) and the easing | `0x1CE320`, `0x1CE460`-`0x1CFB20` | `src/entity/waveform/Key.h`, `src/math/easing/Easing.h` |
| `entity/Key.o` (part) | `0x1DC930`, `0x1DCA50` | `src/entity/waveform/Key.cpp` |
| `WaveformResource<float>::GenerateTable` | `0x1CE140` | `src/entity/waveform/Waveform.cpp` |
| `KeyTrackBase::InterpolationToString`, `StringToInterpolation` (in `math/Easing.o`) | `0x212420`, `0x212440` | `src/math/easing/Easing.cpp` |

The tracks are templates, defined in the header; the binary emits each
instantiation's methods where it is used, the float and colour ones with
the waveforms.

## Classes

`KeyTrackBase` holds the interpolation at +8 and declares slots 0-15;
`TKeyTrack<T>` (56 bytes for `ValueTangent<float>`) adds `AddKey` as
slot 16 and keeps the keys in a `PropArray<TKey<T>>` at +16.

| Slots | Members |
| ---: | --- |
| 0-3 | destructors, `SetInterpolation`, `GetInterpolation` |
| 4-8 | `Clear`, `NumKeys`, `Empty`, `GetFirstFrame`, `GetLastFrame` |
| 9-12 | `RemoveKeyAtIndex`, `MoveKey`, `OffsetFrames`, `RemoveKeysInFrameRange` |
| 13-16 | `GetKeyFrameAtIndex`, `Reserve`, `Sort`, `AddKey` |

`AddKey` binary-searches the first key at the frame and replaces its
value, or inserts the new key before or after the keys already at the
frame. `RemoveKeysInFrameRange` removes the keys from the start up to,
but not at, the end. A new track's interpolation is 0; the waveforms set
1 and the volumetric scattering's default height density 3. The easing
table at `0x18EEC60` (`sInterpolationNames`) names the values from -1
("Invalid"): None, Linear, Spline, Hermite, Sigmoid and the easing curves
from 5 to 53. `InterpolationToString` returns a value's label, null out of
range; `StringToInterpolation` returns the value of a label, -1 when none
matches.

`ValueTangent<T>` (16 bytes for float) is a value with in and out
tangents and the auto-tangent flag; `TKey<T>` is a frame and a value.

The emitted vtable of `TKeyTrack<ValueTangent<float>>` matches the
binary's slot order.

The interpolation field's upper 16 bits hold flags (0x20000000 from the
revision 0 upgrade, `UpgradeInterpolation` `0x1DC930`; 0x40000000 on the
rotation keys), so the samplers read the low 16 bits.

## Sampling

`FindKeysAtFrame` finds the keys around a frame (both the first before it,
both the last after it, none without keys) and the position between them;
`SampleValueAtFrame` blends them through `_DoSampleValueAtFrame`: Spline
samples `_SampleSplineInterpValue`, the others ease the position
(`InterpolationEase`, `0x1CE460`) and blend it in
`_SampleLinearInterpValue`. Values that do not blend keep the first key's
until the eased position reaches 1. The float spline (`0x1DCA50`) is a
cubic Hermite curve whose tangents are scaled by the span; an automatic
tangent is the slope through the neighbouring keys (`SplineTangent`), and
a stored tangent steeper than 500 holds the first key's value.

The easing curves are Penner's, with the configured back overshoot (2),
elastic amplitude and period, polynomial power (6) and stair-step power
(5) (`src/math/easing/Easing.h`); the in/out forms ease the first half in
and the second half out (or the reverse). The elastic in/out pair is
emitted out of line (`0x1CF650`, `0x1CF8C0`).

`WaveformResource<T>::SetCurve` replaces a curve's interpolation and keys
unless they are the same, rebakes its 256 samples (`GenerateTable`,
`0x1CE140`, clamped to the value range) and recomputes whether any value
is non-zero (`mNonZero`, formerly read as a baked flag). The volumetric
scattering's default height density is the Hermite curve through (0, 1)
and (1, 1).

## Not reconstructed

- The other `Key.o` spline samplings (`TKey<float>` at `0x1DC9A0`).

## Weak evidence

- `OffsetFrames` (not in the map) and the interpolation enumerator
  names, which follow the easing table's labels.
- `InterpolationEase`, `UpgradeInterpolation`, the easing curve names
  (only `EaseBounceOut` is in the map), `SameKeys`, `_UpdateNonZero`,
  `WaveformKey`, `mNonZero` and `sInterpolationNames`.
