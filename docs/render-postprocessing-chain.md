# Post-processing chain

A scene's post-processing is a `RndPostProcCom` ("PProc") that lists stage
objects, each holding a `RndPostProcStageCom` subclass. All six map objects
are real `Component` classes (see [entity-components.md](entity-components.md));
their sources are in `src/render/postprocessing/chain`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndPostProcCom.o` | `0x62EF10`-`0x62F9FF` | `0x192E2A0` (41 slots) | 64 |
| `render/RndPostProcStageBloomCom.o` | `0x62FA00`-`0x63192F` | `0x192E3F8` (43) | 48 |
| `render/RndPostProcStageCom.o` | `0x631930`-`0x631EEF` | `0x192E600` (43) | 24 |
| `render/RndPostProcStageDOFCom.o` | `0x631EF0`-`0x6331AF` | `0x192E768` (43) | 80 |
| `render/RndPostProcStageFXAACom.o` | `0x6331B0`-`0x633B3F` | `0x192E8D0` (43) | 24 |
| `render/RndPostProcStageShaderGraphCom.o` | `0x633B40`-`0x63463F` | `0x192EA38` (43) | 24 |

## Class registration

| Class | Id | `sId` | `sClassName` | `sPropRegistry` | `sMetaData` | `_Init` | `Init` | `_Create` |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `RndPostProcCom` | `PProc` | `0x1AAA7A8` | `0x1AAA7B0` | `0x1AAA7C0` | `0x1AAA860` | `0x62F260` | `0x3F17F0` | `0x404840` |
| `RndPostProcStageCom` | `PostProcStage` | `0x1AAACA8` | `0x1AAACB0` | `0x1AAACC0` | `0x1AAAD60` | `0x631990` | `0x3F1440` | `0x4047F0` |
| `RndPostProcStageBloomCom` | `PProcBloom` | `0x1AAAA18` | `0x1AAAA20` | `0x1AAAA30` | `0x1AAAAD0` | `0x62FA90` | `0x3F9720` | `0x4059F0` |
| `RndPostProcStageDOFCom` | `PProcDepthOfField` | `0x1AAAF18` | `0x1AAAF20` | `0x1AAAF30` | `0x1AAAFD0` | `0x6320F0` | `0x3F9AD0` | `0x405A40` |
| `RndPostProcStageFXAACom` | `PProcFXAA` | `0x1AAB188` | `0x1AAB190` | `0x1AAB1A0` | `0x1AAB240` | `0x633210` | `0x3F9E90` | `0x405A90` |
| `RndPostProcStageShaderGraphCom` | `PProcShaderGraph` | `0x1AAB408` | `0x1AAB410` | `0x1AAB420` | `0x1AAB4C0` | `0x633BA0` | `0x3FA240` | `0x405AE0` |

`Init` and the factories are emitted with the renderer's other component
registrations; each factory is followed by a placement factory. Every
object's static initializer first sets three shared-header ints (-1, the
invalid `GameObjectId`, then 8 and 4), which are not modelled. The
constructors, destructors, identity slots, `_Imprint` and factories are
reconstructed. `Init` and `_Init` stay declared: the registration helpers,
`ComMetaData`'s strings and the property metadata (bloom's property
callbacks at `0x631630`-`0x631850`) are not modelled.

## RndPostProcCom

`stages` (`+0x18`) is a `PropArray<GameObjectId>`. Slot 29 returns true and
slot 38 is empty (the map's `_LoadResources` and `_EditPoll`).

- `Draw` (`0x62F060`; the map's `Draw(RndContext&, EntityPtr const&,
  RndSceneDrawParams const&) const`) takes the camera's
  `RndSceneInternalContext::CameraData`, the parameters and the
  `RndSceneBatchContext`. For every set stage id whose object exists it
  calls the object's base-class `PostProcStage` component's `Draw`, without
  a null check. The scene drawer calls it from `_DrawPostProc`
  (`0x425792`) with a batch context whose draw target it copies back
  afterwards.
- `UpdateDrawTarget` (`0x62F170`, name inferred) runs each stage's
  `UpdateDrawTarget` instead; the threaded job builder at `0x431AB0` calls
  it to predict the draw targets.
- `HasBloomStage` (`0x62F660`, name inferred) asks whether some stage's
  `GetId` is `PProcBloom`; called at `0x69482E`.

## Stages

`RndPostProcStageCom` holds `enabled` at `+0x16`, in `Component`'s tail
padding, so the base and the FXAA and shader-graph stages are 24 bytes.
`Draw` (`0x631BD0`) and `UpdateDrawTarget` (`0x631BF0`) call slots 41
(`_DrawImpl`) and 42 (`_UpdateDrawTarget`, name inferred) when it is set;
both slots are empty in the base.

A stage reads the scene from the draw target's `mSrcLightAccum` buffer
and writes `mDstLightAccum` (`RndBufferCollection::mLightAccum`, or the
active frame interval's partial-framerate buffer without a scene
context), then swaps the two and clears `mResult`. Slot 42 performs the
swap alone in the bloom, FXAA and shader-graph stages; the DOF stage
keeps the base's empty slot. The full-screen passes draw a `Quad2D` that
keeps the selected shader, within stencil mode 2 under read mask 6, into
the destination with the frame interval's depth-stencil bound. Barriers
move the light-accumulation buffers between render target and shader
resource (`0xC0` with tiled lighting, else `0x80`).

- **Bloom** (`_DrawImpl` `0x6305A0`): `intensity`, `power`, `shape`,
  `use_half_size_buffer`, `value_based`, `overbright_hue_preservation`,
  `overbright_whitening_strength`, `max_overbright_whitening`. It
  thresholds the scene into a downsample buffer (`RndShaderDownsample`),
  blurs it horizontally and vertically (`RndShaderBlur::Select`,
  `0x634BB0`, declared with its `Params`), repeats at the next level with
  the half-size buffer, and composites with `RndShaderBloom`. Above
  1920x1080 it starts one downsample level lower. Split barriers begin
  the destination's and the second level's transitions early.
- **DOF** (`_DrawImpl` `0x632BB0`): `near_blur_distance`,
  `far_blur_distance`, `blur_radius`, `bokeh_overbright_scale`,
  `blur_falloff_function`, `bokeh_tex`. `RuntimeData` (`+0x38`,
  constructor `0x632030`, destructor `0x632050`) holds the bokeh sprite
  buffer (1024 x 28 bytes) and a four-word buffer created by slot 29
  (`0x632060`), and the bokeh texture, which nothing loads in this build.
  The draw normalizes the distances to the camera's clip range, takes the
  overbright threshold from the light manager's exposure, and runs
  `RndCShaderDOFDiscBlur::Dispatch` (`0x6F2BC0`, declared) with the
  destination as an unordered-access target.
- **FXAA** (`_DrawImpl` `0x6333C0`): `RndShaderFXAA::Select` (`0x636570`,
  declared) on the source.
- **ShaderGraph** (`_DrawImpl` `0x633E80`): selects the material
  component of the stage's own object
  (`RndMaterialRuntimeData::SelectShader`, `0x4F8D10`, which takes the
  batch context in this build) and binds the source as shader-graph
  texture 3 (`RndContext::SetShaderNodeTexture`, `0x6BD390`).
