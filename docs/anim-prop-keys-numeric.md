# Numeric property-animation keys (analysis only)

Status: analysed and not reconstructed. The six classes below still have their
original stub headers under `src/anim/props/` (only `Init()` is declared).
This page records what the binary shows, so the reconstruction can follow the
`PropKeysBoolCom` pattern directly.

## Files (map objects)

| Object | Range | Primary vtable (slots) | Statics sId / sClassName / sPropRegistry / sMetaData | Subtype static (guard) |
|---|---|---|---|---|
| anim/PropKeysColorCom.o | 0x302B50-0x305080 | 0x18F77D0 (76) | 0x19F9C50/58/60, 0x19F9D00 | 0x19F9EA8 (0x19F9EB0), "" |
| anim/PropKeysFloatCom.o | 0x305D40-0x3087DF | 0x18F7E10 | 0x19F9F18/20/30, 0x19F9FD0 | 0x19FA178 (0x19FA180), "" |
| anim/PropKeysQuatCom.o | 0x311080-0x3146FF | 0x18F87E0 | 0x19FA4A0/A8/B0, 0x19FA550 | 0x19FA6F8 (0x19FA700), "quaternion" |
| anim/PropKeysRotationCom.o | 0x314990-0x318BFF | 0x18F8BC8 | 0x19FA730/38/40, 0x19FA7E0 | 0x19FA988 (0x19FA990), "" |
| anim/PropKeysTransformSRTCom.o | 0x31E2E0-0x32233F | 0x18F97D0 | 0x19FAF00/08/10, 0x19FAFB0 | 0x19FB158 (0x19FB160), "" |
| anim/PropKeysVec3Com.o | 0x322340-0x3251FF | 0x18F9BB8 | 0x19FB180/88/90, 0x19FB230 | 0x19FB3D8 (0x19FB3E0), "" |

Per class, the functions are: StaticCreatePropDriver, Register, CreatePropDriver,
an unreferenced `"TKey<kX>"` name function, _Init, RegisterPropStruct<TKey<T>>
and _InitPropKeysBase<T>:

- Color: 0x302B50, 0x302C20, 0x302D00, 0x302DD0, _Init 0x302DE0, 0x1C94C0 (shared), 0x303210.
- Float: 0x305D40, 0x305E10, 0x305EF0, 0x305FC0, _Init 0x305FD0, 0x3064E0 (TKey<float>) and 0x1C9310 (TKey<ValueTangentFloat>), 0x306690.
- Quat: 0x311080, 0x311150, 0x311230, 0x311300, _Init 0x311310, 0x311B20, 0x311CD0.
- Rotation: 0x314990, 0x314A60, 0x314B40, 0x314C10, ctor 0x314C20, _Init 0x314CD0, 0x315420, 0x3155D0.
- SRT: 0x31E3B0, 0x31E2E0 is the factory, 0x31E490, 0x31E560, ctor 0x31E570, _Init 0x31E620, 0x31ED00, 0x31EEB0.
- Vec3: 0x322340, 0x322410, 0x3224F0, 0x3225C0, _Init 0x3225D0, 0x322A10, 0x322BC0.

The TPropDriver vtables are Float 0x18F8108, Vec3 0x18F9EB0, Quat 0x18F8AD8,
Color 0x18F7AC8, Rotation 0x18F8EC0 and SRT 0x18F9AC8. Slot 5 (Poll) checks for
entries and then jumps to _Poll: Float 0x306C30, Color 0x3037B0, Quat 0x312270,
Rotation 0x317A70, SRT 0x321E10 and Vec3 0x324B60.

## Value types and key layouts

- Float: `TKey<ValueTangent<float>>` (20 bytes). The property type is kFloat. The
  binary's default ValueTangent is `{0, 0, 0, mAutoTangents = true}`, seen in
  GetValueAtFrame, UpdateValueAtIndex and the _Init prototype. Key.h's aggregate
  value-initialises it with `mAutoTangents = false`, so it needs a constructor.
- Vec3: `TKey<Vector3>` (16 bytes), kVector3 (20).
- Quat: `TKey<Hmx::Quat>` (20 bytes), kStruct (15) "quaternion".
- Color: `TKey<Hmx::Color>` (20 bytes), kColor (13).
- Rotation: `RotationKey` (32 bytes, TKey 36). It holds `Vector3 mEuler` at 0,
  `Hmx::Quat mQuat` at 12 and `uint8 mFlags` at 28. Flag 1 means the Euler angles
  are stale; flag 2 means the quaternion is stale. Readers refresh the stale part
  with MakeEuler(quat, euler) at 0x2179C0 or Hmx::Quat::Set(const Vector3&) at
  0x216480. The default is `{0, (0,0,0,1), 0}`. kEuler (21). The ctor sets
  interpolation 0x40000001.
