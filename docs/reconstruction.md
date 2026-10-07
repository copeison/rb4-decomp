# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0x3C0` | `game_main` | `src/game/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line.cpp` | Complete behavior and observed container layout reconstructed. |

## Command-line argument layout

IDA shows 16-byte entries containing a string pointer at offset 0 and a handled
flag at offset 8. The owning object begins with the usual three pointers for a
contiguous container: first element, one-past-last element, and capacity. The
reconstructed function walks the first two pointers and marks every unhandled
argument whose first character is `-`.

The structure names are descriptive because original symbols are unavailable.
Offset and size assertions preserve the observed binary layout.
