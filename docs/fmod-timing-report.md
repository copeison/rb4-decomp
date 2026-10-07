# FMOD timing report

`fmod_audio_consume_timing_report` at `0x277BA0` atomically snapshots the
audio thread's timing accumulators and resets their total, sample count, and
maximum fields. It reports average and maximum percentages for the engine mix,
the full FMOD mix, and the rolling `buffer_set` window.

The percentage getters at `0x278C80` and `0x278CB0` divide the accumulated or
maximum elapsed milliseconds by the duration of one audio buffer, then
multiply by 100. The rolling getters at `0x278D10` and `0x278D50` also divide
by the window size because each recorded value is a sum across that window.
The buffer duration is the value cached by `audio_set_mix_format` at
`0xD3BC0`.

Each source timing entry carries a 64-bit engine symbol key. The report adds
that source's average and maximum percentages to two maps, allowing the caller
to combine results from multiple registered audio systems. An optional vector
receives the source keys. The caller at `0x262B80` supplies this vector only
for the first audio system, while the maps accumulate every system.

That caller also invokes `FMOD::System::getCPUUsage` before consuming the
engine timings. Its five FMOD CPU values and ten engine output slots are local
telemetry data; four of the engine slots remain zero because `0x277BA0` does
not write them. No later code in this build publishes the collected values,
which is consistent with a telemetry path whose final sink was compiled out.

The timer conversion was previously labeled as seconds. The constant at
`0x125B060` is `1000.0`, so `performance_counter_ticks_to_milliseconds` at
`0x25C0E0` actually computes milliseconds. The reconstructed accumulator
field names and documentation now reflect that unit.
