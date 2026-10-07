# AudioClipFmod lifecycle

The class name `AudioClipFmod` is embedded at `0x125CC07` and initialized by
the static registration routine at `0x266BF0`. Its active playback state keeps
a custom DSP at object offset `0x188`, a low-level channel at `0x190`, routing
handles at `0x198` and `0x1A0`, and a Studio event instance at `0x1A8`.

`audio_clip_fmod_start` at `0x266FD0` first initializes the shared audio-clip
state, creates the clip's custom DSP, and stores the clip as DSP user data. A
Studio-event route is used only when the requested event exists and is not a
one-shot event. That path creates an event instance, applies the requested
key/value parameters, installs callback mask `0x182`, applies the initial
paused state, and starts the instance.

Missing and one-shot events fall back to the low-level path. The same fallback
is the normal path for unqualified clips: FMOD starts the custom DSP on a
paused channel, then optionally routes it through a requested Studio bus. A
spatial clip enables FMOD mode `0x10` and applies its 3D spread. Parent channel
group routing, the channel's base frequency, and the requested pause state are
set before the clip joins its runtime owner's active list.

The runtime controls share the same channel/event split. Pause and resume at
`0x267BB0` and `0x267C00` call the matching FMOD object and update the clip
state. Position accessors at `0x267C60` through `0x267D64` use
`FMOD_TIMEUNIT_MS` for low-level channels and Studio timeline milliseconds for
events. The 3D updater at `0x267E70` rebuilds the engine transform in FMOD's
coordinate system and sends either the position/velocity pair or the complete
Studio attributes. The wrappers at `0x2683F0` and `0x268410` set and get named
Studio event parameters.

Low-level playback uses `audio_clip_fmod_defer_channel_release` at `0x267FB0`.
It enters the stopping state, detaches the clip from its runtime owner, clears
the DSP user-data pointer under that owner's mutex, and sends the channel/DSP
pair to the double-buffered deferred-release queue. It then clears all local
FMOD handles and marks the clip stopped. The queue later stops the channel and
releases the DSP outside the clip callback path.

Studio playback uses `audio_clip_fmod_release_event_instance` at `0x268080`.
The event callback at `0x267310` marks stop-related callbacks ready directly.
On its programmer-sound callback, it locates each custom `HMX.` DSP, attaches
the clip to the DSP user data, and invokes the clip's attachment notification
when requested by that payload. It takes the audio system sample rate as the
clip's base frequency, inserts `HMXRawAudioBus` at
`FMOD_CHANNELCONTROL_DSP_HEAD`, applies parent-group routing, and joins the
runtime owner's active list before marking the instance ready. Teardown clears
that user data, removes the DSP from the event's channel group, stops the event
immediately, releases the instance, and releases the DSP. If
`getChannelGroup` reports `FMOD_ERR_INVALID_HANDLE`, FMOD has already
invalidated the event-owned objects, so the method only clears its pointers.

The shared HMX Studio DSP payload is only partially named. Its fields at
offsets `0x10` and `0x18` are proven to be the active clip pointer and a
non-null attachment-notification flag. The leading 16 bytes remain reserved
until a plugin constructor or owner establishes their meaning.

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
