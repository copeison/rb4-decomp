# Audio components

The emitter and listener components of the map's `audio` module are real
`Component` classes (see [entity-components.md](entity-components.md)).
Their sources are in `src/audio/core/components`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `audio/AudioEmitterCom.o` | `0x31D70`-`0x392DF` | `0x18DF2E8` (59 slots) | 568 |
| `audio/AudioListenerCom.o` | `0x392E0`-`0x39F5F` | `0x18DF990` (41 slots) | 88 |

## Class registration

Each class has `sId` and `sClassName` (both `"AudioEmitter"` and
`"AudioListener"`), `sPropRegistry` and `sMetaData`:

| Static | AudioEmitterCom | AudioListenerCom |
| --- | ---: | ---: |
| `sId` | `0x19C7768` | `0x19C7C80` |
| `sClassName` | `0x19C7770` | `0x19C7C88` |
| `sPropRegistry` | `0x19C7780` | `0x19C7C90` |
| `sMetaData` | `0x19C7820` | `0x19C7D30` |
| `_Init` | `0x32110` | `0x39420` |
| `Init` (in `SoundManager.o`) | `0x42F0` | `0x4B70` |
| `_Create` (in `SoundManager.o`) | `0xCF80` | `0xD280` |

`sClassName` is the symbol `GameObject::CreateComponent` takes; it replaces
the former `gAudioEmitterComClass` and `gAudioListenerComClass`. The
identity slots (`GetId` through `AsComponent`, `_GetPropRegistry`,
`_GetMetaData`) and the factories are reconstructed. `IsA` walks
`sMetaData` and its superclasses.

The inline `Init` pushes the "metadata" heap, sets the metadata's
`mCreatable` and `mSuperclassRoot`, runs `_Init`, records the class size in
a shared size map, fills `Component::sFactory` and the placement map after
it (`0x19E2938`, the map's `Component::sVtables`) and calls
`ComMetaData::Init` with a temporary component's `_HasPostPoll`. `Init`,
`_Init` and `_Imprint` stay declared: the size map, the property metadata's
attributes and the thread's imprint state are not modelled.
`SoundManager::_InitComponents` calls `AudioEmitterCom::Init` and
`FusionPatchCom::Init` in the binary's order.

## AudioEmitterCom

The properties are `is_named_emitter` (`+0x16`, in `Component`'s tail
padding), `emitter_name`, `is_2D`, `allow_dialog_overlap` and the
`tempo_aware_buses` struct (`TempoAwareBusProps`, `+0x38`: a
`use_preloaded_bank` flag and a `PropArray` of `bus`/`preloaded_bus`
pairs). `RuntimeData` (`+0x68`, constructor `0x37710`) holds the song and
content positions, the section name, the tempo and speed sent to tempo
listeners, the master music handle, the world transform, the
`CompositeGenerator` (`+0xE8`), the tempo listener list and its lock, the
joypad listener, the dialog line's handle and flag, the tempo-aware bus
lists, the render target and the `AudioEmitter` interface (`+0x228`) with
its back pointer.

Slots 41-58 are the component's own: `HasMixGroup` (false; weak name), the
`PlayMusic` and `PrepareMusic` overloads, `GetMasterMusic`, the music
position getters, the two `PlayDialog` overloads, the dialog queries,
`SetAllowDialogOverlap`, `AddGenerator` and `_Poll(Transform const&)`. The
poll copies the transform, polls the composite generator, binds the
tempo-aware FMOD buses through the bus interface's slot 8 and follows the
master music's position, section and tempo.

A named emitter joins `sNamedEmitters` (`0x19C7730`, guarded at
`0x19C7720`) under its emitter name when it enters, but only for names
`AddGameWideEmitterName` (`0x35450`) created; `GetGameWideEmitter`
(`0x35820`) returns the last one.

Not reconstructed: `Handle` (`0x35B40`), `_Imprint`, the
`PlayDialog(Symbol, ...)` overloads (`0x34D90`, `0x37BE0`), the
`RuntimeData` copy constructor (`0x37F70`), the editor enumeration helper
at `0x358E0`, the `_Init` accessors and lambdas (`0x37D70`-`0x390B0`) and
`DefaultEmitterProxyCom` (`0x36B50`-`0x37710`), which the object also
holds.

## AudioListenerCom

One listener is active (`sActiveListener`, `0x19C7EE8`): the first to enter
or poll, or the one whose scene entity sends `entity_active_changed` with a
true value. The active listener's poll moves the sound manager's listener
to its object's `TransCom` transform under `sActiveListenerLock`
(`0x19C7ED8`). `_Enter` subscribes through an embedded
`MsgSource::EventSinkElem` (`+0x30`) to the entity that `FindSceneEntity`
(`0x398B0`) finds: the nearest entity, through the instancing objects,
whose resource is a `RndSceneResource`. `SetHardwareMapId` stores the
pad's id (`+0x18`); `+0x1C` is the flag `HasJoypadEmitter` reads.

## Weak evidence

- `AudioEmitter` as the interface's name, `HasMixGroup`, `mTempo`/`mSpeed`
  and the `SongPos` field names before the measure.
- `mOptionsSwitch` on `RndOverlayOptionsCom` and the listener's
  `mHardwareMapped`, `mHardwareMapping` and `mHardwareMappingIndex`.
