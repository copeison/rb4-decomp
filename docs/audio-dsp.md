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
