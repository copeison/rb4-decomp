# Render camera context

`RndCameraContext` (`render/RndCameraContext.o`) holds a camera's view and
projection state for one render context. It is 9312 bytes, and each
`RndContext` embeds two: the main view and a copy used by the stereo target
modes.

| Offset | Field | Contents |
| --- | --- | --- |
| `0` | `mCamera` | the camera object |
| `8` | `mTargetMode` | `RndTargetMode` |
| `12` | `mViewportSize` | `Vector2` |
| `20` | `mDepthRange` | `Vector2` |
| `28` | `mProjectionRect` | `Hmx::Rect` |
| `48` | `mFrustum` | 8 corners, 12 edges, 6 planes |
| `496` | `mValid` | |
| `500` | `mLodMask` | defaults to 7 |
| `512` | `mLodData[3]` | 368 bytes each |
| `1616` | `mPrimaryView` | 1096 bytes |
| `2712` | `mViews` | one view per render-target slice, up to six |

A view holds the world and inverse-world transforms, the view, projection,
view-projection and inverse view-projection matrices, and the view-space and
world-space frusta.

## Target modes

`RndTargetMode` now has these values:

| Value | Mode | Slices |
| --- | --- | --- |
| -1 | none | |
| 0 | 2D | 1 |
| 1 | stereo | 2 |
| 2 | cube | 6 |
| 3 | left eye | 1 |
| 4 | right eye | 1 |

Only the 2D and cube names come from the renderer's own use. The stereo and
eye names follow the slice table and `gWhichEye`, which is the mode minus 3.

## Use from the context

`RndContext::SetCamera` (`0x6BD220`) sets the main camera and copies the
context to the second one. For the stereo and eye modes it then refreshes the
second context's target info. Unless a camera override is installed, it
re-syncs the camera constants.

`_SyncCameraCBuffer` (`0x6BCCD0`) writes the constants from one of three
sources, then selects the camera buffer:
- the override set by `SetCameraCBufferOverrideContext` (`0x6BD370`);
- the identity view-projection;
- the main camera context.

## RndContext fields

Recovering the camera also settled several `RndContext` fields:

| Offset | Field |
| --- | --- |
| `+24` | the colour targets, `FixedVector<RndTextureBase*, 8>` |
| `+112` | the depth target |
| `+120`, `+128`, `+136` | the viewport origin, size and depth range |
| `+18776` | the camera override |

## Not yet reconstructed

These are declared but not reconstructed:
- `Project`, `Unproject` and `CalcProjectedHeight`;
- `_CalcWorldXfms`, `_CalcProjectionMatrices`, `_CalcPrimaryFrusta` and
  `_CalcFrusta`.

The camera object is treated through `GameObject const*`, as in the map. The
binary reads near, far and an orthographic flag from it, so its real type is
probably a camera component.
