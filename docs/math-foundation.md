# Math foundation

The math module (`src/math`) follows the map's `math/` object files.

| Object | Contents |
| --- | --- |
| `Trig.o` | `TrigTableInit` (`0x219610`), `TrigTableTerminate`, `Sine` (`0x219690`), `FastSin` |
| `Half.o` | `Half::Set` (`0x1179780`) |
| `TruncatedRoundedCone.o` | the constructor and `SetAngleTopRadiusAndLength` |
| `Matrix3.o` | `Det` (`0x2152E0`) and `Hmx::Matrix3::sID` |
| `Matrix4.o` | `Det` (`0x11798A0`), `Invert` (`0x1179A80`) and `Hmx::Matrix4::sID` |
| `Transform.o` | `Multiply` (`0x2187E0`), `Transform::sID` and `sZero` |
| `Vector2.o`, `Vector3.o`, `Vector4.o` | the zero and axis constants |

## Details

- **`Sine`** interpolates a 256-entry table of value and slope pairs at
  `0x19E66F0`, filled by `TrigTableInit`.
- **`Invert`** takes an epsilon, not a determinant. It computes the
  determinant itself and multiplies the adjugate by zero when that determinant
  is within the epsilon.
- **`Half::Set`** truncates. Its overflow and underflow paths store infinity or
  the sign, then fall through and are overwritten by the truncated bits; the
  reconstruction keeps that.
- **The vector `Det`, `Invert` and `Multiply`** follow the binary's order of
  additions and subtractions, so they round the same way.
- **The identity constants** are filled by static initializers in the binary.
  The source defines them as constant aggregates with the same values.

`TrigInit` (`0x219770`) registers the script functions `sin`, `cos` and `tan`,
which take degrees, and `asin`, `acos` and `atan`, which return degrees.

## Not yet reconstructed

- Two 36-byte objects at `0x19E669C` and `0x19E66C0`, filled by the static
  initializer at `0x2195C0`.
