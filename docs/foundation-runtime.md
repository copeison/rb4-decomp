# Foundation runtime

The formatter, threads, clocks, CPU timers and keyboard messages of `src/utl`
and `src/os`.

## Per-thread data

The map's build keeps each thread-local value in its own `TLSValue<T>`. This
build puts them all in one per-thread block. The descriptor is a
function-local static returned by `0x258B80`, and the creator at `0x259570`
mallocs `0x105500` bytes and zeroes them. Each object's static initializer
records the descriptor and its value's offset. These offsets are known:

| Offset | Value | Owner |
| --- | --- | --- |
| `0x3410` | the thread's name (`Thread::ThreadName`) | `utl/Thread.o` |
| `0x3430` | `TlsMkStringData`, 16 ring buffers of 4096 bytes | `utl/MakeString.o` |
| `0x3458` | `FormatBuf`, two format slots and their depth | `utl/MakeString.o` |
| `0x5460` | the thread's CPU timers and its id | `os/PerfMgr.o` |
| `0x5488` | the thread's running-timer stack | `os/PerfMgr.o` |

The reconstruction declares each value `thread_local`, as the
`EntityResource` and `DataThread` values already are.

## FormatString

`utl/MakeString.o` (`0x2471A0`-`0x248546`) formats into `MakeStringBuf`'s
next ring buffer. `InitializeWithFmt` copies the format into the next
4096-byte format slot. A third nested formatter fails with `"double
failure"` through `HmxFail` (`0x35CC30`).

Each `operator<<` does the following:
- ends the format at the next specifier;
- formats one argument with `HmxSnprintf` (`0x9290`), which returns -1 on
  truncation;
- moves past the specifier with `_UpdateType`.

`_UpdateType` sorts the next specifier into int, `s` string, `a`/`f`/`g`
float or none. The `DataNode` overload converts the node to that type.

The binary has two 64-bit overloads (`0x247AB0`, `0x247BC0`) that the map
does not list and nothing calls.

## Threads

`Thread::SetCurrentName` (`0x258E50`) does three things:
- copies the name into the thread data;
- records it in the first free slot of a 32-entry `{pthread, name}` table
  (`0x19E8300`), guarded by a `CritSec` at `0x19E8800`;
- on the first call, records `s_MainThreadID` and calls
  `_InitMainThreadAffinity` (`0x25C3A0`), which sets the processor count
  (`0x19E8818`) to 6 and pins the main thread to processor 3.

`BuildAffinityMask` falls back to every counted processor.

`ThreadMap::GetTaskSettings` (`0x258C60`) searches the task table at
`0x19B0410`. The table holds groups of 16 records of `0x210` bytes. A group
applies once the processor count reaches its minimum, and a later match
overrides an earlier one. Each record holds a name, a thread count and up to
16 thread settings. Only `"job_manager"` uses four threads. Unknown names get
`"unknown_task"` (`0x19B4620`). The function returns the thread count.

## ThreadCall

`utl/ThreadCall.o` (`0x259D00`-`0x25A136`) runs `ThreadCallback`s one at a
time on the `"Hmx ThreadCall"` thread:
- calls queue in a 1000-entry ring at `0x19E88C0`;
- `ThreadCallPoll` (`0x259F90`) reports the finished head call through
  `ThreadDone` and signals the semaphore for the next one.

The `0x219B80` that `FmodAudioStreamResource` calls is a wrapper that polls
ThreadCall, two other services and `TimeMgr`. It is not `ThreadCallPoll`;
the source names it `core_poll_and_update_time`. `FmodAudioStreamResource`
uses `ThreadCallback` from `utl/threading/ThreadCall.h`.

## TimeMgr clocks

This build moves the map's `TimeMgr` time accessors into `TimeMgr::Clock`
(`0x25A990`-`0x25AD50`). The manager's clock is at `+240`. The clock holds a
vector of four 32-byte `Timeline`s, one for each `TimeUnits`:
- seconds;
- beats;
- UI seconds;
- tutorial seconds.

A timeline keeps its time, its previous two times and a pause flag. Deltas
measure from the previous time, or from the one before when the clock's
mode is 1.

`0x25AB20` is `UISeconds` and `0x25AC70` is `Seconds`.

The rest of `TimeMgr` is not reconstructed.

## CPU timers

`thePerfMgr` (`0x19E7CD0`) is a 96-byte `PerfTimerMgr`. Each thread keeps
its timers in its thread data, indexed by the timer name's index in the
shared name table:
- `GetTimer(unsigned long)` registers the thread on first use;
- the timer is created from the configuration entry when there is one;
- `_ThreadTerminate` (`0x24A330`) deletes a thread's timers when the thread
  exits.

`Poll(bool)` (`0x249D40`) closes every timer's frame through
`PerfTimer::EndFrame` (`0x24A820`). `RndDevice` calls it as a frame begins
and ends. The renderer still calls it as `render_frame_phase_callbacks`.

## Keyboard messages

Messages derive from the map's `Message<N>`. The class holds the message
array, plus inline storage for an `N + 2`-node array that a message built
from its arguments uses. `KeyboardKeyMsg` is a `Message<5>`:
- the wrapping constructor is at `0x392240`;
- `Message<5>`'s destructor is at `0x392150`.

`KeyboardOverride` (`0x3A1970`) swaps the override sink at `0x1A03960`. The
keyboard's subscriber object (`0x1A03958`) is not modelled.
