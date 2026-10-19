# Key tracks

The templated key tracks the property animations and waveforms keep
their keys in (the map's `entity/Key.o` and its template instances).
Names follow [naming.md](naming.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `TKeyTrack<ValueTangent<float>>` instance | `0x1C9870`-`0x1CA2FD`, vtable `0x18EA4A0` | `src/entity/waveform/Key.h` |
| `TKeyTrack<Hmx::Color>` instance | `0x1CB730`-`0x1CBCD5`, vtable `0x18EA600` | `src/entity/waveform/Key.h` |

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
table at `0x18EEC60` names the values: None, Linear, Spline, Hermite,
Sigmoid and the easing curves from 5 to 53.

`ValueTangent<T>` (16 bytes for float) is a value with in and out
tangents and the auto-tangent flag; `TKey<T>` is a frame and a value.

The emitted vtable of `TKeyTrack<ValueTangent<float>>` matches the
binary's slot order.

## Not reconstructed

- `SampleValueAtFrame` (`0x1CE320` for `ValueTangent<float>`), the easing
  dispatch it calls (`0x1CE460`) and the spline sampling of `Key.o`
  (`0x1DCA50` for `ValueTangent<float>`).
- `KeyTrackBase::InterpolationToString` and `StringToInterpolation`.

## Weak evidence

- `OffsetFrames` (not in the map) and the interpolation enumerator
  names, which follow the easing table's labels.
