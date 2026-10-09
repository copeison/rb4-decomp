# Math foundation

The math module (`src/math`) follows the map's `math/` object files.

| Object | Contents |
| --- | --- |
| `Trig.o` | `TrigTableInit` (`0x219610`), `TrigTableTerminate`, `Sine` (`0x219690`), `FastSin` |
| `Half.o` | `Half::Set` (`0x1179780`) |
| `TruncatedRoundedCone.o` | the constructor and `SetAngleTopRadiusAndLength` |
| `Matrix3.o` | `Det` (`0x2152E0`) and `Hmx::Matrix3::sID` |
| `Matrix4.o` | `Det` (`0x11798A0`), `Invert` (`0x1179A80`) and `Hmx::Matrix4::sID` |
| `Interp.o` | `Interpolator` and its linear, exp, invexp, log, atan and cubic curves (`0x212590` to `0x214928`) |
| `Rot.o` | `MakeEuler` (`0x215B10`) and `IsVertical` (`0x215BC0`) |
| `Transform.o` | `Multiply` (`0x2187E0`), `Transform::sID` and `sZero` |
| `Vector2.o`, `Vector3.o`, `Vector4.o` | the zero and axis constants |

## Geometry

- **`Frustum`** (`SetPerspective`, `SetOrtho`, `SetCorners` and
  `_DoUpdateHull`) keeps corners, edges and planes.
- **Planes** can be transformed and intersected.
- **`BoundingHull`** (`math/Geo.o`) grows a 27-direction hull around points
  and turns it into an optimized bounding sphere. `RndMeshUtl::ComputeBoundingSphere`
  uses it.

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

## Interpolators

- **`Interpolator::New`** makes `linear`, `exp`, `invexp`, `atan` and `cubic`
  curves with default parameters; `piecewiselinear` has a static symbol but no
  class, and nothing makes a `LogInterpolator`. Script arrays give the outputs
  before the inputs: `(type startY endY startX endX extra)`.
- **The vtables** (`0x18EEE28` to `0x18EEFD8`, no RTTI) have seven slots:
  `Eval`, `ClampEval`, `ReverseEval`, `ClampReverseEval`, `Reset(DataArray const*)`
  and the destructors. `Sync`, `Save` and `Load` are not virtual. The base
  `ReverseEval` returns zero, so `ATanInterpolator` and `CubicInterpolator`
  have no reverse.
- **Single and double precision**: `LogInterpolator` keeps its logarithms as
  doubles of `logf` results and evaluates `exp` in double precision.
- **`CubicInterpolator`** solves a 3x3 system through `Invert` for an odd cubic
  past a dead zone; its field names rest on that solve alone.

`TrigInit` (`0x219770`) registers the script functions `sin`, `cos` and `tan`,
which take degrees, and `asin`, `acos` and `atan`, which return degrees.

## Not yet reconstructed

- Two 36-byte objects at `0x19E669C` and `0x19E66C0`, filled by the static
  initializer at `0x2195C0`.
