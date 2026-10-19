# State graph runtime

The state graphs of the map's `stategraph` module: the graph resource,
the driver component that runs graphs for an entity, and the running
instance. Names follow [naming.md](naming.md); the graph components are
in [stategraph-components.md](stategraph-components.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `stategraph/StateGraph.o` (`Init`) | `0x6F6090` | `src/stategraph/graph/StateGraph.cpp` |
| `stategraph/StateGraphDriverCom.o` | `0x7013A0`-`0x703CB5` | `src/stategraph/drivers/StateGraphDriverCom.cpp` |
| `stategraph/StateGraphInstance.o` | `0x703CC0`-`0x706C38` | `src/stategraph/runtime/StateGraphInstance.cpp` |
| `stategraph/StateGraphResource.o` | `0x706C40`-`0x70840B` | `src/stategraph/resources/StateGraphResource.cpp` |

`StateGraph.o` (`0x6F6090`-`0x7013A0`) also holds every class's `Init`
and `_Create` and the inline methods the templated classes share; the
classes define theirs with the class. Every class `StateGraph::Init`
registers is reconstructed (see
[stategraph-components.md](stategraph-components.md)).

## Module registration

`StateGraph::Init` (`0x6F6090`) registers the graph resource, the
driver, every node, transition and condition class and the three blend
tree components (`BlendTreeBase`, `ClipBlendTree`, `ClipJukebox`), the
`DrivenAudioParamSettings` property struct (304 bytes) and the groups of
templated node and condition types (`StateNodeSetValueInit`,
`StateNodeSetPropertyInit_A`-`_C`, `StateConditionComparisonInit_A`/`_B`,
`StateConditionPropertyComparisonInit_A`-`_C`,
`StateConditionEventParameterInit`, and three
`StateNodeIncrementPropertyInit` groups this build adds, the last
empty). `App::Initialize` called it as the placeholder
`game_audio_register_types`.

## Graph resource

`StateGraphResource` (200 bytes, vtable `0x193A6A0`, 33 slots) is an
`EntityResource` with the extension `state`. A new entity gets a
`StateNodeRoot` on its root object. The graph's revision (`mRev`, in the
entity resource's tail padding) follows the entity from entity revision
25 on; `Save` writes 2. A graph saved before revision 1 has its root's
`StateNode` replaced by a `StateNodeRoot` that takes its properties
through a memory stream, and `_PostLoad` points every transition at the
node that lists it and loads the transitions' blend curves.

The editor helpers create graph objects named after their class
(`GetBaseName` strips a prefix and everything from the first underscore;
the base node is "No-Op"), find the transition that owns a condition and
the blend tree that holds a node, and list the transitions and
conditions no node reaches (`GetUnusedObjects`). `GetContextEntityPrototype`
is the entity of the entity resource the graph is inlined into.

## Driver

`StateGraphDriverCom` (456 bytes, vtable `0x193A518`, 41 slots) keeps its
state in `RuntimeData` (constructor `0x7013A0`), which the imprint copy
builds afresh:

- the running instances, in activation order;
- the sounds the graphs play under a name (`mSoundHandles`) and without
  one; `_Poll` forgets unnamed sounds that ended, `_Exit` stops them all;
- the queue of resource path properties waiting for their resource
  (`ResourceLoaderJob`: a `PropRef` and a path) and the current
  `ResourceLoader`;
- a poll counter and the phase the next every-other-frame graph takes;
- the entity's clock, taken in slot 29.

`_Enter` activates every inlined graph whose root starts on enter.
`Activate` keeps a running instance when no object is given and
restarts it otherwise. `_Poll` polls each instance due this frame
(`ShouldPoll`) and deletes those that finish, then assigns the loaded
resources one loader at a time. The driver precedes the `AnimDriverCom`
in the component order.

Named sounds live in the driver's table or, for
`SoundHandleScope::kSoundHandleGlobal`, in `sGlobalSoundHandles`
(`0x1AB2998`, guarded by `0x1AB2988`); a name is refused while its sound
still plays. `SoundManager::sAddSoundHandleFunc` points at the static
`AddSoundHandle` (`0x701970`), which `_Init` sets.

## Running graph

`StateGraphInstance` (184 bytes, from the pool, vtable `0x193A670`, a
`MsgSink`) runs one graph for an entity:

- the node instances, newest first, and the running transitions, the
  transition at index *i* blending from instance *i* + 1 into instance
  *i*;
- one `TransitionScope` (80 bytes) for each node from the outermost
  folder to the current node: the node, its transitions with their
  condition instances and whether they are synchronized, the time spent
  in the scope by clock and `TimeUnits` (since the incoming transition
  began and since it ended), and two flags;
- the current node's timing position at the last search, the random
  value `GetRandom` draws once per search, and the poll phase.

`_Poll` (`0x704730`) skips the time on its first call, gives each
transition its blend's share of what the newer ones left and the oldest
node the rest, drops what a finished transition leaves behind, polls the
nodes oldest first, and then takes transitions until none applies, at
most a thousand in a row and never past 32 node instances.
`_FindTransition` searches the scopes from the given one outwards: a
transition qualifies when its condition is met and, when synchronized,
when the current node's timing position crossed its start time since the
last search; an unconditional unsynchronized transition is the fallback
when nothing else applies. While a transition runs, only interruptible
ones are considered, and only when the running one can be interrupted.
A node that finished with no transition left ends the graph.

Entering a folder enters its start node and takes any transition that
leaves it at once. `_ActivateTransition` enters the target, lets it pick
a blend partner among the older instances, and drops every other node at
once for a transition that takes no time.

The entity's sinks deliver `on_entity_destroyed`,
`on_object_destroyed` and `on_component_destroyed`; the graph
deactivates when a node instance, a transition or a condition it runs or
may run uses the object.

## Weak evidence

- The names of the driver's sound handle overloads and scopes,
  `StopGlobalSounds`, `FindNodeInstance`, `GetInlineOrLoaded`, and the
  instance's scope machinery (`TransitionScope`, `TransitionEntry`,
  `_EnterNode`, `_PushScopes`, `_PopScopes`, `_EndIncomingTransitions`,
  `_FindTransition`) are inferred from their bodies.
- `StateGraphInstance::Poll` is the map's name for the thunk at
  `0x7046F0`; the body at `0x704730` is named `_Poll` and the timed copy
  at `0x704700` `PollTimed`.
- The resource's `FindTransitionWithCondition`, `FindParentBlendTree` and
  `GetUnusedObjects` are inferred; the map's `FindTransitionOrigin` has
  no match in this build.
- The `StateNodeIncrementPropertyInit` names are inferred; the
  `StateNodeSetPropertyInit_A`-`_C` letters match the map's type lists.
- `BlendTreeBaseCom`, `ClipBlendTreeCom` and `ClipJukeboxCom` follow
  their class ids.
