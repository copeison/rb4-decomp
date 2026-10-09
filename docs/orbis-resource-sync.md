# Orbis resource synchronization

The render context keeps a compact list of resources that have been released
by one command stream and may need to be acquired by another. Each 24-byte
record contains the resource pointer, a four-byte GPU label, and the current
render-system epoch.

`PS4Context::_SignalResource` (`0x8EB3E0`) allocates and clears a label the first
time it is called for a batch. Graphics command buffers write `1` after the
color/depth reads complete; compute command buffers write the same value after
compute completion. Additional resources in the batch reuse that label, while
each resource still receives its own tracking record.

`PS4Context::_WaitForResource` (`0x8EB590`) finds the record for a resource and
waits until its label equals `1`. The comparison uses the full 32-bit mask.
After the wait, every record that shares the label is removed in one stable
compaction pass. This matches the batch semantics: one GPU signal can release
several resources and the first acquire consumes the whole group.

The records are a `std::vector<ResourceSignal>` at `0x228D0`; the binary's
growth (`_Reserve`, `0x8EBFE0`) and its "vector<T> too long" check are the
standard library's. Graphics signals use `writeAtEndOfPipe(kEopCbDbReadsDone)`
(`0x28`) and compute signals `writeReleaseMemEvent(kReleaseMemEventCsDone)`
(`0x2F`); waits use `waitOnAddress` with `kWaitCompareFuncEqual`.

## Label ring

`_AllocateResourceLabel` (`0x8E7EB0`) hands out 32-bit labels from a ring:
- **State.** The ring lives at `0x228F0`-`0x2291F`: the labels, the first
  label of each frame parity, the last frame, the next label and the
  capacity.
- **One frame later.** The first allocation of a frame drops the signals
  recorded before the previous frame.
- **Two or more frames later.** The ring is reset instead.
- **Growth.** When the next label would reach the first label of the
  previous frame, the ring is replaced by one twice the size (named
  `"labels"`), and the old one is deferred for deletion.

The constructor reserves 32 signal records, sets the capacity to 32 and
allocates one label, which grows the ring to 64 labels. The destructor frees
the timestamps and then the labels.
