# Bone masks and inverse kinematics

The bone-mask resource and component and the IK components of the map's
`anim` module. Names follow [naming.md](naming.md); the skeletons are in
[anim-skeleton.md](anim-skeleton.md), the playback runtime in
[anim-runtime.md](anim-runtime.md) and the component base in
[entity-components.md](entity-components.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/CharBoneChannelMask.o` | `0x2E14E0`-`0x2E1AEF` | `src/anim/char/CharBoneChannelMask.cpp` |
| `anim/CharBoneMaskCom.o` | `0x2E1B00`-`0x2E4C0F` | `src/anim/char/CharBoneMaskCom.cpp` |
| `anim/CharBoneMaskResource.o` | `0x2E4C10`-`0x2E4ECF` | `src/anim/char/CharBoneMaskResource.cpp` |
| `anim/CharIk.o` | `0x2F1360`-`0x2F157F` | `src/anim/char/CharIk.cpp` (header `CharIkCom.h`) |
| `anim/CharIkBoneInterpCom.o` | `0x2F1580`-`0x2F511F` | `src/anim/char/CharIkBoneInterpCom.cpp` |
| `anim/CharIkDriverCom.o` | `0x2F5120`-`0x2F66FF` | `src/anim/char/CharIkDriverCom.cpp` |
| `render/CharResource.o` (id only) | - | `src/render/scene/CharResource.h` |

Each class's `Init`, factory and placement factory are emitted in
`anim/AnimInit.o` and defined with the class: `CharBoneMaskResource`
(`0x2AE160`, `0x2B5E90`), `CharBoneMaskCom` (`0x2AE800`, `0x2B5F50`,
`0x2B5F80`), `CharIkCom` (`0x2AEBE0`, `0x2B5FA0`, `0x2B5FE0`),
`CharIkDriverCom` (`0x2AEFC0`, `0x2B6280`, `0x2B6300`) and
`CharIkBoneInterp` (`0x2AF460`, `0x2B6540`, `0x2B65C0`). `CharIkCom`'s
virtual methods are inline in the map's header and emitted there too
(`0x2B6020`-`0x2B6274`). `Resource::GetOrLoad<CharBoneMaskResource>`
(`0x2E1330`) is a template emitted by the object before.

## CharBoneChannelMask

A bone's channel mask (16 bytes, no vtable): the position, scale and
inverse-parent-scale flags, a rotation (`None`, `Full`, `X`, `Y`, `Z` as 0,
1, 2, 3, 4) and the bone's name. `SetSRTIPS` keeps the position, full
rotation and both scales, `SetDisable` masks everything, and
`GetBitfield` packs the kept channels into bits 0-6 (position, scale,
inverse parent scale, full, x, y, z). The object also holds the rotation
labels (`GetRotationAllowedValues`), `operator!=` and the stream
operators (a revision 0, the name, three flag bytes, the rotation), whose
callers are in the character animation code.

## CharBoneMaskResource

An entity resource (200 bytes, vtable `0x18F5930`, 33 slots; extension
"bonemask", category "Animations", parent "EntityResource", metadata
`0x19F8A20`) whose new entities get a `CharBoneMaskCom` on the root.
`Save` only forwards to `EntityResource::Save`, and the map's `Load`
override is gone: slot 4 is `EntityResource::Load`.

## CharBoneMaskCom

The bone mask (192 bytes, vtable `0x18F5508`, 41 slots; class id
"BoneMask", alias "CharBoneMaskCom", category Animation, allowed in a
`CharBoneMaskResource`). Its properties are the actions "disable_all",
"enable_all" and "harvest_preview_chars" (empty in this build),
"hide_inactive" (not saved), "bone_channel_masks", "skeleton_file",
"subtree_index", "blend_mode", "blend_from_root", "constraints" (a source
bone a target bone follows), "harvest_anims_under" and "preview_chars"
(`CharResource` files). Revision 1; the handler converts revision 0's
float "blend_order" into "subtree_index". A mask's help names the parent
bone, its display name is the bone, and with "hide_inactive" a mask that
keeps nothing is hidden.

When its resources load (slot 29) it takes the skeleton file, inlined into
the entity or loaded, and keeps the masks while their names follow the
skeleton's bones. Otherwise it rebuilds them in the skeleton's order,
keeping each remaining bone's mask and disabling new ones. Without a
skeleton the masks are cleared. `GetSkeletonResource` (`0x2E37A0`) hands
the loaded skeleton to the bone-mask builders (callers `0x2E0594`,
`0x32D470`).

## IK components

`CharIkCom` (40 bytes, vtable `0x18F1958`, 43 slots; class id "CharIk",
category Character, allowed in `CharResource` entities) is registered but
not created. It adds slot 41 `Apply(CharSkeletonPose&, CharSkeleton
const*)` (empty) and slot 42 `GetPriority` (0, never called), and a
`LinkedList::Node` through which a driver links it.
`_InitAsSuperclass` (`0x2F3C20`) makes it a subclass's superclass, as
`RndDrawInstanceCom`'s does. Other modules' IK classes call it too.

`CharIkDriverCom` (96 bytes, vtable `0x18F6C68`; class id "CharIkDriver",
root only) holds "ik", a list of object and component-class references.
The object lists the objects with a `CharIkCom`, the component their
`CharIkCom` classes, and an element shows as "object : component" or its
index. When its resources load it relinks the referenced `CharIkCom`s in
order (`GameObject::FindCom`), and `Apply` runs them.

`CharIkBoneInterp` (88 bytes, vtable `0x18F6970`; class id
"CharIkBoneInterp") poses "targets" (`BoneTarget`, 152 bytes): a target
bone, a weighted position from "position_calculation" (`PositionSource`,
24 bytes), and a primary and a secondary axis (`AxisSpec`, 48 bytes) each
blended from `AxisSource`s (24 bytes) that take another bone's x, y or z
axis or the direction to it ("look at"). `BoneTarget::Apply` averages the
positions by the weights' magnitudes, normalizes the primary axis, makes
the secondary perpendicular to it, and sets the third axis to their cross
product in the order that keeps the frame right-handed. It works in the
pose's object space (`CharSkeletonPose::GetObjectXfm`, `SetObjectXfm`).
The bone indices are resolved from the skeleton of the root's
`RndSkeletonPoseCom` when the resources load, and the bone properties list
that skeleton's bones.

| Statics | Address |
| --- | --- |
| `CharBoneMaskCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19F87B0`, `0x19F87B8`, `0x19F87C0`, `0x19F8860` |
| `CharBoneMaskResource::sMetaData`, `Id()::id` | `0x19F8A20`, `0x19F7CB8` |
| `CharIkCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19F8E28`, `0x19F8E30`, `0x19F8E40`, `0x19F8EE0` |
| `CharIkBoneInterp::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19F90E0`, `0x19F90E8`, `0x19F90F0`, `0x19F9190` |
| `CharIkDriverCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19F9360`, `0x19F9368`, `0x19F9370`, `0x19F9410` |
| `CharResource::Id()::id` | `0x19F8A08` |

The emitted vtables of the five classes match the binary slot for slot
(`orbis-objdump -r`).

## Weak evidence

- `CharIkBoneInterp::mResolved` (offset 40): the constructors clear it and
  nothing in the module reads or sets it.
- Names absent from the map: `CharBoneChannelMask::operator!=`,
  `GetRotationAllowedValues`, the stream operators and the enumerators;
  `CharBoneMaskCom::GetSkeletonResource` and `mSkeleton`;
  `CharIkCom::mIkNode`, `IkNode` and `_InitAsSuperclass`;
  `CharIkDriverCom::mIks`; `CharIkBoneInterp::_ResolveBones`; the
  `sClassName` statics.
- The map's `_Fixup(bool)` of `CharBoneMaskCom`, `CharIkCom` and
  `CharIkDriverCom` has no counterpart in this build's vtables.
- `ComMetaData::mReserved`, which `CharIkBoneInterp::_Init` sets as do
  `CharClipCom::_Init` (`0x2E6360`) and the "PathMesh" registration
  (`0x5E45B0`); it has no identified reader.

## Not reconstructed

- The empty functions without callers at `0x2E3870` (`CharBoneMaskCom.o`)
  and `0x2F4530` (`CharIkBoneInterpCom.o`).
- `CharResource`, of which only the id is declared.
