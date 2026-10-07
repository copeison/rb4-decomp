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

Each pool entry contains an event runtime beginning 136 bytes into the object.
That runtime owns the Studio event instance and its fade state. Event creation,
callbacks, playback controls, and fade behavior remain the next reconstruction
milestone.
