# AudioClipFmod lifecycle

The class name `AudioClipFmod` is embedded at `0x125CC07` and initialized by
the static registration routine at `0x266BF0`. Its active playback state keeps
a custom DSP at object offset `0x188`, a low-level channel at `0x190`, routing
handles at `0x198` and `0x1A0`, and a Studio event instance at `0x1A8`.

Low-level playback uses `audio_clip_fmod_defer_channel_release` at `0x267FB0`.
It enters the stopping state, detaches the clip from its runtime owner, clears
the DSP user-data pointer under that owner's mutex, and sends the channel/DSP
pair to the double-buffered deferred-release queue. It then clears all local
FMOD handles and marks the clip stopped. The queue later stops the channel and
releases the DSP outside the clip callback path.

Studio playback uses `audio_clip_fmod_release_event_instance` at `0x268080`.
The event callback at `0x267310` marks stop-related callbacks ready directly.
On its programmer-sound callback, it locates each custom `HMX.` DSP, attaches
the clip to the DSP user data, inserts the DSP into the event channel group,
and then marks the instance ready. Teardown clears that user data, removes the
DSP from the event's channel group, stops the event immediately, releases the
instance, and releases the DSP. If
`getChannelGroup` reports `FMOD_ERR_INVALID_HANDLE`, FMOD has already
invalidated the event-owned objects, so the method only clears its pointers.

`audio_clip_fmod_stop_and_wait` at `0x2682A0` prevents concurrent updates and
runs both release paths. When a Studio system is available, it sleeps for one
millisecond and calls `Studio::System::update` until the event callback permits
teardown and the state reaches stopped. If the audio system is unavailable,
it detaches the runtime link and clears the stale handles without calling into
FMOD.

This analysis also corrects the FMOD 1.10 result values used by the source
model: error 30 is `FMOD_ERR_INVALID_HANDLE`, while error 31 is
`FMOD_ERR_INVALID_PARAM`. The file callbacks return 31 for null callback
arguments; the Studio event teardown specifically tests 30.
