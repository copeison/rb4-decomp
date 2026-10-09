# Render lights

The light components of the map's `render` module are real `Component`
classes (see [entity-components.md](entity-components.md)). Their sources
are in `src/render/lighting/lights`. `RndLightPointCom` and
`RndLightSpotCom` derive from `RndLightCom`; `RndLightProbeCom` derives
from `Component` directly.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndLightPointCom.o` | `0x490060`-`0x496C9F` | `0x1906B90` (54 slots) | 424 |
| `render/RndLightProbeCom.o` | `0x498080`-`0x49DB9F` | `0x1907370` (41 slots) | 184 |
| `render/RndLightSpotCom.o` | `0x49F2E0`-`0x4A867F` | `0x19077E0` (54 slots) | 664 |

Each class has `sId` and `sClassName` (both `"LightPoint"`, `"LightProbe"`
and `"LightSpot"`), `sPropRegistry` and `sMetaData`:

| Static | RndLightPointCom | RndLightProbeCom | RndLightSpotCom |
| --- | ---: | ---: | ---: |
| `sId` | `0x1A888C8` | `0x1A88DE8` | `0x1A892F8` |
| `sClassName` | `0x1A888D0` | `0x1A88DF0` | `0x1A89300` |
| `sPropRegistry` | `0x1A888E0` | `0x1A88E00` | `0x1A89310` |
| `sMetaData` | `0x1A88980` | `0x1A88EA0` | `0x1A893B0` |
| `_Init` | `0x490340` | `0x49B290` | `0x49F9A0` |
| `Init` | `0x3F0120` | `0x3F04E0` | `0x3F08C0` |
| `_Create` | `0x404660` | `0x4046B0` | `0x404700` |

`Init` and `_Create` are emitted together before the renderer's components
(`0x3F0000`-`0x405000`), as `SoundManager.o` emits the audio ones. The
identity slots, the factories, the constructors and destructors are
reconstructed; `Init` and `_Init` stay declared for the reasons given in
[audio-components.md](audio-components.md).

`_Imprint` (slot 10) is reconstructed for every light: `RndLightCom`
(`0x46F490`), `RndLightDirectionalCom` (`0x4757B0`), `RndLightPointCom`
(`0x495620`), `RndLightSpotCom` (`0x4A6CD0`) and `RndLightProbeCom`
(`0x49CE90`). Under a `ScopedImprint` each places a copy at the buffer's
next 8-byte boundary, marks it imprinted and returns `_ImprintProps` past
it. The copy constructors keep the properties, copy the property arrays
(`environments`, `distance_fracs`, `quality_settings`, the probe's state
names) through `PropArrayBase::_Copy`, which skips while imprinting, and
default-construct every `RuntimeData`; the map's
`RuntimeData(RuntimeData const&)` of `RndLightCom` and `RndLightSpotCom` is
modelled that way. They are inlined, except the directional light's
(`0x475A30`).

The binary's slot table differs from the map's: the map's
`_LoadResources(ObjPtr const&)` is slot 29 (`Component::_OnResourcesLoaded`)
and `_PreDestroy` is slot 20. Every override address was checked against
the vtables with `orbis-objdump -r`.

## RndLightProbeCom

A probe captures the scene around it into a diffuse (16-texel) and a
specular (128-texel, six mips) cube texture for each of the light manager's
probe states (`"default"`, new states `"new_state"`). The textures are
inlined into the probe's entity resource under
`light_probe_cubetex_0_<object serial>_<state>_diff` and `_spec`
(`_MakeResourcePath`). The light manager blends the two current states'
textures through cube-texture arrays that `SyncTexarrays` (`0x49A300`)
builds from every probe, and draws each visible probe through the compute
buffer (`AddToComputeBuffer`, 96-byte entries) or the untiled
`DrawDeferredLightNoCompute`.

The properties are `enabled` (`+0x16`, in `Component`'s tail padding),
`falloff_start`, `falloff_end`, `falloff_function`, and the `capture`
group: `range`, `include_atmosphere` and `background_color`. An unnamed
symbol array sits at `+0x28`. `RuntimeData` (`+0x68`, constructor
`0x4982B0`) caches the object's `TransCom` and `RndDrawNodeCom`, keeps the
bookkeeping and visibility flags, the per-state `StateRuntimeInfo` vector
(two `ResourcePtr<RndTextureCubeResource>` and two captured
`RndPixelDataCube` pointers), the constant buffer, the texture-array
layer, the `use_raw_texture` and `isolate` properties (which the registry
binds to runtime fields) and the compute-buffer index.

The probe joins the scene light manager's probe list when its resources
load (`_AddToBookkeeping`, `RndLightMgrCom::AddProbe`) and leaves it when
it is destroyed with its instance or object (`_RemoveFromBookkeeping`).
The poll keeps the draw node's sphere at the falloff end; the edit poll
also follows the isolation flag (`RndLightMgrCom::SetIsolatedProbe`) and
reloads the textures when one changed on disk.

Not reconstructed: slot 29 (`0x49C080`, it needs the light manager's
probe-state array and the inlined-resource lookup), `StateRenamed`, the
capture pipeline (`Capture`, `_CreateCameraEntity`, `_ProcessDiffuse`,
`_ProcessSpecular`, `_DownsampleTo`, `CreateCaptureHelperTexture`,
`_PixelToSolidAngle`, `_ReplaceInlineCubetexResource`), the drawing
(`_FrustumExcludes`, `DrawDeferredLightNoCompute`, `_GetBlendTextures`,
`_GetComputeShaderData`, `AddToComputeBuffer`), `SyncTexarrays` and the
`_Init` handlers.

## RndLightPointCom

The properties after `RndLightCom`'s are `bulb_radius` (`+0xE8`),
`falloff_start`, `falloff_end`, `falloff_function`, `cookie` (a
`RndTextureCubeResource` path, `+0xF8`), and the `shadows` group:
`casts_shadows`, `quality_settings` (one `{casts_shadows, soften_shadows}`
element per quality level, sized by `_PostCreate`), `only_flagged_objects`,
`shadow_map_resolution` (256, 512 or 1024), `shadow_offset` and the three
softness settings. The `intensity_calculator` struct (`luminous_flux`,
`temperature`) has no storage in the object; its handlers use two globals
(`0x19BDC30`, `0x19BDC34`).

`RuntimeData` (`+0x148`) holds the sphere's dirty flag, the cookie, the
deferred and shadow-generation constant buffers (from the light globals'
point shaders), the geometry clip plane, the shadow cube, the
shadow-contribution slot, and the shadow camera's scene entity, object id
and camera. Slot 29 creates the constant buffers, loads the cookie
(`Resource::GetOrLoad<RndTextureCubeResource>`, emitted here at
`0x4924E0`) and tells the light manager when it changed, then creates the
shadow data and syncs the sphere.

Slots 41-53 are capabilities 3, units `"W/sr"`, the frustum test, the
untiled draw (`"Point Light"`), the compute-buffer entry, type 0, the
cookie texture, no shadows (slot 49 returns false in this build), the
shadow cube draw, the shadow-contribution slot, the shadow contribution
(`"Shadow Contrib Point Light"`, `"Shadow Contrib Geo"`, `"Shadow Blur"`)
and `_IsOnImpl` (a falloff end above 0.0001). The frustum test, the draws,
`_CalcGeoClipPlane`, `_GetComputeShaderData`, `_SyncShadowGenConstants`
and `_CreateShadowMapData` are not reconstructed.

## RndLightSpotCom

The properties after `RndLightCom`'s are `bulb_radius`, `falloff_start`,
`falloff_end`, `falloff_function`, `start_angle` and `end_angle` (full
cone angles), `angle_falloff_function`, `truncation`, the `shadows` group
(`casts_shadows`, `quality_settings`, `only_flagged_objects`,
`cast_context`, `shadow_offset` at `+0x140`, which
`RndDefaults::GetLightingShadowOffset` reads, and the softness settings),
`cookie` (a 2D texture path, `+0x150`) and `cookie_tiling`. The
`intensity_calculator` struct adds `beam_angle`.

`RuntimeData` (`+0x160`, constructor `0x49F4A0`) holds the geometry's
dirty flag, the cookie, the two constant buffers, the light volume (a
`TruncatedRoundedCone` from half the end angle, the falloff end and the
truncation) and a larger mesh cone, the world bounds (`Capsule`) and cap
sphere, the geometry clip plane, the shadow depth layer and contribution
slot, the four shadow side planes and the shadow camera.

`_SyncGeometry` (`0x4A28A0`) rebuilds both cones. The mesh cone grows by
the reciprocal cosines of the light globals' spotlight-mesh steps so that
the tessellated mesh covers the volume. Every falloff, angle and
truncation setter and handler calls it. The poll moves the draw node's
sphere around the mesh cone and, while the light is on, updates the world
bounds and the shadow planes. Slot 51 takes one spot shadow depth layer
(`RndLightMgrCom::AcquireSpotShadowDepthLayers`) and a
shadow-contribution slot.

Not reconstructed: slot 29 (the inlined-resource lookup and
`RndTexture2DResource`'s layout), the frustum test, the draws
(`"Spot Light"`, `"Shadow Contrib Spotlight"`, `"Shadow Contrib Geo"`,
`"Shadow Blur"`), the compute-buffer entry, the cookie texture slot,
`_CreateShadowMapData`, `_SyncShadowPlanes`, `_CalcGeoClipPlane`,
`_SyncShadowGenConstants` and `_CalcCameraXfms`.

## Supporting declarations

- `RndDrawNodeCom` (`src/render/scene`) declares the bounding sphere, its
  dirty flag and the flags the lights read; `SetLocalSphere` is the inlined
  sphere update.
- `RndTextureCubeResource` (`src/render/textures`) declares the class id
  and the texture at `+0xC8`.
- `Capsule` (`src/math/geometry`) is the two-sphere hull at `0x117E050`;
  matching it to the map's `Capsule.o` is weak.
- `TruncatedRoundedCone::SetRadiiAndLength` (`0x117E370`).
- `RndLightMgrCom::AddProbe`, `RemoveProbe`, `SetIsolatedProbe`,
  `AcquireShadowContribution` and `AcquireSpotShadowDepthLayers`.

## Weak evidence

- The probe's `mStateNames`, `_FrustumExcludes`, `_GetBlendTextures` and
  `GetStateTexture`; the spot light's `GetAngleFalloffParams`,
  `_SyncShadowPlanes`, `mCameraInside` and `_CalcGeoClipPlane` (`0x4A4700`,
  no caller).
- `ShadowQualitySettings`, and the runtime field names of all three
  classes.

## RndLightCom, RndLightDirectionalCom, RndLightMgrCom and RndLightGlobals

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndLightCom.o` | `0x46C950`-`0x4705FF` | `0x1904DC0` (54 slots) | 232 |
| `render/RndLightDirectionalCom.o` | `0x470600`-`0x47730F` | `0x19051B0` (54 slots) | 672 |
| `render/RndLightMgrCom.o` | `0x480280`-`0x48ED8F` | `0x1906480` (41 slots) | 1856 |

