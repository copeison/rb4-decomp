# Render GPU statistics

The render system embeds a 128-byte `RndGpuStatsMgr` at offset `0xE00`.
Construction at `0x62AAE0` initializes its two pointer arrays, four-slot frame
ring, query counter, backend pointer, and recursive mutex directly.

Initialization at `0x62ACB0` allocates a 384-byte `GPU Total` root statistic,
then enumerates the platform GPU counters. Each counter receives its reported
name, scale, and numeric index and is parented to the total statistic. The root
records that it has children, the root-statistic pointer array grows by
doubling, and the completed array is sorted directly by the 64-bit statistic
key at offset `0x28`. Each root has a cleared 32-byte outer array prefix and a
352-byte statistic base constructed at offset `0x20`; child parent pointers
refer to that base rather than the outer allocation.

Frame begin at `0x62AF80` now owns the query lifecycle directly. It derives a
nested statistic name from the active 16-byte context scope, finds or creates
the statistic, assigns the next query ID, grows the selected history slot by
doubling, begins the backend query through render-context vtable slot `0xF8`,
and pushes the `{statistic, query ID}` scope record. Statistic lookup and
creation at `0x62B2C0` is also source-owned: it searches by interned full-name
key, inserts new 352-byte statistics in order, reuses or creates the matching
384-byte root, copies hardware-counter scale and index metadata when present,
and records each new statistic in its root's pointer array. Frame end at
`0x62B5B0` pops the scope and closes the query through vtable slot `0x100`.
Frame finish at
`0x62B960` resolves the completed slot, advances the four-entry history ring
under the recursive mutex, and clears the new slot's sample range for every
statistic. Resolution at `0x62B9E0` is source-owned: it activates a pending
render frame, resolves each retained query through render-context vtable slot
`0x108`, and accumulates elapsed time plus six 64-bit hardware counters into
the selected 80-byte result slot. Runtime statistics maintain rolling query
count and elapsed-time averages over at most 50 samples, along with the latest
and maximum elapsed values. When backend statistics are active, a
`GPU Total {Remainder}` child subtracts all explicitly measured total children
from the total. Each 384-byte root then sums its children and publishes the
largest smoothing history observed among them.

Destruction at `0x62ABA0` dynamically releases runtime statistics, then tears
down every 384-byte root directly. Each root owns a 32-byte child pointer
array, the 56-byte statistic base at offset `0x20`, four 32-byte history arrays
at offset `0x58`, and two 80-byte result slots at offset `0xD8`; the histories
are released in reverse order before the embedded name record and child array.
Runtime statistics use the same base, histories, and result slots, followed by
their interned full-name key at offset `0x158`. The block then unwinds the
recursive mutex and frees both top-level pointer arrays by capacity.

## Class

The block is the map's `RndGpuStatsMgr` (`src/render/debug/RndGpuStatsMgr.{h,cpp}`),
embedded in `RndDevice` at 3584. In this build it has no vtable and no
`eastl::map`. Instead it keeps two arrays sorted by symbol address: every
statistic by full name, and the per-name `StatBlock`s. Its lock is a
`CritSec`. `BeginStatBlock` (0x62AF80) returns a query key, or -1 without a
backend. `EndStatBlock` (0x62B5B0) ignores negative keys. `EndFrame`
(0x62B960) advances the frame through `_NextFrame` (0x62C0E0, also
inlined), and `_GatherStats` (0x62B9E0) resolves and smooths the results.
The overlays query it through `GatherTimers` (0x62C150; it appends the
statistics or the per-name totals and sorts them by full name, ignoring the
sort mode), `GetBudget` (0x62C610, zero), `GetAverageMs` (0x62C710) and
`GetWorstMs` (0x62C840); `ResetTimers` (0x62C970) clears the history frames.
The CSV and text printers at 0x62CA60-0x62D120 are not reconstructed, and
`GetMs` was not located in this build.
