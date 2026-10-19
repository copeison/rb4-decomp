# Character clips

The blend-tree-compatible character clips of the map's `anim` module: the
component that defines a clip on an object and the resource it builds and
plays. Both are newer than the reference map, so their names come from the
binary's strings ("CharClip", "CharClipResource", "charclip",
"CharClipConfig::BeatTrackEntry") and their callers. Names follow
[naming.md](naming.md); the playback runtime is in
[anim-runtime.md](anim-runtime.md) and the skeletons in
[anim-skeleton.md](anim-skeleton.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/CharClipCom.o` | `0x2E5C90`-`0x2EFF7F` | `src/anim/char/CharClipCom.cpp` |
| `anim/CharClipResource.o` | `0x2EFF80`-`0x2F135F` | `src/anim/char/CharClipResource.cpp` |

`CharClipResource::Init` (`0x2AFCC0`) and its factory (`0x2B7370`),
`CharClipCom::Init` (`0x2AFE30`), its factory (`0x2B73A0`) and placement
constructor (`0x2B73D0`) are emitted in `anim/AnimInit.o` and defined with
the classes. `CharClipCom.o` also holds the element operations of
`PropArray<CharClipConfig::BeatTrackEntry>` (vtable `0x18F5BA0`) and the
`std::function` wrappers of its registry's lambdas (vtables `0x18F5BF0` to
`0x18F6780`); the templates and lambdas produce them.

## CharClipConfig

What a clip is (136 bytes): a frame range of a source FBX animation
("source_fbx", "start_frame", "end_frame" exclusive), two beat/frame pairs
that align a beat-synced clip ("beat_track"), what it plays relative to
("relativity", "relative_source_fbx", "relative_frame"), the blend mode,
the blend-tree slot, "blend_from_root" and one `CharBoneChannelMask` per
skeleton bone ("bones"). `operator==` compares every field, pair and mask.
The stream form is revision 2: only the first two pairs are written, the
masks under their own revision 1, and the subtree index last as a word.

## CharClipCom

`CharClipCom` (432 bytes, vtable `0x18F5A48`, 41 slots; class id
"CharClip", category Animation, allowed in `CharResource`; description
"Defines a BlendTree-compatible character animation clip.", author
"Matt Sharpe") holds the edited configuration, the playback units, the
looping and the events, the clip resource, the configuration the resource
was last built from, and the editor's preview.

- `_OnResourcesLoaded` loads `"<layer path>_clips/<object>_<serial>.charclip"`
  (dots become underscores; "temp" for an unsaved layer), hands the resource
  the object id, units and looping, matches the bone masks to the root's
  `RndSkeletonPoseCom` skeleton by name (new bones keep every channel when
  the clip had none), takes the source or relative frame range after a
  file change, and sorts the events by frame.
- `Refresh` rebuilds a changed clip while listening to the entity's
  "component_created", then loads and re-enters the components created
  meanwhile; `_EditPoll` calls it after a frame change or while the clip
  plays. `CharClipResource::Create` is empty in this build: the tools build
  clips.
- The preview ("preview", "pause", "frame", none saved) plays the clip,
  looping, through the root's `AnimDriverCom`; one clip previews at a time
  (`sPreviewCom`, `0x19F8CF8`), and the skeleton returns to its bind pose
  when the last playback stops.
- "playback_units" offers CharAnim's units ("Real-time", "Beat", "Beat
  Aligned", "Downbeat Aligned", "Two Beat Aligned"). The registry is at
  revision 1: an older clip without beat/frame pairs gets two.
- The events array is registered by a copy of the helper
  `CharAnimCom.o` also has (`0x2E8F10` here, `0x2D8EE0` there): each event
  shows a summary and a "frame_editor_view" that edits its frame as a
  frame, a time and a percentage, through `GetEventFrame` and
  `SetEventFrame` (`0x32C8D0`, `0x32CBF0`).

| Statics | Address |
| --- | --- |
| `CharClipCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19F8AA0`, `0x19F8AA8`, `0x19F8AB0`, `0x19F8B50` |
| `CharClipCom::sPreviewCom` | `0x19F8CF8` |
| `CharClipResource::sMetaData`, `sBuildCallback` | `0x19F8D98`, `0x19F8DF0` |
| `CharClipResource::Id()`'s local static | `0x19F7D38` |

## CharClipResource

`CharClipResource` (232 bytes; extension "charclip", category
"Animations") is a `Resource` with the `AnimResource` interface at offset
48. Its primary vtable (`0x18F67D0`, 30 slots) has the resource's 13 slots,
then the overriders of the animation interface in declaration order; the
interface's own vtable is at `0x18F68D0`. It holds the configuration it was
cached with, a `CharAnim`, the units and looping its component sets, the
source and relative frame ranges and the id of the defining object.

- Revision 1 of the cached file is the configuration, the `CharAnim` when
  there is one and the four frame bounds; `LoadFile` reads the cached file
  only. `Fail` is true without a `CharAnim`.
- The range runs from 0 to the `CharAnim`'s last frame; the authored range
  is the source range, and event frames are offset by the clip's start
  frame. Every unit but real time plays on beats, through the `CharAnim`'s
  beat map; the aligned units keep 1, 4 or 2 beats in phase.
- `GetObject` and `GetEvents` find the defining object and its
  `CharClipCom`'s events in the playing entity.

Both emitted vtables match the binary slot for slot (`orbis-objdump -r`).

## Weak evidence

- `CharClipCom::Refresh`, `_UpdatePreviewRange`, `DisableAllBones`,
  `EnableAllBones`, `_GetDriver`, `_GetSkeletonPose`,
  `_OnComponentCreated`, `sPreviewCom` and the field names.
- `CharClipCom::mClipPath`: constructed empty and never read or copied.
- `CharClipResource::Create` (empty), `GetConfig` and
  `SetBuildCallback`/`sBuildCallback` (no callers; the callback's type is a
  guess) and the field names.
- `CharClipConfig`'s stream operators and `operator==`; the type name
  comes from the "CharClipConfig::BeatTrackEntry" registration.
- `RndSkeletonPoseCom::mSkeletonResource`, `mSkeleton` and `mPose`, which
  the clips read (`0x58`, `0x60`, `0x98`).

## Not reconstructed

Nothing in the two objects is left out.