| Static | RndLightCom | RndLightDirectionalCom | RndLightMgrCom |
| --- | ---: | ---: | ---: |
| `sId` | `0x1A873B0` | `0x1A87638` | `0x1A88308` |
| `sClassName` | `0x1A873B8` | `0x1A87640` | `0x1A88310` |
| `sPropRegistry` | `0x1A873C0` | `0x1A87650` | `0x1A88320` |
| `sMetaData` | `0x1A87460` | `0x1A876F0` | `0x1A883C0` |
| `_Init` | `0x46D760` | `0x4709A0` | `0x486830` |
| static initializer | `0x470530` | `0x477240` | `0x48ECC0` |

The class ids are `"Light"`, `"LightDirectional"` and `"LightMgr"`. Each
initializer first sets the shared header's null id and thread-group widths
(`-1, 8, 4`). The objects' own null-id copies are at `0x1A873A0`,
`0x1A87628` and `0x1A882F8`.

### RndLightCom

The light interface takes slots 41-53. This build adds slot 42,
`_GetIntensityUnitsImpl` (`"unknown units"`, `"W/m^2"` for directional
lights and `"W/sr"` for point and spot lights). It drops the map's
`_GetCookiePathImpl`. The parameters of slots 43-52 after the map's are
inferred from the callers in `RndLightMgrCom` and `RndLightEnvironCom`:

