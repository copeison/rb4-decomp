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
| Resolved unambiguously | 706 |
| Ambiguous | 0 |
| Unresolved | 0 |

The resolved set includes the C and C++ runtimes and the kernel, POSIX, GNM,
NP, HTTP, USB, pad, video, save-data, dialog, PlayGo, user-service, FMOD, and
FMOD Studio APIs.

Three private `libScePad` exports omitted from the supplied SDK stubs are now
resolved through the reviewed `tools/ps4_known_nids.csv` map. Their names and
call-site evidence are documented in `docs/special-pad-reader.md`. The CSV's
`name_source` column distinguishes SDK-stub results from reviewed additions.

The 135 middleware imports are resolved by hashing the reviewed original
linker names in `tools/ps4_symbol_names.csv`. See `docs/fmod-imports.md` for the
FMOD 1.10.04 evidence, Itanium C++ symbol recovery, and call-site checks.

## Reproduce the export

With the local SDK at `tools/ps4-sdk/`, run:

```powershell
python tools/resolve_ps4_imports.py files/eboot.elf `
  --sdk tools/ps4-sdk `
  --output analysis/exports/imports.csv
```

The CSV preserves the PLT and GOT addresses, encoded symbol, NID, decoded
library and module names, resolved API name, name source, source stub
libraries, and resolution status. Only exact library-specific NID matches are
applied to IDA.
