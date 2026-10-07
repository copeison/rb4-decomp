# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0xA0` | `game_initialize` | `src/game/initialize.cpp` | Complete top-level initialization order and arguments recovered; two option-field meanings remain unknown. |
| `0x3C0` | `game_main` | `src/game/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x997590` | `stage_presence_id_to_symbol` | `src/game/stage_presence.cpp` | All 22 enum values and their interned symbol strings recovered. |
| `0xBB06A0`, `0xBB5D40` | `ui_layout_id_to_symbol`, layout asset map | `src/ui/ui_layout_id.cpp` | All 108 ordered IDs and their 92 direct layout paths recovered. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line.cpp` | Complete behavior and observed container layout reconstructed. |

## Game initialization

`game_initialize` performs a fixed startup sequence: core services, UI and
stage-presence identifiers, `config/rockband.dta`, sound, engine type
registrations, primary game systems, UI resources, Dingo backend networking,
time-stretch audio, and player assignments. It then loads
`kLayoutGameStartup` and marks remaining command-line switches handled.

The game-system option block is exactly 16 bytes. Its first three bytes are set
to true and its final eight bytes are zero. Rendering initialization reads the
second flag directly; the meanings of the first and third flags are retained as
unknown fields until their virtual consumer is recovered.

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

The loader at `0xBB5D40` supplies direct `.layout` paths for 92 of those IDs.
The same include now drives `ui_layout_primary_path`, keeping every recovered
path aligned with its enum value. Sixteen deprecated, fallback, or unsupported
IDs have no direct path. SOMP IDs 101 and 102 also fall through in the original
switch to preload the subsequent session layouts; the helper reports the first
path associated with each requested ID.