- slot 43 takes the camera's extra planes (`RndLightCullPlanes`, a
  `VectorAdapter<Vector4>`);
- slots 44 and 45 take the probe the untiled pass combines
  (`RndLightProbeParams`);
- slots 46 and 49 take the quality level;
- slot 50 takes the drawer's show and hide flags and instance lists;
- slots 50-52 take the light manager.

The properties are `enabled` (`+0x16`), `environments`, `color`,
`intensity`, `illumination_type` (type 3 subtracts light: white adds
nothing), `light_wrap`, `clip_plane` and `volumetric`. `RuntimeData`
(`+0x68`, constructor `0x46CAC0`) also holds `environ_bits`,
`active_environments`, `isolate` and `tiled_cookie_index`. It caches the
`TransCom` and `RndDrawNodeCom`, the clip plane and the master intensity,
and keeps the light's buffer index.

A light registers with the light manager of the nearest scene above it,
and with each `RndLightEnvironCom` that its `environments` name, when its
resources are ready (slot 30, `_InitBookkeeping`). A light in an instanced
entity uses the instancing object's `nested_light_environments` instead.
The light unregisters through `_PreDestroy` and re-registers when its
environments change. `IsOn` needs the light to be enabled and entered,
with an intensity above 0.0001 and a color that adds light.

