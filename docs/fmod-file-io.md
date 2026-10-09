# FMOD file I/O

FMOD reads packaged files through the engine's file system. The callbacks
installed by `FModSystem` are in `src/audio/fmod/io/FmodFileWrapper.cpp`; the
map has no object for them, and the names follow the reader thread's name,
`FmodFileWrapper`.

## Synchronous callbacks

`FmodFileOpen` at `0x27A5A0` allocates a 40-byte `FmodFileWrapper` holding
the path, a recursive `CritSec` and the engine file. `FmodFileWrapper::Open`
at `0x279FE0` opens the file in mode 2, prepares it for streaming through
`File` slot 17, and reports its 32-bit size. A failed open frees the wrapper
and returns `FMOD_ERR_FILE_NOTFOUND`.

Reads and seeks hold the wrapper lock. A short read returns
`FMOD_ERR_FILE_EOF`; seeks are absolute. A null wrapper returns
`FMOD_ERR_INVALID_PARAM`. Close releases the engine file and the wrapper.

## Asynchronous reader

`FmodFileWrapperStartReader` at `0x27A1C0` creates the condition named
`Condition` and starts the `FmodFileWrapper` thread with the `stream_reader`
task settings. Requests use the 56-byte `FMOD_ASYNCREADINFO` layout.

`FmodFileAsyncRead` inserts each request before the first queued request of
equal or higher priority, and the worker takes requests from the back, so the
highest priority runs first and equal priorities stay first-in, first-out.
For each request the worker seeks, reads, and calls the completion callback
with `FMOD_OK`, `FMOD_ERR_FILE_EOF` or, for a missing handle,
`FMOD_ERR_FILE_BAD`.

Cancelling a queued request removes it; cancelling the running request waits
until it completes. `FmodFileWrapperStopReader` at `0x27A460` wakes the
worker, joins it and destroys the condition.
