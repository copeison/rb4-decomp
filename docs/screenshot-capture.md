# Screenshot capture

The pre-frame callback in the main loop is the renderer's screenshot path. A
request sets one byte at `0x1A7306C`; `screenshot_capture_pending` at `0x43B130`
reads it, and a successful capture clears it.

## Resolution selection

`screenshot_capture_frame` at `0x43B140` uses the active frame owner's first
render target when the resolution mode is zero. Modes one through five select
fixed output sizes recovered from the two tables at `0x1281700` and
`0x1281720`:

| Mode | Width | Height |
| ---: | ---: | ---: |
| 1 | 1,280 | 720 |
| 2 | 1,920 | 1,080 |
| 3 | 3,840 | 2,160 |
| 4 | 7,680 | 4,320 |
| 5 | 9,024 | 5,076 |

The mode-name helper reports mode zero as `Window Dimensions`; fixed modes use
their full dimensions followed by the reduced `16:9` aspect ratio.

The capture remains pending when the selected extent has a zero dimension. A
valid extent clears the request and recreates the resource named `Screenshot`
when its dimensions differ from the cached target.

## Render and save

The helpers at `0x448730`, `0x448760`, and `0x4487C0` read the first render
target's extent, material draw-debug mode, and buffer debug view. The capture
copies both debug selections so screenshots match the displayed renderer view.

`screenshot_capture_to_file` at `0x43B240` binds the screenshot target, invokes
the supplied render callback, submits and resolves the target, reads its image
data back, and writes `../screenshots/screenshot.png`. Platform command-buffer
and image-resource details remain isolated behind adapters while their class
layouts are still incomplete.
