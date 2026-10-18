# UI manager and layouts

The UI manager, the layout resource and the UI startup of the map's `ui`
module, with the meta-game objects the main loop reaches them through.
Names follow [naming.md](naming.md); the components the layouts hold are
built on the component base in [entity-components.md](entity-components.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `ui/UI.o` (part) | `0x891990`-`0x894D55` | `src/ui/core/UI.cpp` |
| `ui/UIMgr.o` | `0x8C8440`-`0x8CC5B7` | `src/ui/manager/UIMgr.cpp` |
| `ui/UILayoutResource.o` (part) | `0x8AAA40`-`0x8B1BAF` | `src/ui/layout/UILayoutResource.cpp` |
| `UILayoutCom` (part; no map object) | `0x8A9340`-`0x8AAA3F` | `src/ui/layout/UILayoutCom.cpp` |
| `ui/UIInputCom.o` (part) | `0x89ED90`-`0x89EF0F` | `src/ui/input/UIInputCom.cpp` |
| `rb_meta/RBMetaStateCom.o` (part) | `0xBB06A0` | `src/rb_meta/state/RBMetaStateCom.cpp` |

`UI.o` starts before `0x894000`, where [progress.md](progress.md) places the
`ui` range; the class factories it emits follow from `0x894FA0`
(`UILayoutResource::_Create`, `new` of 1248 bytes, then `UIInputCom`'s of 408
bytes and the others). The component classes `UI::Init` registers are declared, one
header per object, in `src/ui/input`, `src/ui/layout`, `src/ui/lists` and
`src/ui/widgets`; only their registration is declared.

## Startup and the main loop

`App::Initialize` calls `UI::Init` (`0x891990`) after `Rnd::Init`. It creates
`theUI` (`UIMgr::Init`, `0x8C8880`), registers the layout resource and twelve
component classes, and registers `UI::Terminate` (`0x894D20`) as an exit
callback. Each registration is the class's inline `Init`, emitted in `UI.o`:

| Init | Class | Class symbol | `sClassName` |
| --- | --- | --- | --- |
| `0x891A30` | `UILayoutResource` | `"UILayoutResource"` (parent `"TransEntityResource"`) | — |
| `0x891BA0` | `UIInputCom` | `"UIInputCom"` | `0x1AC8760` |
| `0x891F80` | `UIInputReceiverCom` | `"UIInputReceiverCom"` | `0x1AC8A58` |
| `0x892360` | `UILayoutCom` | `"UILayout"` | `0x1AC8D58` |
| `0x892740` | `UIListCom` | `"UIListCom"` | `0x1AC9040` |
| `0x892B20` | `UIListDtaProviderCom` | `"UIListDtaProvider"` | `0x1AC9358` |
| `0x892F00` | `UIListItemCom` | `"UIListItem"` | `0x1AC9818` |
| `0x8932B0` | `UIScrollbarCom` | `"UIScrollbar"` | `0x1AC9BB0` |
| `0x893660` | `UISliderCom` | `"UISliderCom"` | `0x1AC9E18` |
| `0x893A10` | `UIComboBoxEnumCom` | `"UIComboBoxEnum"` | `0x1AC8018` |
| `0x893F60` | `UIComboBoxFileCom` | `"UIComboBoxFile"` | `0x1AC8280` |
| `0x8944B0` | `UIComboBoxSymCom` | `"UIComboBoxSymbol"` | `0x1AC84F0` |
| `0x894960` | `UIListEditorDataCom` | `"UIListEditorData"` | `0x1AC95B8` |

Each component class keeps `sId` eight bytes before `sClassName`, then its
property registry and metadata, built by the object's static initializer
(`UIInputCom`'s at `0x8A4820`, `UILayoutCom`'s at `0x8AA970`).

The placeholder adapters the main loop used are replaced by the binary's
functions:

| Placeholder | Address | Function |
| --- | --- | --- |
| `ui_register_types` | `0x891990` | `UI::Init` |
| `ui_load_layout_by_id` | `0xBB5D40` | `RBMetaState::Goto(RBUILayout, bool)` |
| `g_ui_layout_controller` | `0x1AFF588` | `theRBMetaState` |
| `ui_layout_controller_update` | `0xBB58C0` | `RBMetaState::Poll` |
| `ui_event_queue_update` | `0xADE120` | `DialogMgrCom::Poll` |
| `ui_layout_consume_skip_frame` | `0x8B1840` | `UILayoutResource::UpdateSkipDrawState` |
| `UILayout` | — | `UILayoutResource` |
| `UILayoutId`, `UILayoutIdToSymbol` | `0xBB06A0` | `RBUILayout`, `RBUILayoutToSym` |

The map places `RBMetaState`, `RBUILayout` and `RBUILayoutToSym` in
`rb_meta/RBMetaStateCom.o` and `DialogMgrCom` in `rb_meta/DialogMgrCom.o`.
`DialogMgrCom::Poll` is static: it pops the first 256-byte dialog request
under a lock and shows it once no dialog shows or is pending and
`UIMgr::IsInTransition` is false. `RBMetaState::Goto` goes to a layout
that is not current through `UIMgr::Goto` with the layout's path; its added
flag only skips a name report that is compiled out.

## UIMgr

`UIMgr` is a `MsgSource` of 184 bytes; its vtable at `0x195E4D0` has
`MsgSource`'s seven slots, with the destructors (`0x8C8710`, `0x8C8860`) and
`Handle` (`0x8CBAE0`) overridden. `theUI` is at `0x1AC9A78`.

| Offset | Member | |
| ---: | --- | --- |
| 24 | `mJoypadClient` | a `JoypadClient` with the d-pad from the sticks and repeat mask `0xF000` |
| 32 | `mCurrentLayout` | `ResourcePtr<UILayoutResource>` |
| 40 | `mTransitionState` | none, started, loading, entered |
| 48 | `mLayoutLoader` | the transition's `ResourceLoader` |
| 56 | `mPreloadingEnabled` | the `"ui"` config's `preloading_enabled` |
| 64 | `mSceneLoaders` | `eastl::vector<SceneLoader*>` |
| 96 | `mPreloadedScenes` | `eastl::vector<ResourcePtr<EntityResource>>` |
| 128 | `mPreviousLayoutName` | the path the transition left |
| 136 | `mReloadOnTransition` | the `"ui"` config's `reload_on_transition` |
| 137 | `mReleasingLayout` | set while the layout is released |
| 144 | `mKeyboardSink` | the keyboard subscription |

The constructor (`0x8C8440`) also sets the script global `ui` to the
manager and copies the config's `verbose_locale_notifies` into
`Locale::sVerboseNotify` (`0x19FDDFC`).

### Transitions

`Goto` (`0x8C9BE0`, `0x8C9F50`) and the `goto` command run
`_StartTransition` (`0x8C9C30`): outside a transition and for a path other
than the current layout's, it exports `transition_started` with the old and
new paths as resource-path nodes, acquires a loader for the new layout
(not started) and plays the old layout's exit (`UILayoutResource::Exit`).
`Poll` (`0x8C8900`) then advances the state:

1. Started: with `reload_on_transition` the old layout is released at once.
   The loader starts (`ResourceLoader::Start`), and the state moves to
   loading.
2. Loading: once the loader is done and the old layout's scenes no longer
   play their exit, the scenes of both layouts reset their draw
   registrations, the old layout is released while the allocator batches
   the frees, `destroyed_current_layout` is exported, the loaded layout
   becomes current and enters, and `transition_entered_new_layout` goes to
   its scenes and the manager's sinks.
3. Entered: once no scene plays its enter, `transition_done_before_poll` is
   sent, the layout polls, and `transition_done` follows; both carry the old
   and new paths as symbols.

Outside these steps the current layout polls once per frame. The poll then
moves the scenes `SceneLoader`s finished into the preloaded scenes and
deletes the loaders.

`Draw` (`0x8C9A80`) draws the current layout into the device's current
targets with a zero clear color, or clears the targets
(`RndDrawUtl::ClearBuffers`, `0x3DF9C0`) when there is no layout.
`GetCurrentLayout` returns null while the layout is being released.
`App::RunOneFrame` skips the draw while `UpdateSkipDrawState` takes a
pending skipped frame.

### Messages

`Handle` sends `key`, `key_release`, `button_down` and `button_up` to the
layout's scenes, the last in poll order first, until one handles it, and
only outside a transition; an unhandled message falls through to the later
types. `joypad_connect` goes to every scene. The commands are `goto`,
`add_scene`, `remove_scene` (each with a path), `set_reload_on_transition`
and `is_in_transition`.

### Preloading

`PreloadScene` (`0x8CB2D0`) takes `PreloadArgs` (three flags: load now,
set the scene header's `copy_on_instance`, enter once loaded). A scene loaded
now is kept at once; any other gets a `SceneLoader` (40 bytes, vtable
`0x195E518`) on the call thread, whose `ThreadDone` (`0x8CAF50`) prepares the
scene. `SceneLoader` extends `ResourceLoader` (`entity/Resource.o`; 32 bytes,
vtable `0x195E548`), whose inline members this object emits: `Start`
(`0x8C9700`), `ThreadStart` (`0x8CC310`), `ThreadDone` (`0x8CAFD0`) and the
destructors. The layout loader comes from `ResourceLoader::AcquireLoader`
(`0x1167F10`) and goes back through `ReleaseLoader` (`0x1168010`), which
defers deleting a loader whose call still runs.

## UILayoutResource

A layout is a `RndDrawableEntityResource` of 1248 bytes whose metadata names
`TransEntityResource` as the parent (`sMetaData` at `0x1AC8FB8`, extension
`layout`). Each scene is an object with a `RndSceneInstanceCom`; the root's
`UILayoutCom` orders their polls and draws. The vtable at `0x195D1C0` has 39
slots and matches the compiled one slot for slot.

| Offset | Member |
| ---: | --- |
| 784 | `mQueuedScenes`: path, mode (front, before or after an object), anchor object, threaded poll |
| 816 | `mQueuedRemovals` |
| 848 | `mEntering`, `mExiting` |
| 856 | `mClearJob`: a `PollDepBase` with the draw parameters (vtable `0x195D308`) |
| 1208 | `mSceneDrawJobs` |
| 1240 | `mFramesToSkipDraw`, `mSerialPollFrames` |

Scenes are added and removed through the queues (`AddSceneToFront`,
`AddSceneAtDrawOrder`, `AddSceneAfterObject`, `RemoveScene`); bit 0 of the
flags applies the queue at once. `_AddQueuedScenes` (`0x8AAC60`) creates
each scene's object, places it next to its anchor in both orders, reenters
its components and reports it ready; `_RemoveQueuedScenes` (`0x8AB2D0`)
destroys the objects. Both release the threaded poll range first and resync
the masking scenes after.

`_EnterEntity` (`0x8AC810`) enters the objects itself, creates the layout
component's poll manager and groups, and keeps the entered scenes for the
next sync of the poll range. `_OnEntityReady` (`0x8ADA20`) and
`_EnterImmediately` (`0x8AD8F0`) collect and enter the ready scenes.
`HandleMsg` (`0x8ADB70`) exports a message to the scene entities.
`Exit`, `IsExiting` and `IsEntering` (`0x8AEB90`, `0x8AECC0`, `0x8AEE40`)
play and test the scenes' `UIInputCom` transitions, which run the
`enter_animation` and `exit_animation` state graphs on the scene root's
`StateGraphDriverCom`. `_LoadEntity` (`0x8ABB50`) reads the layout's
revision (4) and gives older layouts their root components and a draw
order built from the scene instances' draw orders.

`UILayoutCom` (280 bytes, vtable `0x195D018`) holds the `poll_order`,
`draw_order` and `hmd_mirror_overlays` arrays at 32, 72 and 112, the poll
manager and groups at 152-168, the threaded scenes and the groups to start
at 176 and 208, the ready scenes at 240 and the released flag at 272.

`_PollEntity` (`0x8ACC60`) applies the scene queues, then, for an active
game-mode layout, enters ready scenes at once when the range was
released, selects the layout's `PollMgr`, takes the audio lock as
"use_pollmgr_audio_lock" asks (left alone while `mSerialPollFrames` counts
down), starts the drawer group, syncs on the scene group and then on
everything, and runs the post-polls of the scene group, each threaded
scene's groups and the drawer group. `_SyncThreadedPollRange`
(`0x8AC1E0`) orders the scenes' groups: a run of threaded scenes polls
after the groups the last serial scene left, a serial scene after the
whole run; the layout's own entity polls in the layout's group after them.
`_ClearThreadedPollRange` (`0x8AB6E0`) unlinks them again when a scene
group has jobs.

The draws (`_Draw` `0x8AFDC0`, `_StartDrawJobs` `0x8B05E0`,
`_FinishDrawJobs` `0x8B1100`) walk the showing scenes; each first draws
the depth of its showing masking scenes (`DrawMaskingScenes`, `0x8AF870`),
clears only as the first scene and outputs to the back buffer only as the
last (`_SetSceneDrawParams`, `0x8AFB90`). The job draw keeps one
`SceneDrawJob` (408 bytes, vtable `0x195D368`) per scene for the masking
draw and chains each scene's draw jobs after it; without scenes the
`ClearJob` runs. `DrawHmdMirrorOverlays` (`0x8AF240`, no caller) draws the
HMD mirror overlays without clearing.

## Weak evidence

- Not in the map, named from their bodies: `UIMgr::ReleaseCurrentLayout`,
  `IsTransitionReady`, `SwapToPreloadedLayout`, `GotoPreloadedLayout`,
  `UnloadPreloadedScene`, `GetPreloadedScene`, `AddSceneAfterObject`,
  `_OnSetReloadOnTransition`, `_ResetSceneDrawRegistrations`,
  `_GetSceneComs`; `SetCopyOnInstance`; the transition states.
- `UILayoutResource`: `SetScenesActive`, `GetSceneInstances`,
  `IsSceneQueuedForRemoval`, `_FindSceneObject`, `_GetShowingScenes`,
  `ClearJob`, `mEntering`, `mExiting` (no writer found) and
  `mSerialPollFrames` (meaning not identified). `_SyncThreadedPollRange`
  (`0x8AC1E0`) takes the map's name from its callees, and
  `_ClearThreadedPollRange` (`0x8AB6E0`) is named as its counterpart.
  `_ValidateSceneData` takes the map's name for a body whose checks are
  compiled out. `SceneDrawJob`, `DrawMaskingScenes`, `_GetMaskingScenes`,
  `_SetSceneDrawParams`, `DrawHmdMirrorOverlays`,
  `_AssignPartialFramerateScenes` and `UILayoutCom::mThreadedScenes` and
  `mStartPollGroups` (nothing found fills it) are named from their bodies.
- `UILayoutCom` and `UIListEditorDataCom` are named after their class
  symbols; the map, which is older, has neither.
- `JoypadClient::SetInputEnabled` (`0x39F6E0`), `RndDrawUtl::ClearBuffers`
  and `GameObject::_ReenterComponents` (`0x115DA0`) are named from their
  bodies.
- The preloaded scenes are typed `ResourcePtr<EntityResource>`: the map's
  vector holds `RndSceneResource`s, while this build keeps layouts there.

## Not reconstructed

- `UILayoutResource::_AssignPartialFramerateScenes` (`0x8AD420`, the
  end of the poll), which gives the partial-framerate scenes distinct
  intervals through a bit vector (vtable `0x195D3C8`, the map's
  `utl/BitVector.o` family) that is not modelled.
- The rest of `UILayoutCom` and the UI component classes, including their
  `Init` registrations: `InitComponentClass` needs each class's
  constructor, statics and `_Init`, which are not reconstructed.
- `ui/UIMsg.o` (the button and select messages).
- Outside the `ui` module: `RBMetaState`, `DialogMgrCom`, `JoypadClient`,
  `ResourceLoader::AcquireLoader` and `ReleaseLoader`, and
  `RndDrawUtl::ClearBuffers` are declared only.
