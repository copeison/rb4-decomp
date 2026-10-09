# Core binary I/O

Engine-wide binary I/O lives under `src/core/io`.

## Engine files

`engine_file.cpp` owns the thin public wrappers around the engine file
interface. Each one forwards to a single virtual slot:

| Address | Wrapper | Slot |
| ---: | --- | ---: |
| `0x378940` | `engine_file_open` | forwards to `0x376D40` |
| `0x378950` | `engine_file_failed` | `+0x58`; null files fail |
| `0x378960` | `engine_file_close` | `+0x08`; null-safe |
| `0x378A10` | `engine_file_read` | `+0x18` |
| `0x378A40` | `engine_file_write` | `+0x28` |
| `0x378A50` | `engine_file_seek` | `+0x38` |
| `0x378A60` | `engine_file_tell` | `+0x48` |
| `0x378A70` | `engine_file_flush` | `+0x40` |
| `0x378A80` | `engine_file_eof` | `+0x50` |
| `0x378A90` | `engine_file_get_size` | `+0x60` |

The file-system open routine at `0x376D40` remains an adapter boundary. FMOD's
file callbacks share this header; their inline `+0x88` prepare-for-reading call
is still declared beside them.

## BinStream

`BinStream` is the 40-byte common base: dispatch, two `-1` words, a byte flag,
the byte-swap flag at `+0x14`, an owned cipher generator at `+0x18`, and a
platform value at `+0x20`. The 15-slot base dispatch at `0x18EF020` leaves
flush, tell, eof, fail, and the read/write/seek implementations pure.

- `bin_stream_read` (`0x21A280`) queries `Fail` and ignores the result. It
  then calls `ReadImpl` and XORs every byte with the next Park-Miller value
  when a cipher is attached.
- `bin_stream_read_endian` (`0x21ABF0`) adds an in-place 2/4/8-byte swap.
- `bin_stream_write_endian` (`0x21ACB0`) stages swapped values in an
  eight-byte temporary. Encrypted writes go through a 512-byte chunk buffer.
- Slot 9 (`0x21AB60`) reads, then returns the size or zero after failure.
- Slot 11 (`0x219C30`) patches a 64-bit size prefix at a saved position.
- `random_generator_next` (`0x117B0E0`) is the minimal-standard Park-Miller
  generator, using Schrage's decomposition.

## FileStream

`FileStream` is 592 bytes. It stores the engine file at `+0x28`, a 512-byte
`strncpy` name at `+0x30`, a fail flag at `+0x230`, the size at `+0x238`, and an
optional checksum with a running byte count at `+0x240`/`+0x248`.

Construction (`0x2443A0`) uses platform value 3 and records the open failure.
It queries the size only after a successful open. Destruction (`0x2444A0`)
closes only named, successfully opened files. It then releases the checksum:
its name string at `+0xD8`, then its SHA-1 context reset at `+0x08`
(`0x117B560`), then the allocation. Reads that return a short count set the
fail flag. Successful reads update the checksum through `0x367C50`. Short
writes and negative seek results also set the fail flag.
