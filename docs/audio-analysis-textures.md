# Audio-analysis render textures

The render system owns a pointer at offset `0xDF8` to the 72-byte
`AudioAnalysisTextureSet`. The object uses two inline texture-pointer slots and
an overflow-capable float vector beginning at object offset `0x28`. Its
constructor, destructor, frame preparation, rebuild, update, and width query
are the function family at `0x457620` through `0x458540`.

Frame preparation queries eight audio sources and records the maximum width
requested for each of two channels. A positive width rebuilds its texture when
the slot is empty or the existing texture width differs. The rebuilt resources
are 2D float textures named `Audio Analysis`, with eight rows populated by
replicating each source's channel values across the requested row width.

After rebuilding when needed, the update path copies current source values
into each texture payload. It then invokes texture vtable slot `0x90` for each
active texture with the current render context. The common frame path now owns
the width check and rebuild decision directly; data extraction and texture
population stay behind focused adapters while their audio-source types remain
unidentified.

## Reconstruction

The audio analysis has eight slots, `AudioAnalysis::sSlots` at `0x19C9960`, each
0x21D8 bytes. They are the shared state of the "HMX.Analysis" FMOD DSP, and an
`AnalyzerCom` component sets them up. Each slot holds:
- a VU meter;
- an FFT of power-of-two size;
- an eight-band filter bank;
- a semitone filter bank.

The map has none of these names; the source calls the class `AudioAnalysis`
and places it in `src/audio/core/analysis`.

`RndAudioTextures` builds two float textures with one row per slot:
- **Texture 0** holds the FFT bins; its width is the largest FFT size.
- **Texture 1** holds the semitone bank; its width is the largest semitone
  range.

The source is laid out as follows:
- `GetRequestedWidths` (`0x4584B0`) computes both widths.
- `Rebuild` (`0x4578B0`) recreates the textures with a placeholder pattern.
- `Update` (`0x458110`) copies each active slot's results into its row, then
  syncs the textures.

Three binary quirks are kept:
- `Rebuild` leaves deleted texture pointers in place.
- It writes eight placeholder values even into a shorter vector.
- `Update` does not check for missing textures.
