# FMOD resources

Both FMOD resources derive from the engine's `Resource`, whose entity module
is not reconstructed; `src/audio/core/resources/Resource.h` declares only the
members they use. The sources are in `src/audio/fmod/resources`.

## FmodAudioStreamResource

A streamed audio file. The vtable is at `0x18F0BA0` and the object is
`0x68` bytes. `_Init` at `0x271D00` registers the `mp3`, `wav`, `aac`, `ogg`
and `m4a` extensions under the `Streaming Audio` category.

| Offset | Field | Meaning |
| ---: | --- | --- |
| `+0x30` | `mFile` | Resolved file; the empty symbol when unavailable. |
| `+0x38` | `mLengthMs` | Cached length, `-1` until measured. |
| `+0x3C` | `mStatus` | 0 ok, 1 file not found, 2 no sound, 3 unsupported format. |
| `+0x40` | `mDecodeFailed` | Set when a decode block is rejected. |
| `+0x48` | `mAsyncCritSec` | Guards the decode in progress. |
| `+0x58` | `mAsyncProcess` | Running `_FMODSoundAsyncSampleProcessor`. |
| `+0x60` | `mSound` | Stream kept open by `OpenSound`. |

`LoadFile` at `0x272340` resolves the file and checks that it exists, opens a
temporary stream to read its format, and accepts 16-bit mono or stereo PCM.
`Fail` reports a resource whose file did not resolve. A resource that opened,
even with an unsupported format, is registered under its file name, the key
that playback requests use; `Find` at `0x271C20` looks it up and destruction
removes every entry through `0x271830`. `GetOrLoad` at `0x2720F0` returns a
loaded resource or creates and loads a new one.

`GetLengthMs` at `0x272770` measures and caches the length, opening the
stream only for the measurement when it is not already open.

### Decoding to PCM

`_FMODSoundAsyncSampleProcessor` is a `ThreadCallback` (vtable `0x18F0C18`)
that decodes a stream for an `AsyncSampleProcessor`. Its decode buffer is
tagged `FMODSoundToPCMCallback`, the name earlier reconstructions used for the
class. `FmodAudioStreamResource::StartAsyncSampleProcessor` (`0x272B80`)
resets the processor chain (`ChainReset`), creates the sound and queues the
work item unless the creation failed (`mCreateFailed`). `ThreadStart` at
`0x272F60` waits for the sound to open unless cancelled, announces the
sample rate, format, channel count and length (`ChainInit`), and feeds one
block at a time (`ChainProcessSampleFrames`). A rejected block marks the resource as failed
and cancels the decode. The processor is told the decode finished or was
cancelled, and `ThreadDone` deletes the work item. Its destructor clears the
resource's pointer to it under the resource lock.
`StopAsyncProcess` at `0x272C60` cancels a running decode and can wait for it.

## FModBankResource

A Studio bank loaded into every Studio system. The vtable is at
`0x18F0C78`; `_Init` at `0x273AF0` registers the `bank` extension under the
`FMod Banks` category. Every live bank is also kept in a global list at
`0x19F2E50`. `Fail` reports a resource with no loaded banks.

The loaders use EASTL maps and are only declared:

- `_ResolvePlatformPath` at `0x274710` replaces a `desktop` or `Desktop` path
  component with `PS4` and localizes `_eng.bank` and `/eng.bank` through the
  sound manager.
- `_Load` at `0x274200` loads the bank into each Studio system through
  `_LoadIntoSystem` at `0x274A80`, which loads the sample data, updates Studio
  until it is loaded, locks every bus channel group and flushes commands.
- `_UnloadAll` at `0x273980` unloads each bank and keeps updating its Studio
  system while it reports `UNLOADING`.
- `GetEvents` at `0x273C10` and `GetBuses` at `0x273F00` list up to 2,048
  event or bus paths from the first loaded bank.
- The helpers at `0x274920`, `0x274C40` and `0x274CF0` derive the localized
  master bank, the strings bank and the master bank paths.
