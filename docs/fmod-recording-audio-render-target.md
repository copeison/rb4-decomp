# FMOD recording audio render target

The concrete target at `0x275E20` combines the generic file-recording target
with an embedded FMOD audio state at object offset `0x1E0`. Its original class
name is absent from the executable, so `FmodRecordingAudioRenderTarget` is a
descriptive reconstruction. The confirmed string `RecordingAudioRenderTarget`
at `0x125D194` is the worker-thread name.

Construction initializes the generic recorder and embedded audio state,
selects the requested speaker configuration, starts the custom
`HMX.BufferedOutput` backend, and binds a small output callback object. The
callback at `0x276340` calls FMOD's `readfrommixer` function with the recorder's
floating-point buffer and frame count. This is the bridge that supplies each
recording iteration with freshly mixed samples.

The vtable begins at `0x18F0D40`. Most overrides forward to the embedded audio
state:

- `0x276130` and `0x276140` suspend and resume the low-level FMOD mixer.
- `0x276170` returns the embedded state's voice pool.
- `0x2761A0` and `0x2761D0` lock and unlock both object layers in reverse order.
- `0x276200` runs the FMOD Studio update when the state is active.
- `0x276210` and `0x276230` register or remove a mix consumer.
- `0x276250` dispatches mix buffers while both object layers are locked.
- `0x276290` returns the audio output dispatcher.
- `0x2762B0` returns the embedded FMOD audio state.

Async recording starts at `0x276080`. It reads the `async_audio_record` thread
settings, names the worker `RecordingAudioRenderTarget`, invokes the generic
recording loop through the thunk at `0x276110`, and exposes a wait operation at
`0x276120`. Destruction stops that generic recording path, unregisters the
target, tears down the worker and FMOD state, and finally destroys the base
object.

The cleaned source now uses the recovered engine thread wrapper for this
worker. It retains `std::function` for the still-semantic generic recording
callback and recursive mutexes for the observed lock order; the launch,
affinity, result, and join path match the executable.
