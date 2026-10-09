# Audio DSP and analysis

Signal-processing code of the audio module: the pitch tracker the mics use,
its analysis helpers and filters, and the phase-vocoder pitch shifter used by
the sampler and an FMOD plug-in. Sources are in `src/audio/core/analysis`
and `src/audio/core/dsp`.

## PitchDetector

`PitchDetector` (`PitchDetector.o`, `0xE2A80` to `0xE32A3`, 112 bytes) tracks
the pitch of 16-bit input. The constructor allocates three 1 KiB buffers
(`mbDecimated`, `mbSOS`, `mbFullCC`) and a fourth-order `IIR4PoleFilter`
low-pass. `SetSampleRate` derives the decimation (rate / 6000), the shortest
period (a 1,320 Hz pitch) and the window (twice the decimated rate / 65,
rounded up to 16 samples).

`AnalyzeBlock` slides the window by the samples the block adds, filters every
input sample, smooths the result (`0.3` per sample) and stores each decimated
sample with a running sum of squares. It also tracks an input envelope whose
block maximum is the fourth output. The level is the square root of the
window's energy over its length. When `mComputePitch` is set it runs
`ShiftedDotProduct`, `FindCCPeak` and `RefinePeriod2`. The pitch is reported
as a MIDI note, `12 * log2(f / 440) + 69`. The energy output is
`12 * gain * level / mNoiseFloor`. This build resets the noise floor to 1
after each block, so the energy is not normalized. The map's signature has
three outputs; this build adds the envelope peak.

## SndAnalysis

`SndAnalysis.o` (`0xE32B0` to `0xE4B68`) holds float Numerical Recipes FFTs
(`four1`, `realft`, `RealFT`) and the period search:

| Function | Address | Role |
| --- | --- | --- |
| `ShiftedDotProduct` | `0xE37D0` | Autocorrelation over half the window. |
| `FindPeaks` | `0xE38F0` | Local maxima; candidates in a 100-entry static at `0x19E2710`. |
| `MultipleOf` | `0xE39D0` | Rounded multiple test. |
| `FindCCPeak` | `0xE3A00` | Normalized correlation peaks above 0.75, weighted by `period^log2(1/1.4)`. |
| `FindIntPeriod` | `0xE3CA0` | Period from peak spacing; not called. |
| `RefinePeriod2` | `0xE3F80` | Fractional period, at most two steps. |
| `RefinePeriod` | `0xE4210` | Older interpolation; not called. |
| `AddPeakSqWeight`, `FFTtoPeriod`, `FFTtoPeriod2` | `0xE4610` to `0xE4710` | Spectral peak interpolation. |
| `MakeWindowFunc` | `0xE4780` | Rectangular, triangular, Hamming and Blackman windows. |

`std::sort` on `int` (`0xE4B70`, `0xE4CB0`) is the library's instantiation.

## IIR filters

`IIRFilter.o` (`0xE5060` to `0xE569B`) has `IIRFilter`, a transposed
direct-form II filter of any order, and `IIR4PoleFilter`, a 100-byte
fourth-order version laid out in four-wide rows. Lane 0 of each row holds the
fourth-order term. `FilterSlow` (`0xE54F0`) runs the delay line in order and is
the one `PitchDetector` uses; `FilterFast` and `FilterProtoFast` are identical
copies that fold the feedback into the input. `Begin` and `End` are empty.

## SmbPitchShift

`SmbPitchShift` (`0xDEF10` to `0xE0485`, vtable `0x18E61E8` without type
information, `0x64070` bytes) is Stephan Bernsee's `smbPitchShift` with Hann
windows from shared tables (`InitTables`, 128 to 4096 samples, at
`0x19DA8A0` to `0x19DE6A0`). Its per-channel state is
`ChannelState` (`0x1C00C` bytes), with three channels: left, right and their
mix. The analysis and synthesis arrays are shared.

| Slot | Address | Method |
| ---: | --- | --- |
| 0-1 | `0xE0350`, `0xE0360` | Destructor. |
| 2 | `0xE0370` | `SetTimeStretchMode(algorithm, formantMode)`, empty. |
| 3 | `0xE0380` | `Reset`, empty. |
| 4 | `0xDF280` | `SetSampleRate`: bin width, clears the state. |
| 5 | `0xDF250` | `GetLatency`: frame size minus step. |
| 6-7 | `0xE0390`, `0xE03A0` | `HasPositionOffset`, `GetPositionOffset`: false and 0. |
| 8 | `0xDF300` | `Render`: decodes at `rate * timeRatio`, shifts by `pitchRatio / timeRatio`. |
| 9 | `0xDFB90` | `ProcessStereo`: low bins analysed from the mix. |
| 10 | `0xDF2B0` | `UpdateSampleRate` from the bound `AudioData`. |

