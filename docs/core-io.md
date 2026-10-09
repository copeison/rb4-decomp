# Engine file and stream I/O

The engine's binary I/O is split across the original modules:

- `src/os/files` holds `File` and `StreamChecksum`.
- `src/utl/streams` holds `BinStream` and `FileStream`.
- `src/math` holds the `Rand2` cipher and the `CSHA1` context.

Names follow the reference map; see [naming.md](naming.md).

## File

`File` is the platform file interface. The map's `File*` wrappers each
forward to one virtual slot. They take the file as `void*`, as the map does.

| Address | Wrapper | Slot |
| ---: | --- | ---: |
| `0x378940` | `FileOpen` | forwards to `File::NewFile` (`0x376D40`) |
| `0x378950` | `FileFail` | `Fail` (`+0x58`); null files fail |
| `0x378960` | `FileClose` | deleting destructor (`+0x08`); null-safe |
| `0x378A10` | `FileRead` | `Read` (`+0x18`) |
| `0x378A40` | `FileWrite` | `Write` (`+0x28`) |
| `0x378A50` | `FileSeek` | `Seek` (`+0x38`) |
| `0x378A60` | `FileTell` | `Tell` (`+0x48`) |
| `0x378A70` | `FileFlush` | `Flush` (`+0x40`) |
| `0x378A80` | `FileEof` | `Eof` (`+0x50`) |
| `0x378A90` | `FileSize` | `Size` (`+0x60`) |

`File::NewFile` (`0x376D40`) has not been reconstructed. FMOD's file callbacks
also call an inline prepare-for-reading slot (`+0x88`), which is declared
beside them.

## BinStream

`BinStream` is the 40-byte common base. Its fields are:

- the vtable pointer;
- two `-1` words and a byte flag (`+0x08`, `+0x0C`, `+0x10`);
- the byte-swap flag `mLittleEndian` at `+0x14`;
- the owned `Rand2` cipher `mCrypto` at `+0x18`;
- the platform value at `+0x20`.

The base vtable (`0x18EF010`) has 15 slots. Flush, tell, eof, fail and the
read, write and seek implementations are pure.

- `Read` (`0x21A280`) calls `Fail` and ignores the result. It then calls
  `ReadImpl`, and when a cipher is attached it XORs every byte with the next
  `Rand2::Int` value.
- `ReadEndian` (`0x21ABF0`) adds an in-place 2-, 4- or 8-byte swap.
- `WriteEndian` (`0x21ACB0`) stages swapped values in an eight-byte temporary.
  Encrypted writes go through a 512-byte chunk buffer.
- `operator>>(Symbol&)` (`0x21A300`) reads a length-prefixed symbol, staged on
  the stack or in a named heap block.
- `ReadAsync` (slot 9, `0x21AB60`) reads, then returns the size, or zero after
  a failure.
- `PatchSize` (slot 11, `0x219C30`) patches a 64-bit size prefix at a saved
  position. The map's nearest name is `WriteSkipMark`.
- `Rand2::Int` (`0x117B0E0`) is the minimal-standard Park-Miller generator,
  using Schrage's decomposition.

## FileStream

`FileStream` is 592 bytes. Its fields are:

- the file handle at `+0x28`;
- a 512-byte `strncpy` name at `+0x30`;
- a fail flag at `+0x230`;
- the size at `+0x238`;
- an optional checksum at `+0x240`, with a running byte count at `+0x248`.

Construction (`0x2443A0`) uses platform value 3 and records the open failure.
It queries the size only after a successful open.

Destruction (`0x2444A0`) closes only named files that opened successfully,
then deletes the checksum. The checksum is deleted through its class
`operator delete` (`MemFree`). Its members are destroyed in this order:

1. The `String` at `+0xD8`.
2. The `CSHA1` at `+0x08` (`~CSHA1`, `0x117B560`).

A short read sets the fail flag, and a successful read updates the checksum
(`StreamChecksum::Update`, `0x367C50`). Short writes and negative seek results
also set the fail flag.

## Memory

`os/memory/MemMgr.h` declares the tracked heap:

- `MemAlloc` (`0x37AE70`) and `MemFree` (`0x37B800`).
- `MemOrPoolAlloc` and `MemOrPoolFree` (`0x37C020`, `0x37C040`), which send
  requests of 128 bytes or less to the small-block pool.
- The thread-local temporary-heap scope `MemPushTemp` and `MemPopTemp`
  (`0x37AA30`, `0x37AAF0`).

The global `operator new`/`new[]` (`0x37BF40`, `0x37BF60`) forward to
`MemAlloc`, and `operator delete`/`delete[]` jump to `MemFree`. Classes whose
deleting destructors call `MemFree` directly use Milo's `DELETE_OVERLOAD`.

The EASTL allocator `HmxAllocator::allocator` (`0x252CF0`, `0x252D30`)
ignores its object and labels allocations `"StlAlloc"`.

## Timer

`Hmx::Timer` (`utl/Timer.o`) converts cycle-counter ticks:
- `Init` (`0x25C0A0`) stores 1000 divided by `sceKernelGetTscFrequency()` as
  milliseconds per tick.
- `CyclesToMs` (`0x25C0E0`) multiplies by that value.
- `MsToCycles` (`0x25C110`) divides by it, returning zero before `Init`.

`core_initialize` calls `Init`.

## Pixel data

`RndPixelData::LoadBuffers` (`0x686B10`) reads a revision, then the pixels of
each mip level in the chain:
- for revisions up to 5, the size comes from the level's format bits and
  dimensions;
- later revisions store each level's size.

Flag 4 at `+0x30` stops after the first level.
