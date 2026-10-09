# Script console input

The console overlay's command line (`src/render/debug/overlays`) is
`RndConsoleOverlay::ConsoleInput` (vtable `0x1939418`, constructor
`0x6E18C0`), a subclass of a script line editor, `ConsoleLineEditor` (an
inferred name). The editor sits at `0x11AECB0`-`0x11B0D20`, after
`JsonResource`, among code that is not in the reference map. Its vtable,
`0x19A5818`, has 10 slots. The map's build had the editor's methods on
`RndConsoleOverlay` (`_History*`, `_TabCompletion*`, `_ExecuteCommand`), and
the source keeps those names.

| Slots | Members |
| ---: | --- |
| 0-1 | destructors (`0x11AF060`, `0x11AF110`); the history strings are never freed |
| 2-5 | the line and the cursor, stored by the subclass (pure here) |
| 6-7 | around a command: the console adds and removes its reflection on `TheDebug` |
| 8 | control+L: the console forgets its output lines |
| 9 | after a command: the console takes and releases the reflection's lock |

The editor's flags enable the editing keys (1) and the cursor keys (2); the
console sets both. `HandleKeyboardMsg` (`0x11AF1C0`) handles typing,
backspace, delete, home, end, left and right, history browsing with up and
down, tab completion and enter. With control, left and right jump words,
up and down search the history for the text before the cursor, L clears
the output and C clears the line.

The history holds up to 50 strings from a pool created by the constructor;
the last entry is the line being typed. `_HistoryAddCommand` moves a
repeated command to the end. Tab completion collects the script functions
(`GetDataFuncs`, `0x2222B0`) and the variables (`DataForEachVariable`,
`0x236E70`; both names inferred) that start with the word before the
cursor. A word after `$` completes only variables. Repeated tabs cycle
through the matches, and back to the typed word.

`_ExecuteCommand` (`0x11AF710`) parses the line. It evaluates a single
command or variable, executes anything else, and restores the default
entity afterwards. `mOutputType` (0 while parsing, 1 while running) tags
the console's output lines. The "Evaluates to" text it builds is never
shown.
