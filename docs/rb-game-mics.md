# Game microphones

The game's microphone manager and its microphones. Names follow
[naming.md](naming.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_game/GameMicManager.o` | `0x9BF460`-`0x9BFED0` | `src/rb_game/mics/GameMicManager.{h,cpp}` |
| `rb_game/GameMic.o` | `0xA38BA0`-`0xA399F0` | `src/rb_game/mics/GameMic.{h,cpp}` |

## GameMicManager

112 bytes. Bases `MicManagerInterface` (the vtable at `0x19628B8` has its
five slots: destructors, `HandleMicsChanged`, `SetPlayback`,
`GetEnergyForMic`) and `MsgSource` at offset 8 (`0x19628F0`). `Init`
(`0x9BF490`) sets `TheGameMicManager` (`0x1AE6CA0`), registers `Terminate`
(`0x9BF600`) as an exit callback and hands the manager to
`theMicClientMapper->SetMicManager`. The manager keeps one `GameMic*` for
each of four microphone ids; `HandleMicsChanged` creates the connected ones,
deletes the others and exports `GameMicsChangedMsg` ("game_mics_changed",
vtable `0x1962938`, a `Message<0>`).

Everything is reconstructed. `ApplyPlayback` (`0x9BFDA0`) starts or stops
the playback of the channel `GameMic::GetMyMic` returns (`MicChannel` slots
13 and 14); `CreateMic`, `HandleMicsChanged` and `SetPlayback` inline it in
the binary and call it in the source. The map's fake microphones, effects,
proximity and pitch-correction members are not in this build. Weak names:
the option flags and effect values at +48-+67 (`mFxValues[0]` is the
sensitivity `GameMic` uses while `mHighSensitivity` is on) and
`mPlaybackPending` (+0x69, never set in this object). The static
initializer (`0x9BF460`) builds header statics nothing reads.

## GameMic

Not polymorphic, 32864 bytes: the microphone id, a recorder slot among six
static flags at `0x1AEF328`, the input WAV stream (`SetInputFile`), the
recording stream ("mic_output_fonix.wav", "mic_record_%d.raw"), a
`PitchDetector`, the detected and published peak, energy and pitch (the
energy at +0x44 is read by `GameMicManager::GetEnergyForMic`) and two
sample buffers. The member order matches the map. Every function of the
object is reconstructed, including the map's `GetDataSampleRate`
(`0xA39750`) and the uncalled `NormalizingCopy` (`0xA39790`). The fields
at +0x20 and +0x48 are now the detector's and the published peak (floats).
The wave writer the destructor calls (`0x111D110`) is declared in
`GameMic.cpp` as `WriteWaveFile` (its object is not identified).

## Microphone channels

`mic/core/MicChannelMgr.h` declares the game's microphone channels: the
0x20740-byte `MicChannelMgr` at `0x1ADF350` (created at `0x8EC9C0`, vtable
`0x195FEE0`, 13 slots following the map's `MicHwManager_FMOD`: `GetMic` is
slot 8) and its four `MicChannel`s (vtable `0x195FF58`, 24 slots following
the map's `Mic`/`Mic_FMOD`: `SetMute` 7, `SetVolume` 10, `StartPlayback`
13, `StopPlayback` 14, `GetRecentBuf` 16, `GetContinuousBuf` 17,
`GetDroppedSamples` 18, `GetSampleRate` 19, `GetName` 23; the gain at
+0x14). The class names are inferred; slots 11, 12 and 15 are weak.
