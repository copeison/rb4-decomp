# Character animations

The character side of the map's `anim` module: the compressed animation
and its raw source, the channel buffers they blend through, the skeleton
pose, the driver that plays them and their resources and component.
Names follow [naming.md](naming.md); the playback runtime is in
[anim-runtime.md](anim-runtime.md) and the skeletons in
[anim-skeleton.md](anim-skeleton.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/CharAnimCom.o` | `0x2D8150`-`0x2DD2EF` | `src/anim/char/CharAnimCom.cpp` |
| `anim/CharAnimDriver.o` | `0x2DD2F0`-`0x2DF92F` | `src/anim/drivers/CharAnimDriver.cpp` |
| `anim/CharAnimRawResource.o` | `0x2DF930`-`0x2DFE8F` | `src/anim/char/CharAnimRawResource.cpp` |
| `anim/CharAnimResource.o` | `0x2DFE90`-`0x2E14DF` | `src/anim/char/CharAnimResource.cpp` |
| `anim/CharBoneStateSetFormat.o` | `0x2E4ED0`-`0x2E5C8F` | `src/anim/char/CharBoneStateSetFormat.cpp` |
| `anim/CharLipsyncResource.o` | `0x2F6700`-`0x2F716F` | `src/anim/char/CharLipsyncResource.cpp` |
| `anim/CharSkeletonPose.o` | `0x2FA3E0`-`0x2FB6CF` | `src/anim/skeleton/CharSkeletonPose.cpp` |
| `anim/CharAnim.o` | `0x32CFA0`-`0x331A4F` | `src/anim/char/CharAnim.cpp` |
| `anim/CharAnimBuffer.o` | `0x331A50`-`0x3330FF` | `src/anim/char/CharAnimBuffer.cpp` |
| `anim/CharAnimRaw.o` | `0x333100`-`0x334DAF` | `src/anim/char/CharAnimRaw.cpp` |
| `anim/CharBoneKeyframeSet.o` | `0x334DB0`-`0x3361CF` | `src/anim/char/CharBoneKeyframeSet.cpp` |
| `anim/CharBoneStateBuffer.o` | `0x3362C0`-`0x3368FF` | `src/anim/char/CharBoneStateBuffer.cpp` |
| lip-sync data (no map object) | `0x336900`-`0x33A9AB` | `src/anim/char/CharLipsync.cpp` |

The registrations and factories (`0x2ADD10`, `0x2ADE80`, `0x2AE2D0`,
`0x2AE440`, `0x2B5E00`, `0x2B5E30`, `0x2B5ED0`, `0x2B5F00`) are emitted in
`anim/AnimInit.o` and defined with their classes. `QuatXfm` (translation
and quaternion, 28 bytes) is in `src/math/transform/QuatXfm.h`.

## Data

- `CharBoneStateSetFormat` (88 bytes): seven channel kinds (position,
  scale, inverse parent scale, rotation, X/Y/Z rotation), the cumulative
  channel count and data end of each, and the bone name of each channel.
  Built from a bone mask's `CharBoneChannelMask`s; saved with a string
  table.
- `CharBoneStateBuffer` (104): a format, its data and one weight per
  channel; `NormalizeChannelData(additive)` divides by the weights (the
  additive form completes scales and rotations with the identity).
- `CharBoneKeyframeSet` (112): format, keyframe count, relative flag,
  keyframe data and optional root `QuatXfm`s. `Apply` clamps the frame,
  adds its two keyframes, keeps the root bones relative to the
  animation's root motion and accumulates the frame's root motion
  (through the loop end when the playback wrapped).
- `CharAnimBuffer` (256): absolute and relative buffers, the frame's root
  motion and its weight, the skeleton. `Apply`/`ScaleDown` walk a set's
  channels and the buffer's same-named channels in step (single-axis
  rotations land on the full rotation's (axis, w) pair); `EndFrame`
  normalizes and composes relative onto absolute (bind pose for unset
  channels; positions add, scales multiply, rotations compose).
- `CharAnim` (192): frame rate, beat track (`TKeyTrack<float>`),
  keyframes, blend mode and blend-tree slot; `Units` (realtime, beat,
  beatAlign1/4/2). Revision 10 (`kRev`, `0x12752E0`).
- `CharAnimRaw` (128): frame rate, beat track, per-bone samples and
  `BoneInfo` (name, inverse-parent-scale flag, bind-pose flag, parent).
- `CharLipsync` (112) and its `Playback` (48): viseme/track names, frame
  offsets and (viseme, weight) byte pairs per track.

## Classes

- `CharAnimCom` (112, vtable `0x18F4BE8`, class id "CharAnim", alias
  "CharAnimCom"; statics `0x19F83B0`-`0x19F8460`, `kSubObjectResource`
  "rawanim" `0x19F8608`): "relative", "relative_anim", "playback_units",
  "is_looping", "events" (with the "frame_editor_view" frame/time/percent
  views), "bone_mask_file", "anim_source_file"; revision 4.
- `CharAnimResource` (224, vtables `0x18F52D8` and `0x18F5468`,
  metadata `0x19F86E8`, `sCacheVisitor` `0x19F8740`): reads the cached
  CharAnim or builds it from the bone mask and raw source (relative to
  itself, another animation or the bind pose).
- `CharAnimRawResource` (56, vtable `0x18F5260`, metadata `0x19F8688`)
  and `CharLipsyncResource` (56, vtable `0x18F6EC0`, metadata
  `0x19F95C0`; reads `<base>_lipsync.xml` or a newer cache).
- `CharSkeletonPose` (120): `BoneData` (local, object transform, dirty),
  channel-to-bone map, root transform, root reset frame.
- `CharAnimDriver` (400, vtable `0x18F51A0`): blends each blend-tree slot
  (layered playbacks hide earlier ones; fully hidden playbacks with
  auto-destruct 3 or 4 stop), writes the pose through the root's
  `RndSkeletonPoseCom` and runs the IK components by priority.

All emitted vtables match the binary slot for slot.

## Weak evidence

- `CharAnimBuffer::mSkeletonPath`, `RndSkeletonPoseCom::GetSkeletonPath`,
  `CharSkeletonPose::SetRootXfm`/`ResetRootXfm`/`mRootResetFrame`,
  `CharAnimRaw::BoneInfo` flag names, `CharBoneKeyframeSet::mRelative`,
  `CharLipsync` and its members, `CharAnimDriver::mDisabled`,
  `GetEventFrame`/`SetEventFrame`, `FastInterp` for `0x216890`, the
  `CharBoneChannel` enumerators, `RegisterAnimEvents`.
- `ResourcePathMetadata::mDefaultExtension` (`0x9D0`): CharAnimCom's tag
  returns the entity file's folder, so the field is more likely a default
  folder.

## Not reconstructed

- `CharAnim::Create` builder (`0x32D4B0`, 9.4 KB) and its helpers (facing
  computation `0x32FA50`/`0x32FC10`, raw sampling `0x330B80`, sorts and
  list formatting `0x3302C0`, `0x331000`-`0x331482`, `0x331590`): the
  offline build from raw data; shipped data loads cached animations.
- `CharLipsync::LoadFromXml` and its FaceFX node graph
  (`0x337520`-`0x33A9AB`).

## Notes for other modules

- `AnimDriverCom::_Poll` (`0x32A170`) passes its root `GameObject` to
  `AnimDriver::Poll`, not the entity; `CharAnimDriver::_Poll` casts back.
- `PropArray<AnimResource::Event>`'s construct (`0x2D9ED0`) zeroes the
  frame; `AnimResource::Event` has no default for it.
- `0x2F7170` is CharSkeleton's constructor, emitted after the lip-sync
  resource.
