# State graph components

The components a state graph's entity is made of: the nodes, the
transitions between them and the conditions that start the transitions.
Each sits on its own object of a `StateGraphResource`'s entity. Names
follow [naming.md](naming.md); the resource, the driver and the running
graph are in [stategraph-runtime.md](stategraph-runtime.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `stategraph/StateBase.o` | `0x807E80`-`0x808AD5` | `base/StateBase.cpp` |
| `stategraph/StateNode.o` | `0x708410`-`0x70AD77` | `nodes/StateNode.cpp` |
| `stategraph/StateNodeAnimate.o` | `0x70AD80`-`0x7178F7` | `nodes/StateNodeAnimate.cpp` |
| `stategraph/StateNodeBlendTreeBase.o` | `0x717900`-`0x718EFA` | `nodes/StateNodeBlendTreeBase.cpp` |
| `stategraph/StateNodeExecute.o` | `0x718F00`-`0x719A37` | `nodes/StateNodeExecute.cpp` |
| `stategraph/StateNodeFolder.o` | `0x719A40`-`0x71AC87` | `nodes/StateNodeFolder.cpp` |
| `stategraph/StateNodeGroup.o` | `0x71AC90`-`0x71E667` | `nodes/StateNodeGroup.cpp` |
| `stategraph/StateNodeIncrementPropertyInit_A.o`-`_C.o` | `0x71E670`-`0x780B1F` | `nodes/StateNodeIncrementPropertyInit_A.cpp`-`_C.cpp` |
| `stategraph/StateNodeModifySound.o` | `0x780B20`-`0x785086` | `nodes/StateNodeModifySound.cpp` |
| `stategraph/StateNodePlaySound.o` | `0x785090`-`0x78CF86` | `nodes/StateNodePlaySound.cpp` |
| `stategraph/StateNodeRoot.o` | `0x78CF90`-`0x78DA27` | `nodes/StateNodeRoot.cpp` |
| `stategraph/StateNodeSetAnimFrame.o` | `0x78DA30`-`0x7913FF` | `nodes/StateNodeSetAnimFrame.cpp` |
| `stategraph/StateNodeSetPropertyInit_A.o`-`_C.o` | `0x791400`-`0x7E9313` | `nodes/StateNodeSetPropertyInit_A.cpp`-`_C.cpp` |
| `stategraph/StateNodeSetValue.o` | `0x7E9320`-`0x7F2307` | `nodes/StateNodeSetValue.cpp` |
| `stategraph/StateNodeSubGraph.o` | `0x7F2310`-`0x7F2E67` | `nodes/StateNodeSubGraph.cpp` |
| `stategraph/StateNodeTriggerAction.o` | `0x7F2E70`-`0x7F3A17` | `nodes/StateNodeTriggerAction.cpp` |
| `stategraph/StateNodeTriggerAudioClip.o` | `0x7F3A20`-`0x7F5216` | `nodes/StateNodeTriggerAudioClip.cpp` |
| `stategraph/StateTransition.o` | `0x7F5220`-`0x7FB537` | `transitions/StateTransition.cpp` |
| `BlendTreeBaseCom.o` | `0x7FB540`-`0x7FC51F` | `blendtrees/BlendTreeBaseCom.cpp` |
| `ClipBlendTreeCom.o` | `0x7FC520`-`0x80368F` | `blendtrees/ClipBlendTreeCom.cpp` |
| `ClipJukeboxCom.o` | `0x803690`-`0x807E7F` | `blendtrees/ClipJukeboxCom.cpp` |
| `stategraph/StateCondition.o` | `0x808AE0`-`0x809A92` | `conditions/StateCondition.cpp` |
| `stategraph/StateConditionComparisonInit_A.o`, `_B.o` | `0x809AA0`-`0x83199F` | `conditions/StateConditionComparisonInit_A.cpp`, `_B.cpp` |
| `stategraph/StateConditionEvent.o` | `0x8319A0`-`0x835D37` | `conditions/StateConditionEvent.cpp` |
| `stategraph/StateConditionEventParameter.o` | `0x835D40`-`0x85968A` | `conditions/StateConditionEventParameter.cpp` |
| `stategraph/StateConditionLogic.o` | `0x859690`-`0x85AE07` | `conditions/StateConditionLogic.cpp` |
| `stategraph/StateConditionNodeFinished.o` | `0x85AE10`-`0x85B307` | `conditions/StateConditionNodeFinished.cpp` |
| `stategraph/StateConditionNot.o` | `0x85B310`-`0x85BCC7` | `conditions/StateConditionNot.cpp` |
| `stategraph/StateConditionObjectDistance.o` | `0x85BCD0`-`0x85D627` | `conditions/StateConditionObjectDistance.cpp` |
| `stategraph/StateConditionPropertyComparisonInit_A.o`-`_C.o` | `0x85D630`-`0x88E53F` | `conditions/StateConditionPropertyComparisonInit_A.cpp`-`_C.cpp` |
| `stategraph/StateConditionRandom.o` | `0x88E540`-`0x88F1A7` | `conditions/StateConditionRandom.cpp` |
| `stategraph/StateConditionSoundProgress.o` | `0x88F1B0`-`0x8903E6` | `conditions/StateConditionSoundProgress.cpp` |
| `stategraph/StateConditionTimeElapsed.o` | `0x8903F0`-`0x891982` | `conditions/StateConditionTimeElapsed.cpp` |

Sources are under `src/stategraph/`. Every class's `Init` and `_Create`,
and the methods the blend tree nodes keep inline, are emitted in
`StateGraph.o`; each source defines them with its class. The templated
groups emit each type's code in the first object that uses it, so a
group's range also holds code of types registered elsewhere.

## Base

`StateBase` (32 bytes, vtable `0x1950668`, 42 slots) is the `Component`
under every graph component. It registers "name" and "object_id", which
go through the object, and a "comment"; a change of the name or comment
sends `appearance_updated` to the entity so the editor redraws the
object. Slot 41, `_Fixup`, resolves the object ids the component links
to; `_OnResourcesLoaded` and `Fixup` call it. The lookups
(`_GetObjectCom<T>`, `_GetObjectBaseCom<T>`) report a missing component
through a compiled-out report. `sContextEntityTagFunc` (`0x1ABF000`)
gives the property tags the entity the graph is inlined into.
`_FixDrivenProp<T>` (`0x727D50` for float) makes a change of a driven
property's members redraw the component.

## Driven values

`DrivenProp<T>` (`props/DrivenProp.h`) is a value that a property of
another component can drive. `Register` (a copy per user; `0x4107C0`
float and `0x410EF0` colour in `RndSceneCom.o`) registers the struct:
"value", "prop" (a `PropRef` of a type `GetCompatibleTypes<T>` allows:
any number for float, else the type itself) and the hidden "is_driven",
with a ">" inline action that toggles the driving; the editor hides the
half that does not apply. A value saved before the struct's revision 1
was a plain "raw_value". `CopyFromProperty` (`0x716F10`) reads the three
members through `GetPropValue<T>`. A colour starts opaque black.

## Nodes

`StateNode` (152 bytes, vtable `0x193A7B8`, 50 slots) is the base node,
the editor's "No-Op". It holds when it is active relative to the
transitions into and out of it (`active_at`, `active_until`), its
rectangle, its outgoing transitions and its folder; `RuntimeData` keeps
the resolved folder, group and transitions. Slots 42-49 give the editor
text, the child nodes, the animation timing (`StateNodeTiming`), the node
instance, whether the node blends, whether it sets colours and rotations
(which shows a transition's HSV and rotation settings), and whether it
has activity of its own. `StateNodeInstance` (32 bytes, vtable
`0x193A958`, 15 slots) is a node in a running graph.

- `StateNodeRoot` (160 bytes, vtable `0x1945AC0`): the root of every
  graph: `start_on_enter` and the poll frequency.
- `StateNodeFolder` (176 bytes, vtable `0x193BD48`, newer than the map)
  holds a subgraph; `_PostCreate` makes its "Entry Point" ("start") and
  "Parent Folder" ("exit") nodes. It has no instance: the runtime enters
  its start node.
- `StateNodeGroup` (232 bytes, vtable `0x193BEE8`) runs its
  "child_nodes" by "mode": all at once (`SimultaneousInstance`, 80
  bytes), in sequence, optionally shuffled or repeated
  (`SequenceInstance`, 96 bytes), or one drawn from `gRand`
  (`PickOneInstance`, 48 bytes).
- `StateNodeExecute` (168 bytes, vtable `0x193BB20`) runs its "cmd"
  script once on entry or every poll while "persistent".
- `StateNodeSubGraph` (160 bytes, vtable `0x194E588`) runs an inlined
  `StateGraphResource` as a `StateGraphInstance` inside its 216-byte
  instance.
- `StateNodeTriggerAction` (376 bytes, vtable `0x194E7B0`, newer than the
  map) runs the action property its "action" `PropRef` names.
- `StateNodeAnimate` (2344 bytes, vtable `0x193AAD0`, 57 slots) plays a
  PropAnim or CharAnim resource or a `CharClipCom` on the graph's
  `AnimDriverCom` with driven range, start, period, weight and scale; it
  is its own `StateNodeTiming` (second base at 152, vtable `0x193ACA8`).
  Its instance (64 bytes) holds the `AnimInstID`.
- `StateNodeBlendTreeBase` (160 bytes, vtable `0x193B910`, 56 slots) is
  the `StateNodeTiming` of the blend tree component on its own object;
  `StateNodeClipBlendTree` (`0x1939EA8`) and `StateNodeClipJukebox`
  (`0x193A0B8`) only require their component.
- `StateNodeSetValue<T>` (448 bytes and vtable `0x193A2C8` for float, 52
  slots) holds a driven "value", which numbers may convert through a
  range or a curve. `StateNodeSetProperty<T>` (672 bytes, `0x193C310`)
  writes it to a "target_prop", blending from the value an earlier node
  applied; `StateNodeIncrementProperty<T>` (newer than the map, numbers
  only) adds, subtracts, multiplies or divides it. The value nodes
  register 20 types, the property nodes 20 in groups A-C and the
  increment nodes 9 in groups A and B (C is empty).
- `StateNodeSetAnimFrame` (480 bytes, vtable `0x1945C60`) is a
  `StateNodeSetValue<float>` whose value is an animation frame.
- `StateNodePlaySound` (728 bytes, vtable `0x1945118`) plays a sound event
  on an `AudioEmitterCom` or the 2D emitter, with driven FMOD parameters
  ("params", `DrivenAudioParamSettings`, 304 bytes) and volume, and
  registers the handle with the driver. `StateNodeModifySound` (728
  bytes, `0x19449F0`) sets or blends a parameter of a named sound, mutes,
  fades, pauses or stops it. `StateNodeTriggerAudioClip` (168 bytes,
  `0x194E9D8`) triggers its object's `AudioClipCom` and finishes at once,
  on its start or on its end.

## Blend trees

`BlendTreeBaseCom` (24 bytes, vtable `0x194F490`, 51 slots, not
creatable) is the base of the components that mix clips (`CharClipCom`s
of the context entity) and subtrees. Slots 41-50 list and edit the
children, give the tree's units and average clip length and create a
`BlendTreeBaseCom::Instance` that plays the clips through the
`AnimDriverCom`. `ClipBlendTreeCom` (304 bytes, `0x194F638`) crossfades
the two children whose "value" encloses a float parameter read through a
`PropRef`; `ClipJukeboxCom` (304 bytes, `0x19500A8`) plays its clips in
order or by weight, crossfading over the driven "crossfade_frames".

## Transitions

`StateTransition` (600 bytes, vtable `0x194ECF0`, 42 slots) links its
origin and target nodes and names its condition. It blends for a driven
duration along a driven blend curve or synchronized to the nodes'
animation timing within a start window, and converts between seconds,
beats and the nodes' lengths. `_Init` (`0x7F5420`, revision 5) adds editor
views of the synchronized blend's start, duration and offset as a
percentage, as frames and as seconds or beats (accessors `0x7F8EF0`-
`0x7F9580`), and hides each setting that does not apply. The "units"
take their allowed values from `gTimeUnitsAllowedValues` (`0x1AC7CC0`, the
context entity's `AnimDriverCom` timelines). `StateTransitionInstance` is
a running transition: `Begin`, `Poll` and `End`.

## Conditions

`StateCondition` (40 bytes, vtable `0x1950818`, 54 slots) holds the
evaluation mode: reevaluate every frame, evaluate once, or keep the first
true or false result. `Create` makes a condition of a `Type`; the two
logical types share `StateConditionLogic`. Slots 42-53 create the
instance and edit, check and describe the children.
`StateConditionInstance` (32 bytes, seven slots) evaluates a condition
in a running graph and holds the result as the mode asks (`mHeld`).

- `StateConditionLogic` (112 bytes, vtable `0x1956DD0`): And or Or over
  any number of children, which may not be parameter comparisons.
- `StateConditionNodeFinished` (vtable `0x1957020`): met once the node of
  the transition's scope finished.
- `StateConditionNot` (48 bytes, vtable `0x1957228`): the inverse of its
  one child.
- `StateConditionComparison<T>` (512 bytes and vtable `0x1950C58` for
  float, 59 slots) compares a value with driven "value_a", or with the
  span to "value_b", optionally against a random value between them.
  Slots 54-58 describe the compared value and the allowed operators; the
  instance (`0x1955080`) reads the value through slot 8.
- `StateConditionPropertyComparison<T>` (736 bytes, `0x19578F8`) compares
  a "prop" with a specific value, the value when the node was entered or
  last frame's value. `StateConditionEventParameter<T>` (ten types)
  compares a parameter of its parent `StateConditionEvent`'s event.
- `StateConditionEvent` (192 bytes, `0x1954AC0`) subscribes its instance
  (a `MsgSink`) to an event a component class of the prototype entity
  exports; its parameter comparisons are its children.
- `StateConditionTimeElapsed` (520 bytes, `0x195BFC0`),
  `StateConditionRandom` (512 bytes, `0x195BAA0`),
  `StateConditionObjectDistance` (528 bytes, `0x19573E8`, newer than the
  map) and `StateConditionSoundProgress` (536 bytes, `0x195BD30`) are
  float comparisons of the time spent in the scope, the graph's random
  draw, the distance between two objects and a named sound's progress.

## Weak evidence

- `StateNode::UsesColorBlending`/`UsesRotationBlending` and
  `_HasActivity`, the blend partner and weight slots of
  `StateNodeInstance`, and the `StateNodeTiming` slot names follow their
  uses.
- `StateCondition::_IsValid`, `StateConditionInstance::_IsResultCurrent`
  and `mHeld` are inferred.
- `StateTransition::_LoadBlendCurve` avoids `Component::_LoadResources`;
  `StateTransitionInstance::End` is the body at `0x7F8A20`, which the
  instance vectors' destruction does not call.
- `DrivenPropType`, `GetPropValue`, `SetDrivenPropDefault`,
  `gTimeUnitsAllowedValues`, `GetAnimTypeAllowedValues` and the
  transition's accessor and tag helper names are inferred; the meaning of
  the transition's `BlendsOverTime` test is a guess.
- The classes newer than the map (Folder, TriggerAction,
  TriggerAudioClip, ObjectDistance, SoundProgress, the increment nodes
  and the blend tree components) have inferred member names. The binary's
  strings suggest the blend tree classes were `StateNodeBlendTree`,
  `BlendTree`, `ClipBlendTree` and `ClipJukebox`.
- Field names of the nodes and conditions follow their properties.