### RndLightDirectionalCom

The light travels along the object's -y axis. Its properties are the 2D
`cookie` and `cookie_tile_size` and the `shadows` group: `casts_shadows`,
`max_distance`, `cascades` (`num_cascades`, `distance_fracs`),
`max_offscreen_occluder_dist`, `softening`, `only_flagged_objects`,
`cast_context`, `shadow_offset` and `quality_settings`. The runtime data
holds:

- the cookie;
- the deferred and shadow-generation constant buffers;
- the shadow camera's scene entity and its camera;
- the shadow depth layer and contribution slot;
- four cascade cameras.

These parts are reconstructed:

- `_GetComputeShaderData` (the 112-byte
  `RndComputeBufferStructs::RndCSLightDirectional`);
- the deferred constants and the probe's falloff and transform (slot 44);
- the full-screen draw (`"Directional Light"` and `"Directional Light +
  Probe"`, slot 45);
- the compute-buffer entry;
- the cookie lookup, which tries the entity's inlined resources first;
- `_SetCascadeCamera`;
- the shadow-resource reservation.

These are not reconstructed:

- `_SyncShadowResources` (`0x473B50`), because it needs `Entity::Copy`
  (`0xEE330`), which `Entity.h` does not declare;
- the cascade fitting (`0x472E30`, `0x4732F0`, `0x473600`);
- slots 50 and 52 (`"Shadow Contrib Directional"`).

### RndLightMgrCom

The properties are:

- `default_environment` and `ambient_occlusion`;
- `master_intensity_mult`;
- the tonemapping settings;
- `material_smoothness_adjustment` and `cookie_texture_size`;
- `probe_lighting` (`states`, `cur_state_a`, `cur_state_b` and
  `cur_state_blend`);
- `capture`;
- `quality_settings` (`tonemapping_enabled`,
  `max_shadow_casting_spotlights` and `spotlight_shadowmap_resolution`);
- the `lights` filters.

The map's `RuntimeData` (constructor `0x4803B0`, destructor `0x48CD50`)
starts at `+0xC8`. It is declared flat in the class because the probe
reads its members by name. It holds:

- the registered environments, lights and probes;
- the cookie and probe texture arrays;
- the spot shadow depth array;
- the per-frame shadow counters;
- the isolated light and probe;
- the pending probe-state renames;
- two `CullResults` (the camera, and the first depth slice for target
  mode 1);
- the tiled light buffers;
- two event subscriptions.

These functions are reconstructed:

- the identity slots, the constructors (the copy one is inlined into
  `_Imprint`), the destructor and `_Imprint`;
- `Handle` (`"leaving_playmode"` and `"leaving_record_mode"`);
- `_PostCreate`, `_OnResourcesLoaded`, `_Exit`, `_Poll` and their edit
  forms;
- the environment, light and probe bookkeeping, and the probe states
  (`_RemoveProbeState`, `GetAllowedProbeStates`);
- `CullLights`, `_FrustumCullLights`, `_FillTiledLightBuffers` and
  `_FillSliceZeroLightIds`;
- the tiled cull (`_CullTiledLights`, its pass `0x484620` and the stereo
  pass `0x484980`, `"Lighting"`);
- `StoreCullResults` and `RestoreCullResults` (the copy goes to
  `RndSceneCullResults::mLights`; `CullResults`' implicit copy
  assignment is emitted at `0x484DA0`);
- `AccumDeferredLight` (`"Shadows"`, `"Lighting"`) with
  `_AccumTiledDeferredLight`, `_AccumUntiledDeferredLight`
  (`combine_single_probe`) and `_AccumUntiledDeferredProbes`
  (`"Accumulate Probes"`);
