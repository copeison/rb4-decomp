# Animation runtime

The playback core of the map's `anim` module: the animation interface the
resources share, the options and ids of a playback, the drivers that
advance playbacks and send their events, the root component that owns
the drivers, the component that plays an instanced entity's animations,
the property animation resource and the property blender. Names follow
[naming.md](naming.md); the skeletons are in
[anim-skeleton.md](anim-skeleton.md), the key tracks in
[anim-key-tracks.md](anim-key-tracks.md), the key components in
[anim-prop-keys.md](anim-prop-keys.md) and the character classes in
[anim-characters.md](anim-characters.md), [anim-ik.md](anim-ik.md) and
[anim-clips.md](anim-clips.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/AnimInit.o` (`Anim::Init`, three `Init`s and factories) | `0x2A7080`, `0x2A7330`, `0x2A76E0`, `0x2ADFF0`, `0x2B1CD0`, `0x2B1D20`, `0x2B5E60` | `src/anim/core/AnimInit.cpp` and the classes' sources |
| `anim/AnimCom.o` | `0x326BE0`-`0x3276E7` | `src/anim/drivers/AnimCom.cpp` |
| `anim/AnimDriver.o` | `0x3276F0`-`0x3297EF` | `src/anim/drivers/AnimDriver.cpp` |
| `anim/AnimDriverCom.o` | `0x3297F0`-`0x32C6B6` | `src/anim/drivers/AnimDriverCom.cpp` |
| `anim/AnimInstID.o` | `0x32C6C0`-`0x32C6E8` | `src/anim/core/AnimInstID.cpp` |
| `anim/AnimOptions.o` | `0x32C6F0`-`0x32C757` | `src/anim/core/AnimOptions.cpp` |
| `anim/AnimResource.o` | `0x32C760`-`0x32CF92` | `src/anim/core/AnimResource.cpp` |
| `anim/PropAnimDriver.o` | `0x2FCAC0`-`0x2FD209` | `src/anim/drivers/PropAnimDriver.cpp` |
| `anim/PropAnimResource.o` | `0x2FD210`-`0x2FFD5F` | `src/anim/props/PropAnimResource.cpp` |
| `anim/PropertyBlenderCom.o` | `0x2D48E0`-`0x2D814F` | `src/anim/props/PropertyBlenderCom.cpp` |

The binary's order differs from the map's: `AnimInit.o` comes first, then
the key components, the resources, the IK components, `AnimCom.o`,
`AnimDriver.o`, `AnimDriverCom.o`, `AnimInstID.o`, `AnimOptions.o` and
`AnimResource.o`. The map's `TAnimDriver<T>` template is folded into
`AnimDriver` in this build.

## Registration

`Anim::Init` (`0x2A7080`; the startup code called it as the placeholder
`animation_register_types`) registers `AnimResource::Event` as a property
struct and then, in order: `AnimCom`, `AnimDriverCom`, `PropAnimResource`,
`PropAnimCom`, the key components (`PropKeysBoolCom`, eight
`PropKeysIntCom<T>`, `PropKeysFloatCom`, `PropKeysVec3Com`,
`PropKeysRotationCom`, `PropKeysTransformSRTCom`, `PropKeysQuatCom`,
`PropKeysObjCom`, `PropKeysSymCom`, `PropKeysStrCom`, `PropKeysColorCom`,
two `PropKeysWaveformCom<T>`), the character resources
(`CharAnimResource`, `CharAnimRawResource`, `CharSkeletonResource`,
`CharBoneMaskResource`, `CharLipsyncResource`), `CharAnimCom`,
`CharBoneMaskCom`, `CharIkCom`, `CharIkDriverCom`, `CharIkBoneInterp`,
`PropertyBlenderCom`, `CharClipResource` and `CharClipCom`. Every class's
`Init` and factory are reconstructed. The binary emits them in
`anim/AnimInit.o`; the source defines them with their classes, as
`InitComponentClass<T>` for the components and `_Init`, the factory and
`ResourceMetaData::Init` for the resources.

## Animation interface

`AnimResource` (vtable `0x18FA340`, 18 slots, no destructor) is the
second base of `PropAnimResource` and `CharAnimResource` (offset 200,
vtables `0x18F7308` and `0x18F5468`) and of `CharClipResource` (offset
48, `0x18F68D0`).

| Slots | Members |
| ---: | --- |
| 0-3 | `AsResource` (and its const form), `GetObject(Entity*)`, `GetCharData` |
| 4-8 | `IsLooping`, `SetLooping`, `GetStartFrame`, `GetEndFrame`, `GetUnits` |
| 9-10 | `GetBlendMode`, `GetSubTreeIndex` (0 by default; the character resources read their `CharAnim`) |
| 11-13 | `GetTimeFromFrame`, `GetFrameFromTime`, `GetTimeAlignment` |
| 14-17 | `GetEvents(Entity*)`, `GetAuthoredStartFrame`, `GetAuthoredEndFrame`, `GetEventFrameOffset` |

An event (`AnimResource::Event`, 16 bytes) is a frame and a name; a new
one is at frame 0 with the empty name, as the event arrays' element
constructor (`0x2D9ED0`) makes it. A clip's events are authored on its
source animation, so `EventFrameToFrame` subtracts slot 17.
`GetBlendModeAllowedValues` lists the `BlendMode` labels ("Additive",
"Layered"). `GetEventFrame` and `SetEventFrame` (`0x32C8D0`, `0x32CBF0`)
read and write an event's "frame" for the accessors of the character and
clip components' "frame_editor_view": they drop the accessor path's last
node, name the node before it "frame" and go through the component's
registry (`_GetProp`, `_SetProp`).

## Options and ids

`AnimOptions` (64 bytes) is what "start_anim" fills: the timeline (4 takes
the resource's), the range and start in time or frames
(`kInvalidTime`, 1e30, leaves them unset), the period, the scale,
`start_from_current_frame`, `start_paused`, a manual-time flag, the
blend-tree slot, the playback it replaces and its name. `AnimInstID`
(16 bytes) is a driver and a playback id; `sNullID` (`0x19BDA28`) has
no driver and id -1.

## Playbacks

`PlaybackInfo` (112 bytes) holds the resource, the clock, the range in
time units, the weight, the scale and the period's rate, the looping and
wrap state, the current time, the alignment period, the last polled
frame, the paused and manual flags, the auto-destruct mode, the
blend-tree slot, the blend in and the frame delta.

- `SetOptions` takes the range from the options or the resource (frames
  are converted with slot 11), swaps a reversed range, starts at the
  beginning or, for a negative speed, the end, and calls `_DoAlignment`.
- `AdvanceCurrentTime` adds the clock's delta (clamped to 1/15 on the
  second-based timelines) times scale and rate, keeps a beat-aligned
  playback in phase with its clock, wraps a loop (counting the passes)
  or clamps the time, measures the frame delta and eases the blend in
  with smoothstep.
- `SendEvents` exports `AnimEventMsg` ("anim_event": event, frame,
  weight, anim path, object, playback id) for every event the frame delta
  crossed, through every wrap of a loop, using binary searches over the
  sorted events.

`AnimDriver` (64 bytes, vtable `0x18F51D8`, five slots: destructors,
`StartAnim`, `_OnStopAnim`, `_Poll`) keeps an `eastl::list` of playbacks
sorted by blend-tree slot. `Poll` advances every playback, lets the
subclass apply them, then removes the finished auto-destructing ones and
pauses the others. `AnimDriverCom::_Poll` (`0x32A170`) passes its object,
the entity's root, so `Poll`, slot 4 and `PropDriver::Poll` take the root
`GameObject` (the map's `ObjPtr`). `PropAnimDriver` (96 bytes) feeds each
property animation's key components (`PropAnimResource::HarvestAllKeys`)
to one `PropDriver` per driven `PropRef`, creating them with the key
component's slot 41 and deleting them when their last keys stop.
`CharAnimDriver` (400 bytes) blends the character animations
([anim-characters.md](anim-characters.md)).

## PropAnimResource

`PropAnimResource` (216 bytes, extension "propanim", vtable `0x18F7190`,
45 slots; the animation interface at 200, vtable `0x18F7308`) is an
entity resource whose root holds the `PropAnimCom` and whose other
objects each hold one property's key component. Slots 33-44 are the
interface's overriders, in declaration order; most of the thunks repeat
their bodies.

- Looping and the frame range are the `PropAnimCom`'s (an unmeasured
  range gives 0). "time_units" holds a timeline (0-3), the same counted in
  frames (4-7, one frame a unit) or the default 8, which takes the
  "default_timeline" of the `AnimDriverCom` on the root of the entity
  resource the animation is inlined in. Frames convert at 30 a second and
  16 a beat (`AnimDriver::GetDefaultFramesPerUnit`); between seconds and
  beats an animation inlined in an `RBSongResource` converts through the
  song's `BeatSource` (`MsToTick`/`TickToBeat` one way, `BeatToTick`/
  `TickToMs` the other). It has no events and no alignment.
- `_LoadEntity` reads the resource revision (`mRev` at 208, from entity
  revision 33); `Save` writes revision 2. Before revision 2 each
  `PropKeysQuatCom` is replaced by a `PropKeysRotationCom` of the same
  property whose keys hold the quaternions with their Euler angles.
- The editing helpers find, add and remove keys: `FindKeys`,
  `_FindNextKeys`, `AddKeys` (refusing a property that overlaps one
  already keyed, `PropPath::Contains`), `RemoveKeys` (by property, object
  or component; removing an object also drops it from "target_order"),
  `RemoveAllKeys`, `HasKeys`, `QueryValidPropTarget` and `ApplyFrame`.
  `HarvestAllKeys(keys, includeIncomplete)` (`0x2FEAE0`) lists the key
  components; the driver asks only for complete properties.

## PropertyBlenderCom

`PropertyBlenderCom` (312 bytes, vtable `0x18F49A0`, 41 slots; class id
"PropertyBlenderCom", author "Chris McCarthy", allowed in a
`TransEntityResource`) blends between named frames of a property
animation ("anim"): "configurations" lists `Configuration`s (a name and a
frame, 16 bytes), "current_configuration" picks one (`SetConfiguration`,
`0x2D5F20`) and "target_frame" makes a "_custom_" one. When its resources
load (slot 29) it takes the entity's clock, loads the animation
(preferring a copy inlined in the entity) and creates an empty
`PropAnimResource` to blend from; on pass 0 of slot 30 it fixes the
animation's stale ids (`EntityResource::FixupStaleIds`, declared), copies
its keys into the blend animation (`AddKeys`), harvests its keys and
creates one `PropDriver` per key and the two playbacks. `_Poll` (slot 33)
starts a blend when the configuration changes or "force_blend" is set,
exporting "prop_blend_start": the blend animation's keys take the
properties' current values, the previous playback holds frame 0 and the
current one the configuration's frame. Each poll the current weight
follows "blend_curve" over "duration" (`Waveform<float>::Evaluate`,
declared) and the previous one its complement; at weight 1 it exports
"prop_blend_done" and removes the playbacks. The registration, the
property accessors, `PropArray<Configuration>`'s element operations
(vtable `0x18F4AF8`) and `Resource::GetOrLoad<PropAnimResource>`
(`0x2D7710`) are in the object; `Init` (`0x2AF8E0`) and the factories
(`0x2B7320`, `0x2B7350`) are in `anim/AnimInit.o`. Statics: `sId`
`0x19F8108`, `sClassName` `0x19F8110`, `sPropRegistry` `0x19F8120`,
`sMetaData` `0x19F81C0`. The emitted vtable matches the binary.

## AnimDriverCom

`AnimDriverCom` (592 bytes, vtable `0x18FA1E8`, 41 slots; class id
"AnimDriver", alias "AnimDriverCom") lives on the root object. Its one
property is "default_timeline"; it exports "anim_event" with the "event",
"frame", "weight", "anim" and "object" parameters. When its resources
load it takes the entity's clock (`TimeMgr::GetClock`) and enters both
drivers; it polls them in both modes and clears them when it exits for
anything but the entity's own exit.

`StartAnim(Resource*, AnimOptions const&)` picks the driver from the
resource's class id (`CharClipResource` shares the character driver),
replaces the named playback or `mReplaceID` (taking its time or frame
for `start_from_current_frame`), lets the project callback
`sProjectTimelineCallback` (`0x19FB8F0`, set by the game at `0xBB5590`)
choose the timeline, starts the playback and records it under its name.
`_FindResource` prefers a resource inlined into the entity. The handlers
are "start_anim", "stop_anim" and "apply_propanim_frame".

## AnimCom

`AnimCom` (24 bytes, vtable `0x18FA020`, 47 slots; class id "Animator",
alias "AnimCom") adds six slots (41-46: `StartAnim`, `StopAnim`,
`SetAnimWeight`, `SetAnimScale`, `SetAnimLoop`, `Clear`) that forward to
the `AnimDriverCom` of the entity its object's `InstanceCom` instances.

| Statics | Address |
| --- | --- |
| `AnimCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19FB410`, `0x19FB418`, `0x19FB420`, `0x19FB4C0` |
| `AnimDriverCom::sId`, `sClassName`, `sPropRegistry`, `sMetaData` | `0x19FB690`, `0x19FB698`, `0x19FB6A0`, `0x19FB740` |
| `AnimDriverCom::sProjectTimelineCallback` | `0x19FB8F0` |
| `AnimOptions::kInvalidTime`, `kInvalidPeriod` | `0x1275090`, `0x1275094` |
| `AnimInstID::sNullID` | `0x19BDA28` |
| `PropAnimResource::sMetaData`, `Id`'s local static | `0x19F9950`, `0x19F7BC8` |

The emitted vtables of `AnimCom`, `AnimDriverCom`, `AnimDriver`,
`PropAnimDriver`, `AnimEventMsg`, `AnimResource` and `PropAnimResource`
(both parts) match the binary slot for slot (`orbis-objdump -r`).

## Not reconstructed

- `PropAnimResource::_GetSongBeatSource` (`0x2FE040`): it reaches the
  song's `BeatSource` through `RBSongResource`'s song information
  component (`0xF27480`), its MIDI resource (`0xF216D0`) and that
  resource's slot 19, classes of `rb_audio` that are not modelled.
- The initializer of a header's symbol at `0x19F9938` (`0x2FFC40`) in
  `PropAnimResource.o`.
- The other gaps of the module are listed with their classes: the offline
  `CharAnim::Create` builder and the lip-sync XML import
  ([anim-characters.md](anim-characters.md)), two empty functions
  without callers ([anim-ik.md](anim-ik.md)) and the `Key.o` samplings
  ([anim-key-tracks.md](anim-key-tracks.md),
  [anim-prop-keys-numeric.md](anim-prop-keys-numeric.md)).

## Weak evidence

- Slot names 0-3 and 14-17 of `AnimResource`, `GetAuthoredStartFrame`,
  `GetAuthoredEndFrame`, `GetEventFrameOffset`, `EventFrameToFrame`,
  `FrameToEventFrame`, `GetEventFrame` and `SetEventFrame`.
- `AnimOptions::mManualTime`, `PlaybackInfo::mManualTime`,
  `mAutoDestruct` values 2 and 3, `_AlignToClock`, `GetTimingPosition`,
  `SendEvents`, `AnimEventMsg`, `AnimDriver::GetInstID`,
  `AnimDriver::GetBaseUnits`, and the `PropDriver` slot that takes the
  entity.
- `AnimDriver::GetDefaultFramesPerUnit` and `GetBaseUnits` sit in
  `PropAnimDriver.o`'s range; their out-of-line copies have no caller, and
  `PropAnimResource` inlines them.
- `PropAnimResource`'s names absent from the map: `kRev`, `mRev`,
  `GetPropAnimCom`, `_GetRawUnits`, `_IsInSong`, `_GetSongBeatSource`,
  `MakeErrorName`, `SetTargetOrder`, `GetTargetOrder` and `OffsetKeys`
  (no caller; what its threshold means is a guess); the flag arguments of
  `HarvestAllKeys` and `HasKeys`; `BeatSource`'s slot order, inferred
  from the conversions alone.
- `PropertyBlenderCom`'s field names, `BlendInfo` and its `mPlaying`,
  `EntityResource::FixupStaleIds` and the file's helpers. The map's
  `_LoadResources(ObjPtr const&)` has the body of slot 29 and its
  `_Fixup(bool)` may be slot 30; its `_Enter` has no counterpart in this
  build's vtable.
