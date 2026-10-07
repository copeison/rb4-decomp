#!/usr/bin/env python3
"""Build an IDA database and export a reproducible initial analysis snapshot."""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Any, Iterable

from ida_domain import Database
from ida_domain.database import IdaCommandOptions


SOURCE_PATH_RE = re.compile(
    r"(?:[A-Za-z]:[\\/]|/)[^\x00\r\n]{3,240}?"
    r"\.(?:cpp|cxx|cc|c|hpp|hh|h|inl|asm|s|elf)",
    re.IGNORECASE,
)

FOCUSED_DECOMPILATIONS = {
    "game-initialize": 0xA0,
    "game-run-frame": 0x190,
    "game-main": 0x3C0,
    "audio-get-sample-rate": 0xD3BA0,
    "audio-set-mix-format": 0xD3BC0,
    "audio-dispatch-output-blocks": 0x1127880,
    "audio-output-dispatcher-set-sample-rate": 0x1127B90,
    "dingo-initialize": 0x33F090,
    "stage-presence-id-to-symbol": 0x997590,
    "ui-layout-id-to-symbol": 0xBB06A0,
    "ui-load-layout-by-id": 0xBB5D40,
    "command-line-mark-switches-handled": 0x252BC0,
    "special-pad-reader-open": 0x8D28A0,
    "special-pad-reader-append-calibration-samples": 0x8D2EA0,
    "special-pad-reader-take-calibration-samples": 0x8D3000,
    "special-pad-reader-set-calibration-mode": 0x8D3060,
    "audio-update-listener-attributes": 0x262300,
    "audio-build-fmod-3d-attributes": 0x27ACB0,
    "fmod-buffered-output-get-num-drivers": 0x2763F0,
    "fmod-buffered-output-get-driver-info": 0x276400,
    "fmod-buffered-output-initialize": 0x276480,
    "fmod-buffered-output-close": 0x2764B0,
    "fmod-buffered-output-update": 0x2764C0,
    "fmod-buffered-output-get-handle": 0x276520,
    "fmod-load-modules-and-set-thread-affinity": 0x261F60,
    "fmod-audio-state-initialize": 0x276F30,
    "fmod-audio-initialize": 0x2773C0,
    "fmod-audio-attach-studio-system": 0x277840,
    "fmod-audio-detach-studio-system": 0x277A80,
    "fmod-audio-dispatch-mix-buffers": 0x2781C0,
    "fmod-register-custom-dsp-plugins": 0x278270,
    "fmod-system-callback": 0x2783E0,
    "fmod-audio-initialize-custom-output": 0x2786D0,
    "audio-timing-accumulator-finish": 0x278880,
    "audio-rolling-timing-accumulator-finish": 0x278910,
    "fmod-file-handle-open": 0x279FE0,
    "fmod-async-file-reader-initialize": 0x27A1C0,
    "fmod-async-file-reader-thread": 0x27A270,
    "fmod-async-file-reader-shutdown": 0x27A460,
    "fmod-file-seek": 0x27A4D0,
    "fmod-file-read": 0x27A530,
    "fmod-file-open": 0x27A5A0,
    "fmod-file-close": 0x27A6D0,
    "fmod-file-async-read": 0x27A780,
    "fmod-file-async-cancel": 0x27A850,
}