- `GenerateAmbientOcclusion` (`RndSSAOCom`, new declaration-only
  header), `SetFwdLightingConstants` and `SetNoFwdLightingConstants`;
- `TonemapScene` (`"Tonemapping"`) and `UpdateDrawTarget` (`0x4867C0`);
- `_SyncCookieTexArrays` and its pixel helpers (`0x48B870`,
  `0x48B9D0`); the cookie arrays are `[0]` 2D, `[1]` 2D rendered, `[2]`
  cube and `[3]` cube rendered;
- `CaptureAllProbes` and `ZeroAllProbes`;
- `DrawShadowMaps`, the shadow-slot allocators, `_InitBuffers` and
  `_SyncSpotShadowDepthTexArray`.

The pass after `AccumDeferredLight`'s collection is the scene draw's
`RndSceneDrawTarget`, which the light slots 45 and 52 and
`RndLightEnvironCom::DrawDeferredLightNoCompute` also take. `+0x690` is
`mAmbientOcclusionGenerated`.

These are not reconstructed:

- slot 30 (`0x480D80`): `Component::AreResourcesReady` (`0xE8250`)
  forwards the load pass (0-9) of `Entity::_LoadResources` in `esi`, so
  slot 30 is `_AreResourcesReady(int pass)`; the light manager answers
  false on pass 0 and does its work on pass 1. `Component.h` must declare
  the parameter first;
- `_Init` and its handler `0x489DC0`.

### RndLightGlobals

`LoadResources` (`0x47F8D0`) runs while rendering is initialized or
resources are precached. It loads `inline_lighting_textures.entity` and
`skin_diffusion.bmp`, and takes the two hair reflectance textures from the
first one's `RndTextureUtilityCom`. `GetSkinDiffusionTexPath` (`0x47FCD0`)
is also reconstructed.

### Supporting declarations

- `RndLightEnvironCom` (new header): the environment index, the light and
  culled-light lists, and `AddCulledLight`, which is emitted in the light
  manager's object at `0x48AE80`.
- `RndTextureUtilityCom` (new header).
- `RndLightUtl::GetLightTypeName` and `RndLightType` (new header).
- `RndDrawNodeCom::mNestedLightEnvironments` (`+0x38`) and `mInvWorldXfm`
  (`+0xB4`).
- `RndTexture2DResource::mTexture` (`+0x30`).

### Weak evidence

- The names of slot 42, `RndLightProbeParams`, `RndLightCullPlanes` and
  `GetLocalClipPlane`.
- The camera type of slots 45 and 52.
- The light manager's `CookieArrayElemInfo` fields, `mUnusedList`,
  `mReserved` and the cookie-array order.
- `RndLightEnvironCom::mRegistered`.
- `ShadingModeKeepsLights` and `ShadingModeKeepsProbes`.

## RndLightEnvironCom

`render/RndLightEnvironCom.o` (`0x478B70`-`0x47A3BB`, vtable `0x1905D68`,
41 slots, 248 bytes; sId `0x1A87B60` "LightEnviron", sClassName
`0x1A87B68`, sPropRegistry `0x1A87B70`, sMetaData `0x1A87C10`; factory
`0x404390`) is a set of lights. Everything after `Component` is the map's
`RuntimeData` (destructor `0x47A0A0`); an imprint starts it afresh.
`_Enter` and `_EditEnter` register with the scene's light manager, which
indexes up to six environments (`SetEnvironIndex`, `0x478E00`, moves the
environment's bit in its lights' `environ_bits`). An object or component
exit unregisters and drops the manager's default environment if it was
this one; every exit forgets the lights. `LightAdded` (`0x478EC0`) and
`LightRemoved` (`0x4790E0`) keep the lights and their ids in
"sibling_lights" (same entity) or "nested_lights" (with the entity beside
each id); as in the binary, removing a light that is not listed drops the
last one. `DrawDeferredLightNoCompute` (`0x479330`, "Deferred Lighting")
draws the culled lights additively through slot 45 with the stencil test
on the environment's index (0 for the unindexed default, otherwise the
index plus 2); only the first directional light gets the probe. The
subtracting lights (illumination type 3) then multiply, add in debug-misc
mode 16, or are skipped in mode 15. `_Init` (`0x479650`) is not
reconstructed.

## RndLightFlareCom

