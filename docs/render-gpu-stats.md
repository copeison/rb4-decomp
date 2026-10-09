# GPU statistics

`RndGpuStatsMgr::Stat` derives from a profiler base class with vtable
`0x18EF420`, which the CPU timers also use. The map has no name for it; the
source calls it `PerfTimerBase` and puts it in `os/PerfTimer.o`, whose
`PerfTimer` it most resembles. The base:
- builds the slash-separated full name from its parents
  (`UpdateFullName`, `0x24B760`) and a sort name (`GetSortName`, `0x24B8C0`);
- reads `expanded`, `isolated` and `budget` from the stat's array in the
  `gpu_timer` system configuration (`LoadConfig`, `0x24BA10`);
- declares five pure accessors: counts, milliseconds, averages and worst
  case per frame.

What looked like hardware counters in the manager are budget categories. The
map has no names for these either; the source calls them
`gBudgetCategories` (`0x19F2700`). `InitBudgetCategories` (`0x25ECB0`) reads
the `budget_categories` configuration, and each category has a name, a
`gpu_budget` and a list of `filepaths`. `RndGpuStatsMgr::Init` creates one
block per category, and a stat inherits its category's budget.