`AudioDecoder::SetAudioData` stores the data and the decoder in the shifter
(`+0x8`, `+0x10`) and calls slot 10. The decoder's render slot forwards to
slot 8 when a shifter is bound, and slot 8 reads through the decoder's plain
render (slot 7). `ProcessChannel` (`0xDF3C0`) shifts one strided channel.
Channels after the first reuse the first channel's analysis below the mono
cutoff (`SetMonoCutoff`, `0xDF260`). `Fft` (`0xDF940`) is `smbFft`. The
constants show that `M_PI` was single precision.

Slot 2 receives the keyzone's `timestretch_settings`: the `algorithm` index
and a formant mode, 1 with `maintain_formant` and 2 without.
`GetTimeStretchAlgorithmName` (`0xE03E0`) and `GetTimeStretchAlgorithm`
(`0xE0400`) convert the index to and from `as_authored`, `default`,
`elastique_pro`, `elastique_eff_with_formant` and `elastique_mobile`. The
static initializer at `0xE03B0` registers the `smbPitchShift` category
through `0x247110`; that registry is not reconstructed.

## BiquadFilter

`BiquadFilter::Coefs::MakeFromSettings` (`0xD9B40`, `BiquadFilter.cpp`)
resets the coefficients to a pass-through and, for enabled settings,
designs the RBJ audio EQ cookbook response at the stored sample rate. The
frequency is clamped to the Nyquist rate. The map's `_Make*Coefs` members are
inlined: low-, high- and band-pass use `mQ` as Q; the peaking filter treats
it as a bandwidth (scaled by 2 ln 2); the shelves treat it as the slope.
The gain is `10^(dB / 40)`. The string, response and test members of the
map's object are not reconstructed.

## BitCrusher

`BitCrusher` (`BitCrusher.o`, `0xDA390` to `0xDA9E5`, 176 bytes) crushes up
to 16 channels of an `AudioBuffer<float>`, planar or interleaved. Each input
sample is clamped to `[-1, 1]` and mixed with its crushed version by the wet
level. The crushed sample is the input scaled to 16 bits, with the low
`mCrush` bits cleared, then scaled back. A new crushed sample is taken only
when the hold counter is 0. The counter wraps after the sample-and-hold
factor (`SPL::Parameter`, `[1, 16]`, spec at `0x124DCF8`). `SetWet` takes a
percentage, and `Process` ramps to it over 2 ms (`Setup` sets the ramp's
increment to `500 / rate`). The constructor sets the jump flag, and nothing
clears it, so the wet level effectively always jumps. Each channel starts
from the stored counter and ramp, and the last channel's state is kept. The
16-bit scale is a guarded local static (`0x19DA850`, with its reciprocal at
`0x19DA860`). `0xDA280` comes before the object and is not part of it. It is
the growth helper of an `eastl::vector` of 40-byte allowed values, called
from `BiquadFilter.o`'s `0xD8D60`.

## DistortionEffect

`DistortionEffect` (`DistortionEffect.o`, `0xDCD50` to `0xDE919`, 1,336 bytes)
has the following members:

- a type (`SetType` clamps it to 0 to 4);
- linear input and output gains (`10^(dB / 20)`);
- a bias that is always 0;
- three `BiquadFilter::Coefs` with per-channel histories for eight channels;
- eight up- and eight downsampling `FIRFilter32`s and a one-channel
  oversampling buffer.

`SetupFilter` designs an enabled filter at the effect's sample rate and
clears its history. `SetOversample` clears the FIR histories when it turns
oversampling on. It sizes the buffer to four times `Audio::sBufferSize`.

`Process` (`0xDE8D0`) runs `_ProcessChannel` (`0xDDA50`) over each planar
channel. `_ProcessChannel` first runs the filters flagged as pre-clip. Nothing
sets that flag in this build, because this build's `SetupFilter` is shorter
than the map's. When oversampling, it calls `FIRFilter32::Upsample4x`
(`0xDE5B0`) on each sample. Every sample `x` then becomes
`curve((bias + x) * in) * out`, with these curves:

