# Audio render targets

## Audio properties

`Audio::SetAudioSystemProperties` at `0xD3BC0` caches the engine's sample
rate, seconds per sample, buffer size, buffers per second and milliseconds
per buffer in globals. `GetSamplesPerSecond` (`0xD3BA0`) and
`GetSecondsPerSample` (`0xD3BB0`) read them. The defaults are 48 kHz with
1,024-sample buffers. The source is `src/audio/core/system/Audio.cpp`, which
also holds the engine's recursive `CritSec`.

## AudioRenderTarget

A render target is a named output that mixes generators. `FModSystem` and the
recording targets derive from it. The vtable is at `0x19A0108`, the
constructor at `0x1127FC0`, and the object is `0x118` bytes. The class name is
not in the map. The engine registry at `0x19C90B0` is an
`eastl::map<Symbol, AudioRenderTarget*>` followed by the default target at
`0x19C90E8`, which also answers the empty name.

Its fields are the name at `+0x08`, the type at `+0x10` (1 for an
`FModSystem`, 2 for a recording target), the `AudioMixer` at `+0x18`, the
voice pool at `+0x98`, the premix callback list at `+0xB0` under the lock at
`+0xA0`, the sample rate at `+0xC8`, the buffer size and count at `+0xCC` and
`+0xD0`, the raw speaker count at `+0xD8`, and a map of CPU timers at `+0xE0`.

FMOD generators resolve the `FModSystem` behind a target from its type: the
target itself for type 1, or the system embedded at `+0x1E0` for type 2.

## AudioMixer

The mixer at `+0x18` feeds its `AudioBusCallable` clients in 128-sample
blocks. It is an `FmodPremixCallback` (base vtable `0x18E5B30`); its vtable
is at `0x19A00E0`. `ExecutePremix` at `0x1127880` first promotes the
callables queued since the last mix, then, for each block, calls
`_PrepareToMakeSamples` on every callable and `_MakeSamples` on each one once.
An atomic guard in each callable makes the second pass run at most once per
block. The original assumes the buffer length is a multiple of 128.

## AudioBus

`AudioBus` is the source a bus generator renders (vtable `0x18E5A78`,
`0xC0` bytes). It holds an `AudioBuffer<float>` at `+0x08`, the block size,
channel count, sample rate and its reciprocal, an owner told on destruction
at `+0xA8`, and the bus lock at `+0xB0`. Slot 2 is `Prepare`, slot 3 sets the
sample rate, slot 4 `Process` is pure in this build, slots 5 to 9 are
the map's inline lock helpers, and slot 10 `IsVirtualInstrument` is true only
for the instrument buses such as `FusionSampler`. `AudioBusGenerator`
reports its source's answer through `AudioBusCallable::IsVirtualInstrument`.

## Recording targets

`RecordingAudioRenderTarget` (vtable `0x19A01A0`, `0x1E0` bytes) records its
mix offline on a worker thread to a 16-bit `WaveFile`. Its constructor opens
the output `FileStream`, sizes the interleaved mix buffer from the speaker
configuration, and creates a `TransEntityResource` with an emitter component
that plays into the target. `RecordLoop` at `0x1129160` mixes one buffer per
pass, waits while a watched generator is paused, and patches the wave sizes
when stopped. `FmodRecordingAudioRenderTarget` at
`0x275E20` adds an `FModSystem` at `+0x1E0` that renders through
`HMX.BufferedOutput`, and a worker thread at `+0x520`. Neither name is in the
map; the worker thread is named `RecordingAudioRenderTarget`, and no caller
constructs the FMOD target in this build.

Construction initializes the embedded system with output type 1 and two
buffers, sets the speaker configuration, initializes buffered output, and
binds the output callback that calls `readfrommixer` with the recording
buffer and buffer size. Most virtual methods forward to the embedded system;
`Lock` and `Unlock` take both objects' locks. `StartAsyncRecording` at
`0x276080` runs the generic recording loop on a thread using the
`async_audio_record` task settings.

## Reconstructed core

`src/audio/core/output` holds `AudioBus.cpp` (the map's `audio/AudioBus.o`,
`0xBEF90` to `0xBF2B0`, plus the `AudioBusCallable` members at `0x47650`
to `0x47730`), `AudioRenderTarget.cpp` (the mixer, every render target
member from `0x1127770` to `0x11287C0`, `AudioMixer::RemoveCallable` at
`0x57560` and the registry at `0xC0F70` to `0xC1700`) and
`RecordingAudioRenderTarget.cpp` (`0x11289D0` to `0x1129360`).
`src/audio/core/streams/StreamReaderThread.cpp` is the map's
`audio/StreamReaderThread.o` (`0x264140` to `0x264700`) with this build's
plain `StreamReader` members at `0x263EC0` and `0x263F70`; the map's global
`theStreamReaderThread` is a member of the buffered-stream manager here.

Slots 7 to 9 and 16 of the render target share their code with
`FModSystem` and `FmodRecordingAudioRenderTarget` and are written from those
copies. The base `Update` at `0x11287B0` returns nothing; the source returns
zero. The unused mixer stubs at `0x1127B60` and `0x1127B70` are not modelled.