def write_csv(path: Path, fieldnames: list[str], rows: Iterable[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def format_permissions(permissions: int) -> str:
    return "".join(
        (
            "r" if permissions & 4 else "-",
            "w" if permissions & 2 else "-",
            "x" if permissions & 1 else "-",
        )
    )


def collect_analysis(
    db: Database,
    database_path: Path,
    export_dir: Path,
    docs_path: Path,
) -> dict[str, Any]:
    import ida_auto
    import ida_entry
    import ida_funcs
    import ida_hexrays
    import ida_name
    import ida_nalt
    import ida_segment
    import idautils
    import idc

    ida_auto.auto_wait()

    segment_rows: list[dict[str, Any]] = []
    for start in idautils.Segments():
        segment = ida_segment.getseg(start)
        if segment is None:
            continue
        segment_rows.append(
            {
                "name": ida_segment.get_segm_name(segment),
                "class": ida_segment.get_segm_class(segment),
                "start": f"0x{segment.start_ea:X}",
                "end": f"0x{segment.end_ea:X}",
                "size": segment.end_ea - segment.start_ea,
                "permissions": format_permissions(segment.perm),
            }
        )

    function_rows: list[dict[str, Any]] = []
    for start in idautils.Functions():
        function = ida_funcs.get_func(start)
        if function is None:
            continue
        name = ida_name.get_name(start) or f"sub_{start:X}"
        function_rows.append(
            {
                "address": f"0x{start:X}",
                "end": f"0x{function.end_ea:X}",
                "size": function.end_ea - start,
                "name": name,
                "library": bool(function.flags & ida_funcs.FUNC_LIB),
                "thunk": bool(function.flags & ida_funcs.FUNC_THUNK),
            }
        )

    entry_rows: list[dict[str, Any]] = []
    for index in range(ida_entry.get_entry_qty()):
        ordinal = ida_entry.get_entry_ordinal(index)
        address = ida_entry.get_entry(ordinal)
        entry_rows.append(
            {
                "ordinal": ordinal,
                "address": f"0x{address:X}",
                "name": ida_entry.get_entry_name(ordinal) or ida_name.get_name(address),
            }
        )

    strings = list(idautils.Strings())
    source_paths: set[str] = set()
    for item in strings:
        value = str(item)
        source_paths.update(match.group(0) for match in SOURCE_PATH_RE.finditer(value))

    write_csv(
        export_dir / "segments.csv",
        ["name", "class", "start", "end", "size", "permissions"],
        segment_rows,
    )
    write_csv(
        export_dir / "functions.csv",
        ["address", "end", "size", "name", "library", "thunk"],
        function_rows,
    )
    write_csv(
        export_dir / "entries.csv",
        ["ordinal", "address", "name"],
        entry_rows,
    )
    (export_dir / "source_paths.txt").write_text(
        "".join(f"{path}\n" for path in sorted(source_paths, key=str.lower)),
        encoding="utf-8",
    )

    entry_address = entry_rows[0]["address"] if entry_rows else "0x920"
    entry_ea = int(entry_address, 16)
    entry_function = ida_funcs.get_func(entry_ea)
    if entry_function is None:
        ida_funcs.add_func(entry_ea)
        ida_auto.auto_wait()
        entry_function = ida_funcs.get_func(entry_ea)

    disassembly: list[str] = []
    if entry_function is not None:
        for address in idautils.FuncItems(entry_function.start_ea):
            line = idc.generate_disasm_line(address, 0) or ""
            disassembly.append(f"{address:016X}: {line}")
    (export_dir / "entrypoint.asm").write_text(
        "\n".join(disassembly) + ("\n" if disassembly else ""),
        encoding="utf-8",
    )

    decompiler_available = bool(ida_hexrays.init_hexrays_plugin())
    pseudocode = ""
    if decompiler_available and entry_function is not None:
        try:
            pseudocode = str(ida_hexrays.decompile(entry_function.start_ea))
        except Exception as exc:  # IDA raises several version-specific types.
            pseudocode = f"/* Entry-point decompilation failed: {exc} */\n"
    (export_dir / "entrypoint.c").write_text(pseudocode, encoding="utf-8")

    if decompiler_available:
        for output_name, address in FOCUSED_DECOMPILATIONS.items():
            function = ida_funcs.get_func(address)
            if function is None:
                continue
            try:
                focused_pseudocode = str(ida_hexrays.decompile(function.start_ea))
            except Exception as exc:  # IDA exposes version-specific exception types.
                focused_pseudocode = f"/* Decompilation failed: {exc} */\n"
            (export_dir / f"{output_name}.c").write_text(
                focused_pseudocode, encoding="utf-8"
            )

    named_functions = sum(
        1
        for row in function_rows
        if not str(row["name"]).startswith(("sub_", "loc_", "nullsub_"))
    )
    largest_functions = sorted(
        function_rows, key=lambda row: int(row["size"]), reverse=True
    )[:20]
    summary = {
        "input_path": ida_nalt.get_input_file_path(),
        # Database.path reports the loader input while creating a new IDB.
        # Record the requested persistent output path instead.
        "database_path": str(database_path),
        "image_base": f"0x{ida_nalt.get_imagebase():X}",
        "minimum_address": f"0x{db.minimum_ea:X}",
        "maximum_address": f"0x{db.maximum_ea:X}",
        "segment_count": len(segment_rows),
        "function_count": len(function_rows),
        "named_function_count": named_functions,
        "entry_count": len(entry_rows),
        "entry_address": entry_address,
        "string_count": len(strings),
        "source_path_count": len(source_paths),
        "decompiler_available": decompiler_available,
        "largest_functions": largest_functions,
    }
    (export_dir / "summary.json").write_text(
        json.dumps(summary, indent=2) + "\n", encoding="utf-8"
    )

    largest_lines = "\n".join(
        f"| `{row['address']}` | `{row['name']}` | {row['size']:,} |"
        for row in largest_functions[:10]
    )
    docs_path.parent.mkdir(parents=True, exist_ok=True)
    docs_path.write_text(
        f"""# IDA baseline

This snapshot was generated by `tools/ida_export.py` from the IDA-compatible
copy of the extracted executable.

## Analysis totals

| Metric | Value |
| --- | ---: |
| Image base | `{summary['image_base']}` |
| Address range | `{summary['minimum_address']}` to `{summary['maximum_address']}` |
| Segments | {summary['segment_count']:,} |
| Functions | {summary['function_count']:,} |
| Named functions | {summary['named_function_count']:,} |
| IDA strings | {summary['string_count']:,} |
| Recovered source paths | {summary['source_path_count']:,} |
| Hex-Rays available | {str(summary['decompiler_available']).lower()} |

## Largest discovered functions

| Address | Current name | Size (bytes) |
| --- | --- | ---: |
{largest_lines}

The complete machine-readable exports are under `analysis/exports/`. These are
the baseline for later naming and source reconstruction passes.
""",
        encoding="utf-8",
    )
    return summary


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="IDA-compatible input ELF")
    parser.add_argument(
        "--database",
        type=Path,
        default=Path("analysis/ida/eboot.i64"),
        help="persistent IDA database path",
    )
    parser.add_argument(
        "--exports",
        type=Path,
        default=Path("analysis/exports"),
        help="directory for tracked analysis exports",
    )
    parser.add_argument(
        "--docs",
        type=Path,
        default=Path("docs/ida-baseline.md"),
        help="generated Markdown summary",
    )
    parser.add_argument(
        "--rebuild", action="store_true", help="rebuild the IDA database from the ELF"
    )
    args = parser.parse_args()

    input_path = args.input.resolve()
    database_path = args.database.resolve()
    export_dir = args.exports.resolve()
    docs_path = args.docs.resolve()
    database_path.parent.mkdir(parents=True, exist_ok=True)
    export_dir.mkdir(parents=True, exist_ok=True)

    if database_path.exists() and not args.rebuild:
        open_path = database_path
        options = IdaCommandOptions(auto_analysis=True)
    else:
        open_path = input_path
        options = IdaCommandOptions(
            auto_analysis=True,
            new_database=True,
            output_database=str(database_path),
            log_file=str(database_path.with_suffix(".log")),
        )

    with Database.open(str(open_path), options, save_on_close=True) as database:
        summary = collect_analysis(database, database_path, export_dir, docs_path)

    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
