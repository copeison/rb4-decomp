# FMOD audio initialization

The primary audio startup routine at `0x2773C0` uses the FMOD Studio 1.10.04
API. Its cleaned reconstruction is in `src/audio/fmod_audio_system.cpp`, with
the required ABI subset declared in `src/audio/fmod_api.h`.

## Platform setup

Before creating an FMOD system, the function at `0x261F60` loads
`/app0/libfmod.prx` and `/app0/libfmodstudio.prx` with
`sceKernelLoadStartModule`. It looks up the platform affinity groups
`audio_render` and `mic_reader`, assigns the audio-render mask to the FMOD
worker threads, assigns the microphone-reader mask to the recording thread,
and submits the table through `FMOD_Orbis_SetThreadAffinity`.

The field names in FMOD's private Orbis affinity structure are not available,
so the cleaned source does not yet declare that table. The module names,
configuration keys, masks, and call order are visible directly in the
executable.

## Primary system creation

`fmod_audio_initialize` creates a Studio system with header version
`0x00011004`, stores the owning audio object as user data on both the Studio
and low-level systems, and requests the configured output type. If that output
is unavailable it retries with `FMOD_OUTPUTTYPE_AUTODETECT`.

The routine then changes three queue and thread settings before initialization:

| Settings block | Field | Change |
| --- | --- | --- |
| Low-level | `stackSizeMixer` | Add 16 KiB. |
| Low-level | `commandQueueSize` | Multiply by four. |
| Studio | `commandqueuesize` | Multiply by four. |

Both settings structures are zero-initialized and begin with their exact byte
size: 120 bytes for `FMOD_ADVANCEDSETTINGS` and 20 bytes for
`FMOD_STUDIO_ADVANCEDSETTINGS`. The reconstruction preserves those sizes and
the two observed low-level field offsets with compile-time assertions.

After selecting the requested software-channel count, the routine initializes
Studio with normal Studio and low-level flags. On success it installs the
game's synchronous and asynchronous file callbacks, reads the active driver
sample rate and DSP-buffer length, initializes the engine audio clock, and
registers the low-level callback for `FMOD_SYSTEM_CALLBACK_PREMIX` and
`FMOD_SYSTEM_CALLBACK_POSTMIX`.

## Custom DSP plugins

The helper at `0x278270` registers these descriptions in order:

1. `HMX.Analysis`
2. `HMX.BitCrusher`
3. `HMX.Delay`
4. `HMX.Filter`
5. `FMOD Gain`
6. `HMX.SignalTap`
7. `HMX.SmbPitchShift`
8. `HMX.Stutter`
9. `HMX.SoundClashSlot`
10. `HMX.Tremolo`
11. `HMX.Vibe`
12. `HMX.Wah`

The getter order is exact. Plugin names come from their corresponding DSP
description data; the gain entry uses the standard FMOD name even though the
game supplies its own description getter.

## Other initialization modes

`fmod_audio_attach_studio_system` at `0x277840` attaches an externally created
Studio system and rebuilds the local driver, clock, plugin, semaphore, and
callback state. `fmod_audio_initialize_custom_output` at `0x2786D0` registers
the game's output plugin, applies stored DSP-buffer and software-format values,
then initializes Studio with non-default flags. These paths are exported as
analysis evidence but remain separate from the primary cleaned routine until
their owning state layout is reconstructed.
