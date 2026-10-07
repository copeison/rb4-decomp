# Audio speaker configuration

`fmod_audio_configure_speakers` at `0x2783A0` translates the engine's four
speaker configurations into the fields passed to FMOD's software-format and
custom-output initialization calls.

| Engine configuration | FMOD speaker mode | Raw channels |
| --- | --- | ---: |
| Mono (`0`) | `FMOD_SPEAKERMODE_MONO` (`2`) | 1 |
| Stereo (`1`) | `FMOD_SPEAKERMODE_STEREO` (`3`) | 2 |
| 5.1 (`2`) | `FMOD_SPEAKERMODE_5POINT1` (`6`) | 6 |
| 7.1 (`3`) | `FMOD_SPEAKERMODE_7POINT1` (`7`) | 8 |

Values outside zero through three leave the current speaker mode and channel
count unchanged. The mapping is stored as two four-entry tables at
`0x125D1F0` and `0x125D200` in the executable. The clean reconstruction uses
an explicit switch in `src/audio/fmod_audio_system.cpp`.