`src/render/lighting/flares/RndLightFlareCom.{h,cpp}` reconstructs
`render/RndLightFlareCom.o` (`0x47A3C0`-`0x47E7AF`, "LightFlare", vtable
`0x1905F10` with 46 slots, object 376 bytes). It derives from
`RndDrawInstanceCom`, and slots 41-45 have been checked against
`0x1937BB0`. Every slot of the emitted vtable matches the binary. The
nested `SubflareDrawable` vtable (`0x1906080`, 3 slots) and the
`PropArray<Subflare>` vtable (`0x19060A8`, 8 slots) match in slot order.

- The properties: "light_source_radius", "size" (Vector2), "angle_radians"
  (with an "angle" alias in degrees), "light", "color", "intensity",
  "cam_centering_intensity", "spotlight_angle_mult" and "subflares". Each
  `Subflare` (44 bytes) has "type" (Starburst or Ghost), "material",
  "scale", "tint", "intensity_mult", "intensity_scaling", "divergence" and
  "divergence_accel".
- `_PostCreate` adds one starburst. `_OnResourcesLoaded` (slot 29, the
  map's `_LoadResources`) creates the occlusion query, named after the
  object, and caches the TransCom. `_Enter` gives each subflare a drawable,
  registers the query with the scene drawer's `RndOcclusionQueryMgr`, and
  sets the draw node's local sphere to the light source radius.
- `_Poll` hides the flare in these cases: the draw node is hidden, the
  options set "hide_flares" (unless the options are suppressed), the light
  is hidden, or the light subtracts. Otherwise it takes the light's color
  and intensity. For a spotlight it also stores the light's world z axis
  and `RndLightSpotCom::GetAngleFalloffParams`. It then enables the query
  and copies the draw node's world sphere into it.
- Slots 43 and 44 work like RndMeshCom's instance sync. Each instance draws
  its subflare's drawable with that subflare's material. Without one it
  falls back to the object's material, the referenced material, or the
  additive default (slot 42). The normal transform is the identity and the
  atmosphere is disabled.
- `_DrawSubflare` (`0x47CE60`) skips a subflare that the query has culled
  for the context's view. Otherwise it weights the intensity with a GGX
  distribution of the camera-axis angle (roughness
  `1 - 0.94 * cam_centering`, or 0.06 above 1), the light's master
  intensity, and a spotlight's `sample_function_table` falloff. A ghost
  dims by its scale squared. A starburst converts part of its intensity
  change into size. The quad is drawn with
  `RndDrawUtl::DrawRotatedQuad2D` inside the query's predication. A
  diverging subflare is drawn twice, mirrored about the light source along
  the line through the screen center. One that does not diverge is drawn
  once at double intensity.
- Not reconstructed: `_Init` (`0x47A780`) with its std::function handlers
  (`0x47D880`-`0x47E4D0`), and the subflare type registration
  (`PropArray<Subflare>::Init(Symbol)`, `0x47BC70`).
- Supporting changes: `RndDrawUtl::RotatedQuad2DParams` and
  `DrawRotatedQuad2D` (`0x3E1180`), `RndOcclusionQuery::SetShaderConstants`
  (`0x5F7E00`), and `RndOcclusionQueryMgr::RegisterQuery` (`0x5F8110`) and
  `GetViewIndex` (`0x5F8590`, an inferred name). `sample_function_table`
  (`0x645F20`) has moved out of RndShaderMgr.cpp's anonymous namespace and
  is now declared in RndShaderMgr.h.
- `RndOcclusionQuery::mResults` holds the query's world sphere, and
  `mFrame` holds its manager index. The fields keep their names.

`RndLightFlareEditCom` (`0x47E7B0`-`0x47EEDF`, vtable `0x1906298` with 59
slots, "LightFlareEditorData") is not reconstructed.

`RndLightHairUtl` and `RndLightSkinUtl` are not in this build. In the
older map they come after `RndLightGlobals.o` and
`RndLightProbeEditCom.o`. In this binary `RndLightGlobals` runs straight
into `RndLightMgrCom` at `0x480280`, and the probe edit component's static
initializer (`0x49F210`) ends where `RndLightSpotCom` begins (`0x49F2E0`).
The skin diffusion and hair reflectance textures are loaded as assets
(`skin_diffusion.bmp`, and `RndTextureUtilityCom` in
`inline_lighting_textures.entity`).