| Type | Name | Curve |
| ---: | --- | --- |
| 0 | Clean | `tanh`. |
| 1 | Warm | `sin`. |
| 2 | Dirty | Hard clip to `[-1, 1]`. |
| 3 | Soft | `x - x^3 / 3` within `[-1, 1]`, else `±0.66666`. |
| 4 | Asymmetric | Uses `0.5 x + bias`. `0.630035` from `0.320018`; `3.9375 x - 6.153 x^2` from `-0.08905`; `-0.9818` below -1; between, `0.75 ((1.032847 - abs(x))^12 + 0.333 (0.032847 - abs(x)) - 1) + 0.01`. |

When oversampling, each group of four samples goes through the channel's
downsampler. Then the remaining filters run. `Settings::GetAllowedTypes`
(`0xDD4D0`) lists the names with empty help text.

`FIRFilter32` (vtable `0x18E61A8`) indexes a 32-entry history modulo 32.
Like `FIRFilter`, all its members are inline. The map emits them weakly in
`DistortionEffect.o`, and the binary has them at `0xDE920` to `0xDEDF3`.
`Upsample4x` writes the sample and advances the index by 4, so the skipped
slots stay zero. It returns four polyphase sums of eight taps each.
`GetSample` pairs tap `i` with the sample written `32 - i` steps earlier for
`i = 0`, and `i` steps earlier otherwise. All 16 filters share the
symmetric 32-tap low-pass at `0x124E080`. The binary vectorizes the
`GetSample` reduction and drops `Upsample4x`'s initial `0 +`. This suggests
the object was built with relaxed floating-point rules.

## AmpSimulator

`AmpSimulator` (`0x1090650` to `0x1090BCA`, 56 bytes) is newer than the map.
Its name, its member names and `AmpSimulator.cpp` are inferred. It holds an
`AmpModel*` and eight parameters (presence, bass, middle, treble, master,
preamp, channel switch, output gain), which default to 6, 7, 4, 7, 6, 6, 0
and 10. It also has a flag and an int that only the constructor writes, and
a clip-hold counter.

- `SetParameter` stores the value, snapping the switch to 0 or 1. It passes
  the model a tenth of the value, or the switch value as stored.
- `GetParameter` (`0x1090880`, not called) returns ten times the stored
  value.
- `Process` lets the model render. It sets the counter to 344 blocks when
  the model's clip flag (`+0x28`) is set and counts it down otherwise.
  Without a model, it copies the first channel.
- `SetModel` deletes the old model and creates one of the five models below.
  Index 5 selects none. It then sends `mParameters[0]` to every parameter,
  which is the binary's behaviour.
- `ModelToString` (table `0x199F3F0`), `StringToModel` and
  `IsParameterUsedByModel` (`0x10908D0`) handle the `model_type` names and
  the controls each model has.

| Model | Name | Vtable | Size | Constructor |
| ---: | --- | --- | ---: | --- |
| 0 | `jcm800` | `0x199F430` | `0x3D0` | `0x1091050` |
| 1 | `ac30tb` | `0x199F470` | `0x468` | `0x1092B10` |
| 2 | `ad30` | `0x199F4B0` | `0x420` | `0x1094520` |
| 3 | `engl` | `0x199F4F0` | `0x460` | `0x1095F10` |
| 4 | `twin` | `0x199F530` | `0x468` | `0x1097B40` |

The models have no type information, so their class names are inferred.
Their slots are the destructors, `Process`, `IsParameterUsed` and
`SetParameter`. They are only declared, in `AmpSimulator.h`. Each
constructor is about 3 KiB and each `Process` over 2 KiB of filter and
convolution code. The shared convolution helpers (`0x1090BE0`, `0x1090D10`)
drive an FFT convolver (`0x1099BE0`).

## Modulation

`src/audio/core/modulation` holds the Fusion sampler's control sources.

- `ADSR` (`ADSR.o`, `0xBD740` to `0xBDBA7`). `State` (40 bytes): sample rate
  `+0x00`, level `+0x04`, stage `+0x08`, samples since the start `+0x0C`,
  settings `+0x10`, attack, decay and release rates `+0x18` to `+0x20`,
  depth mode `+0x24`. `Advance` moves through attack, decay, sustain and
  release; `GetValue` scales the level by the depth. The five editor
  `ParameterSpec`s (`0x19B00FC`) are unused.
- `LFO` (`LFO.o`, `0xBDD10` to `0xBE7BF`, 48 bytes): phase `+0x00`
  (double), output range `+0x08`, depth mode `+0x10`, settings `+0x18`,
  cycles per sample `+0x20`, seconds per sample `+0x28` and the tempo the
  increment was computed for `+0x2C`. The shapes come from
  `sApplyWaveshapingTbl` (`0x19B0140`); `Noise` is not in it. A beat-synced
  LFO scales its frequency by the tempo over 60.
