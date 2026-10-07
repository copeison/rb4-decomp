# PS4 import recovery

Orbis executables replace imported symbol names with 11-character NIDs. This
executable's generic ELF analysis therefore began with address-named PLT stubs,
which obscured both platform calls and the surrounding game logic.

The PS4 SDK 5.008 stub files retain a `.dynsym` symbol table and a parallel
`.scenid` table. `tools/resolve_ps4_imports.py` converts each binary NID to the
same modified Base64 form used by the executable, then joins it with the
executable's dynamic symbols and jump relocations.

## Current coverage

| Result | Imports |
| --- | ---: |
| Total PLT entries | 706 |
| Resolved unambiguously | 568 |
| Ambiguous | 0 |
| Unresolved | 138 |

The resolved set includes the C and C++ runtimes and the kernel, POSIX, GNM,
NP, HTTP, USB, pad, video, save-data, dialog, PlayGo, and user-service APIs.
The executable's own dynamic module table identifies the unresolved entries
precisely: 76 are from `libfmod`, 59 from `libfmodstudio`, and three from
`libScePad`. The FMOD names require matching middleware import libraries or
independent call-site recovery. The three pad NIDs are not present in the
supplied 5.008 stub set and will be recovered from their callers.

## Reproduce the export

With the local SDK at `tools/ps4-sdk/`, run:

```powershell
python tools/resolve_ps4_imports.py files/eboot.elf `
  --sdk tools/ps4-sdk `
  --output analysis/exports/imports.csv
```

The CSV preserves the PLT and GOT addresses, encoded symbol, NID, decoded
library and module names, resolved API name, source stub libraries, and
resolution status. Only unique matches are safe to apply automatically to IDA.
