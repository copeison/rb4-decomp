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

| Class | Id | `sId` | `sClassName` | `sPropRegistry` | `sMetaData` | `_Init` |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| `RndCMAACom` | `AntiAliasing` | `0x1A75358` | `0x1A75360` | `0x1A75370` | `0x1A75410` | `0x44EBB0` |
| `RndAtmosphereCom` | `Atmosphere` | `0x1A75618` | `0x1A75620` | `0x1A75630` | `0x1A756D0` | `0x4514D0` |
| `RndFogCom` | `Fog` | `0x1A75890` | `0x1A75898` | `0x1A758A0` | `0x1A75940` | `0x451DD0` |
| `RndSkyCom` | `Sky` | `0x1A75B20` | `0x1A75B28` | `0x1A75B30` | `0x1A75BD0` | `0x453060` |
| `RndVolumetricScatteringCom` | `VolumetricScattering` | `0x1A75DB8` | `0x1A75DC0` | `0x1A75DD0` | `0x1A75E70` | `0x454A70` |
| `RndSSAOCom` | `SSAO` | `0x1A89888` | `0x1A89890` | `0x1A898A0` | `0x1A89940` | `0x4AC540` |

The identity slots, the constructors, destructors and `_Imprint` copies are
reconstructed. `_Init` and `RndAtmosphereCom::_InitAsSuperclass`
(`0x4522E0`, emitted in the fog's object and shared with the volumetric
scattering) stay declared: the property metadata is not modelled. The
quality-settings arrays are `PropArray`s of one entry per
`RndQualityLevel`, sized to three by `_PostCreate` (slot 19).

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
`Waveform<float>`, modelled as `HeightDensity` only as far as the class
touches it), `height_range_begin`, `height_range_end` and
`volumetric_light_intensity`. `RuntimeData` (`+0x88`) holds the scaled
density, the dirty flag and the baked height texture.

`BeginAsyncUpdate` runs `RndCShaderVScatCalcDensityInscattering` (skipped
for the right eye of a shared stereo volume) and
`RndCShaderVScatAccumScattering`; `ApplyDeferred` runs
`RndCShaderVScatDeferred` and swaps the target's light accumulation;
`EndAsyncUpdate` and `SkipApplyDeferred` record the resolution in the
target. The fog range is the atmosphere's clamped to the camera.
`_OnResourcesLoaded` and `_CreateTexture` stay declared: they need
`Waveform<float>` and the inlined-resource path helper (`0x1CFEE0`).

## Ambient occlusion and antialiasing

`RndSSAOCom` (`radius`, `angleBias`, `intensity`) builds a 256 by 256
noise texture when its resources load and dispatches `RndCShaderSSAOGen`
into the frame interval's AO buffer (`GenerateAO`).

`RndCMAACom` (`edge_threshold`, `non_dominant_edge_threshold`) runs the
four CMAA compute shaders over the light accumulation (`Draw`), clearing
the edge buffers on a collection's first draw and alternating them with
`RndBufferCollection::mCMAAState`. The shaders' `Dispatch` functions, like
the volumetric and SSAO dispatches, are declared only.
