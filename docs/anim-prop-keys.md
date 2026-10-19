# Property animation keys

The key components of the map's `anim` module: one object per animated
property in a `PropAnimResource`'s entity, each holding a key track and
the reference to the property it drives, and the drivers that blend the
playing animations' keys into the properties. Names follow
[naming.md](naming.md); the tracks are in
[anim-key-tracks.md](anim-key-tracks.md), the numeric key classes in
[anim-prop-keys-numeric.md](anim-prop-keys-numeric.md) and the playback
runtime in [anim-runtime.md](anim-runtime.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `anim/PropKeysCom.o` | `0x305350`-`0x305C8F` | `src/anim/props/PropKeysCom.cpp` |
| `anim/PropKeysBoolCom.o` | `0x2FFD60`-`0x302A91` | `src/anim/props/PropKeysBoolCom.cpp` |
| `anim/PropKeysIntCom.o` | `0x308AB0`-`0x30A4AF` | `src/anim/props/PropKeysIntCom.cpp` |
| `anim/PropKeysObjCom.o` | `0x30E2A0`-`0x310FAF` | `src/anim/props/PropKeysObjCom.cpp` |
| `anim/PropKeysStrCom.o` | `0x318DB0`-`0x31B3B1` | `src/anim/props/PropKeysStrCom.cpp` |
| `anim/PropKeysSymCom.o` | `0x31B470`-`0x31E1F0` | `src/anim/props/PropKeysSymCom.cpp` |
| `anim/PropKeysWaveformCom.o` | `0x325320`-`0x326BDF` | `src/anim/props/PropKeysWaveformCom.cpp` |
| `anim/PropAnimCom.o` | `0x2FBC00`-`0x2FCAB1` | `src/anim/props/PropAnimCom.cpp` |

The class templates' methods (`PropKeysTypeCom<T>`, `PropKeysIntCom<T>`,
`PropKeysWaveformCom<T>`, `TPropDriver<T>`) are emitted where they are
first used; most of the integer and waveform instances sit in
`anim/AnimInit.o` (`0x2B7390`-`0x2D3B4F`).

## PropKeysBaseCom

The base (264 bytes, vtable `0x18F7BB8`, 73 slots; interface
"PropKeys") holds the driven property ("driven_prop", a `PropRef`) and the
editor's "hidden_in_editor" and "color_in_editor". Slots 41-72:

| Slots | Members |
| ---: | --- |
| 41-42 | `CreatePropDriver`, `HasTangents` |
| 43-54 | the key track's operations (`SetInterpolation` to `GetKeyFrameAtIndex`, with `OffsetFrames`) |
| 55-60 | `GetKeysType`, `GetKeysSubType`, `IsValidInterpolation`, `IsColorKeys`, `IsRotationKeys`, `HasSerializedValue` |
| 61-65 | `GetValueAtIndex`, `SetValueAtIndex`, `UpdateValueAtIndex`, `SetValueAtFrame`, `GetValueAtFrame` |
| 66-72 | `CanHaveExtra`, `GetExtraAtIndex`, `SetExtraAtIndex`, `AddNewKey`, `ApplyFrame`, `CloneKey`, `_Setup` |

`RegisterFactory` (`0x305350`) maps a property type and struct name to the
key class and its driver factory (`0x19F9EC0`); `NewKeysType`
(`0x305500`) creates the class's component on a new object named "Keys
type <type> <subtype>" and sets it up for the property. The key values'
metadata is remapped to the driven property's through
`sRemapKeyValue` (`0x19F9EF8`, `_RemapKeyValue` `0x305800`), the context
entity being the entity of the resource that inlines the animation
(`_GetContextEntityPrototype`, `0x3054E0`).

## PropKeysTypeCom<T>

The typed keys (320 bytes) take the key track `TKeyTrack<T>` as their
second base at 264, so the second vtable (17 slots) holds inlined thunks
of the forwarding slots 43-54. Slots 73-75 convert a value to and from a
`DataNode` and give the default. The values are read and written through
`PropRef::Get<T>` and `_SetProp` as `PropValueType<T>`. Each class's
`_Init` registers the "keys" array of `TKey<T>` structs and then the base
properties (`_InitPropKeysBase<T>`, revision 1, whose handler upgrades an
old interpolation with `UpgradeInterpolation`, `0x1DC930`).

`TPropDriver<T>` (256 bytes, from the pool, labelled "PropDriver") keeps
one entry per playback: the playback, its keys and the driven component.
The discrete values (bool, integers, objects, symbols) take the value of
the heaviest playback that is not paused. The strings are sampled but not
applied; the waveforms play the last playback's keys, blending a key's
waveform into the next one's through the waveform's second resource.

| Class | Value | Vtable | Statics (`sId`, `sClassName`, registry, metadata) |
| --- | --- | --- | --- |
| `PropKeysBoolCom` | `bool` | `0x18F73A8` | `0x19F99E0`, `0x19F99E8`, `0x19F99F0`, `0x19F9A90` |
| `PropKeysIntCom<signed char>` (and seven more) | integers | `0x18F1CA0` ... `0x18F3C90` | `0x19F6368`, `0x19F60D0`, `0x19F62B0`, `0x19F60E0` ... |
| `PropKeysObjCom` | `GameObjectId` | `0x18F83F8` | `0x19FA220`, `0x19FA228`, `0x19FA230`, `0x19FA2D0` |
| `PropKeysSymCom` | `Symbol` | `0x18F93E8` | `0x19FAC70`, `0x19FAC78`, `0x19FAC80`, `0x19FAD20` |
| `PropKeysStrCom` | `String` ("string" struct) | `0x18F9000` | `0x19FA9F0`, `0x19FA9F8`, `0x19FAA00`, `0x19FAAA0` |
| `PropKeysWaveformCom<float>`, `<Hmx::Color>` | `Waveform<T>` | `0x18F4120`, `0x18F45B0` | `0x19F78E8`, `0x19F7648`, `0x19F7830`, `0x19F7658` (float) |

The integer and waveform classes are named "PropKeysIntCom_kInt8" ...
"PropKeysIntCom_kUint64" and "PropKeysWaveformCom_kWaveformFloat"/"...Color".
The emitted vtables (both parts) match the binary slot for slot.

## PropAnimCom

`PropAnimCom` (80 bytes, vtable `0x18F6FB0`; class id "PropAnim", alias
"PropAnimCom") on the animation entity's root: the frame range its keys
span ("frame_range_start"/"frame_range_end", measured on enter unless
regression testing, `UpdateFrameRange` `0x2FC2D0`), "time_units",
"is_looping" and "target_order". Statics `0x19F96E0`, `0x19F96E8`,
`0x19F96F0`, `0x19F9790`.

## Weak evidence

- Slot names 57-60 (`IsValidInterpolation`, the map's
  `ValidInterpolations`; `IsColorKeys`, `IsRotationKeys` and
  `HasSerializedValue` have no caller in the module), `OffsetFrames`,
  `_RemapKeyValue`, `sRemapKeyValue`, `StaticKeyTypeName` (no callers),
  the registry's key and value types, and the field names.
- `PropAnimCom::mTimeUnits`' default 8 is not one of the clocks' units.
- The waveform keys' blend writes the eased position into the waveform's
  first override datum and the interpolation's 0x20000000 flag into the
  second; what the flag selects is not decoded.
