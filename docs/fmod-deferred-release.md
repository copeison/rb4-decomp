# FMOD deferred release queue

The audio state keeps two queues of FMOD channel-control and DSP pairs. A
voice teardown calls `fmod_defer_channel_dsp_release` at `0x278A00` after
clearing the DSP's user data. The function locks the queue, checks that an FMOD
Studio system is still attached, and appends the pair to the active buffer.

`fmod_process_deferred_releases` at `0x278DF0` is registered as a mix consumer
through `audio_register_mix_consumer` at `0x1128380`. During the next premix
dispatch it swaps the active buffer under the queue mutex, then processes the
old buffer without holding that mutex. Every channel control is stopped before
its paired DSP is released, and the processed buffer is cleared for reuse.
Producers can therefore enqueue into the other buffer while cleanup is
running.

Detaching an external Studio system clears both buffers under the same mutex
without calling FMOD. This matches the binary's shutdown path, where the
system pointers are about to become unavailable.

The executable stores each queue as an EASTL vector with a 16-byte pair and
preallocates room for 64 entries. The clean reconstruction in
`src/audio/fmod/system/fmod_deferred_release.cpp` uses two standard vectors and preserves
the locking, buffer swap, and FMOD call order.
