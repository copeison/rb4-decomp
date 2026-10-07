# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0x3C0` | `game_main` | `src/game/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x997590` | `stage_presence_id_to_symbol` | `src/game/stage_presence.cpp` | All 22 enum values and their interned symbol strings recovered. |
| `0xBB06A0` | `ui_layout_id_to_symbol` | `src/ui/ui_layout_id.cpp` | Invalid ID plus all 108 ordered layout IDs recovered. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line.cpp` | Complete behavior and observed container layout reconstructed. |

## Command-line argument layout

IDA shows 16-byte entries containing a string pointer at offset 0 and a handled
flag at offset 8. The owning object begins with the usual three pointers for a
contiguous container: first element, one-past-last element, and capacity. The
reconstructed function walks the first two pointers and marks every unhandled
argument whose first character is `-`.

The structure names are descriptive because original symbols are unavailable.
Offset and size assertions preserve the observed binary layout.

## Stage presence IDs

The function at `0x997590` initializes a guarded table of 22 eight-byte engine
symbols and indexes it directly with the requested ID. Every symbol string is
embedded beside the function, which makes the enum order exact. The cleaned
source uses a function-local static array to express the same one-time
initialization without reproducing compiler guard internals.

## UI layout IDs

The function at `0xBB06A0` contains a guarded table for IDs 0 through 107 and a
separate `kLayoutInvalid` result for ID -1. The ordered list lives in
`src/ui/ui_layout_list.inc` so the enum and symbol table share one readable
source of truth. ID `0x2C` is `kLayoutGameStartup`, matching the startup layout
loaded by `game_initialize`.
