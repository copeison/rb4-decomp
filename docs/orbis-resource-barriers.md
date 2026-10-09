# Orbis resource barriers

`PS4Context::_ResourceBarrierImpl` (`0x8EAA10`) consumes 32-byte
`RndResourceBarrier` records. Each record holds:
- a type: transition, aliasing or unordered access;
- a phase: immediate, begin or end;
- the `RndShaderResource`;
- a subresource, which is mip-major, or -1 for all;
- the states before and after.

The state bits are render target (`0x4`), unordered access (`0x8`), depth
write (`0x10`), stream output (`0x100`), copy destination (`0x400`) and resolve
destination (`0x1000`).

## Phases

Each barrier that needs synchronization is handled by its phase:
- **Immediate barriers** add their cache actions and request one wait at the
  end.
- **Begin barriers** signal a label shared by the whole call
  (`_SignalResource`) and add their cache actions.
- **End barriers** wait for the resource's label (`_WaitForResource`).

## Barrier types

- **Unordered-access barriers** use `kCacheActionInvalidateL1`.
- **Aliasing barriers** are ignored.
- **Transitions** are ignored when the state does not change.

A transition's cache actions come from its target state:
- render target, depth write and resolve destination take none;
- every other state takes `kCacheActionWriteBackAndInvalidateL1andL2`.

Before a 2D texture becomes a render target, the graphics ring waits for the
back buffer's pending-presentation label of the main window's active buffer
to clear.

## Leaving render-target or depth-write state

The target is decompressed on the graphics ring, unless it is an end
barrier, which only waits:
- **Color targets.** The context waits for color-buffer writes over the
  target's slices (`kExtendedCacheActionFlushAndInvalidateCbCache`) and
  triggers `kEventTypeFlushAndInvalidateCbPixelData`. It then runs
  `Gnmx::decompressDccSurface` if DCC is enabled. Otherwise it runs
  `eliminateFastClear` and `decompressFmaskSurface`.
- **Depth targets.** The context waits for depth writes
  (`kExtendedCacheActionFlushAndInvalidateDbCache`) and flushes the HTILE
  cache when the HTILE is texture-compatible. It then runs
  `decompressDepthSurface`.

A 2D array's target is copied and narrowed to the barrier's slice, which is
the subresource divided by the mip count. A subresource of -1 views every
slice.

After decompressing, the context resets the draw state, as `_BeginFrameImpl`
does. On the graphics ring:
- a begin barrier signals the resource;
- other phases request the final wait.

When recording compute, it switches to the graphics pipeline with
`RndContext::SetActivePipeline` (`0x6BD8A0`) and signals there. It then
switches back and, unless the barrier is a begin, waits for the signal.

## Other transitions

- **From unordered access or copy destination**, a transition always
  synchronizes.
- **From resolve destination**, it does nothing.
- **From a read state**, it synchronizes only when the target is a write
  state, and then with no cache action.

## The final wait

When an immediate barrier asked for it, the context waits:
- **On the graphics ring**, it writes a label at `kEopCsDone` with the
  accumulated actions plus `kCacheActionWriteBackAndInvalidateL1andL2`, and
  waits for it. This is the same end-of-pipe wait `_SetRenderTargetsImpl`
  uses after a clear.
- **On a compute queue**, it triggers `kEventTypeCsPartialFlush`.

If cache actions remain:
- the graphics ring flushes them with `waitForGraphicsWrites(0, 1, 0, …)`;
- a compute queue flushes them with `flushShaderCachesAndWait`.
