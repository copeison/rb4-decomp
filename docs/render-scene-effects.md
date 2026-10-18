# Render scene effects

The scene's atmosphere, sky, ambient occlusion and antialiasing are real
`Component` classes (see [entity-components.md](entity-components.md)).
The scene component names the objects that hold them (`atmosphere`, `sky`,
`antialiasing`; the light manager's `ambient_occlusion`), and the scene
drawer runs their passes.

| Object | Range | Vtable | Size | Source |
| --- | --- | ---: | ---: | --- |
| `render/RndCMAACom.o` | `0x44E9E0`-`0x45048F` | `0x19027F0` | 72 | `postprocessing/antialiasing` |
| `render/RndAtmosphereCom.o` | `0x451450`-`0x451C6F` | `0x1902BD8` | 32 | `atmosphere` |
| `render/RndAtmosphereGlobals.o` | `0x451C70`-`0x451CEF` | - | 8 | `atmosphere` |
| `render/RndFogCom.o` | `0x451D10`-`0x452C9F` | `0x1902D30` | 64 | `atmosphere` |
| sky (not in the map) | `0x453030`-`0x453BFF` | `0x1902EF0` | 32 | `atmosphere` |
| `render/RndVolumetricScatteringCom.o` | `0x453C00`-`0x45760F` | `0x1903048` | 152 | `lighting/volumetric` |
| `render/RndSSAOCom.o` | `0x4AC310`-`0x4ADB7F` | `0x1908190` | 88 | `lighting/ambient_occlusion` |

Every vtable has 41 slots. `RndFogCom` and `RndVolumetricScatteringCom`
derive from `RndAtmosphereCom`; the others from `Component`. The sky's
class is named `RndSkyCom` after its class id; the map spells the
volumetric class `RndVolumtericScatteringCom`, but this build's own type
string is `"RndVolumetricScatteringCom::QualitySettings"`.

## Class registration

| Class | Id | `sId` | `sClassName` | `sPropRegistry` | `sMetaData` | `_Init` | `Init` | `_Create` |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `RndCMAACom` | `AntiAliasing` | `0x1A75358` | `0x1A75360` | `0x1A75370` | `0x1A75410` | `0x44EBB0` | `0x3EC880` | `0x4041B0` |
| `RndAtmosphereCom` | `Atmosphere` | `0x1A75618` | `0x1A75620` | `0x1A75630` | `0x1A756D0` | `0x4514D0` | `0x3EC0F0` | `0x404110` |
| `RndFogCom` | `Fog` | `0x1A75890` | `0x1A75898` | `0x1A758A0` | `0x1A75940` | `0x451DD0` | `0x3EDB10` | `0x404340` |
| `RndSkyCom` | `Sky` | `0x1A75B20` | `0x1A75B28` | `0x1A75B30` | `0x1A75BD0` | `0x453060` | `0x3F83A0` | `0x4057D0` |
| `RndVolumetricScatteringCom` | `VolumetricScattering` | `0x1A75DB8` | `0x1A75DC0` | `0x1A75DD0` | `0x1A75E70` | `0x454A70` | `0x3F1BA0` | `0x404890` |
| `RndSSAOCom` | `SSAO` | `0x1A89888` | `0x1A89890` | `0x1A898A0` | `0x1A89940` | `0x4AC540` | `0x3F0CA0` | `0x404750` |

The identity slots, the constructors, destructors and `_Imprint` copies are
reconstructed, and so are the registrations. `Init` is
`InitComponentClass<T>` in `render/RndInit.o`; the atmosphere is a
superclass root that is not created itself. Its builder registers the
"Atmosphere" interface. `RndAtmosphereCom::_InitAsSuperclass` (`0x4522E0`)
is emitted in the fog's object and shared with the volumetric scattering.
The quality-settings arrays are `PropArray`s of one entry per
`RndQualityLevel`, sized to three by `_PostCreate` (slot 19). Each is
registered as a struct type (`RegisterPropStruct`, at `0x44F260`,
`0x4ACD10` and `0x456000`), fixed in size and summarized by the level's
name. Each class's revision handler sizes the array for older saves:
CMAA and SSAO at revision 1, the volumetric scattering at revision 4.
Notes on the builders:

- Both distance change callbacks (`0x451B60`, `0x451B80`) pull the start in
  to the end, as `_SyncEndDist` does.
- The fog and the volumetric resolution offer their values as allowed
  values. The sky's "texture_resolution" does the same.
- The volumetric handler (`0x456C10`) moves the old `fog_start_dist` and
  `fog_end_dist` into the atmosphere's distances. Before revision 2 it ends
  the fog at the far plane. Before revision 1 it copies `resolution` into
  every level.
- The volumetric builder makes the default height density
  (`gDefaultHeightDensity`, `0x1A76020`) a type-2 waveform. Its inlined
  keys (0, 1) and (1, 1) are not reconstructed: the key tracks are not
  modelled.
- SSAO's per-level `enabled` help repeats CMAA's text, as in the binary.

## Atmosphere, fog and sky

`RndAtmosphereCom` holds `enabled` (`+0x16`), `start_dist` and `end_dist`
(in meters, `gUnitsPerMeter` at `0x19B033C`; the end defaults to a
kilometer). `_PostCreate` makes it the scene's atmosphere when the scene
has none; `SetStartDist` and `SetEndDist` keep the range ordered.

`RndFogCom` adds `falloff_function` and a `RuntimeData` (`+0x28`) holding
the falloff constants and the fog's constant buffer, refreshed by
`_OnResourcesLoaded` and both polls (`_SyncFalloffParams`). `ApplyDeferred`
(`0x452390`) draws a stencil-tested quad over the light accumulation with
`RndShaderFogDeferred` (whose `Select`, `0x452D20`, is reconstructed; its
constructor no longer registers it). The shader lives in
`RndAtmosphereGlobals`, which `RndDevice` embeds at `0xDE8` as
`mAtmosphere`; `Init` and `Terminate` create, register and delete it.

`RndSkyCom` draws its object's `RndMaterialCom` into
`RndBufferCollection::mAtmosphere[texture_resolution]` (`UpdateTexture`,
"Update Sky Texture"); the fog fades into that texture.

## Volumetric scattering

The properties are `quality_settings` (`enabled`, `resolution` of 128, 256
or 512 slices), `fog_density`, `use_camera_end_dist`, `height_density` (a
`Waveform<float>`), `height_range_begin`, `height_range_end` and
`volumetric_light_intensity`. `RuntimeData` (`+0x88`) holds the scaled
density, the dirty flag and the baked height texture.

`BeginAsyncUpdate` runs `RndCShaderVScatCalcDensityInscattering` (skipped
for the right eye of a shared stereo volume) and
`RndCShaderVScatAccumScattering`; `ApplyDeferred` runs
`RndCShaderVScatDeferred` and swaps the target's light accumulation;
`EndAsyncUpdate` and `SkipApplyDeferred` record the resolution in the
target. The fog range is the atmosphere's clamped to the camera.
`_CreateTexture` bakes the height density into a 128 by 8 texture
(`RndTextureUtl::CreateWaveformFloatTextureResource`, declared) unless the
waveform's resource blends two baked curves (type 3).
`_OnResourcesLoaded` stays declared: it needs the inlined-resource path
helper of `entity/Waveform.o` (`0x1CFEE0`).

## Ambient occlusion and antialiasing

`RndSSAOCom` (`radius`, `angleBias`, `intensity`) builds a 256 by 256
noise texture when its resources load and dispatches `RndCShaderSSAOGen`
into the frame interval's AO buffer (`GenerateAO`).

`RndCMAACom` (`edge_threshold`, `non_dominant_edge_threshold`) runs the
four CMAA compute shaders over the light accumulation (`Draw`), clearing
the edge buffers on a collection's first draw and alternating them with
`RndBufferCollection::mCMAAState`. The shaders' `Dispatch` functions, like
the volumetric and SSAO dispatches, are declared only.
