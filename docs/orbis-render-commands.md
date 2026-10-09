# Orbis render commands

`PS4Context::_DispatchComputeImpl` at `0x8EB870` routes direct compute dispatches
to the command stream currently owned by the render context: `GfxContext::dispatch`
or `ComputeContext::dispatch`, whose SDK inline bodies are the CUE
`preDispatch`, `dispatchWithOrderedAppend` and `postDispatch` sequence below. Graphics recording
first flushes the dirty Gnmx compute state, emits `sceGnmDispatchDirect`, and
then completes the graphics-context dispatch transition. Standalone compute
recording flushes its CUE state when dirty and emits the same direct dispatch
through the selected compute command buffer. Other recording modes do nothing.

The group counts are forwarded unchanged on all three axes. The active frame
selects one of two graphics contexts or banks of nine compute contexts, and the
active compute index selects the command buffer within that bank.

The marker methods at `0x8EB970` and `0x8EB9D0` (`pushMarker` and `popMarker`) push and pop Gnm debug markers
on that same active graphics or compute stream. Every pushed marker uses the
fixed packed color `0xFF0000FF`; the caller supplies the marker name. Unknown
recording modes leave both marker operations empty.
