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
texture renderers). The camera's `_Init` is reconstructed with its clip,
pixel-accuracy and field-of-view callbacks (`0x6B8310`-`0x6B83A0`), and so
is the scene's (see below). `RndEntityCom::_Init` offers the level names
(`RndUtl::GetLodName`, `0x4423A0`); the classes' `Init` registrations are
reconstructed. `RndDrawableEntityCom::_Init` is reconstructed:
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

The runtime members are the entity's `PollMgr` (`+0x120`), the split
poll jobs and their `PollGroup`s (`+0x128`), the scene's
`PollGroups` (`+0x148`: "SceneDrawer PG" and "Scene PG"), the
`RndSceneDrawer` (`+0x150`), the frame interval (`+0x158`), the
partial-framerate scene index (`+0x160`), the frame the interval started
(`+0x170`), the property hysteresis texture and its position (`+0x178`,
`+0x180`), the entered flag (`+0x184`) and the root's `TimelinesCom`
(`+0x188`).

Slot 29 (`_OnResourcesLoaded`, the map's `_LoadResources`) sizes the
globals to four entries, creates the poll manager and the scene drawer,
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
named "poll-split-job: <n>" for its perf timer, in a new poll group
"<name> PG" whose head and tail take the releasing job's group.
`LinkPollSplitJobs` (`0x408A80`) adds each job to its group and chains
the groups; `ClearPollSplitJobs` (`0x408600`) destroys them.
`CreatePollGroups` (`0x408B10`) builds the scene's `PollGroups`
(`0x4121A0`; `Reset` `0x412450`, destructor `0x412750`), a
`PollGroupMap` whose scene group (key 1) polls after the drawer group
(key 0); a scene below another stops its polls from waiting for its
drawer's.

### Registry

`_Init` (`0x40B490`) registers the struct types "GlobalEntry<float>",
"GlobalEntry<Hmx::Color>" and "PropHysteresisEntry"
(`RegisterPropStruct`, `0x410090`-`0x4103F0`), makes the class a
`RndDrawableEntityCom` ("Global scene settings", `RndSceneResource` only)
and registers the properties. The groups `clear_depth`, `globals`,
`shadergraph_data` (with the nested `prop_hysteresis`), `lod` and
`utilities` have no storage; their members carry the component's offsets.
The globals' fixed-size arrays name and summarize each entry by its index
(`GlobalEntry<T>::GetSummary`, `0x413C70`/`0x4158E0`: the driving property
or the value, then the comment); their `value` is the map's
`DrivenProp<T>` from `src/stategraph/props/DrivenProp.h` (`Register` at
`0x4107C0`/`0x410EF0` is declared but not reconstructed). The hysteresis settings are read-only without
entries and recreate the texture (`0x412A10`-`0x412C50`); the array's
insert and remove hooks (`0x412A90`, `0x412B60`) do too, and the "reset"
action refills it (`_ClearPropHysteresisTexture`, `0x411B40`). The LOD
distances keep medium at most low (`0x412C60`, `0x412C80`). The revision
handler (revision 5) reads the saved properties from the component in the
third context pointer: the old `post_proc` (revision 0), LOD on
(below 2), a single hysteresis row from `shadergraph_data` (below 3),
`atmosphere`/`volumetric_scattering` as `sky`/`atmosphere` (below 4), and
a stripped warning for a half-rate `framerate` (below 5).
`mTextureSize` is unsigned: `texture_size` registers type 7.

Not reconstructed: `UpdatePropHysteresisTexture` (`0x409710`, the
per-frame texel update), the job's `_DoPoll` (`0x407F30`: the audio lock
and the entity's poll window) and `DrivenProp<T>::Register`.

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
"draw" `PollMgr` (`+0x118`), three `NoOpDrawJob`s that order the draw
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

`CreateEntity` builds the default scene ("main_camera" 20 units back,
"light_mgr" and "default_env") and clears the root EntityHeaderCom's
"copy_on_instance" through the inlined property write (`_SetProp`,
`0x18A440`). `_LoadEntity` reads the scene revision and runs the upgrades
of the seven older layouts; `_PostLoad` resolves the material sharing
(`0x50E4D0`) after a non-cached load. An instance or instance pool
requires a draw node, and destroying a draw node destroys them. The
classes the upgrades name without a model (`RndEntityInstanceCom`, the
audio-emitter and transform editor data, the instance pool) are reached through `extern Symbol`s; `AnimDriverCom` and
`StateGraphDriverCom` have symbol-only headers under `src/anim` and
`src/stategraph`.

The first immediate enter (`_EnterImmediately`, `0x438670`) creates the
scene's poll groups and gives the scene group callbacks (the
`std::function` members at `0x43AC70`-`0x43AEA0`): before its polls the
frame interval steps and either marks the entity unpolled (flag 0x2000)
or records the interval's start frame; after them the mark is cleared.
`_PollEntity` (`0x43A130`) polls the groups under the scene's manager:
the drawer group starts, the manager syncs on the scene group and then on
everything, and both groups run their post-polls. A thread that already
polls syncs the scene's timelines (`TimelinesCom::_UpdateParentClock`,
`_SyncClock`, through `_SyncTimelines`, `0x43A580`) and polls through the
base class; an edit-mode scene polls once per frame interval between the
drawer's poll and post-poll. Slot 22, `_OnPolledImmediately` (`0x439AE0`,
called at the end of `Entity::_PollImmediately`), queues the drawer in
the selected drawer group and rebuilds the split poll jobs: a window ends
at each object with a "ScenePollSplit" component (vtable `0x18FF610`, not
modelled; its class symbol is `gScenePollSplitComId`), weighs a tenth of
its components, and the objects' components receive their window's job
through `Component` slot 26. `_OnComponentCreated` (`0x43A6D0`) joins new
lights (`RndLightCom::AddToEnviron`, `0x46CCD0`, declared), draw nodes and
entity instances to the light manager's default environment.
`RndSceneInstanceCom::GetSceneDrawerPollGroup` and `GetScenePollGroup`
(`0x4366C0`, `0x4366E0`) return the instanced scene's groups.

