# Render scenes

A scene is an entity resource whose root object carries the scene
component. The classes below are real `Component` and `EntityResource`
subclasses (see [entity-components.md](entity-components.md) and
[entity-resources.md](entity-resources.md)). Their sources are in
`src/render/scene` and `src/render/context`. The scene drawer has its own
sections.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndSceneCom.o` | `0x407F30`-`0x419170` | `0x18FFA70` (41 slots) | 400 |
| `render/RndCameraCom.o` | `0x6B72B0`-`0x6B88BB` | `0x19370E8` (41 slots) | 176 |
| `render/RndEntityCom.o` (`RndDrawableEntityCom`) | `0x6C05D0`-`0x6C0B7B` | `0x19378B0` (41 slots) | 32 |
| `render/RndDrawableEntityResource.o` | `0x6C0B80`-`0x6C14FA` | `0x1937A08` (39 slots) | 784 |
| `render/RndSceneResource.o` | `0x4384A0`-`0x43AEE0` | `0x19013C0` (39 slots) | 784 |

## Class registration

| Static | RndSceneCom | RndCameraCom | RndDrawableEntityCom |
| --- | ---: | ---: | ---: |
| `sId` | `0x1A722A8` | `0x1AAF8F0` | `0x1AB00A0` |
| `sClassName` | `0x1A722B0` | `0x1AAF8F8` | `0x1AB00A8` |
| `sPropRegistry` | `0x1A722C0` | `0x1AAF900` | `0x1AB00B0` |
| `sMetaData` | `0x1A72360` | `0x1AAF9A0` | `0x1AB0150` |
| `_Init` | `0x40B490` | `0x6B7420` | `0x6C06F0` |
| `_Create` (in `RndInit.o`) | `0x4048E0` | `0x404160` | none |

The class ids are `"Scene"`, `"Camera"` and `"DrawableEntity"`. The
identity slots, the constructors, the destructors and the factories are
reconstructed, and so is `_Imprint` (`0x411CE0`, `0x6B8170`, `0x6C0960`):
an inlined copy constructor keeps the properties, copies the scene's
`floats`, `colors` and hysteresis `entries` through `PropArrayBase::_Copy`,
and starts the run-time state afresh (the scene's poll groups, drawer and
counters, `isolate_lod`, the camera's stereo eyes and the drawable entity's
texture renderers). The scene's and the camera's `_Init` stay declared;
their property metadata is not modelled. `RndDrawableEntityCom::_Init` is reconstructed:
"Goes on the root object of all drawable entities", a rendering class
allowed in `RndDrawableEntityResource`s whose interface is its own class
symbol. `_InitAsSuperclass` (`0x4105A0`, emitted in `RndSceneCom.o`) makes
it the superclass of a subclass's metadata under the "metadata" heap.

## RndDrawableEntityCom

The map's older build calls this base `RndEntityCom`; this build's metadata
names it `RndDrawableEntityCom`. Its `RuntimeData` (`+0x18`, constructor
`0x6C0600`) holds the entity's texture renderers (`RndTexRendererMgr`, 360
bytes, `0x6A8150`), which `ObtainTexRendererMgr` (`0x6C06B0`, the map's
`ObtainPollMgr`) creates on demand and the destructor deletes.

## RndSceneCom

`RndSceneCom` derives from `RndDrawableEntityCom`. Its members follow the
properties: `draw_order`, `clear_type`, `clear_color` (black by default),
the `clear_depth` group's `after_post_proc_0` and `_1`, the object ids
`camera`, `aux_camera`, `light_manager`, `sky`, `atmosphere`,
`post_proc_0`, `post_proc_1` and `antialiasing` (`+0x3C`-`+0x5B`, none by
default), `mask_buffer` (enabled, signed distance, width 6), the shader
graph globals' `floats` and `colors` arrays (240- and 248-byte elements: a
comment symbol and a driven value), `trans_0` to `trans_3`,
`prop_hysteresis` (`+0xC8`: timeline, texture size 64, wrap mode 1,
filter mode 2 and a `PropArray<PropHysteresisEntry>` of 880-byte rows) and
the LOD settings (`+0x100`: medium 200 and low 1000, `enable_lod`,
`num_draw_entries`, `never_cull`, `isolate_lod` at `+0x168`). The
property references inside the array elements are the map's `PropRef`
(216 bytes, constructor `0x17F4F0`), which is not modelled under
`src/entity`; `RndSceneCom::PropRefData` mirrors its members.

The runtime members are the entity's `PollGroup` (`+0x120`), the split
poll jobs and their nested `PollMgr`s (`+0x128`), the scene's
`PollGroups` (`+0x148`: "SceneDrawer PG" and "Scene PG"), the
`RndSceneDrawer` (`+0x150`), the frame interval (`+0x158`), the
partial-framerate scene index (`+0x160`), the frame the interval started
(`+0x170`), the property hysteresis texture and its position (`+0x178`,
`+0x180`), the entered flag (`+0x184`) and the root's `TimelinesCom`
(`+0x188`).

Slot 29 (`_OnResourcesLoaded`, the map's `_LoadResources`) sizes the
globals to four entries, creates the poll group and the scene drawer,
takes the first light manager in the entity when none is named, creates
the "Prop Hysteresis" texture (one row per entry filled with its default
values, in the supported 64-bit float format) and finds the entity's
clocks. Entering and polling tell the clocks whether the scene runs at a
partial frame rate; exiting resets and destroys the poll groups.

The getters look the named object up in the scene entity and return its
component: `GetCamera(index)` (`0x408D90`, `0x408E20`), `GetLightMgr`
(`0x408ED0`, `0x408F60`), `GetSky`, `GetAtmosphere` (by interface),
`GetVolumetricScattering` (on the atmosphere's object), `GetPostProc` and
`GetAntialiasing`. The setters store the component's object id.

### Frame intervals

`GetFramerateType` (`0x408C10`) is the entity clocks' delta mode while the
renderer draws partial-framerate scenes (`RndConfig`: a nonzero scene
limit, the switch, and tiled lighting), and 0 otherwise. The clocks are the
root's `TimelinesCom` or `TimeMgr::GetClock` (`0x25A9B0`). An interval is
one frame for type 0 and two for type 1. `StepFrameInterval` (`0x408CD0`)
advances the counter modulo the interval and returns 1 when it wraps; the
counter starts at -1, so the first step starts interval 0. The scene
resource's poll steps it and records the device frame count whenever a new
interval starts, and the drawer selects the partial-framerate buffers by
the counter.

### Split polls

`AddPollSplitJob` (`0x408810`) adds a `PollSplitJob` (a `ThreadedJob`,
vtable `0x18FFBC8`, 184 bytes) for a window of the entity's poll order,
named "poll-split-job: <n>" for its perf timer, under a new nested manager
"<name> PG" that shares the releasing job's manager.
`LinkPollSplitJobs` (`0x408A80`) queues each job on its manager and chains
the managers; `ClearPollSplitJobs` (`0x408600`) destroys them.
`CreatePollGroups` (`0x408B10`) builds the scene's `PollGroups`; a scene
below another stops its polls from waiting for its drawer's.

Not reconstructed: `_Init`, `UpdatePropHysteresisTexture`
(`0x409710`, the per-frame texel update), the job's `_DoPoll`
(`0x407F30`: the audio lock and the entity's poll window), the
`PollGroups` class (`0x4121A0`-`0x412750`) and the array type
registrations (`0x410090`, `0x410240`, `0x4103F0`).

## RndCameraCom

The members follow the properties: `near_distance` (0.1),
`far_distance` (1000), `orthographic`, `pixel_accurate`,
`perspective_fov` (radians, shown in degrees; 0.6024) and `ortho_height`
(10), then the stereo eyes (`+0x2C`: a flag and two eyes of a transform and
four side angles), which `RndCameraSettings` reads. The static getters
return the defaults and the largest far-to-near ratio, 10000. The property
callbacks keep the far distance at least twice the near one and the near
distance at most half the far one, and give a pixel-accurate camera the
content height.

Slot 29 gives a pixel-accurate orthographic camera the content height.
Slot 33 (`_Poll`) runs only for the scene's main camera: it projects into
the instancing scene's `RndSceneInstanceCom::CalcProjectionRect` for the
PS4 resolution, or the whole target, and calls
`RndSceneDrawer::SetSceneCamera`.

## RndDrawableEntityResource and RndSceneResource

`RndDrawableEntityResource` derives from `TransEntityResource`. It owns a
"draw" `PollGroup` (`+0x118`), three `NoOpDrawJob`s that order the draw
jobs ("head", "after tex renderers", "tail"; vtable `0x1937B50`) and the
current draw's job list (`+0x2F0`). Slots 33-38 are pure: draw now, start
and finish the draw jobs, and draw, start and finish the texture renderers.
Game code calls them through public wrappers (`0x6C1130`-`0x6C1310`).
`Draw` (`0x6C0F10`, the map's `RndEntityResource::Draw`) picks the
immediate or the job path; it is not reconstructed.

`RndSceneResource` saves scene revision 7 after `TransEntityResource`'s
data and registers the "scene" extension in the "Scenes" category. Its
draw slots go to the root scene component's drawer and texture renderers;
entering wraps `TransEntityResource::_EnterEntity` in the drawer's
`EnterPrelude` and `EnterCoda`, and exiting calls `ExitPrelude` first. The
first ready entity unhooks the scene's poll groups and split jobs. It is
instanced by `RndSceneInstanceCom` and counts as an editor entity.

Not reconstructed: `CreateEntity` (the default scene with "main_camera",
"light_mgr" and "default_env"), `_LoadEntity` (the scene revision upgrades
of seven older layouts), `_PostLoad`, `_PollEntity`, `_EnterImmediately`,
`_DestroyLayerEntity`, `_OnComponentCreated` and the required-component
slots, which use classes and property writes not modelled yet.

## RndLookAtCameraCom

`render/RndLookAtCameraCom.o` (`0x4066D0`-`0x407F2B`, vtable `0x18FF8C8`,
41 slots, 32 bytes; sId `0x1A71FC8` "LookAtCamera", sClassName
`0x1A71FD0`, sPropRegistry `0x1A71FE0`, sMetaData `0x1A72080`) turns its
object towards the main camera of the nearest scene up the entity chain.
Its "facing_type" (`+0x18`, default 1) is an `RndBillboardType` and its
"facing_axis" (`+0x1C`, default 4, local -Y) picks the axis that points at
the camera. `_Poll` (`0x407000`) asks `ComputeBillboardXfm` (`0x441E50`,
inferred name, in `src/render/drawing/RndBillboard.cpp`) for the facing
rotation, moves it into the parent's frame, rebuilds an orthonormal basis
around the facing Y axis (or, for "Camera Keep Z", around the current local
Z axis) and stores it as the transform's local Euler angles. The camera's
object polls first (`_GetPollDeps`). `_Init` (`0x406710`) and the property
callbacks (`0x407C10`-`0x407E50`) are not reconstructed.

`ComputeBillboardXfm` serves the draw instances' "billboarding" too
(`0x463199`). Its types are None, Camera (the camera's up axis), Camera
XY (world Z up), Camera Keep Z (the object's own Z axis) and Camera No
Roll (world Z as the up reference), after the labels at `0x1901DB0`.