- TransformSRT: `TransformSRTKey` (56 bytes, TKey 60). It holds `PropXfm mXfm` at
  0, `Hmx::Quat mQuat` at 36 and `uint8 mFlags` at 52, with the same flags. The
  default is the identity transform, `(0,0,0,1)` and 0. kXfm (22). Interpolation
  is 0x40000001.
- The other classes default to interpolation 1 (Linear).

The binary's default constructors differ from the repo's headers.
`Hmx::Color()` is (0,0,0,1), `Hmx::Quat()` is (0,0,0,1), `Hmx::Matrix3()` is the
identity and `PropXfm()` is the identity. In the repo, Color is `= default` and the
others are aggregates.

## Overridden slots (beyond the PropKeysBoolCom pattern)

- Float: HasTangents (42) returns true. IsValidInterpolation (57) returns true.
  - 73: `DataNode(value.mValue)`.
  - 74: `{Float(), 0, 0, true}`.
  - 75: `{0, 0, 0, true}`.
- Vec3: 57 returns true.
  - 73: an array of three float nodes (MakeDataArray, kDataArray).
  - 74: `value.Array()` is called once and discarded, then
    `Vector3{Array()->Float(0), Float(1), Float(2)}`.
  - 75: zero.
- Color: 57 returns `interpolation != Spline`. 58 returns true.
  - 73: `DataNode(const Hmx::Color&, bool)` at 0x237F90 with false. This builds a
    kDataColor node; without the flag it adds a `(gamma linear)` array.
  - 74: `DataNode::Color(bool gamma, const DataArray*)` at 0x17CA70 with
    (false, nullptr). This is the map's inline DataNode::Color: Evaluate, then
    LiteralColor.
  - 75: (0,0,0,0).
- Quat: 57 returns true.
  - 73: with forEditor, the Euler angles in degrees (×57.29578) as three floats;
    otherwise x, y, z, w. The 4-node MakeDataArray is emitted at 0x313FB0.
  - 74: an array of three gives degrees → Quat::Set → Normalize; four gives x, y, z, w.
  - 75: (0,0,0,0).
  - 63 (0x312890), 69 (0x3127D0) and 70 (0x312910) treat the driven property's
    storage as an `Hmx::Matrix3`, found through PropRef::Find(Component*, void**)
    at 0x17FC70. 63 calls Quat::Set(Matrix3) at 0x2165F0. 70 calls
    MakeRotMatrix(quat), copies the result and sets the TransCom dirty flag
    (`mDirtyFlags |= 1`, offset 72).
- Rotation:
  - 57, 59 and 60 return true.
  - 63 (0x315CC0) and 69 (0x315B70) take `Get<PropRotation>` (kEuler) and build
    RotationKey(euler): the identity quaternion, then `Set(euler)`, then
    `flags &= ~2`.
  - 70 (0x315E00) samples, refreshes the Euler angles and sets kEuler.
  - 73: refreshes the Euler angles and returns three floats.
  - 74: like Vec3, then RotationKey(euler).
  - 75: the default key.
- SRT:
  - 57, 59 and 60 return true.
  - 63 (0x31F810) and 69 (0x31F450) use Get<PropXfm> (0x31F530, identity
    default), then `TransformSRTKey(xfm)`.
  - 70 (0x31F8F0) refreshes the Euler angles and sets kXfm.
  - 73: refreshes and returns `new DataArray(9)` of floats.
  - 74: nine floats.
  - 75: the default key.

## Drivers

- Float `_Poll`: a weighted mean of the sampled values over the unpaused
  playbacks. If the summed weight is above 0, it sets `{sum/weights, 0, 0, true}`
  through the generic _SetDrivenProperty (0x306D60, kFloat).
- Color `_Poll`: on the first sample, `sum = sample*w`. After that,
  `sample *= w; sum += sample`. Finally `sum *= 1/weights`, then it sets
  kColor (0x305080).
- Quat `_Poll`: on the first sample, `sample *= w; sum = sample`. After that it
  accumulates `scaled = {x|w|, y|w|, z|w|, w*w}`, adding it when
  `dot(sum, scaled) >= 0` and subtracting it otherwise. At the end, when the
  weights are above 0 and `dot(sum, sum) != 0`, it accumulates sum by
  1/weights the same way and normalises (0x215A30). It then writes
  MakeRotMatrix into the Matrix3 storage and sets the TransCom dirty flag.
  0x312720 is the same write as an unreferenced out-of-line specialization.
- Vec3, Rotation and SRT use the generic heaviest-weight `_Poll`. Rotation's
  (0x316250) and SRT's (0x31FD70) _SetDrivenProperty refresh the Euler angles
  first and set kEuler or kXfm.

