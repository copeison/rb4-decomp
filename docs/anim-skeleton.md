# Character skeletons

The skeleton and its resource from the map's `anim` module. Names follow
[naming.md](naming.md); the playback runtime is in
[anim-runtime.md](anim-runtime.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/CharSkeleton.o` | `0x2F7260`-`0x2FA3DB` | `src/anim/skeleton/CharSkeleton.cpp` |
| `anim/CharSkeletonResource.o` | `0x2FB6D0`-`0x2FBBFB` | `src/anim/skeleton/CharSkeletonResource.cpp` |
| `anim/CharBoneTransform.o` (inline) | - | `src/anim/skeleton/CharBoneTransform.h` |

`CharSkeletonResource::Init` (`0x2ADFF0`) and its factory (`0x2B5E60`)
are emitted in `anim/AnimInit.o` and defined with the class.

## CharBoneTransform

A bone's transform relative to its parent (52 bytes): a quaternion
(`Hmx::Quat`, added in `src/math/rotation/Quat.h`), a scale, a
translation and the parent's scale. `ToTransform` scales the rotation's
rows by the bone's scale and divides its columns by the parent's, so a
bone does not inherit its parent's scale.

## CharSkeleton

`CharSkeleton` (248 bytes, no vtable) holds five vectors: the parents (a
root is its own parent, parents come first), the child table, the bind
transforms, the names and the mesh-to-bone transforms, then a transform,
a rotation and a scale of the whole skeleton. `_ComputeChildIndices`
builds the child table as one offset per bone and an end offset, the
children of each bone (found among the following bones until a bone
whose parent comes earlier) and the roots; `GetChildIndices` and
`GetRootIndices` read it. `ComputeProjectSpaceXfm` composes a bone's bind
transform with its ancestors'. `GetAllowedValueBones` lists the names
for a bone property, sorted without regard to case. `Load` and `Save`
use revision 4; the mesh-to-bone transforms come before the names.

## CharSkeletonResource

`CharSkeletonResource` (64 bytes, vtable `0x18F6F38`, 13 slots;
extension "skel", category "Characters", metadata `0x19F9680`) owns the
skeleton and the FBX path. Only revision 1 of the cached file holds a
skeleton; reading the source file fails. `Fail` is true without a
skeleton. The emitted vtable matches the binary slot for slot.

## Weak evidence

- `CharSkeleton::mRootTransform`, `mRootRotation` and `mUnitScale`: no
  reader was found; the constructor makes them identity and 1.
- `CharBoneTransform::ToTransform` and the field names.
