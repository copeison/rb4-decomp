# HMX.BufferedOutput

The executable contains a statically registered FMOD output plug-in named
`HMX.BufferedOutput`. Its descriptor begins at `0x19B4948`, and its six
implemented callbacks are between `0x2763F0` and `0x276520`. The cleaned
reconstruction is in `src/audio/fmod/io/fmod_buffered_output.cpp`.

The descriptor uses output plug-in API version 3, plug-in version 1, and the
direct-mix method. Its start, stop, mixer, 3D-object, and other optional
callbacks are null. The 168-byte descriptor layout and observed callback
offsets are preserved in `fmod_api.h`.

The plug-in exposes 128 virtual drivers. Driver names use the pattern
`null_output_%d`; every driver reports the engine sample rate, stereo speaker
mode, and two channels. Initialization stores the owning audio state in
`FMOD_OUTPUT_STATE::plugindata`, selects floating-point PCM output, and copies
the configured speaker mode and raw-speaker count. Close and get-handle are
successful no-ops.

The update callback retrieves the audio state from `plugindata` and dispatches
to its configured engine callback, returning that callback's result to FMOD.
The callable wrapper's class layout is not yet reconstructed, so the cleaned
source represents its virtual invocation with a named adapter.

`fmod_audio_initialize_custom_output` at `0x2786D0` registers this descriptor,
selects the returned plug-in handle, applies the requested DSP buffer and
software format, and initializes FMOD with synchronous Studio updates plus
the low-level stream-from-update, mix-from-update, and right-handed 3D flags.
It then registers the same 12 DSP descriptions, file callbacks, and pre/post
mix callback used by the primary output path.
