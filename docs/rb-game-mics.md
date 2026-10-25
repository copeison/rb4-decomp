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

Everything is reconstructed except `ApplyPlayback` (`0x9BFDA0`), which
calls slot 13 or 14 of the microphone channel `GameMic::GetMyMic` returns;
`CreateMic`, `HandleMicsChanged` and `SetPlayback` inline it in the binary
and call it in the source. The map's fake microphones, effects, proximity
and pitch-correction members are not in this build. Weak names: the option
flags and effect values at +48-+67 and `mPlaybackPending` (+0x69, never set
in this object). The static initializer (`0x9BF460`) builds header statics
nothing reads.

## GameMic

Not polymorphic, 32864 bytes: the microphone id, a recorder slot among six
static flags at `0x1AEF328`, the input WAV stream (`SetInputFile`), the
recording stream ("mic_output_fonix.wav", "mic_record_%d.raw"), a
`PitchDetector`, the published energy (+0x44, read by
`GameMicManager::GetEnergyForMic`) and two sample buffers. The member order
matches the map. Reconstructed: `Reset`, `AccessContinuousSamples`,
`KillThread`, `StartDetection` and `UpdateThread`. Declared: the
constructor, destructor, `SetInputFile`, `GetMyMic`,
`SetEnablePitchDetection`, `SetMute`, `ThreadProcessOneFrame` and `Update`,
which use the game's microphone channels: the 0x20740-byte object at
`0x1ADF350` (created at `0x8EC9C0`, vtable `0x195FEE0`, `GetMic` in slot
8), whose channels (vtable `0x195FF58`) are not modelled.
