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
    "audio-register-mix-consumer": 0x1128380,
    "dingo-initialize": 0x33F090,
    "stage-presence-id-to-symbol": 0x997590,
    "ui-layout-id-to-symbol": 0xBB06A0,
    "ui-load-layout-by-id": 0xBB5D40,
    "command-line-mark-switches-handled": 0x252BC0,
    "performance-counter-ticks-to-milliseconds": 0x25C0E0,
    "performance-counter-milliseconds-to-ticks": 0x25C110,
    "special-pad-reader-open": 0x8D28A0,
    "special-pad-reader-append-calibration-samples": 0x8D2EA0,
    "special-pad-reader-take-calibration-samples": 0x8D3000,
    "special-pad-reader-set-calibration-mode": 0x8D3060,
    "audio-update-listener-attributes": 0x262300,
    "audio-clip-fmod-start": 0x266FD0,
    "audio-clip-fmod-dsp-set-position": 0x266CB0,
    "audio-clip-fmod-dsp-read": 0x266CD0,
    "audio-clip-fmod-event-callback": 0x267310,
    "audio-clip-fmod-pause": 0x267BB0,
    "audio-clip-fmod-resume": 0x267C00,
    "audio-clip-fmod-get-channel-position-ms": 0x267C60,
    "audio-clip-fmod-get-position-ms": 0x267CC0,
    "audio-clip-fmod-set-position-ms": 0x267D30,
    "audio-clip-fmod-update": 0x267D70,
    "audio-clip-fmod-process-release": 0x267E50,
    "audio-clip-fmod-update-3d-attributes": 0x267E70,
    "audio-clip-fmod-prepare-for-audio-reset": 0x267F10,
    "audio-clip-fmod-defer-channel-release": 0x267FB0,
    "audio-clip-fmod-release-event-instance": 0x268080,
    "audio-clip-fmod-clear-dsp-user-data": 0x2681E0,
    "audio-clip-fmod-release-dsp": 0x268260,
    "audio-clip-fmod-stop-and-wait": 0x2682A0,
    "audio-clip-fmod-set-event-parameter": 0x2683F0,
    "audio-clip-fmod-get-event-parameter": 0x268410,
    "fmod-audio-bus-generator-return-to-pool": 0x268380,
    "fmod-audio-bus-manager-get-setting": 0x268510,
    "fmod-audio-bus-manager-type-symbol": 0x268520,
    "fmod-audio-bus-manager-extension-symbol": 0x2685C0,
    "fmod-audio-bus-manager-retain": 0x268660,
    "fmod-audio-bus-manager-prepare-all-for-audio-reset": 0x2686D0,
    "fmod-audio-bus-manager-stop-all": 0x268740,
    "fmod-audio-bus-manager-active-handles": 0x2687B0,
    "fmod-audio-bus-manager-set-setting": 0x268900,
    "fmod-audio-bus-manager-initialize-pool": 0x268910,
    "fmod-audio-bus-manager-shutdown-pool": 0x268A60,
    "fmod-audio-bus-manager-acquire": 0x268B40,
    "fmod-audio-bus-manager-create-from-resource": 0x268CA0,
    "fmod-audio-bus-manager-create-from-options": 0x268D80,
    "fmod-audio-bus-generator-initialize-sound": 0x268EF0,
    "fmod-audio-bus-generator-register-bus-path": 0x2692E0,
    "fmod-audio-stream-generator-initialize-pool-slot": 0x2692B0,
    "fmod-audio-stream-generator-destruct": 0x2693B0,
    "fmod-audio-stream-generator-delete": 0x269440,
    "fmod-audio-stream-generator-update": 0x2694D0,
    "fmod-audio-stream-generator-try-start-sound": 0x269980,
    "fmod-audio-stream-generator-apply-volume": 0x269B40,
    "fmod-audio-stream-generator-update-3d": 0x269B70,
    "fmod-audio-stream-generator-request-pause": 0x269BF0,
    "fmod-audio-stream-generator-request-resume": 0x269C00,
    "fmod-audio-stream-generator-get-position-ms": 0x269C10,
    "fmod-audio-stream-generator-set-position-ms": 0x269C60,
    "fmod-audio-stream-generator-set-playback-rate": 0x269C70,
    "fmod-audio-stream-generator-clear-loop-points": 0x269CB0,
    "fmod-audio-stream-generator-set-loop-points": 0x269CD0,
    "fmod-audio-stream-generator-prepare-for-audio-reset": 0x269CF0,
    "fmod-audio-stream-generator-stop": 0x269D00,
    "fmod-audio-stream-generator-return-to-pool": 0x269FB0,
    "fmod-audio-stream-manager-create-fallback": 0x26A270,
    "fmod-audio-stream-manager-get-setting": 0x26A280,
    "fmod-audio-stream-manager-type-symbol": 0x26A290,
    "fmod-audio-stream-manager-extension-symbol": 0x26A330,
    "fmod-audio-stream-manager-retain": 0x26A3D0,
    "fmod-audio-stream-manager-prepare-all-for-audio-reset": 0x26A450,
    "fmod-audio-stream-manager-stop-all": 0x26A4C0,
    "fmod-audio-stream-manager-active-handles": 0x26A530,
    "fmod-audio-stream-manager-set-setting": 0x26A680,
    "fmod-audio-stream-manager-initialize-pool": 0x26A690,
    "fmod-audio-stream-manager-shutdown-pool": 0x26A850,
    "fmod-audio-stream-generator-type-symbol": 0x26AA20,
    "fmod-buffered-stream-manager-create-from-resource": 0x26AB60,
    "fmod-buffered-stream-manager-create-from-options": 0x26AC30,
    "fmod-buffered-stream-generator-initialize": 0x26ADA0,
    "fmod-buffered-stream-generator-initialize-pool-slot": 0x26AFF0,
    "fmod-buffered-stream-generator-create-sound": 0x26B060,
    "fmod-buffered-stream-generator-initialize-buffers": 0x26B150,
    "fmod-buffered-stream-generator-acquire-bus-generator": 0x26B490,
    "fmod-buffered-stream-generator-update": 0x26B9A0,
    "fmod-buffered-stream-generator-finish-sound-open": 0x26BAD0,
    "fmod-buffered-stream-generator-set-position-ms": 0x26BDC0,
    "fmod-buffered-stream-generator-return-to-pool": 0x26C1B0,
    "fmod-buffered-stream-generator-refill-buffers": 0x26C480,
    "fmod-buffered-stream-generator-render-callback": 0x26C720,
    "fmod-buffered-stream-generator-compute-sync-adjustment": 0x26D8D0,
    "fmod-buffered-stream-interpolate-six-sample-window": 0x26D940,
    "fmod-buffered-stream-generator-enable-sync": 0x26DAB0,
    "fmod-buffered-stream-generator-set-sync-target-ms": 0x26DBA0,
    "fmod-buffered-stream-generator-disable-sync": 0x26DBD0,
    "fmod-buffered-stream-manager-type-symbol": 0x26DC10,
    "fmod-buffered-stream-manager-extension-symbol": 0x26DCB0,
    "fmod-buffered-stream-manager-retain": 0x26DD50,
    "fmod-buffered-stream-manager-initialize-pool": 0x26E000,
    "fmod-buffered-stream-manager-shutdown-pool": 0x26E270,
    "fmod-buffered-stream-generator-type-symbol": 0x26E3A0,
    "fmod-dialog-generator-initialize-event": 0x26E990,
    "fmod-dialog-programmer-sound-callback": 0x26EA10,
    "fmod-dialog-generator-return-to-pool": 0x26EBE0,
    "fmod-dialog-manager-create": 0x26EC60,
    "fmod-dialog-manager-type-symbol": 0x26EE90,
    "fmod-dialog-manager-extension-symbol": 0x26EF30,
    "fmod-dialog-manager-retain": 0x26EFD0,
    "fmod-dialog-manager-active-handles": 0x26F120,
    "fmod-dialog-manager-initialize-pool": 0x26F280,
    "fmod-dialog-manager-shutdown-pool": 0x26F420,
    "fmod-dialog-generator-pause": 0x26F6E0,
    "fmod-dialog-generator-resume": 0x26F6F0,
    "fmod-dialog-generator-set-event-parameter": 0x26F760,
    "fmod-dialog-generator-get-event-parameter": 0x26F770,
    "fmod-dialog-generator-update": 0x26FAB0,
    "fmod-dialog-generator-stop-and-wait": 0x26FB10,
    "fmod-dialog-generator-type-symbol": 0x26FAC0,
    "fmod-studio-sound-generator-initialize-event": 0x26FC30,
    "fmod-studio-sound-generator-event-callback": 0x26FF50,
    "fmod-studio-sound-generator-update": 0x2702C0,
    "fmod-studio-sound-generator-prepare-for-audio-reset": 0x270620,
    "fmod-studio-sound-generator-stop-and-wait": 0x2706A0,
    "fmod-studio-sound-generator-set-event-parameter": 0x2707D0,
    "fmod-studio-sound-generator-get-event-parameter": 0x270800,
    "fmod-studio-sound-generator-configure-fade": 0x270880,
    "fmod-studio-sound-generator-configure-volume-transition": 0x2709B0,
    "fmod-studio-sound-generator-type-symbol": 0x2715B0,
    "audio-build-fmod-3d-attributes": 0x27ACB0,
    "fmod-buffered-output-get-num-drivers": 0x2763F0,
    "fmod-buffered-output-get-driver-info": 0x276400,
    "fmod-buffered-output-initialize": 0x276480,
    "fmod-buffered-output-close": 0x2764B0,
    "fmod-buffered-output-update": 0x2764C0,
    "fmod-buffered-output-get-handle": 0x276520,
    "fmod-load-modules-and-set-thread-affinity": 0x261F60,
    "audio-consume-all-timing-reports": 0x262B80,
    "fmod-audio-state-initialize": 0x276F30,
    "fmod-audio-initialize": 0x2773C0,
    "fmod-audio-attach-studio-system": 0x277840,
    "fmod-audio-detach-studio-system": 0x277A80,
    "fmod-audio-consume-timing-report": 0x277BA0,
    "fmod-audio-dispatch-mix-buffers": 0x2781C0,
    "fmod-register-custom-dsp-plugins": 0x278270,
    "fmod-audio-configure-speakers": 0x2783A0,
    "fmod-system-callback": 0x2783E0,
    "fmod-audio-initialize-custom-output": 0x2786D0,
    "audio-timing-accumulator-finish": 0x278880,
    "audio-rolling-timing-accumulator-finish": 0x278910,
    "audio-timing-average-buffer-percent": 0x278C80,
    "audio-timing-maximum-buffer-percent": 0x278CB0,
    "audio-rolling-timing-average-buffer-percent": 0x278D10,
    "audio-rolling-timing-maximum-buffer-percent": 0x278D50,
    "fmod-defer-channel-dsp-release": 0x278A00,
    "fmod-process-deferred-releases": 0x278DF0,
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
