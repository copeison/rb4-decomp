# Meta helpbar and list providers

The helpbar widget's component from the map's `rb_meta/RBHelpbarCom.o` and
the base of the game's list providers from
`rb_meta/RBUIListProviderBaseCom.o`. Names follow [naming.md](naming.md);
the component machinery is in [entity-components.md](entity-components.md)
and the lists in [ui-system.md](ui-system.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBUIListProviderBaseCom.o` | `0xCB5B10`-`0xCB64BF` | `src/rb_meta/ui/RBUIListProviderBaseCom.{h,cpp}` |
| `rb_meta/RBHelpbarCom.o` | `0xED3250`-`0xED7EAF` | `src/rb_meta/ui/RBHelpbarCom.{h,cpp}` |
| helpbar registration and factory | `0xB81280`, `0xBAA450`, `0xBAA480` | `src/rb_meta/ui/RBHelpbarCom.cpp` |

## RBUIListProviderBaseCom (200 bytes)

An abstract `Component` (identity slots pure, no statics, no registration;
the map's `_InitBase<T>` template registers the subclasses' shared
properties). The vtable at `0x19784F0` has 46 slots: Component's 41, then
`_GetListCom` (`0xCB6490`, null), `_BuildListData` (pure),
`_GetListFocusIndex` (`0xCB64A0`, 0), `_OnListFocusChanged` (`0xCB64B0`)
and `_OnListSelection` (`0xCAF970`, unhandled; folded with other classes,
as is `_HasPostPoll` `0xCAF960`).

| Offset | Field |
| ---: | --- |
| 24 | `mData` (`UIListData`) |
| 152 | `mListOnObject` |
| 160 | `mInstance` (the object's `InstanceCom`) |
| 168 | `mList` |
| 176 | `mNeedsUpdate` (the last build failed) |
| 184 | `mFocusIndex` (-1) |
| 192 | `mTrackFocus` (true) |

Slot 29 finds the object's `UIListCom`, else the instance component whose
entity's root holds one. `_UpdateList` (`0xCB5E00`) builds the rows,
installs them and focuses the subclass's row (the first when out of range);
`_InstallSelf` (`0xCB5ED0`) finds the list through the instance or
`_GetListCom`. `_Poll` and `_EditPoll` run `_PollImpl` (`0xCB6020`): without
a list or after a failed build they post-poll (the game mode through
`Entity::QueueForPostPoll`, `0xF5F40`), otherwise they report focus changes.
`Handle` passes `"ui_select"` to `_OnListSelection` with the focused row.

## RBHelpbarCom (480 bytes)

"Com that handles everything in helpbar widget" (author Ralph Hebb,
category UI, root only, allowed resource `RndEntityResource`). The vtable
at `0x198D590` has 41 slots and overrides the destructors (`0xED32D0`,
`0xED3300`), `Handle` (`0xED65A0`), the identity slots
(`0xED6AA0`-`0xED6B20`, `0xED6C60`, `0xED6C70`), slot 29 (`0xED3FF0`),
`_Enter` (`0xED4E90`) and `_Poll` (`0xED51D0`).

| Offset | Field | Property |
| ---: | --- | --- |
| 24, 28 | `mPreviousLayout`, `mNextLayout` (`kLayoutInvalid`) | `previous_layout`, `next_layout` |
| 32, 40 | `mBackSound`, `mAcceptSound` | `back_sound` ("shell/ui/os_back"), `accept_sound` ("shell/ui/guitar_select") |
| 48 | `mRuntime` (`RuntimeData`, 432 bytes, constructor `0xED3340`, destructor `0xED6CE0`) | |
| 48, 104 | prompt tables by layout and by override name | |
| 160 | `mCurrentPromptInfo` | |
| 192 | `mPrompts` (`PropArray<PromptInfo>`, 7) | `prompts` (read-only) |
| 232 | `mSceneEntity` | |
| 240 | `mLastLayout` | |
| 248 | `mPromptCount` | `prompt_count` (not saved) |
| 256, 257 | `mConfigChanged`, `mButtonOverrideChanged[15]` | |
| 272 | `mNoButtonOverride` | |
| 280-479 | five subscriptions | |

`PromptInfo` (24 bytes) is `action`, `token`, `secondary_token`. Slot 29
starts every layout with seven unset prompts, reads
`ui/shared/scripts/helpbar_config.dta` (`(helpbar_config (layout_configs
(layout (action token [secondary])...)...) (override_configs (name
...)...))`) and shows the prompts. `_UpdatePromptInfo` (`0xED4BE0`) takes
`theRBMetaState`'s override configuration or the current layout's prompts,
applies its button overrides (`RBMetaState::GetNoButtonOverride` hides a
prompt), counts up to the last prompt with a token and copies them into
`prompts`. `_Poll` updates after a layout change or an override message and
exports `"helpbar_config_changed"` and, outside a UI transition,
`"helpbar_changed_msg"` (`HelpbarChangedMsg`, vtable `0x198D290`), which the
slot and transition events also send. Cancel and confirm (and backspace and
enter keys) send `"helpbar_back"`/`"helpbar_accept"` (pad) to the scene
root's other components (`GameObject::ExportToSiblings`, `0xF08B0`), play
the sound and go to `previous_layout`/`next_layout`.

Statics: `sId` `0x1B40848` and `sClassName` `0x1B40850` ("RBHelpbar"),
`sPropRegistry` `0x1B40860`, `sMetaData` `0x1B40900`, the empty prompt list
`0x1B40828`, `PropArray<PromptInfo>::sType` `0x1B407F8`; the static
initializer is `0xED7DA0`.

## Weak evidence

`RuntimeData` and every field name, `PromptTokens`, `HelpbarChangedMsg`,
`_ParsePromptInfo`, `_ParsePrompt`, `_GetCurrentPromptInfo`,
`_DumpPromptInfo`, `_OnUserSlotChanged`, `_OnTransitionDone`, the list
provider's fields, `UIKeyboardKeyMsg::GetKeyData` and
`DataNode::LiteralInt`. The map's `HelpbarPromptOverrideMsg` is the
repository's `HelpbarOverrideMsg`; its `_GetToken` returns one symbol and
`_UpdateButtonOverride` takes no argument.
