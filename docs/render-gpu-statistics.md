# Render GPU statistics

The render system embeds a 128-byte `RenderGpuStatBlock` at offset `0xE00`.
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
statistic.

Destruction at `0x62ABA0` dynamically releases runtime statistics, destroys
and releases every 384-byte root statistic, unwinds the recursive mutex, and
frees both pointer arrays by capacity.
