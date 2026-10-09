# FMOD Studio generators

## FmodStudioSoundGenerator

`FmodStudioSoundGenerator` plays one FMOD Studio event. Its vtable is at
`0x18F09F0`, the manager's at `0x18F0B00`, and a pool entry is `0xD0` bytes.
The source is in `src/audio/fmod/playback/FmodStudioSoundGenerator.cpp`.

`FmodStudioSoundGeneratorManager::Play` at `0x270B20` rejects dialog
requests (format 4) and requests whose render target has no Studio system.
It keeps names that already carry an `event:` or `snapshot:` scheme and
prefixes others with `event:/`, in a 256-character stack string. The event
must resolve with `lookupID` before a voice is taken. A voice whose setup
fails is not returned to the pool.

`_Setup` at `0x26FC30` resolves the event, records its length, creates the
instance and stores the user data: the generator, or the dialog generator
that embeds it. It resets both gain ramps, applies the initial gain, mute and
parameters, installs `_EventCallback` (or the caller's callback) for every
event callback type, and starts the event, paused if requested. An unpaused
3D voice applies the emitter's attributes first.

`_EventCallback` at `0x26FF50` marks the event safe to release on the
destroy and stop callbacks. On the start callback it points the event's
`HMX.*` DSP plugins at the generator.

`Poll` at `0x2702C0` releases a stopped event and reports the voice
stopped. A dialog event that is still stopping is cut immediately. Otherwise
it refreshes the 3D attributes, advances the gain and mute ramps when the
timeline has moved, and applies their product as the event volume while
playing.

`Stop` at `0x270620` stops a paused event immediately and lets a playing
event fade. `Kill` at `0x2706A0` removes the callback, stops the event, and
polls and flushes Studio commands until the stop callback arrives.
`_ReleaseEvent` at `0x270660` stops and releases the event at once.

## FmodDialogGenerator

`FmodDialogGenerator` plays spoken dialog whose lines are programmer sounds.
It derives from the engine's `DialogGenerator`, which forwards the name of
every created sound to a sink supplied with the request. Its vtable is at
`0x18F08D8`, the manager's at `0x18F0838`, and a pool entry is `0x1A0` bytes.
The type name comes from the string at `0x125CF45`.

The generator embeds a `FmodStudioSoundGenerator` at `+0x88` and delegates
pause, resume, stop, positions, parameters, gain, mute, `Poll` and `Kill` to
it. `Setup` at `0x26E990` accepts only format 4, keeps the request's sink,
sets a one-hour placeholder length, and starts the embedded generator with
`_ProgrammerSoundCallback` and itself as user data.

`_ProgrammerSoundCallback` at `0x26EA10` creates each programmer sound from
the Studio sound table as a nonblocking, accurate-time compressed sample,
returns its subsound index and passes its name to the sink. It releases the
sound on the destroy callback and replaces the placeholder length when FMOD
reports the sound playing. Once the voice is stopping, every callback only
marks the event safe to release.

`FmodDialogGeneratorManager::Play` at `0x26EC60` only adds `event:/` when the
name has no scheme. `Release` releases the embedded event before returning
the voice.