- `ModulatorTarget` (`ModulatorTarget.o`, `0xBEBE0` to `0xBEF83`, 40
  bytes): name, default range magnitude, units, the float the modulators add
  to, and a depth mode (additive, subtractive or centered).
  `kDummyTarget` (`0x19B0180`) stands for no target.
- `Modulator` (`Modulator.o`, `0xBE8D0` to `0xBEBD2`, 80 bytes): the target,
  a depth `SPL::Parameter` (`[0, 1]`, 0.5) at `+0x08`, a range-magnitude
  parameter (`[0, 1000000]`, 1) at `+0x28` and the target's range for that
  magnitude at `+0x48`. `Modulate` adds the depth-shaped value mapped onto
  the range to the target's float.

The depth modes shape a value `v` with depth `d` as `d v`, `1 - d v` (the
ADSR and the LFO compute `1 + d - d v`) and `d v + (1 - d) / 2`.
`SPL::ParameterSpec` (minimum, default, maximum), `SPL::Parameter` (a
clamped value with a change callback) and `SPL::Range` (start, length) are
in `src/audio/core/dsp/ParameterSpec.h`.

## Delay

`Delay` (`Delay.o`, `0xDA9F0` to `0xDCD4F`, 512 bytes, vtable `0x18E6180`)
is a feedback delay line. Layout: channel count `+0x28`, integer sample
rate `+0x2C`, the interpolation's extra frame `+0x30`, line length (a power
of two) `+0x34`, the line `AudioBuffer<float>` `+0x38`, write position
`+0xB8` and mask `+0xBC`, the feedback, wet and dry `SPL::Parameter`s
`+0xC0` to `+0x11F`, the delay time `+0x120` and beat sync `+0x124`, the
wet, dry, feedback and delay `SPL::Ramper`s `+0x128` to `+0x1E7`, the
longest and current delays in frames `+0x1E8` and `+0x1EC`, the output gain
`+0x1F0`, tempo `+0x1F4`, speed `+0x1F8` and a first-block flag `+0x1FC`.

- `Prepare` (`0xDADD0`) sizes the line to `ceil(rate * 0.001 * maxMs) + 1`
  frames rounded up to a power of two, clears it and resets the ramps.
- The setters (`0xDB220`, `0xDCA30` to `0xDCB20`) recompute the delay as
  `rate * time`, with the time `60 / tempo * beats / speed` while synced,
  clamped to the longest delay.
- `_ApplyNewParams` (`0xDB270`) retargets changed ramps; the first block
  jumps to the targets.
- `Process` reads the line at the ramped delay with linear interpolation,
  writes `delayed * feedback + input` back and outputs
  `(input * dry + delayed * wet) * outputGain`. The gain ramps advance
  every 32 frames (20 ms ramps), the delay ramp every 2 frames (0.5 s). The
  planar overload (`0xDB510`) restarts the ramps for each channel and keeps
  the last channel's; the interleaved one (`0xDBEE0`) serves `DelayPlugin`.

The feedback spec is `[0, 1]` with default 0 (`0x124DD60`); wet and dry share
`[0, 1]` with default 1 (`0x124DD6C`), and the constructor sets both to 0.5.
`DelayPlugin` sets the output gain from decibels. `UnitTest` (`0xDCB70`) is
empty. `DelayPlugin::GetDSPDescription` (`0x27DAC0`) installs the plugin's
callbacks and is not reconstructed.

## TempoListener

This build gives `TempoListener` an object of its own (`0x5B2A0` to
`0x5B56C`, `TempoListener.cpp`); the map emits its destructor in
`DelayPlugin.o`. The owner is the `AudioEmitter` interface (`+0x228` in
`AudioEmitterCom`): its slot 1 stores itself in the listener and slot 2
removes the listener. `Unregister` (`0x5B390`) calls slot 2 under
`sCritSec` (`0x19C87A8`), which `GetCritSec` (`0x5B4C0`) returns for the
component's own locking. The object's static initializer (`0x5B4D0`) also
sets the word at `0x19C87A0` to -1, which is not modelled.

## Meter

`Meter.o` holds only the static initializer (`0xD30F0`) of
`Meter::sAudioMeterCritSec` (`0x19C9910`), defined in `Meter.cpp`.
