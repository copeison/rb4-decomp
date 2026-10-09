# Event sources and poll jobs

The two bases of `Entity`: `MsgSource` from the map's `utl/MsgSink.o` and
`PollDepBase` from `utl/PollDep.o`. Names follow [naming.md](naming.md);
the entity built on them is in [entity-resources.md](entity-resources.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `utl/MsgSink.o` (part) | `0x248AB0`-`0x249610` | `src/utl/messages/MsgSink.cpp` |
| `utl/PollDep.o` | `0x24BC80`-`0x24CB60` | `src/utl/threading/PollDep.cpp` |

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
`_AddPostPoll` and `_RemovePostPoll` link the job into its manager's
post-poll queue (`PollMgr` `+440`).

| Slots | Members |
| ---: | --- |
| 0-1 | destructors |
| 2-5 | `IsPollEnabled`, `ThreadPoll` (pure), `PostPoll`, `GetPollName` |
| 6-9 | `_OnAddPollDep`, `_OnRemovePollDep`, `AsPollMgrDep`, `AsPollMgrJob` |

The emitted vtables of both classes match the binary slot for slot
(`orbis-objdump -r`).

## Weak evidence

- The slot names, except `ThreadPoll`, are inferred. Slots 8-9 return null
  in every class found; when they do not, the base reads a nested
  `PollMgr*` right after itself (`_NestedPollMgr`).
- `MsgSink::GetSinkObject`, `MsgSource::RemoveAllSinks`, the bucket names,
  `PollMgr::mSharePollMgr`, `PollDepBase::mAuxNode` (only linked by the
  constructors), `TryRemovePollDep`, `_OnPolled` and `_DoThreadPoll` are
  inferred from their bodies and callers.
- The map's `ExportToSink(DataArray*, EventSink*)`, `AddSink(MsgSink*,
  Symbol, Symbol)` and `RemoveSink(MsgSink*, Symbol, Symbol)` have other
  signatures; the names follow their roles.
- `0x248D00`, which returns its argument and has no caller, sits in
  `MsgSink.o`'s range and is not identified.
