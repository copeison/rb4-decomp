# OS runtime

The memory manager, debug channel, system configuration and their helpers
in `src/os`. Names follow the reference map; see [naming.md](naming.md).
The binary is link-time ordered, so an object's functions can sit among
another's (`AppendStackTrace` of `os/System.o` lies inside `os/Debug.o`).

## Memory

`os/MemMgr.o` (`0x37A550`-`0x37C0A0`) owns up to 16 `MemHeap`s
(`gHeaps`, `0x19FF240`; `gNumHeaps`, `0x1A01B40`). `MemInit` (`0x37AB70`)
reads the `mem` configuration and builds each heap from `PlatformAlloc`
memory. The current heap is a per-thread stack (`tMemStack`,
`0x1A01B88`) driven by `MemPushHeap`/`MemPopHeap`; `MemPushTemp` selects
the temporary heap. `MemAlloc` (`0x37AE70`) retries a full heap in its
fallback heap before it fails. `gMemLock` (`0x1A01B90`)
guards all heap calls.

`MemHeap` (`os/MemHeap.o`, `0x37C750`-`0x37EC90`) is a 0x290-byte
free-list allocator:
- each block starts with a packed header word of size, alignment and flags;
- free blocks (`FreeBlock`, 24 bytes) stay sorted by address;
- the strategy picks `FirstFit`, `BestFit`, `LRUFit` or `LastFit`;
- frees can be batched and sorted with `eastl::sort` before they merge.

`os/PoolAlloc.o` serves requests of 128 bytes or less from
`FixedSizeAlloc` pools fed by one `ChunkAllocator` (`gChunkAlloc`,
`0x1A01C48`). `os/Mem_PS4.o` maps flexible memory for `AddPS4Heap`.

## Debug

`TheDebug` (`0x19FDB40`) is the 0x200-byte `Debug` (vtable `0x18FAF08`).
`Notify`, `Fail` and `Exit` format through `FormatString` and copy the text
to every reflected `TextStream`. A failure fills the `CrucibleReport`
(`0x19FC1E0`) with the stack trace and the map file's symbols, then shows the
modal dialog unless `gHmxNoModal` is set. `HmxExit` (`0x35CC50`) runs the
exit callbacks and terminates the process; `Main` calls it at shutdown.
`NotifyUniqueStr` remembers each text in a `StringTable` so it is reported
once.

`GlitchBreaker` (`os/GlitchBreaker.o`) is a watchdog thread. It breaks
into the debugger when `Reset` is not called within its period.

## System

`SystemInit` (`0x368000`, `os/System.o`) defines the build's macros and
reads the configuration (`gSystemConfig`, `0x19FE548`). The file comes from
the disk instead of the archive when `host_config` is set, and the `config`
option replaces it. `SystemInit` then runs every subsystem's `Init` in
order. `SystemPoll` (`0x369940`)
advances `gSystemTimer` and polls the services. The language helpers map
between `Symbol` language names and their abbreviations. `os/System_PS4.o`
supplies the platform language, the stack capture and the modal dialog.

`CallOnMainThread` (`0x3902B0`) queues callbacks that `CallOnMainThreadPoll`
runs from the main loop.

## Not reconstructed

`AppChild`, `Archive`, `ArkFile`, `AsyncFile`, `ContentMgr`, `Locale`,
`MovieMgr`, `PlatformMgr`'s virtuals, `MapFile` and `MemTrack` are declared
with their addresses only.
