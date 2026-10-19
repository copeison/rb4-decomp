# Numeric property-animation keys

The float, vector, quaternion, colour, rotation and transform key
components of the map's `anim` module. They follow the
`PropKeysBoolCom` pattern described in
[anim-prop-keys.md](anim-prop-keys.md): each is a `PropKeysTypeCom<T>`
with its own `_Init`, slots 73-75 and a `TPropDriver<Keys>`. Names follow
[naming.md](naming.md); the key tracks are in
[anim-key-tracks.md](anim-key-tracks.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/PropKeysColorCom.o` | `0x302B50`-`0x30534F` | `src/anim/props/PropKeysColorCom.cpp` |
| `anim/PropKeysFloatCom.o` | `0x305D40`-`0x308AAF` | `src/anim/props/PropKeysFloatCom.cpp` |
| `anim/PropKeysQuatCom.o` | `0x311080`-`0x31498F` | `src/anim/props/PropKeysQuatCom.cpp` |
| `anim/PropKeysRotationCom.o` | `0x314990`-`0x318DAF` | `src/anim/props/PropKeysRotationCom.cpp` |
| `anim/PropKeysTransformSRTCom.o` | `0x31E2E0`-`0x32233F` | `src/anim/props/PropKeysTransformSRTCom.cpp` |
| `anim/PropKeysVec3Com.o` | `0x322340`-`0x32531F` | `src/anim/props/PropKeysVec3Com.cpp` |
| `RotationKey`, `TransformSRTKey` (sampled in `entity/Key.o`) | - | `src/entity/waveform/RotationKey.h` |

Each object holds the class's `StaticCreatePropDriver`, `Register`,
`CreatePropDriver`, `StaticKeyTypeName`, `_Init` (with
`RegisterPropStruct<TKey<T>>` and `_InitPropKeysBase<T>`), the class's
and `PropKeysTypeCom<T>`'s virtual methods, the driver and the static
initializer. `Init` and `_Create` are emitted in `anim/AnimInit.o`. The
rotation and transform classes have out-of-line constructors.

## Classes

| Class | Value (`TKey` size) | Type | Vtable, key track | Driver vtable | Statics (`sId`, `sClassName`, registry, metadata) | `Init`, `_Create` |
| --- | --- | --- | --- | --- | --- | --- |
| `PropKeysFloatCom` | `ValueTangent<float>` (20) | Float | `0x18F7E10`, `0x18F8080` | `0x18F8108` | `0x19F9F18`, `0x19F9F20`, `0x19F9F30`, `0x19F9FD0` | `0x2AAA60`, `0x2B2990` |
| `PropKeysVec3Com` | `Vector3` (16) | Vector3 | `0x18F9BB8`, `0x18F9E28` | `0x18F9EB0` | `0x19FB180`, `0x19FB188`, `0x19FB190`, `0x19FB230` | `0x2AAF10`, `0x2B2B00` |
| `PropKeysQuatCom` | `Hmx::Quat` (20) | struct "quaternion" | `0x18F87E0`, `0x18F8A50` | `0x18F8AD8` | `0x19FA4A0`, `0x19FA4A8`, `0x19FA4B0`, `0x19FA550` | `0x2ABC40`, `0x2B3CC0` |
| `PropKeysColorCom` | `Hmx::Color` (20) | Color | `0x18F77D0`, `0x18F7A40` | `0x18F7AC8` | `0x19F9C50`, `0x19F9C58`, `0x19F9C60`, `0x19F9D00` | `0x2ACF00`, `0x2B5C90` |
| `PropKeysRotationCom` | `RotationKey` (36) | Euler | `0x18F8BC8`, `0x18F8E38` | `0x18F8EC0` | `0x19FA730`, `0x19FA738`, `0x19FA740`, `0x19FA7E0` | `0x2AB3C0`, `0x2B2C70` |
| `PropKeysTransformSRTCom` | `TransformSRTKey` (60) | Xfm | `0x18F97D0`, `0x18F9A40` | `0x18F9AC8` | `0x19FAF00`, `0x19FAF08`, `0x19FAF10`, `0x19FAFB0` | `0x2AB800`, `0x2B3470` |

The emitted primary vtables (76 slots), key-track vtables (17 slots) and
driver vtables (6 slots) match the binary slot for slot
(`orbis-objdump -r` against the binary's vtable contents).

All classes use author "Matt Sharpe", the animation category and the
allowed resource `PropAnimResource`. They interpolate linearly by
default; the rotation and transform keys add the `0x40000000` flag,
which blends the Euler angles one by one.

## Overridden slots

- Float: `HasTangents` (42) and `IsValidInterpolation` (57) return true.
  The values are a float node, `{Float(), 0, 0, true}` and zero.
  `ValueTangent`'s default is `{0, 0, 0, true}` (default member
  initializers), as the binary's prototype and samples show.
- Vec3: 57 returns true. The values are a three-float array, the node's
  array (read once and dropped first) and zero. The linear sampling is the
  inline `Interp`.
- Color: 57 rejects Spline; 58 (`IsColorKeys`) returns true. The values
  go through `DataNode(const Hmx::Color&, bool)` (`0x237F90`, with false)
  and `DataNode::Color(bool, const DataArray*)` (`0x17CA70`), and the
  default is (0, 0, 0, 0). A key's colour defaults to opaque black.
- Quat: 57 returns true. For the editor the value is the Euler angles in
  degrees, otherwise x, y, z and w (the four-node `MakeDataArray`,
  `0x313FB0`); three numbers are degrees (`Quat::Set`, then `Normalize`),
  four the components. Slots 63, 69 and 70 are `PropKeysTypeCom<Hmx::Quat>`
  specializations that read and write a `Hmx::Matrix3` property found with
  `PropRef::Find(Component*, void**)` (`0x17FC70`); 70 writes the rotation
  matrix and sets the `TransCom`'s dirty flag 1.
- Rotation and TransformSRT: 57, 59 (`IsRotationKeys`) and 60
  (`HasSerializedValue`) return true. 63, 69 and 70 are specializations
  that read the Euler angles (`Get<PropRotation>`) or the transform
  (`Get<PropXfm>`, `0x31F530`), build the key and, for 70, refresh the
  angles and set kEuler or kXfm. The values are the three angles or the
  nine floats (`new DataArray(9)`).

## Key values

`RotationKey` (32 bytes) holds `Vector3 mEuler`, `Hmx::Quat mQuat` at 12
and `mFlags` at 28; `TransformSRTKey` (56 bytes) holds `PropXfm mXfm`,
`mQuat` at 36 and `mFlags` at 52. Flag 1 marks the angles stale and flag 2
the quaternion; the readers refresh the stale form on use
(`UpdateEuler`, through `MakeEuler(const Hmx::Quat&, Vector3&)`,
`0x2179C0`), so the members are mutable. Building a key from angles sets
the quaternion with `Quat::Set(const Vector3&)` (`0x216480`); building a
rotation key from a quaternion (old quaternion keys converted by
`PropAnimResource::_LoadEntity`) sets the angles.

The key's "value" is accessor-only (getters `0x317560` and `0x321900`,
setters `0x3175E0` and `0x321980`, the key index at `path[-2]`); the key
is saved as "serialized_value" (the angles or transform, "quat" and
"flags"). The rotation key struct's registry is at revision 1, whose
handler (`0x317C40`) reads the old "value pitch/roll/yaw" floats. The
quaternion key's "value" is a struct whose "euler_rotation" (kind 1)
holds the accessor angles x, y and z in degrees (`0x313BF0`-`0x313EE0`,
the key index at `path[1]`, the setters wrapping the angle into [-π, π)),
and "quat_rotation" the hidden quaternion.

## Drivers

- Float and Color blend the unpaused playbacks' samples by weight and set
  the mean when the weights are positive (`_Poll` `0x306C30` and
  `0x3037B0`; the colour sum and sample start as opaque black).
- Quat (`0x312270`) accumulates the weighted samples on the running sum's
  hemisphere (x, y and z scaled by the weight's magnitude, w by the
  weight), accumulates the sum with itself scaled by the inverse weights,
  normalizes it and writes its rotation matrix as `_SetDrivenProperty`
  (`0x312720`, unreferenced) does.
- Vec3, Rotation and TransformSRT use the template's heaviest-playback
  `_Poll` (`0x324B60`, `0x317A70`, `0x321E10`); the rotation and transform
  `_SetDrivenProperty` (`0x316250`, `0x31FD70`) refresh the angles first.

## Key.o sampling

The `TKeyTrack<T>` specializations are declared with their addresses:
`TKeyTrack<Vector3>::_SampleSplineInterpValue` `0x1DCCF0`,
`TKeyTrack<Hmx::Quat>::_SampleSplineInterpValue` `0x1DD190`,
`TKeyTrack<Hmx::Color>::_SampleLinearInterpValue` `0x1DEA70` (gamma
space; the track's `SampleValueAtFrame` is `0x2424A0`),
`TKeyTrack<RotationKey>` `0x1DD340`/`0x1DD4B0` and
`TKeyTrack<TransformSRTKey>` `0x1DD9B0`/`0x1DDC50`. The vector and
quaternion linear samplings are inline (`Interp`, `FastInterp`).

## Not reconstructed

- The `Key.o` samplings above, `PropRef::Find(Component*, void**)`, the
  `DataNode` colour conversions, the four-node `MakeDataArray`,
  `Quat::Set(const Vector3&)` and `MakeEuler(const Hmx::Quat&, Vector3&)`
  are declared only.

## Differences and weak evidence

- `PropRef::Get<T>` returns a value-initialized `T` when the property is
  missing; the binary's `Hmx::Color`, `Hmx::Quat` and `PropXfm` defaults
  are (0,0,0,1), the identity quaternion and the identity transform. The
  shared types keep their aggregate defaults, so only that failure path
  differs; the keys use explicit defaults elsewhere.
- The binary's `_SetDrivenProperty` takes the keys' `PropRef`; the
  template takes its path. The quaternion specialization therefore finds
  the storage through the registry instead of `PropRef::Find`.
- That the quaternion keys' component is a `TransCom` rests on the dirty
  flag at offset 72 and the `Hmx::Matrix3` storage.
- `RotationKey` and `TransformSRTKey` field names, `UpdateEuler`, the
  flag names, the accessor helpers (`KeyQuat`, `AccessedKey`,
  `WrapAngle`, `AccumulateQuat`, `ReadSavedAngle`) and the `DataNode`
  colour flag's name are inferred.
