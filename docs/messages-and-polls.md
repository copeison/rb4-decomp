# Event sources and poll jobs

The two bases of `Entity`: `MsgSource` from the map's `utl/MsgSink.o` and
`PollDepBase` from `utl/PollDep.o`, and the poll manager that runs the jobs
(`utl/PollMgr.o`). Names follow [naming.md](naming.md); the entity built on
them is in [entity-resources.md](entity-resources.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `utl/MsgSink.o` (part) | `0x248AB0`-`0x249610` | `src/utl/messages/MsgSink.cpp` |
| `utl/PollDep.o` | `0x24BC80`-`0x24CB60` | `src/utl/threading/PollDep.cpp` |
| `utl/PollMgr.o` | `0x24CB60`-`0x2518E0` | `src/utl/threading/PollMgr.cpp` |

## MsgSource

`MsgSource` (24 bytes, vtable `0x18EF390`) is a `MsgSink` with a list of
events. This build's object is much smaller than the map's: it has no
property sinks, and each subscription (`EventSinkElem`, 40 bytes) belongs to
its subscriber, which passes it to `AddSink` (slot 4). An `EventSink`
(40 bytes, from the small-block pool) holds an event's subscriptions; the
sinks of every event, under the empty symbol, stay first.

`Export` (slot 5) sends a message to those sinks and then to its event's.
`ExportToSink` moves the subscriptions to a local list and back one at a
time, so a sink may unsubscribe while it handles the message. Each sink
receives the message under its handler's type, and the type is restored
afterwards. The message counts as handled when a sink returns anything but
an unhandled node. `Handle` (slot 2) subscribes for "addsink_private"
(`OnAddSink`) and exports everything else. `RemoveSink` (slot 6) unlinks a
subscription; `RemoveAllSinks` unlinks them all and frees the events.

## PollDepBase

`PollDepBase` (144 bytes, vtable `0x18EF498`, ten slots) is a job that the
poll manager runs after the jobs it depends on. Each job keeps its afters
(the jobs that wait for it), the number of them that wait this frame, and
two counters of the jobs it still waits for. The counters swap each frame.
`_FreeAfters` releases a job's afters: each counts the release for the next
frame and, at zero, is queued in one of five buckets. The bucket depends on
the job's thread flag, its weight against `PollMgr::sMaxPollWeight` and
whether jobs wait for it. A job that may not poll (slot 2) releases its own
afters instead. `EarlyFreeToPoll` releases one after before the job ends,
when the "toggle_early_free_to_poll" switch and the thread's context allow.
`_AddPostPoll` and `_RemovePostPoll` link the job into its group's
post-poll queue (`PollGroup` `+440`).

| Slots | Members |
| ---: | --- |
| 0-1 | destructors |
| 2-5 | `IsPollEnabled`, `ThreadPoll` (pure), `PostPoll`, `GetPollName` |
| 6-9 | `_OnAddPollDep`, `_OnRemovePollDep`, `AsPollGroupHead`, `AsPollGroupTail` |

`ThreadPoll` and `PostPoll` take a thread index; workers pass their mask
bit, the main thread and the post-poll 0.

## PollGroup and PollMgr

The map's `PollMgr` is split in this build, as the strings show ("PollMgr
PollGroup", "<PollGroup Head>", "PollMgr Worker %d"):

- `PollGroup` (544 bytes, small-block pool) brackets a set of jobs between
  a head job (vtable `0x18EF4F8`) and a tail job (`0x18EF558`), both
  `PollGroupJob`s (`0x18EF5B8`) that remember the group. Every job waits
  for the head; the tail waits for the leaf jobs (`_AddLeafJobs`, kept in
  the `+112` list). A group polls after another through its head's
  dependency on the other's tail, and nests in another through its head
  and tail. The head runs the start callback and the tail the end
  callback (`SetCallbacks`); an unthreaded group's tail runs its
  post-polls at once.
- `PollMgr` (88 bytes) owns a threaded group and the main-thread job list.
  `Select` makes it the polled manager (`sCurrent`) and the thread's, with
  a `PollGroupMap` for the entities (`ThreadPollContext::mPollGroups`);
  `QueueForPoll` sorts released jobs onto its list and the workers' heavy
  and light lists. `Sync` polls on the calling thread until a group
  finished or nothing is left, then loads the queued component resources
  (`QueueForLoadResources`). A finished group is queued on the manager
  by its tail (`_Finish`).
- `PollGroupMap` (vtable `0x18FFC60`, inline) keys groups; it is the base
  of `RndSceneCom::PollGroups`.

`Init` reads `poll_mgr`'s `enabled_audio_thread_blocking` and
`enabled_early_free_to_poll`, starts one worker per "job_manager" thread
and registers the `pollmgr` script function (`toggle_threads`,
`set_max_weight`, `toggle_sleep_post_job`, `set_thread_priority`,
`toggle_early_free_to_poll`). Workers take one heavy job, else light jobs
for their mask up to `sMaxPollWeight` (20); the main thread takes its own
jobs first. With audio thread blocking one worker, on the "audio_render"
processor or among the last, polls only while it holds the default audio
render target's mix (`TryBeginMix`, `EndMix`), which `BlockAudioThread`
requests through seven states (`kUnlocked` to `kUnlocking`, named by the
strings of `sAudioLockStateNames`).

| Statics | Address |
| --- | --- |
| `sWorkerCondition`, `sAudioWorkerCondition` | `0x19E7E70`, `0x19E7E88` |
| `sThreadsEnabled`, `sEarlyFreeToPoll` | `0x19E7EA0`, `0x19E7EA1` |
| `sCS` (the map's), `sAudioWorkerCS`, `sBusyMask`, `sTerminating` | `0x19E7EA8`-`0x19E7EBC` |
| `sWorkerThreads`, `sThreadDescs` | `0x19E7EC0`, `0x19E7EE0` |
| `sThreadedJobs`, `sHeavyJobs` | `0x19E7F00`, `0x19E7F10` |
| `sSleepPostJob`, `sAudioThreadBlocking`, `sAudioBlocked` | `0x19E7F20`-`0x19E7F22` |
| `sAudioLockStateNames`, `sCurrent` | `0x19E7F28`, `0x19E7F48` |
| `sLoadResourcesQueue`, `sLoadResourcesCrit` | `0x19E7F50`, `0x19E7F70` |
| `sMaxPollWeight`, `sAudioLockState` | `0x19B03D8`, `0x19E7FD0` |

Not reconstructed: three empty functions at `0x24F8D0`-`0x24F8F0` and one
at `0x251510`, none of them called; the eastl helpers the object
instantiates are modelled by the containers.

The emitted vtables of both classes match the binary slot for slot
(`orbis-objdump -r`).

## Weak evidence

- The slot names, except `ThreadPoll`, are inferred.
- `PollGroup`, `PollGroupJob`, `PollGroupMap` and every `PollMgr` and
  `PollGroup` member not in the map are named from their bodies and
  strings. Weakest: `PollGroup::mPersistent` and `SetPersistent` (no
  caller), `mPostPollGroup` (never written), `PollMgr::mReserved` (set to
  -1, never read), `CheckForCycles` and its `PollCycleSearch` (the search
  is compiled out), `StartPollGroup`, and `PollMgr::Sync` and `_Poll`
  for the map's `Sync(bool)` and `_Poll()`.
- `MsgSink::GetSinkObject`, `MsgSource::RemoveAllSinks`, the bucket names,
  `PollGroup::mShareParentGroup`, `PollDepBase::mAuxNode` (only linked by the
  constructors), `TryRemovePollDep`, `_OnPolled` and `_DoThreadPoll` are
  inferred from their bodies and callers.
- The map's `ExportToSink(DataArray*, EventSink*)`, `AddSink(MsgSink*,
  Symbol, Symbol)` and `RemoveSink(MsgSink*, Symbol, Symbol)` have other
  signatures; the names follow their roles.
- `0x248D00`, which returns its argument and has no caller, sits in
  `MsgSink.o`'s range and is not identified.