Still undefined: `TransEntityResource`, `BinStream::Write`, the emitter
class symbol at `0x19C7770` and the entity thread state at `0x19B03C8`.

## Voice pool

`FusionVoicePool` (`src/audio/core/fusion`, `0xA0400` to `0xA1AA0`) holds
the Fusion sampler voices of a render target: an array of 784-byte
`FusionVoice`s allocated under "FusionVoice", a vector of `SmbPitchShift`
processors (`0x64070` bytes each, 1024-frame FFT, 4x oversampling) for
keyzones that shift pitch, and the list of client samplers. While creation
is deferred, the voices exist only while a sampler is registered. The hard
limit is clamped to 1-256 and the soft limit to the hard one.

`GetFreeVoice` (`0xA15C0`) releases voices already playing the request's
id and takes a free voice or steals one: never one whose keyzone priority is
zero or below the new keyzone's, otherwise the lowest priority, a released
voice before a held one, the most samples since the volume envelope last
stopped and then the quieter channel peak.
`FastReleaseExcessVoices` (`0xA0DE0`) fast-releases voices over a sampler's
limit, or over the soft limit, in the same order. The flag set at `0xA1250`
stops each voice creating its own Mogg and XMA decoders.

## Sampler voices

`FusionVoice` (`0x9D920` to `0xA0340`) is the whole of `audio/FusionVoice.o`.
`AssignIDs` binds a keyzone, the sampler's two `ADSR` envelopes and two
`LFO`s (`src/audio/core/modulation`, declarations only) and the first of the
PCM, Mogg and XMA `AudioDecoder`s that takes the sample's format, and counts
the voice on the sampler. `AttackWithTargetNote` turns the note's distance
from the root key plus `SetPitchOffset`'s offset into a pitch ratio (clamped
to `kMaxPitchOffsetCents`, twelve octaves), resets the LFOs (retriggered or
joined to the sampler's free-running ones), the gains, the filter and the
pan mix, and starts both envelopes. A start offset past the end of the
sample kills the voice.

`Process` (`0x9F2A0`) renders in control blocks of four frames: the decoder
writes each block straight into the output channels with the volume
envelope and a gain stepped towards the new output gain (trim, channel,
mute and expression gains, the velocity and keyzone volume, and -3 dB of
headroom). The playback rate combines the pitch bend, the portamento and
pitch LFOs (two semitones at full depth), the keyzone's fine tune and the
sample-rate ratio; time-stretched keyzones get the pitch ratio and a time
ratio separately, and tempo-synced ones correct their speed by 1% whenever
they drift more than 5 ms from the sampler's beat. A `BiquadFilter` (direct
form II, in `src/audio/core/dsp`) then runs over the block unless it passes
the signal through. The filter cutoff follows the LFOs (five octaves) and
the assignable envelope (ten octaves); a jump of more than an octave first
dips an `SPL::Ramper` gain over 10 ms, then ramps the coefficients over
15 ms (`_UpdateFilterSettings`) and the gain back (`_RestoreFilterGain`).
The pan mix, a 2x2 matrix scaled by the keyzone's channel gains, ramps
across the call. The channel peaks are kept in `mLevels`; a voice whose
envelope has finished or that has run out of sample dies once both peaks
fall below 0.0001, unless the sampler's portamento holds it. The map's
`PlaySampleSlice` has no counterpart in this build.

`FusionSampler` (`audio/FusionSampler.o`, about `0x95C40` to `0x9D920`) is
only partly reconstructed: its vtable and the members the voices use are
declared, with `VirtualInstrument` (`src/audio/core/instruments`) as its
primary base and `AudioGenerator` at `+312`. `FusionSampler.cpp` defines
`SetVoicePool`, `VoicePoolWillDestruct`, `SetBeat`, `GetMaxNumVoices`, the
pitch-bend, channel-gain and mute accessors and the `_Get*` getters.

## Buffers and wave files

`src/audio/core/buffers/AudioBuffer.cpp` is the map's `audio/AudioBuffer.o`
(`0xD3D10` to `0xD40E0`): `AudioBufferConfig`, the ASCII level meter, the
stripped `Print` traces and the 16-bit conversions. `Configure` is
instantiated for float (`0x13720`) and 16-bit samples (`0x135B0`); owned data
has a guard sample on each side, `0xFEDCF00D` converted to the sample type.

`src/audio/core/formats` holds `WaveFile.cpp` (`0xD6240` to `0xD7920`:
`WaveFile`, `WaveFileMarker`, `WaveFileData`, `WaveHeader` and
`WaveFileWriter`), `Chunks.cpp` (`0xE2110` to `0xE29E0`: `ChunkHeader`,
`IListChunk` and `IDataChunk`) and `ChunkIDs.cpp` (the tags set at
`0xE2040`). A `WaveFile` read from a stream keeps its `IListChunk` to copy
the samples out later; its markers pair the "cue " points, sorted by frame,
with the "labl" texts of the "adtl" list. `PatchDataSize` writes a RIFF
size that leaves out the marker chunks, and `WaveFileWriter`'s byte rate
assumes one channel.
