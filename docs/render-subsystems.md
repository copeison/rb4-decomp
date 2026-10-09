# Render subsystems

`Rnd::Init` (`0x402C30`) starts the renderer's subsystems in order, and
`Rnd::Terminate` stops them. These are reconstructed:

| Subsystem | Init | Terminate | Creates |
| --- | --- | --- | --- |
| `RndDrawUtl` | `0x3DF170` | `0x3DF820` | the debug-draw meshes, including six facing quads |
| `RndEditorDrawUtl` | `0x461D10` | `0x461F50` | the control-point texture and arrow mesh |
| `RndDebugFont` | `0x65C1A0` | `0x65C790` | the built-in bitmap font from `gBitmapData` |
| `RndDebugFont` extended fonts | `0x65CAF0` | `0x65CE70` | the font resources for the system language |
| `RndOcclusionQueryMgr` | `0x5F7ED0` | `0x5F7FB0` | the query sphere and its two shaders |
| `RndOverlayMgr` | `0x5F9150` | | the debug overlays |
| `RndBufferInspection` | `0x6B54A0` | `0x6B54E0` | the inspection shader |
| `RndTexturedQuadCom` | `0x44C8B0` (post-init) | `0x44C930` | the shared quad mesh |
| bounce-plane editor | `0x601880` | `0x601910` | a box mesh |

`RndPixelCanvas` is a real class with a vtable. It is created uninitialized
or filled with a colour, and `RndPixelData::CopyFrom` copies pixel data
between textures.

## Quirks

- **`RndDrawUtl::Terminate`** never frees its nested-cone mesh. This is kept
  from the binary.
- **The control-point texture** is a `ResourcePtr` in the source, so the
  source registers an exit destructor the binary does not have.

## Invented names

These names are not in the map:
- the extended-font functions;
- the bounce-plane editor helpers;
- `RndShaderOcclusionQuery`;
- most overlay classes and their bases. `RndOverlayGraphBase` may be
  misnamed: by link order its object file probably starts with `RndGraph`;
- `RndOverlayMgr::Poll` (0x5F9230).

## Debug overlays

The overlay classes live in `render/debug/overlays`, one file per class, in
link order. Text overlays derive from `RndOverlayTextBase` (0x6E5B00) and
graph overlays from `RndOverlayGraphBase` (0x6E32E0). The timer overlays share
`RndTimersOverlay` (0x6E7850) and `RndTimerGraphOverlay` (0x6E6280), and the
CPU and GPU variants only pick the timer source.

`RndOverlayMgr::Poll` gives the keyboard to the overlays when one wants it.
It installs `gKeyboardOverride` as the keyboard sink and saves the previous
sink, then restores it when focus is released. `DrawAll` (0x5F92D0) stacks the
overlays down the screen, with each one returning the line it ended on.

`RndBufferInspectionShader`'s constructor (0x6B5FA0) and `_InitConfigImpl`
(0x6B6860) are reconstructed too.

### Overlay drawing

Each overlay's `Draw`, `_Print`, `_Update` and keyboard handler is
reconstructed, together with `RndDrawUtl::DrawLine2D` (0x3DFC50), the narrow
`DrawText2D` (0x3E49C0) and `RndBufferInspectionShader::Select` (0x6B6050)
with its 160-byte `Params`.

- `RndTimersOverlay` shows each thread's timers as a tree. Its
  `TimedThreadListView` holds one `ThreadTimersListView` per thread, and that
  holds one `TimerItemView` per timer. In the binary, `HandleKeyboardMsg`
  (0x6E7D40) inlines the map's `SelectNextItem`, `SelectPrevItem`,
  `ExpandItem` and `CollapseItem`; the source keeps them as methods.
- The CPU and GPU timer overlays fill vtable slots 12 to 15: list the threads,
  gather a thread's timers, then print a header and each timer's stats.
- `RndTimerGraphOverlay` keeps a sample series per thread and per timer,
  trims samples older than the graph window, and assigns palette colours by
  use count.
- `RndFramerateOverlay` registers its "show CPU/GPU average" options in slot 7
  (`PropRegistry`) through file-local accessors.
- `RndDevice` slot 15 is `_GetMemoryUsageImpl` (an invented name). It returns
  the local and non-local memory in use.

### Graphs and timer lists

- `RndOverlayGraphBase` maps the axes' units to pixels with
  `_GraphToPixels` (0x6E4D00): x spans the viewport inside the overlay
  margins, y runs from the band's top to its bottom. `_FitAxes` widens a
  range when the other axis' labels (10 pixels, or 21 with a label) would
  leave the band. `_DrawSeries` cuts lines at the band's edges, `_DrawAxes`
  spaces ticks from the origin both ways, and `_DrawLegend` boxes the names
  by the options' legend alignment.
