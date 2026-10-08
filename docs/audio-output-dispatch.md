# Audio output dispatch

`audio_dispatch_output_blocks` at `0x1127880` is the second stage of each FMOD
mix-buffer dispatch. It promotes listeners from a pending intrusive list to the
active list, then divides the configured DSP buffer into 128-sample blocks.

Each block has two listener passes. The first calls the virtual method at slot
`0x10` with the block size, low 32 bits of the mix sequence, block index, final
block flag, and sample rate converted to `float`. The dispatcher stores that
context, clears every listener's atomic guard, and calls the virtual method at
slot `0x18` for listeners that successfully change their guard from zero to
one.

The outer dispatch mutex and the pending-list mutex are recursive. Their depth
counters bracket traversal and list promotion respectively. The original code
assumes that the DSP buffer length is a nonzero multiple of 128; its subtract
loop has no partial-block path.

The cleaned reconstruction in `src/audio/core/audio_output_dispatcher.cpp` uses
standard containers and mutexes to express the recovered behavior. The binary
uses EASTL intrusive lists and PS4 pthread mutexes, whose exact declarations
belong to the later SDK-backed ABI layer.