`RndSceneInstanceCom::_Init` (`0x436AB0`) runs `InstanceCom`'s
registration first (`0x437910`, an `InstanceCom::_InitAsSuperclass` copy
with the file property "file"), allows the class in `UILayoutResource`s
under the `InstanceCom` interface, points "file" at scene resources and
registers `draw_order` (read-only debug, shown as "old draw order"),
`unique_resource`, `use_threaded_poll` (read-only while the entity runs in
the game mode), `reenter_shared_children`, `projection_offset` and
`projection_rect` (sliders whose members default to zero),
`masked_by_scenes` (scene instances other than itself),
`masked_by_dynamic_scenes` (scene paths) and the debug `disable_load_tag`;
the handlers (`0x437FB0`-`0x437FD0`) set the reload and masking flags.
`Init` is `0x3F2800`, `_Create` `0x404C10`.

`RndDrawInstanceCom::_Init` (`0x6C17F0`) describes the draw-instance
interface (requires a draw node, brings a material, drawable entities
only): `billboarding` (every type but Camera No Roll, hidden for classes
that draw no geometry, slot 45), `sort_by`, `sorting_hint` (an int8, shown
only for an opaque Src material), `lods` (a bitfield of the three levels;
its change sets `RuntimeData::mLodsChanged`), the Vector4 structs
`extra_data` and `extra_data_1` and their storage-less colour views
(`0x6C3950`-`0x6C3990`, the setters keep w). `Init` (`0x3F6880`) registers
it as a superclass root that is not created; `_Create` is `0x4055A0` and
`_InitAsSuperclass` `0x449960`. The billboard labels come from
`GetBillboardTypeName`/`Help` (`0x441E10`, `0x441E30`, declared).

Names inferred from bodies and callers: `_OnPolledImmediately` (the
binary shows slot 22 is no `_DestroyLayerEntity`), `_SyncTimelines`,
`gScenePollSplitComId`, the scene-instance group getters, and on
`TimelinesCom` `_UpdateParentClock`, `_SyncClock`, slot 41
`_OnClockSynced` (empty), `mParentClock`, `mClockSynced` and `mFramerate`
("framerate").

## RndTexRendererMgr

`src/render/textures/RndTexRendererMgr.{h,cpp}` reconstructs the texture
renderer manager (`0x6A8150`-`0x6A8B6C`, static initializer `0x6A8B70`,
360 bytes; the name is inferred from its "Tex Renderers" statistic). It
keeps the entity's `RndTexture2DRendererCom`s ("Texture2DRenderer", not
reconstructed) in `RndTexRendererGroup`s (352 bytes, constructor
`0x6948B0` and destructor `0x694930` declared) of renderers that share an
`RndBufferCollection2D`. `AddRenderer` (`0x6A8360`) and `RemoveRenderer`
(`0x6A84F0`) join and leave groups under the lock; `DrawImmediate`
(`0x6A85C0`) draws each pending group's first renderer inside the
statistic block; `AddDrawJobs` (`0x6A8740`) hangs each pending group's
draw and execute jobs between a begin job (vtable `0x1936328`) and an
end job (`0x1936388`) that open and close the statistic block, and
`ClearDrawJobs` (`0x6A8900`) unhooks them. The deferred-context job pair
(`RndExecDeferredContextJob`, vtable `0x1937850`, "draw exec deferred
context %d", and the `RndDeferredContextDrawJob` base, vtable
`0x19377F0`) is in `src/render/context/RndDeferredContextJobs.h`.

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
object polls first (`_GetPollDeps`). `_Init` (`0x406710`) registers both
properties at revision 1 with the billboard types and the +Y (1) and -Y
(4) axes as allowed values; its revision handler (`0x407C90`) turns an
older component's bool "use_camera_up_vector", read through `_GetProp`,
into facing type 1 (set or missing) or 4, with axis +Y. `Init`
(`0x3EE2A0`, emitted in `render/RndInit.o`) registers the class.

`ComputeBillboardXfm` serves the draw instances' "billboarding" too
(`0x463199`). Its types are None, Camera (the camera's up axis), Camera
XY (world Z up), Camera Keep Z (the object's own Z axis) and Camera No
Roll (world Z as the up reference), after the labels at `0x1901DB0`.
