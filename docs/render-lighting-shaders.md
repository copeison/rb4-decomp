# Lighting and tonemap shaders

`RndLightGlobals::_InitShaders` (`0x47F300`) creates and registers the
renderer's lighting shaders. The constructors do not register themselves; the
caller calls `RndShader::_Register` after each `new`.

| Member | Class | Constructor | Source |
| --- | --- | --- | --- |
| `+40` | `RndLightDirectionalDeferredShader` | `0x477310` | `LightDirectionalDeferred.hlsl` |
| `+48` | `RndShaderLightDirectionalShadowGen` | `0x4AB780` | `LightDirectionalShadowGen.hlsl` |
| `+56` | `RndLightPointDeferredShader` | `0x496D10` | `LightPointDeferred.hlsl` |
| `+64` | `RndShaderLightPointShadowGen` | `0x4ABB20` | `LightPointShadowGen.hlsl` |
| `+72` | `RndLightSpotDeferredShader` | `0x4A86F0` | `LightSpotDeferred.hlsl` |
| `+80` | `RndShaderLightSpotShadowGen` | `0x4ABED0` | `LightSpotShadowGen.hlsl` |
| `+88` | `RndLightProbeDeferredShader` | `0x49DE70` | `LightProbeDeferred.hlsl` |
| `+96` | `RndLightProbeDeferredAccumShader` | `0x49DBA0` | `LightProbeDeferredAccum.hlsl` |
| `+184` | `RndTonemapShader` | `0x4ADB80` | `TonemapPS.hlsl` |

The remaining shaders are created only when the PS4 capabilities have the
async-compute feature (bit `0x10`):

| Member | Class | Constructor | Source |
| --- | --- | --- | --- |
| `+112` | `RndCShaderTiledLightsCull` | `0x6D8A20` | `compute/TiledLightsCull.hlsl` |
| `+120` | `RndCShaderTiledLightsApplication` | `0x6D7750` | `compute/TiledLightsApplication.hlsl` |
| `+128` | `RndCShaderTiledLightsInterpolation` | `0x6D9F10` | `compute/TiledLightsInterpolation.hlsl` |
| `+136` | `RndCShaderTiledLightsStereoToMono` | `0x6DA550` | `compute/TiledLightsStereoToMono.hlsl` |
| `+192` | `RndCShaderTonemap` | `0x6DAF60` | `compute/TonemapCS.hlsl` |

## Base classes

`RndLightDeferredShader` (`0x6DB320`, map name) is the base of the five
deferred-light shaders and of the point and spot shadow generators. It owns two
pixel defines and five constant and resource indices. `SelectCommon`
(`0x6DB3B0`) binds the G-buffer, and `_SelectBase` (`0x6DB590`) finishes each
subclass's `Select`. The directional shadow generator derives from `RndShader`
directly.

The two tonemap shaders share a base at `0x4ADC40`. The map has no name for
it, so the source calls it `RndTonemapShaderBase`. In the map's build
`RndCShaderTonemap` configured itself, and its `Dispatch` took two textures and
a float; in this build it takes the base's `Params`.

## Not reconstructed

- `RndLightGlobals::_InitMeshes` is now reconstructed; see
  `docs/render-mesh-builders.md`.

The two tiled-light dispatches that read the camera are reconstructed on top of
`RndCameraContext` (`docs/render-camera-context.md`).

## Open questions

Default textures are fetched through `RndDefaults::GetTexture`. Two things
there look wrong:
- the `TextureFamily` layout in `RndDefaults.h` looks transposed;
- with the scene mask off, the tiled dispatches bind a default that the
  current layout calls the black 3D texture.
