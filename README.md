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

## Repository policy

Reverse-engineering notes, scripts, recovered declarations, and reconstructed
source belong in Git. Generated executables, IDA databases, and local SDK files
do not.
