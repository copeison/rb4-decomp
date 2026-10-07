# FmodDialogGenerator

The strings at `0x125CF1C` and `0x125CF45` identify
`FmodDialogGeneratorManager` and `FmodDialogGenerator`. Their primary vtables
begin at `0x18F0838` and `0x18F08D8`.

The manager accepts resource options whose format field is 4 and reports
`.bank` as its extension. Creation at `0x26EC60` preserves paths that already
use the `event:/` namespace and prefixes ordinary event names with `event:/`.

Pool setup at `0x26F280` allocates 416 bytes per generator. Retention at
`0x26EFD0`, reset and stop passes at `0x26F040` and `0x26F0B0`, active-handle
enumeration at `0x26F120`, return at `0x26EBE0`, and shutdown at `0x26F420`
use the same synchronized free-list and generation-handle scheme as the other
FMOD generator managers.

Each pool entry embeds a `FmodStudioSoundGenerator` beginning 136 bytes into the
object. Its name is present at `0x125CFB3`, and its vtable begins at
`0x18F09F0`. The wrapper delegates pause, resume, timeline position, named
parameters, fades, update, reset, and synchronous stop operations to it.

Initialization at `0x26FC30` resolves the Studio event, reads its timeline
length, creates an instance, installs initial parameter values and a callback,
applies 3D attributes, and starts either playing or paused. Runtime update at
`0x2702C0` follows Studio playback state, refreshes 3D attributes, advances two
independent linear fades from timeline movement, applies their product as event
volume, and releases a stopped event.

The dialog callback at `0x26EA10` handles FMOD programmer-sound creation and
destruction. It looks up `FMOD_STUDIO_SOUND_INFO`, creates the low-level sound,
returns its subsound index, and replaces the wrapper's initial 3,600,000 ms
placeholder length when FMOD reports the sound actually playing.
