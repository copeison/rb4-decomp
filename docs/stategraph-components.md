# State graph components

The components a state graph's entity is made of: the nodes, the
transitions between them and the conditions that start the transitions.
Each sits on its own object of a `StateGraphResource`'s entity. Names
follow [naming.md](naming.md); the resource, the driver and the running
graph are in [stategraph-runtime.md](stategraph-runtime.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `stategraph/StateBase.o` | `0x807E80`-`0x808AD5` | `src/stategraph/base/StateBase.cpp` |
| `stategraph/StateNode.o` | `0x708410`-`0x70AD77` | `src/stategraph/nodes/StateNode.cpp` |
| `stategraph/StateNodeRoot.o` | `0x78CF90`-`0x78DA27` | `src/stategraph/nodes/StateNodeRoot.cpp` |
| `stategraph/StateTransition.o` | `0x7F5220`-`0x7FB537` | `src/stategraph/transitions/StateTransition.cpp` |
| `stategraph/StateCondition.o` | `0x808AE0`-`0x809A92` | `src/stategraph/conditions/StateCondition.cpp` |
| `stategraph/StateConditionLogic.o` | `0x859690`-`0x85AE07` | `src/stategraph/conditions/StateConditionLogic.cpp` |
| `stategraph/StateConditionNodeFinished.o` | `0x85AE10`-`0x85B307` | `src/stategraph/conditions/StateConditionNodeFinished.cpp` |
| `stategraph/StateConditionNot.o` | `0x85B310`-`0x85BCC7` | `src/stategraph/conditions/StateConditionNot.cpp` |

Every class's `Init` and `_Create` are emitted in `StateGraph.o`; each
source defines them with its class. The other node, condition and blend
tree classes are declared only as far as `StateGraph::Init`,
`StateCondition::Create` and the reconstructed code use them (headers
under `nodes/`, `conditions/` and `blendtrees/`).

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

## Nodes

`StateNode` (152 bytes, vtable `0x193A7B8`, 50 slots) is the base node,
the editor's "No-Op". It holds when it is active relative to the
transitions into and out of it (`active_at`, `active_until`), its
rectangle, its outgoing transitions and its folder; `RuntimeData` keeps
the resolved folder, group and transitions. Slots 42-49 give the editor
text, the child nodes, the animation timing (`StateNodeTiming`), the node
instance, and whether the node blends, is exclusive, blocks, and has
activity of its own. `StateNodeInstance` (32 bytes, vtable `0x193A958`, 15 slots) is a node
in a running graph.

`StateNodeRoot` (160 bytes, vtable `0x1945AC0`) is the root of every
graph: `start_on_enter` and the poll frequency (every frame, or every
other one).

## Transitions

`StateTransition` (600 bytes, vtable `0x194ECF0`, 42 slots) links its
origin and target nodes and names its condition. It blends for a driven
duration along a driven blend curve (`DrivenProp<T>`, the float and
`Waveform<float>` copies) or synchronized to the nodes' animation timing
within a start window, and converts between seconds, beats and the
nodes' lengths. `StateTransitionInstance` is a running transition:
`Begin`, `Poll` and `End`. The registry builder (`0x7F5420`) is not
reconstructed.

`DrivenProp<T>` (`src/stategraph/props/DrivenProp.h`) is a value that a
property of another component can drive. `Register` is declared for the
copies `RndSceneCom.o` emits (`0x4107C0` float, `0x410EF0` colour) and is
not reconstructed.

## Conditions

`StateCondition` (40 bytes, vtable `0x1950818`, 54 slots) holds the
evaluation mode: reevaluate every frame, evaluate once, or keep the first
true or false result. `Create` makes a condition of a `Type`; the two
logical types share `StateConditionLogic`. Slots 42-53 create the
instance and edit and describe the children.
`StateConditionInstance` (32 bytes, seven slots) evaluates a condition
in a running graph and holds the result as the mode asks (`mHeld`).

- `StateConditionLogic` (112 bytes, vtable `0x1956DD0`): "operator" is
  And (`kAllOf`, the default) or Or (`kAnyOf`); the hidden
  "child_conditions" array lists any number of children, which may not
  be parameter comparisons. Its instance (80 bytes, vtable `0x1956F90`)
  stops at the first deciding child only when that child's result is
  held; the children after it are evaluated with cached results.
- `StateConditionNodeFinished` (vtable `0x1957020`; instance
  `0x1956FD8`): met once the node of the transition's scope finished.
- `StateConditionNot` (48 bytes, vtable `0x1957228`; instance
  `0x19571E0`): the inverse of its one child; without a child it is
  never met.

## Weak evidence

- `StateNode::IsExclusive`, `IsBlocking`, `_HasActivity`, the blend
  partner and weight slots of `StateNodeInstance`, and the
  `StateNodeTiming` slot names follow their uses.
- `StateCondition::_IsSelectable`, `_PrepareAllowedChildTypes` and
  `StateConditionInstance::_IsResultCurrent` and `mHeld` are inferred.
- `StateTransition::_LoadBlendCurve` avoids `Component::_LoadResources`;
  `StateTransitionInstance::End` is the body at `0x7F8A20`, which the
  instance vectors' destruction does not call.
- The field names of the conditions and their instances are inferred.
