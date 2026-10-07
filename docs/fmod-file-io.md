# FMOD file I/O bridge

The game supplies FMOD with its own file callbacks rather than letting FMOD
open package files directly. The cleaned implementation is in
`src/audio/fmod_file_io.cpp`; their entry points run from `0x27A1C0` through
`0x27A850`.

## Synchronous callbacks

`fmod_file_open` allocates a 40-byte wrapper in the executable, stores the
requested path, creates a recursive mutex named `hx crit sec`, and opens the
engine file in mode 2. A successful open prepares the file object and reports
its 32-bit size to FMOD. A failed engine open destroys the wrapper and returns
`FMOD_ERR_FILE_NOTFOUND`.

Read and seek operations hold the wrapper mutex. Reads return
`FMOD_ERR_FILE_EOF` when the engine returns fewer bytes than FMOD requested;
seeks are absolute and report success after dispatching to the engine file.
Every callback that receives a null wrapper returns
`FMOD_ERR_INVALID_HANDLE`. Close releases the engine file, destroys the mutex,
and frees the wrapper.

The underlying virtual file adapters are now named in IDA:

| Address | Name | Behavior |
| --- | --- | --- |
| `0x378940` | `engine_file_open` | Opens an engine file with a numeric mode. |
| `0x378960` | `engine_file_close` | Dispatches the virtual destructor/close operation. |
| `0x378A10` | `engine_file_read` | Reads bytes into a caller buffer. |
| `0x378A50` | `engine_file_seek` | Seeks to a position and origin. |
| `0x378A90` | `engine_file_get_size` | Returns the file size. |

## Asynchronous reader

The initializer at `0x27A1C0` creates a condition variable and starts a worker
named `FmodFileWrapper` using the `stream_reader` platform thread settings.
Requests use the 56-byte FMOD 1.10.04 `FMOD_ASYNCREADINFO` layout. The observed
offsets are preserved with compile-time assertions in `fmod_api.h`.

`fmod_file_async_read` inserts each request into an ascending priority queue.
The worker removes from the back, so the highest numeric priority runs first.
Insertion before an existing equal priority means equal-priority requests
remain first-in, first-out when removed from the back.

For each request, the worker seeks to `offset`, reads `sizebytes` into
`buffer`, stores the byte count, and calls the request's completion callback
with `FMOD_OK` or `FMOD_ERR_FILE_EOF`. A missing handle is translated to
`FMOD_ERR_FILE_BAD` before the asynchronous completion callback runs.

Cancellation removes a request that is still queued. If the worker is already
processing that request, cancellation waits on the same condition variable
until its completion callback has returned. Shutdown marks the worker for
exit, wakes it, joins it, and destroys the condition variable.
