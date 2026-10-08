# FMOD mix callback

`fmod_system_callback` at `0x2783E0` handles FMOD's premix and postmix events.
Together they bracket one engine mix sequence while holding the audio object's
semaphore.

On premix, the callback waits for the semaphore, verifies that the Studio
system still exists, marks a mix as active, and increments the 64-bit mix
sequence. It starts the `hmx`, `fmod`, and `buffer_set` timers, clears the
per-source timer depth and elapsed ticks, then dispatches the engine's buffer
consumers with the current DSP buffer length and mix sequence. The `hmx` timer
ends immediately after this dispatch.

The mix dispatch at `0x2781C0` holds its own recursive mutex while visiting
registered buffer consumers. It then forwards the same buffer length and
sequence to the output-block dispatcher at `0x1127880`. That stage uses the
low 32 bits of the sequence and divides the buffer into 128-sample blocks for
its listeners.

On postmix, the callback only acts when premix marked a mix as active. It ends
the `fmod` timer and updates its total, sample count, and maximum duration. It
also ends the `buffer_set` timer and inserts its duration into a fixed-size
rolling window. The sum of that window is accumulated as the timer's sample
and compared with its maximum. Finally, the callback invokes every registered
postmix observer, clears the active flag, and releases the semaphore.

The original timing accumulators use the processor timestamp counter and a
spin lock for their statistics. `performance_counter_ticks_to_milliseconds`
at `0x25C0E0` converts ticks with `1000.0` divided by the frequency returned by
`sceKernelGetTscFrequency`. The clean reconstruction keeps these mechanics in
`src/audio/fmod/mixing/fmod_mix_callback.cpp`.

The intrusive source-timer and observer list layouts are known, but their
concrete callback classes are outside this milestone. Named adapter functions
preserve those dispatch points until their owners are reconstructed.
