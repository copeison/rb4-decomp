#!/usr/bin/env python3
"""Extract an unencrypted, uncompressed ELF from a PS4 SELF container."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


PS4_SELF_MAGIC = b"\x4f\x15\x3d\x1d"
ELF_MAGIC = b"\x7fELF"
ET_DYN = 0x0003
ET_SCE_DYNEXEC = 0xFE10
SELF_HEADER_SIZE = 0x20
SELF_ENTRY_SIZE = 0x20
ELF64_HEADER = struct.Struct("<16sHHIQQQIHHHHHH")
ELF64_PROGRAM_HEADER = struct.Struct("<IIQQQQQQ")


@dataclass(frozen=True)
class SelfEntry:
    index: int
    properties: int
    offset: int
    file_size: int
    memory_size: int

    @property
    def encrypted(self) -> bool:
        return bool(self.properties & (1 << 1))

    @property
    def compressed(self) -> bool:
        return bool(self.properties & (1 << 3))


@dataclass(frozen=True)
class ProgramHeader:
    index: int
    kind: int
    flags: int
    offset: int
    virtual_address: int
    physical_address: int
    file_size: int
    memory_size: int
    alignment: int


def checked_slice(data: bytes, offset: int, size: int, label: str) -> bytes:
    end = offset + size
    if offset < 0 or size < 0 or end > len(data):
        raise ValueError(
            f"{label} lies outside the input: offset=0x{offset:X}, "
            f"size=0x{size:X}, input=0x{len(data):X}"
        )
    return data[offset:end]


def parse_self(data: bytes) -> tuple[int, list[SelfEntry]]:
    if len(data) < SELF_HEADER_SIZE or data[:4] != PS4_SELF_MAGIC:
        raise ValueError("input is not a PS4 SELF")

    entry_count = struct.unpack_from("<H", data, 0x18)[0]
    elf_offset = SELF_HEADER_SIZE + entry_count * SELF_ENTRY_SIZE
    checked_slice(data, elf_offset, ELF64_HEADER.size, "embedded ELF header")

    entries: list[SelfEntry] = []
    for index in range(entry_count):
        offset = SELF_HEADER_SIZE + index * SELF_ENTRY_SIZE
        properties, file_offset, file_size, memory_size = struct.unpack_from(
            "<QQQQ", data, offset
        )
        checked_slice(data, file_offset, file_size, f"SELF entry {index}")
        entries.append(
            SelfEntry(index, properties, file_offset, file_size, memory_size)
        )

    unsupported = [
        entry.index for entry in entries if entry.encrypted or entry.compressed
    ]
    if unsupported:
        joined = ", ".join(str(index) for index in unsupported)
        raise ValueError(
            "SELF contains encrypted or compressed entries that this extractor "
            f"cannot process: {joined}"
        )

    return elf_offset, entries


def parse_elf(
    data: bytes, elf_offset: int
) -> tuple[int, int, list[ProgramHeader]]:
    fields = ELF64_HEADER.unpack_from(data, elf_offset)
    ident = fields[0]
    if ident[:4] != ELF_MAGIC or ident[4] != 2 or ident[5] != 1:
        raise ValueError("embedded image is not a little-endian ELF64 file")

    elf_header_size = fields[8]
    program_header_offset = fields[5]
    program_header_size = fields[9]
    program_header_count = fields[10]
    if program_header_size < ELF64_PROGRAM_HEADER.size:
        raise ValueError("ELF program-header entries are too small")

    table_size = program_header_size * program_header_count
    checked_slice(
        data,
        elf_offset + program_header_offset,
        table_size,
        "ELF program-header table",
    )

    headers: list[ProgramHeader] = []
    for index in range(program_header_count):
        offset = elf_offset + program_header_offset + index * program_header_size
        fields = ELF64_PROGRAM_HEADER.unpack_from(data, offset)
        headers.append(ProgramHeader(index, *fields))

    header_bytes = max(elf_header_size, program_header_offset + table_size)
    return header_bytes, program_header_offset + table_size, headers


def extract(input_path: Path, output_path: Path) -> None:
    data = input_path.read_bytes()
    elf_offset, self_entries = parse_self(data)
    header_bytes, program_table_end, program_headers = parse_elf(data, elf_offset)

    output_size = max(
        [header_bytes]
        + [header.offset + header.file_size for header in program_headers]
    )
    output = bytearray(output_size)
    output[:program_table_end] = checked_slice(
        data, elf_offset, program_table_end, "ELF headers"
    )

    chunks: list[tuple[str, bytes]] = [
        (
            f"SELF entry {entry.index}",
            checked_slice(
                data, entry.offset, entry.file_size, f"SELF entry {entry.index}"
            ),
        )
        for entry in self_entries
    ]

    final_entry_end = max(
        (entry.offset + entry.file_size for entry in self_entries),
        default=elf_offset + program_table_end,
    )
    if final_entry_end < len(data):
        chunks.append(("SELF trailing data", data[final_entry_end:]))

    used_chunks: set[int] = set()
    copied_headers: list[int] = []
    for header in program_headers:
        if header.file_size == 0:
            continue
        match = next(
            (
                index
                for index, (_, chunk) in enumerate(chunks)
                if index not in used_chunks and len(chunk) == header.file_size
            ),
            None,
        )
        if match is None:
            continue

        _, chunk = chunks[match]
        start = header.offset
        output[start : start + header.file_size] = chunk
        used_chunks.add(match)
        copied_headers.append(header.index)

    required = [
        header.index
        for header in program_headers
        if header.file_size
        and not any(
            other.index in copied_headers
            and header.offset >= other.offset
            and header.offset + header.file_size
            <= other.offset + other.file_size
            for other in program_headers
        )
    ]
    missing = [index for index in required if index not in copied_headers]
    if missing:
        joined = ", ".join(str(index) for index in missing)
        raise ValueError(f"no SELF payload matched required ELF segments: {joined}")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(output)
    print(
        f"Extracted {input_path} -> {output_path} "
        f"({len(output)} bytes; copied program headers "
        f"{', '.join(map(str, copied_headers))})"
    )


def prepare_ida_copy(input_path: Path, output_path: Path) -> None:
    data = bytearray(input_path.read_bytes())
    if len(data) < ELF64_HEADER.size or data[:4] != ELF_MAGIC:
        raise ValueError("IDA input is not an ELF file")

    elf_type = struct.unpack_from("<H", data, 0x10)[0]
    if elf_type != ET_SCE_DYNEXEC:
        raise ValueError(
            f"expected ET_SCE_DYNEXEC (0x{ET_SCE_DYNEXEC:04X}), "
            f"found 0x{elf_type:04X}"
        )

    struct.pack_into("<H", data, 0x10, ET_DYN)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(data)
    print(
        f"Prepared IDA copy {input_path} -> {output_path} "
        f"(e_type 0x{ET_SCE_DYNEXEC:04X} -> 0x{ET_DYN:04X})"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Extract an unencrypted/uncompressed ELF from a PS4 SELF"
    )
    parser.add_argument("input", type=Path, help="input PS4 SELF (usually eboot.bin)")
    parser.add_argument(
        "output",
        nargs="?",
        type=Path,
        help="output ELF path (defaults to the input path with .elf suffix)",
    )
    parser.add_argument(
        "--ida-output",
        type=Path,
        help="also create an IDA-compatible ELF with e_type normalized to ET_DYN",
    )
    args = parser.parse_args()
    output = args.output or args.input.with_suffix(".elf")
    extract(args.input, output)
    if args.ida_output:
        prepare_ida_copy(output, args.ida_output)


if __name__ == "__main__":
    main()
