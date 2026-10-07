# Rock Band 4 PS4 decompilation

This repository tracks the analysis and reconstruction of the PS4 Rock Band 4
executable. The original SELF is stored as `files/eboot.bin`. Generated ELF and
IDA database files stay local and are excluded from Git.

## Current baseline

- Input: PS4 fake-signed/decrypted SELF, x86-64 little endian
- Executable type: `ET_SCE_DYNEXEC` (`0xFE10`)
- Compiled SDK: PS4 SDK `5.008.001` (`0x05008001`)
- Internal module name: `rockband_ps4_s`
- Original linker output path:
  `D:/TeamCity/buildAgent/work/de1ea97e21c98eba/main/rockband/build/rockband_ps4_s.elf`

See [docs/binary.md](docs/binary.md) for the binary inventory and
[docs/status.md](docs/status.md) for analysis milestones. The first recovered
control flow is documented in [docs/startup.md](docs/startup.md), and the
initial address map is in [docs/subsystems.md](docs/subsystems.md).
Cleaned source progress is tracked in
[docs/reconstruction.md](docs/reconstruction.md), with the main update order in
[docs/frame-loop.md](docs/frame-loop.md). Private pad import recovery and the
special-controller calibration path are described in
[docs/special-pad-reader.md](docs/special-pad-reader.md). FMOD 1.10.04 import
recovery is documented in [docs/fmod-imports.md](docs/fmod-imports.md).
The recovered FMOD startup sequence and custom DSP registration are described
in [docs/audio-initialization.md](docs/audio-initialization.md).

## Recreate the analysis ELF

```powershell
python tools/extract_ps4_self.py files/eboot.bin files/eboot.elf `
  --ida-output files/eboot_ida.elf
```

The native Orbis ELF uses a Sony-specific executable type that stock IDA does
not recognize. Change only `e_type` from `0xFE10` to standard `ET_DYN` (`3`) in
a local copy named `files/eboot_ida.elf` before importing it into IDA. The
extractor's `--ida-output` option automates this preparation.

## Recreate the IDA baseline

Run the exporter with the Python interpreter from IDA's virtual environment:

```powershell
& "$env:APPDATA\Hex-Rays\IDA Pro\venv\Scripts\python.exe" `
  tools/ida_export.py files/eboot_ida.elf --rebuild
```

This creates the ignored persistent database at `analysis/ida/eboot.i64` and
writes reviewable function, segment, entry-point, and source-path exports under
`analysis/exports/`.

## Resolve PS4 imports

Use the matching SDK stubs to translate the executable's compact NIDs back to
their original API names:

```powershell
python tools/resolve_ps4_imports.py
```

The resolver writes `analysis/exports/imports.csv`. It combines the SDK stubs,
the reviewed private-export map, and hashed original names for the bundled FMOD
1.10.04 libraries. The current database contains names for all 706 imports.

## Local PS4 SDK

Place the installed PS4 SDK 5.008 tree at `tools/ps4-sdk/`. That directory is
excluded from Git while the extraction and IDA helper scripts beside it remain
tracked. The headers and stubs are sufficient for import recovery. A future
binary comparison build will also require the missing Orbis compiler and linker
executables.

## Repository policy

Reverse-engineering notes, scripts, recovered declarations, and reconstructed
source belong in Git. Generated executables, IDA databases, and local SDK files
do not.
