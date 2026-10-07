# Binary inventory

## Source artifacts

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `files/eboot.bin` | 29,455,849 bytes | `3ECE8A3779D5EB2003A96C0C4A82639B14A2F2C62E483257B3A0EF4E3569EF8D` |
| `files/eboot.elf` | 29,431,241 bytes | `375479BC8C1CB44373DCF897D770CE56D23BD0D4053A9D420F9912BEF38DA704` |
| `files/eboot_ida.elf` | 29,431,241 bytes | `F3ACBE2EE79AB72D4F664E4FC19D38CB0DF215ABFC194E7EF073F38FF37A1537` |

The two ELF files differ only at offsets `0x10` and `0x11`, where
`eboot_ida.elf` replaces Sony's `ET_SCE_DYNEXEC` value (`0xFE10`) with
`ET_DYN` (`0x0003`) for IDA's generic ELF loader.

## SELF properties

- Magic: `0x1D3D154F`
- Entry count: 10
- All payload entries are unencrypted and uncompressed.
- Embedded ELF offset: `0x160`
- Architecture: AMD64 (`EM_X86_64`)
- ELF OS ABI: FreeBSD
- ELF entry point: `0x920`
- ELF program headers: 11
- ELF section headers: none

## Build metadata

The `PT_SCE_PROCPARAM` segment begins at file offset `0x19B4000`. Its process
parameter structure contains:

| Field | Value |
| --- | --- |
| Structure size | `0x50` |
| Magic | `ORBI` (`0x4942524F`) |
| Entry count | 3 |
| SDK revision word | `0x05008001` = `5.008.001` |

The executable targets the PS4 SDK 5.000 generation. The revision word records
the associated 5.008.001 build revision; it does not mean that the locally
available 5.008 headers and stubs include the matching compiler toolchain.

The `PT_SCE_COMMENT` segment records the original build output path:

```text
D:/TeamCity/buildAgent/work/de1ea97e21c98eba/main/rockband/build/rockband_ps4_s.elf
```

The executable imports standard Orbis services plus FMOD and FMOD Studio. The
initial import set includes audio output, pad and USB input, networking, NP,
save data, trophies, dialogs, PlayGo, video output, video decoding, voice,
remote play, streaming, and system/user services.

The executable contains 706 x86-64 PLT entries. Matching their encoded NIDs
against the local SDK 5.008 stubs recovers 568 exact symbol names. The remaining
138 entries are unresolved: 76 from `libfmod`, 59 from `libfmodstudio`, and
three from `libScePad`. See [imports.md](imports.md) for the method and current
coverage.
