# Render options components

The renderer's debug and editor options are components on the root object
of the options entity (`RndOptionsResource`, in `render/RndOptions.o`).
Their sources are in `src/render/options` and `src/render/debug`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndOptions.o` | `0x468000`-`0x46A830` | `0x1904610` (`RndOptionsResource`, 33 slots) | 200 |
| `render/RndOptionsCom.o` | `0x46A870`-`0x46ACC6` | `0x1904728` (41 slots) | 24 |
| `render/RndOverlayOptionsCom.o` | `0x5F9880`-`0x5FA690` | `0x192B128` (41 slots) | 24 |
| `render/RndLightOptionsCom.o` | `0x48ED90`-`0x49005B` | `0x1906A38` (41 slots) | 64 |
| `render/RndCameraOptionsCom.o` | `0x6B9270`-`0x6B993B` | `0x1937578` (41 slots) | 32 |
| `render/RndMeshOptionsCom.o` | `0x5D0CE0`-`0x5D19FB` | `0x1929A10` (41 slots) | 56 |
| `render/Rnd2DOptionsCom.o` | `0x6D1910`-`0x6D1E6B` | `0x1938830` (41 slots) | 24 |
| `render/RndSelectionOptionsCom.o` | `0x46ACD0`-`0x46B87B` | `0x1904880` (41 slots) | 48 |
| `render/RndSplineOptionsCom.o` | `0x659180`-`0x65A5BB` | `0x19304E0` (41 slots) | 40 |
| `render/RndTextOptionsCom.o` | `0x67C560`-`0x67D42B` | `0x1932A08` (41 slots) | 64 |

| Static | RndOptionsCom | RndOverlayOptionsCom |
| --- | ---: | ---: |
| `sId`, `sClassName` | `0x1A869D0`, `0x1A869D8` ("Options") | `0x1AA7CB8`, `0x1AA7CC0` ("OverlayOptions") |
| `sPropRegistry` | `0x1A869E0` | `0x1AA7CD0` |
| `sMetaData` | `0x1A86A80` | `0x1AA7D70` |
| `_Init` | `0x46A8E0` | `0x5F9900` |

## RndOptionsCom

The base class has no properties. `_Init` makes it a lightweight rendering
class allowed only in `RndOptionsResource`, with editor restrictions 12.
Its one member, `RuntimeData::mSuppressed` (`+0x16`), is what
`RndOptions::SetSuppressed` (`0x469CB0`) copies into every options
component. Each subclass's `_Init` first calls the superclass helper at
`0x3BF4C0` (`_InitAsSuperclass`, inferred), which links the metadata to
`RndOptionsCom`'s, runs `RndOptionsCom::_Init` in the "metadata" heap and
combines the editor restrictions.

## RndOverlayOptionsCom

The component lists one "show" property per registered overlay, plus an
`<overlay>_options` group for overlays with options of their own.
`RndOverlayMgr` sets `mOverlaysChanged` (`+0x17`) when an overlay registers
or unregisters, and `_GetPropRegistry` then rebuilds the registry
(`_RebuildRegistry`, `0x5F9AD0`, inferred name). The getter and setter
lambdas (`0x5FA470`, `0x5FA4A0`) read and set the overlay's showing state.
The live component is the map's `theRndOverlayOpts` (`0x1AA7F18`), set by
`_OnResourcesLoaded` (slot 29) and cleared by `_PreDestroy` (slot 20); it
replaces the former `RndOverlayOptionsCom::sInstance`.

## The other options classes

Each class keeps the editor's display switches as members named after the
properties its `_Init` registers: lights, probes and flares
(`RndLightOptionsCom`), camera frusta (`RndCameraOptionsCom`), the selected
mesh's vertices, normals and tangents (`RndMeshOptionsCom`), scale-9 quad
vertices (`Rnd2DOptionsCom`), wireframes, bounding spheres and distance
lines (`RndSelectionOptionsCom`), spline points and hulls
(`RndSplineOptionsCom`), and text bounds plus the defaults of new text
(`RndTextOptionsCom`). Like the overlay options, each sets the map's
`theRnd*Opts` global in `_OnResourcesLoaded` and clears it in `_PreDestroy`:
`theRndLightOpts` (`0x1A88888`), `theRndCameraOpts` (`0x1AB0028`),
`theRndMeshOpts` (`0x1AA68F8`), `theRnd2DOpts` (`0x1AB15B8`),
`theRndSelectionOpts` (`0x1A86E98`), `theRndSplineOpts` (`0x1AAC7A8`) and
`theRndTextOpts` (`0x1AAE278`). The registry builders (`_Init`, after each
class's deleting destructor) and the mesh options' property callbacks
(`0x5D1730`-`0x5D1930`) are not reconstructed.

## Imprints

Every options class's `_Imprint` copies the component at the buffer's next
8-byte boundary inside a `ScopedImprint` (`0xB4D0`, which sets the
thread's `PropArrayBase::sImprinting`), marks the copy imprinted and appends
its dynamic properties (`_ImprintProps`). The copy keeps the members but
starts a fresh `RndOptionsCom::RuntimeData`.

## RndOptions

`RndOptions` holds the options entity (`gEntity`, `0x1A868C0`) and the
suppressed flag (`gSuppressed`, `0x1A868C8`). `SetSuppressed` copies the flag
into every component of the root object that is an `RndOptionsCom`.
`RndOptionsResource` registers the extension "rndopt" under "Renderer
Options" (`_Init`, `0x469E10`). Its `CreateEntity` names the root object
"options".

Not reconstructed:
- `RndOverlayOptionsCom::_RebuildRegistry`, its lambdas and the
  `std::function` plumbing (`0x5FA4E0`-`0x5FA5D0`).
- `RndOptions::Init`, `gEntityRes` and the nine `*OptionsCom::Init`
  registrations with their factories, which need the registration helpers
  that the audio components also leave declared.
