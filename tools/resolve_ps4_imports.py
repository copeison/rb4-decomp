#!/usr/bin/env python3
"""Resolve PS4 NID imports with the matching SDK stub libraries.

The Orbis linker stores imported names as compact 11-character NIDs.  SDK stub
libraries retain both the original symbol and its binary NID, which lets this
tool recover names without relying on an online symbol database.
"""

from __future__ import annotations

import argparse
import base64
import csv
import struct
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


PT_DYNAMIC = 2
PT_LOAD = 1
PT_SCE_DYNLIBDATA = 0x61000000

DT_NULL = 0
DT_SCE_JMPREL = 0x61000029
DT_SCE_PLTRELSZ = 0x6100002D
DT_SCE_STRTAB = 0x61000035
DT_SCE_STRSZ = 0x61000037
DT_SCE_SYMTAB = 0x61000039
DT_SCE_SYMENT = 0x6100003B
DT_SCE_SYMTABSZ = 0x6100003F

ELF_HEADER = struct.Struct("<16sHHIQQQIHHHHHH")
PROGRAM_HEADER = struct.Struct("<IIQQQQQQ")
SECTION_HEADER = struct.Struct("<IIQQQQIIQQ")
SYMBOL = struct.Struct("<IBBHQQ")
RELOCATION = struct.Struct("<QQq")


@dataclass(frozen=True)
class ProgramHeader:
    type: int
    flags: int
    offset: int
    virtual_address: int
    file_size: int


@dataclass(frozen=True)
class ElfLayout:
    data: bytes
    program_headers: tuple[ProgramHeader, ...]
    section_headers: tuple[tuple[int, ...], ...]
    section_names: tuple[str, ...]


def read_c_string(data: bytes, offset: int) -> str:
    if not 0 <= offset < len(data):
        return ""
    end = data.find(b"\0", offset)
    if end < 0:
        end = len(data)
    return data[offset:end].decode("utf-8", errors="replace")


def parse_elf(path: Path) -> ElfLayout:
    data = path.read_bytes()
    if len(data) < ELF_HEADER.size:
        raise ValueError(f"{path} is too small to be an ELF file")

    header = ELF_HEADER.unpack_from(data)
    ident = header[0]
    if ident[:4] != b"\x7fELF" or ident[4] != 2 or ident[5] != 1:
        raise ValueError(f"{path} is not a little-endian ELF64 file")

    program_offset = header[5]
    section_offset = header[6]
    program_entry_size = header[9]
    program_count = header[10]
    section_entry_size = header[11]
    section_count = header[12]
    section_names_index = header[13]

    if program_count and program_entry_size < PROGRAM_HEADER.size:
        raise ValueError(f"{path} has an invalid program-header size")
    if section_count and section_entry_size < SECTION_HEADER.size:
        raise ValueError(f"{path} has an invalid section-header size")

    program_headers = []
    for index in range(program_count):
        values = PROGRAM_HEADER.unpack_from(
            data, program_offset + index * program_entry_size
        )
        program_headers.append(
            ProgramHeader(
                type=values[0],
                flags=values[1],
                offset=values[2],
                virtual_address=values[3],
                file_size=values[5],
            )
        )

    section_headers = tuple(
        SECTION_HEADER.unpack_from(data, section_offset + index * section_entry_size)
        for index in range(section_count)
    )
    section_names: tuple[str, ...] = ()
    if section_headers and section_names_index < len(section_headers):
        names_header = section_headers[section_names_index]
        names = data[names_header[4] : names_header[4] + names_header[5]]
        section_names = tuple(read_c_string(names, header[0]) for header in section_headers)

    return ElfLayout(
        data=data,
        program_headers=tuple(program_headers),
        section_headers=section_headers,
        section_names=section_names,
    )


def encode_nid(raw_nid: bytes) -> str:
    """Convert an SDK .scenid value to the executable's printable NID."""
    encoded = base64.b64encode(raw_nid[::-1]).decode("ascii").rstrip("=")
    return encoded.replace("/", "-")


