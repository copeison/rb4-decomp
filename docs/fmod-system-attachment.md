# FMOD system attachment

`fmod_audio_attach_studio_system` at `0x277840` connects the audio state to an
already-created FMOD Studio system. This path is separate from both local
startup modes and does not initialize or take ownership of the supplied
system.

Attachment retrieves the low-level system, queries its version and prior user
data, then installs the audio state as user data on both FMOD objects. It reads
the active driver and software format, stores the final sample rate and DSP
buffer length, forwards the sample rate to the 128-sample output dispatcher,
and registers all 12 custom DSP descriptions. Finally, it enables mix
callbacks, performs one Studio update, and leaves release ownership with the
caller.

The locally initialized path also passes the discovered sample rate and DSP
buffer length to `audio_set_mix_format` at `0xD3BC0`. That helper caches the
sample rate, seconds per sample, buffer length, buffers per second, and
milliseconds per buffer used across the audio engine.

Passing a null system takes the detach path. It waits for the mix semaphore,
marks the audio state as shutting down, clears both deferred FMOD release
queues under their mutex, nulls the Studio and low-level system pointers, and
releases the semaphore. `fmod_audio_detach_studio_system` at `0x277A80`
contains the same detach sequence as a standalone method.

The clean reconstruction uses `audio_clear_deferred_fmod_releases` as an
adapter because the queues contain an unreconstructed pair of FMOD channel and
DSP ownership records. Their exact EASTL layout is preserved in the IDA
export, while the public lifecycle behavior is explicit in
`src/audio/fmod/system/fmod_audio_system.cpp`.
