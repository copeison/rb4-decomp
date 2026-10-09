# FmodAudioBusGenerator

`FmodAudioBusGenerator` plays an `AudioBus` source through a custom FMOD DSP
named `HMXRawAudioBus`. It derives from `AudioBusGenerator`, which derives
from `AudioGenerator` and `AudioBusCallable`. The primary vtable is at
`0x18F0208`, the `AudioBusCallable` vtable at `0x18F0338`, and a pool entry is
`0x1C0` bytes. The source is in
`src/audio/fmod/playback/FmodAudioBusGenerator.cpp`.

Earlier reconstructions called this class `AudioClipFmod`. The string
`AudioClipFmod` at `0x125CC07` belongs to a separate scene component whose
static registration is at `0x266BF0`.

## Layout

`AudioBusGenerator` occupies the first `0x188` bytes. The FMOD fields follow:

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x78` | `mMixer` | The render target's `AudioMixer`. |
| `+0x98` | `mChannelData` | Planar output of the last rendered block. |
| `+0xFD` | `mHasSamples` | Set when a block is ready for FMOD. |
| `+0x100` | `mSource` | The `AudioBus` being rendered. |
| `+0x188` | `mDSP` | `HMXRawAudioBus` instance. |
| `+0x190` | `mChannel` | Low-level channel playing the DSP. |
| `+0x198` | `mChannelGroup` | Studio bus channel group. |
| `+0x1A0` | `mStudioBus` | Optional Studio bus route. |
| `+0x1A8` | `mEventInstance` | Optional Studio event hosting the DSP. |
| `+0x1B0` | `mEventCallbackDone` | Set by the event callback. |
| `+0x1B4` | `mFrequency` | Channel or system sample rate. |
| `+0x1BC` | `mKilling` | Set while `Kill` runs. |

The old reconstruction gave its bus-generator subclass a sound, a channel and
length fields. Those belong to `FmodAudioStreamGenerator`: the function at
`0x268EF0` writes them at offsets `+0x50` onward, which in this class is the
`AudioBusCallable` base. It is `FmodAudioStreamGenerator::Setup`.

## The DSP

`sDspDescription` at `0x19B4840` is an FMOD plug-in SDK 110 description with
one output buffer. `_DspCreate` at `0x266CB0` fixes the format at stereo.
`_DspProcess` at `0x266CD0` reads the generator from the DSP user data and,
under the mixer lock, interleaves the last rendered stereo block into FMOD's
output; without a ready block it writes silence. A query reports one stereo
buffer.

## Setup

`Setup` at `0x266FD0` first runs `AudioBusGenerator::Setup`, which binds the
render target's mixer and the source. It creates the DSP on the low-level
system resolved from the render target and stores the generator as DSP user
data.

A Studio-event route uses a non-oneshot event when one exists. The generator
creates an instance, applies the request's parameters, installs
`_EventProgrammerCallback` for the destroy and programmer-sound callbacks,
applies the start-paused flag and starts the event. Any other request plays
the DSP on a paused low-level channel, optionally routed through a Studio
bus. A positional emitter enables `FMOD_3D` and the requested spread. The
channel's frequency is recorded, the pause state applied, and the generator
joins the mixer's pending callables.

When the emitter reports a mix group, the code routes through that group's
channel group, but the getter is inlined as null in this build. The binary
therefore calls `ChannelGroup::addGroup` and `Channel::setChannelGroup` with a
null group; the reconstruction keeps the call.

`_EventProgrammerCallback` at `0x267310` marks the event safe to release on
the destroy callbacks. On `CREATE_PROGRAMMER_SOUND` it points every `HMX.*`
DSP in the event's channel group at the generator, takes the system sample
rate as the frequency, inserts the DSP at `FMOD_CHANNELCONTROL_DSP_TAIL`
(`-3`), and joins the mixer. The earlier reconstruction named this index
`DSP_HEAD`; FMOD's head is `-1`.

## Teardown

`_StopChannel` at `0x267FB0` detaches the generator from the mixer, clears the
DSP user data under the mixer lock, and queues the channel and DSP with
`FModSystem::DeferRelease`. `_StopEventInstance` at `0x268080` waits for the
destroy callback, removes the DSP from the event's channel group, stops and
releases the event and releases the DSP. An `FMOD_ERR_INVALID_HANDLE` from
`getChannelGroup` means FMOD already freed the event's objects.
`_StopChannelAndEventInstance` at `0x267E50` runs both.

`Stop` at `0x267F10` marks the voice stopping, except while a fade to silence
runs, when it only refreshes the fade step from the source's block rate.
`Poll` at `0x267D70` finishes a pending stop or updates the 3D attributes.
`Kill` at `0x2682A0` stops both paths and pumps Studio updates, sleeping 1 ms
each time, until the voice is stopped. Without a live FMOD system it only
drops its handles.

## Manager

`FmodAudioBusGeneratorManager` returns null from `Play`; bus generators are
taken through `_GetGenerator` at `0x268B40` by generators that render buses,
such as `FmodBufferedStreamGenerator`.
