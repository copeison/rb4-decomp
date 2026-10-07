# FmodBufferedStreamGenerator

The strings at `0x125CE80` and `0x125CEB5` identify
`FmodBufferedStreamGeneratorManager` and `FmodBufferedStreamGenerator`. Their
primary vtables begin at `0x18F05C8` and `0x18F0668`. The manager accepts the
stream option form whose format field is 3 and whose streaming flag is set,
and it reports `.mp3` as its extension.

The manager owns a fixed pool of `0x238`-byte generator objects. Pool setup at
`0x26E000`, shutdown at `0x26E270`, stale-handle retention at `0x26DD50`, and
the reset, stop, and active-handle passes use the same synchronized free-list
scheme as the other FMOD generator managers.

Initialization at `0x26ADA0` creates an FMOD sound, configures decoder storage,
builds a vector of `0xC0`-byte stream blocks, and acquires a nested
`FmodAudioBusGenerator`. Sound-open completion at `0x26BAD0` reads the FMOD
format, default frequency, and PCM length before submitting every block to the
engine's streaming queue.

The generator forwards pause, resume, reset, stop, fade, and transition
controls to the nested bus generator. Position is stored in milliseconds.
Seeking at `0x26BDC0` waits for all submitted blocks to become idle, converts
milliseconds to a sample position, resets the ring, and resubmits the required
blocks. Gain is stored at offset `0x1D8` under the generator's stream lock.

`0x26C480` keeps the ring populated around the current block. The large callback
at `0x26C720` supplies decoded samples to an output request, handles underrun
silence, loop and seek boundaries, gain, and sample-rate conversion. Its normal
stereo path linearly interpolates interleaved signed 16-bit PCM and scales it by
`1 / 32768`; the cleaned source reconstructs that path and zero-fills an output
tail after source exhaustion.

The synchronized path enables a six-sample interpolation kernel at `0x26D940`
and adjusts source position toward a target sample. The control functions at
`0x26DAB0`, `0x26DBA0`, and `0x26DBD0` enable synchronization, set its target in
milliseconds, and disable it. That specialized kernel stays in the focused IDA
export until its fitted coefficient scheme is identified.
