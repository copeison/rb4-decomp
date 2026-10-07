# FmodAudioStreamResource

The string at `0x125D053` identifies `FmodAudioStreamResource`, whose vtable
begins at `0x18F0BA0`. Type metadata at `0x271D00` registers `mp3`, `wav`,
`aac`, `ogg`, and `m4a` under the `Streaming Audio` category.

Loading at `0x272340` resolves the platform asset path and checks that it
exists. It then opens a temporary FMOD streaming sound and reads its format.
The direct PCM path accepts signed 16-bit mono or stereo data. A missing file,
unavailable FMOD system, or another sample format records distinct status values
1, 2, and 3; status 0 is the supported path.

Successfully opened resources, including resources marked with unsupported
format status, enter a mutex-protected map keyed by their normalized resolved
path. Registration is at `0x271B20`, lookup at `0x271C20`, and destruction
removes every map entry that points at the resource through `0x271830`.

`0x2720F0` first asks the engine resource system for an existing object and
constructs and loads a 104-byte resource only when necessary. The cleaned
source represents the recovered FMOD probing and registry behavior; the engine
resource reference-counting layer remains expressed by its semantic ownership.