def iter_sdk_symbols(stub_path: Path) -> Iterable[tuple[str, str]]:
    elf = parse_elf(stub_path)
    sections = {
        name: (index, elf.section_headers[index])
        for index, name in enumerate(elf.section_names)
    }
    if ".dynsym" not in sections or ".scenid" not in sections:
        return

    _, symbol_header = sections[".dynsym"]
    _, nid_header = sections[".scenid"]
    string_table_index = symbol_header[6]
    if string_table_index >= len(elf.section_headers):
        return
    string_header = elf.section_headers[string_table_index]

    symbol_size = symbol_header[9] or SYMBOL.size
    symbol_count = symbol_header[5] // symbol_size
    strings = elf.data[string_header[4] : string_header[4] + string_header[5]]
    nids = elf.data[nid_header[4] : nid_header[4] + nid_header[5]]

    for index in range(symbol_count):
        symbol_offset = symbol_header[4] + index * symbol_size
        name_offset = SYMBOL.unpack_from(elf.data, symbol_offset)[0]
        nid_offset = index * 8
        if not name_offset or nid_offset + 8 > len(nids):
            continue
        name = read_c_string(strings, name_offset)
        if name:
            yield encode_nid(nids[nid_offset : nid_offset + 8]), name


def load_sdk_nids(
    sdk_root: Path,
) -> dict[str, dict[str, set[str]]]:
    matches: dict[str, dict[str, set[str]]] = defaultdict(
        lambda: defaultdict(set)
    )
    library_root = sdk_root / "target" / "lib"
    if not library_root.is_dir():
        raise FileNotFoundError(f"SDK library directory not found: {library_root}")

    for stub_path in sorted(library_root.rglob("*.a")):
        try:
            symbols = iter_sdk_symbols(stub_path)
            for nid, name in symbols:
                matches[nid][name].add(stub_path.relative_to(sdk_root).as_posix())
        except (OSError, struct.error, ValueError):
            # A few SDK archives may use the regular ar format rather than the
            # single-object Orbis stub format. They cannot supply .scenid here.
            continue
    return matches


def find_program_header(elf: ElfLayout, header_type: int) -> ProgramHeader:
    for header in elf.program_headers:
        if header.type == header_type:
            return header
    raise ValueError(f"ELF program header 0x{header_type:X} was not found")


def read_dynamic_tags(elf: ElfLayout) -> dict[int, int]:
    dynamic = find_program_header(elf, PT_DYNAMIC)
    tags: dict[int, int] = {}
    for offset in range(dynamic.offset, dynamic.offset + dynamic.file_size, 16):
        tag, value = struct.unpack_from("<QQ", elf.data, offset)
        if tag == DT_NULL:
            break
        tags[tag] = value
    return tags


def require_tags(tags: dict[int, int], required: Iterable[int]) -> None:
    missing = [tag for tag in required if tag not in tags]
    if missing:
        formatted = ", ".join(f"0x{tag:X}" for tag in missing)
        raise ValueError(f"required Orbis dynamic tags are missing: {formatted}")


def find_plt_address(elf: ElfLayout, relocation_count: int) -> int:
    """Locate the x86-64 PLT run by its sequential pushed indices."""
    run_size = relocation_count * 16
    for segment in elf.program_headers:
        if segment.type != PT_LOAD or not segment.flags & 1:
            continue
        start = segment.offset
        end = min(len(elf.data), segment.offset + segment.file_size)
        for offset in range(start, max(start, end - run_size + 1)):
            if elf.data[offset : offset + 2] != b"\xFF\x25":
                continue
            if elf.data[offset + 6] != 0x68 or elf.data[offset + 11] != 0xE9:
                continue
            if struct.unpack_from("<I", elf.data, offset + 7)[0] != 0:
                continue

            valid = True
            for index in range(relocation_count):
                entry = offset + index * 16
                if (
                    elf.data[entry : entry + 2] != b"\xFF\x25"
                    or elf.data[entry + 6] != 0x68
                    or struct.unpack_from("<I", elf.data, entry + 7)[0] != index
                    or elf.data[entry + 11] != 0xE9
                ):
                    valid = False
                    break
            if valid:
                return segment.virtual_address + offset - segment.offset
    raise ValueError("could not locate the executable's PLT stub sequence")