- The map's `TimerItemView::DrawHeader(RndTimersOverlay*, TextStream&,
  char const*, unsigned long)` is 0x6E9120, which prints all the column
  headers; the per-column helper 0x6E8C80 is `_PrintHeader` (invented).
  `_PrintStat` stays a static taking the overlay, which the GPU overlay
  passes and which is unused; its value is an `int`.
- A timer is listed when it is selected, isolated, or worse than
  `gTimerThresholdMs` (0x19B03B8, 0.1) under expanded parents. Isolated
  timers and timers with `mAmbiguousParent` set come after
  "<isolated timers>" and "<ambiguous parents>" headers. Threads sort with the main thread first,
  then the poll workers, then by name and handle.
- `RndTimersOverlay::gSplitFrameTiming` (0x1AB1F04) doubles every timing
  column. It is separate from `RndFramerateOverlay::gSplitFrameTiming`.

### Text and lines

- `RndDrawUtl::DrawLines2D` and `DrawQuadWireframe2D` (0x3E16B0) convert
  start points straight to clip space and end points through pixels, as
  `DrawQuad2D` does.
- The wide `DrawText2D` takes the viewport size and a context pointer;
  `MeasureText2D` passes a null context to lay the text out without drawing
  it. The layout is `RndTypesetter`'s (see below).
  Each style's font draws its glyphs page by page, after eight copies in the
  shadow color offset one pixel around them when `mShadow` is set.

## Not yet reconstructed

- `PollMgr::IsWorkerThread` (0x24F6E0, an invented name),
  `Thread::ThreadIdToName`, `DataVarIndex` and `DataVariable`.
- The typesetter's callees in `utl/UTF8.o` (`WideCharToChar`, `WToUpper`,
  `WToLower` and the CJK line-break rules) and `ObjPtr::MakeErrorName`
  (declared on `GameObject`).
- The console input's slots 6 to 9 (0x6E1910, 0x6E1930, 0x6E1950,
  0x6E1990): they register the console with `TheDebug`, clear its output
  and synchronize with it, but the line editor they belong to is not
  modelled.

## Fonts

`RndFont` keeps one size per resolution. Each size has:
- the tile size, glyph height, space size, spacing and fixed width;
- a list of pages and sorted kerning pairs.

Each `RndFontPage` holds a texture and its glyphs. A glyph's UV rectangle is
left, top, width and height.

`Finalize` sorts the kerning pairs with `std::sort`; the binary uses EASTL's
sort, so pairs with equal keys may end up in a different order.

## Typesetter

`RndTypesetter` (`render/fonts/RndTypesetter.cpp`, 0x67D910-0x681610) lays
wide text out in glyphs. `ProcessText` (0x67E590) copies the params, sizes
per-style page counts on the stack, and runs the layout once per style size:
generate the glyphs, then wrap the lines. Shrink to fit (3) and wrap and
shrink (4) step to the next size until the lines fit the width or the text
fits the height. If no size fits, the last size runs once more as word wrap
(from 4) or with no fitting (from 3). Vertical alignment, justification and
the bounds follow.

- Positions are font pixels with y up. Lines run downward from zero, one
  glyph height plus the vertical spacing apart, measured with the first
  style's font.
- Markup is `<name=value>` between `<` and `>`, without tabs or spaces.
  `<style=name>` pushes a style, `</style>` pops one, and `<icon=name>`
  inserts the icon as a blank glyph, once per icon. Every parsed tag is
  consumed; unknown tags only warn.
- A missing character comes from the style's fallback, the style
  `mNumStyles / 2` further on, when extended fonts exist. Otherwise it is
  drawn as U+25A0, or skipped if the font lacks that too.
- Word wrap breaks before spaces, zero-width spaces, characters after a
  hyphen, and CJK boundaries that the kinsoku sets allow. Truncate (2) drops
  glyphs until three periods fit and ends the text with them.
- The release build drops the warnings. Only their conversion of the text
  through `WideCharToChar` and the error prefix (0x680210) remain, and the
  source keeps both.

The map's `_InitResults`, `_InitContext`, `_InitIconResults`,
`_ApplyAlignment`, `_FinalizeIconResults`, `_IsCharNumeric` and the
`Params`/`Context` overload of `_TryParseMarkup` are inlined and have no
out-of-line copy. `_ApplyJustification` (0x67F560) and both
`_SkipWhitespaceAndMarkup` overloads have out-of-line copies with no
callers. This build adds the word wrap (`_ApplyWordWrap` 0x67F1B0,
`_WrapLine` 0x681470 and `FindLineBreaks` 0x6811F0), a page argument to
`_ProcessOneGlyph`, and the resolution to `RndFont::FindGlyphOnPage`
(0x65D600) and `GetKerning` (0x65D690). The three word-wrap names are
invented, as are `_MakeErrorPrefix`, `RndDebugFont::HasExtendedFonts`
(0x65CEC0), the enumerators in `RndTextEnums.h` (which follow the text
options' display names) and `BufVector`'s members.