## Key.o sampling specializations (entity/Key.o)

| Function | Address | Notes |
|---|---|---|
| TKeyTrack<Vector3>::_SampleLinearInterpValue | inline | t==0 → prev, t==1 → next, otherwise lerp. Vec3's _DoSampleValueAtFrame is 0x324D40 |
| TKeyTrack<Vector3>::_SampleSplineInterpValue | 0x1DCCF0 | Hermite. SplineTangent (from the map) is inlined twice: 2 keys → k1-k0; first key → (k1-k0)·1.5+(k0-k2)·0.25; the last mirrors it; otherwise (k[i+1]-k[i-1])·0.5 |
| TKeyTrack<Hmx::Quat>::_SampleLinearInterpValue | inline | FastInterp at 0x216890 |
| TKeyTrack<Hmx::Quat>::_SampleSplineInterpValue | 0x1DD190 | Catmull-Rom on q[i-1], q[i], q[i+1], q[i+2] (clamped at the ends), with the neighbours flipped onto q[i]'s hemisphere, then Normalize |
| TKeyTrack<Hmx::Color>::_SampleLinearInterpValue | 0x1DEA70 | With interpolation flag 0x20000000, InterpLinearAsGamma_sRGBInHSV. Otherwise LinearToGamma both, lerp, GammaToLinear |
| TKeyTrack<Hmx::Color>::SampleValueAtFrame | 0x2424A0 | generic; spline does nothing |
| TKeyTrack<RotationKey>::_SampleLinearInterpValue | 0x1DD340 | Flag 0x40000000 gives InterpEulerLinear (0x2186E0) on refreshed Euler angles and flags=(f&~3)\|2. Otherwise FastInterp on refreshed quaternions and flags=(f&~3)\|1 |
| TKeyTrack<RotationKey>::_SampleSplineInterpValue | 0x1DD4B0 | large; its tangent helper is 0x1E0AD0 |
| TKeyTrack<TransformSRTKey>::_SampleLinearInterpValue | 0x1DD9B0 | Flag 0x40000000 gives InterpXfmLinear (0x2192E0). Otherwise lerps scale and translation, then FastInterp on the quaternions |
| TKeyTrack<TransformSRTKey>::_SampleSplineInterpValue | 0x1DDC50 | large (0xE1A bytes) |

## _Init notes

- All six classes use author "Matt Sharpe", category 6 and allowed resource
  PropAnimResource.
- Rotation:
  - The key struct sets mHelp before "frame".
  - "value" is accessor-only (offset -1, kEuler) with EulerMetadata. Its getter
    (0x317560) and setter (0x3175E0) find the key index at path[-2].
  - "serialized_value" (offset 4, 32 bytes) holds euler, quat {x, y, z, w} and
    flags (uint8).
  - The key registry is at revision 1. Its handler (0x317C40) reads the old
    "value pitch/roll/yaw" floats from mSavedProps and sets the key's Euler
    angles.
- SRT: the same, with XfmMetadata. The getter is 0x321900 and the setter
  0x321980. "serialized_value" (offset 4, 56 bytes) holds transform, quat and
  flags. There is no revision.
- Quat:
  - "value" is an empty struct. Inside it, "euler_rotation" has mKind 1 and the
    accessor floats x, y, z in degrees, with help "Rotation angle about the X axis
    (ie. pitch)", "(ie. roll)" and "(ie. yaw)".
  - Getter/setter pairs: x 0x313BF0/0x313C60, y 0x313D30/0x313DA0, z
    0x313E70/0x313EE0. The key index is at path[1]. The setters wrap the angle to
    [-π, π) with fmodf and then call Quat::Set.
  - "quat_rotation" (offset 4) holds x, y, z, w with mIsHidden. It has no remap.
- Float: "frame", "value" (remapped), "float_tangent1", "float_tangent2" and "auto".
- Color and Vec3: "frame" and "value" (remapped), with mHelp after them.

## Declarations the reconstruction will need (not added)

- `PropRef::Find(Component*, void**) const` at 0x17FC70.
- `DataNode(const Hmx::Color&, bool)` at 0x237F90 and
  `Hmx::Color DataNode::Color(bool, const DataArray*) const` at 0x17CA70.
- The 4-node MakeDataArray at 0x313FB0.
- `Hmx::Quat::Set(const Vector3&)` at 0x216480 and
  `MakeEuler(const Hmx::Quat&, Vector3&)` at 0x2179C0.
- An inline `Interp(const Vector3&, const Vector3&, float, Vector3&)`.
- PropValueType specializations for ValueTangent<float>, PropRotation and
  PropXfm, in the key headers. The float, Vector3 and Hmx::Color ones already
  exist in `entity/props/PropRef.h`.
