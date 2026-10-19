# Animation runtime

The playback core of the map's `anim` module: the animation interface the
resources share, the options and ids of a playback, the drivers that
advance playbacks and send their events, the root component that owns
the drivers and the component that plays an instanced entity's
animations. Names follow [naming.md](naming.md); the skeletons are in
[anim-skeleton.md](anim-skeleton.md) and the key tracks in
[anim-key-tracks.md](anim-key-tracks.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/AnimInit.o` (`Anim::Init`, three `Init`s and factories) | `0x2A7080`, `0x2A7330`, `0x2A76E0`, `0x2ADFF0`, `0x2B1CD0`, `0x2B1D20`, `0x2B5E60` | `src/anim/core/AnimInit.cpp` and the classes' sources |
| `anim/AnimCom.o` | `0x326BE0`-`0x3276E7` | `src/anim/drivers/AnimCom.cpp` |
| `anim/AnimDriver.o` | `0x3276F0`-`0x3297EF` | `src/anim/drivers/AnimDriver.cpp` |
| `anim/AnimDriverCom.o` | `0x3297F0`-`0x32C6B6` | `src/anim/drivers/AnimDriverCom.cpp` |
| `anim/AnimInstID.o` | `0x32C6C0`-`0x32C6E8` | `src/anim/core/AnimInstID.cpp` |
| `anim/AnimOptions.o` | `0x32C6F0`-`0x32C757` | `src/anim/core/AnimOptions.cpp` |
| `anim/AnimResource.o` (part) | `0x32C760`, `0x32CF10`-`0x32CF92` | `src/anim/core/AnimResource.cpp` |
| `anim/PropAnimDriver.o` | `0x2FCAC0`-`0x2FD209` | `src/anim/drivers/PropAnimDriver.cpp` |

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
`PropertyBlenderCom`, `CharClipResource` and `CharClipCom`. Only
`AnimCom`'s, `AnimDriverCom`'s and `CharSkeletonResource`'s `Init` are
reconstructed; the others are declared in stub headers under
`src/anim/props` and `src/anim/char`.

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

An event (`AnimResource::Event`, 16 bytes) is a frame and a name. A
clip's events are authored on its source animation, so
`EventFrameToFrame` subtracts slot 17. `GetBlendModeAllowedValues` lists
the `BlendMode` labels ("Additive", "Layered").

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
pauses the others. `PropAnimDriver` (96 bytes) feeds each property
animation's key components to one `PropDriver` per driven `PropRef`,
creating them with the key component's slot 41 and deleting them when
their last keys stop. `CharAnimDriver` (400 bytes) is declared only.

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

The emitted vtables of `AnimCom`, `AnimDriverCom`, `AnimDriver`,
`PropAnimDriver`, `AnimEventMsg` and `AnimResource` match the binary slot
for slot (`orbis-objdump -r`).

## Not reconstructed

- The event "frame" accessors in `AnimResource.o` (`0x32C8D0`,
  `0x32CBF0`).
- Every other class `Anim::Init` registers; `PropAnimResource` is
  declared with `ApplyFrame` (`0x2FF4C0`) and `GetPropKeys`
  (`0x2FEAE0`); `CharAnimDriver` with its constructor and destructor.
- The key components, `PropertyBlenderCom`, the character animation,
  bone-mask, IK, lip-sync and clip classes, `CharSkeletonPose`.

## Weak evidence

- Slot names 0-3 and 14-17 of `AnimResource`, `GetAuthoredStartFrame`,
  `GetAuthoredEndFrame`, `GetEventFrameOffset`, `EventFrameToFrame`,
  `FrameToEventFrame`.
- `AnimOptions::mManualTime`, `PlaybackInfo::mManualTime`,
  `mAutoDestruct` values 2 and 3, `_AlignToClock`, `GetTimingPosition`,
  `SendEvents`, `AnimEventMsg`, `AnimDriver::GetInstID`,
  `AnimDriver::GetBaseUnits`, and the `PropDriver` slot that takes the
  entity.
- `AnimDriver::GetDefaultFramesPerUnit` and `GetBaseUnits` have no
  caller and sit in `PropAnimDriver.o`'s range.