def resolve_imports(elf_path: Path, sdk_root: Path) -> list[dict[str, object]]:
    elf = parse_elf(elf_path)
    dynlib = find_program_header(elf, PT_SCE_DYNLIBDATA)
    tags = read_dynamic_tags(elf)
    require_tags(
        tags,
        (
            DT_SCE_JMPREL,
            DT_SCE_PLTRELSZ,
            DT_SCE_STRTAB,
            DT_SCE_STRSZ,
            DT_SCE_SYMTAB,
            DT_SCE_SYMENT,
            DT_SCE_SYMTABSZ,
        ),
    )

    string_offset = dynlib.offset + tags[DT_SCE_STRTAB]
    strings = elf.data[string_offset : string_offset + tags[DT_SCE_STRSZ]]
    symbol_offset = dynlib.offset + tags[DT_SCE_SYMTAB]
    symbol_size = tags[DT_SCE_SYMENT]
    symbol_count = tags[DT_SCE_SYMTABSZ] // symbol_size
    symbols: list[str] = []
    for index in range(symbol_count):
        offset = symbol_offset + index * symbol_size
        name_offset = SYMBOL.unpack_from(elf.data, offset)[0]
        symbols.append(read_c_string(strings, name_offset))

    relocation_offset = dynlib.offset + tags[DT_SCE_JMPREL]
    relocation_count = tags[DT_SCE_PLTRELSZ] // RELOCATION.size
    plt_address = find_plt_address(elf, relocation_count)
    sdk_nids = load_sdk_nids(sdk_root)

    rows: list[dict[str, object]] = []
    for index in range(relocation_count):
        offset = relocation_offset + index * RELOCATION.size
        got_address, info, _addend = RELOCATION.unpack_from(elf.data, offset)
        symbol_index = info >> 32
        encoded_symbol = symbols[symbol_index] if symbol_index < len(symbols) else ""
        parts = encoded_symbol.split("#")
        nid = parts[0] if parts else ""
        library_id = parts[1] if len(parts) > 1 else ""
        module_id = parts[2] if len(parts) > 2 else ""

        candidates = sdk_nids.get(nid, {})
        names = sorted(candidates)
        if len(names) == 1:
            resolved_name = names[0]
            libraries = sorted(candidates[resolved_name])
            status = "resolved"
        elif names:
            resolved_name = " | ".join(names)
            libraries = sorted(
                {library for candidate in candidates.values() for library in candidate}
            )
            status = "ambiguous"
        else:
            resolved_name = ""
            libraries = []
            status = "unresolved"

        rows.append(
            {
                "plt_index": index,
                "stub_address": f"0x{plt_address + index * 16:X}",
                "got_address": f"0x{got_address:X}",
                "symbol_index": symbol_index,
                "encoded_symbol": encoded_symbol,
                "nid": nid,
                "library_id": library_id,
                "module_id": module_id,
                "resolved_name": resolved_name,
                "stub_libraries": ";".join(libraries),
                "status": status,
            }
        )
    return rows


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = list(rows[0]) if rows else []
    with path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "elf", nargs="?", type=Path, default=Path("files/eboot.elf")
    )
    parser.add_argument(
        "--sdk", type=Path, default=Path("tools/ps4-sdk"), help="PS4 SDK root"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("analysis/exports/imports.csv"),
        help="resolved import CSV",
    )
    args = parser.parse_args()

    rows = resolve_imports(args.elf, args.sdk)
    write_csv(args.output, rows)
    resolved = sum(row["status"] == "resolved" for row in rows)
    ambiguous = sum(row["status"] == "ambiguous" for row in rows)
    unresolved = len(rows) - resolved - ambiguous
    print(f"PLT imports: {len(rows)}")
    print(f"Resolved:    {resolved}")
    print(f"Ambiguous:   {ambiguous}")
    print(f"Unresolved:  {unresolved}")
    print(f"Output:      {args.output}")


if __name__ == "__main__":
    main()
