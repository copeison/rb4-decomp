# Script data

The script values that console commands and configuration use are modelled in
`src/utl/data`, after the map's `utl/DataNode.o`, `DataArray.o` and
`DataFunc.o`. `DataArray.cpp` and `DataNode.cpp` hold the reconstructed parts
of the first two.

## DataNode

A `DataNode` is 16 bytes: an 8-byte value and a `DataType`. The type values
follow the Milo engine:
- integer `0`, float `1`, variable `2`, function `3`, object `4`, symbol `5`,
  unhandled `6`;
- the parser directives `7`–`9` and `0x20`–`0x25`;
- array `0x10`, command `0x11`, string `0x12`, property `0x13`, glob `0x14`.

These types are new in this engine (their names are inferred):
- two waveform types, `0x26` and `0x27`;
- `kDataGameObjectId` (`0x28`);
- `kDataResourcePath` (`0x29`). Its value is a path string. `Str` returns the
  path, and `Sym` resolves it through `FileResolvePath`.

A variable node holds the variable's index, not a pointer.

Types with bit `0x10` hold a `DataArray`. A string or glob is a `DataArray`
whose node pointer holds the bytes and whose size is negative.

Copying and destroying a node is inline:
- **Arrays** are reference-counted at `+0x10` of the array.
- **Waveforms** are resources. `AddRefWaveform` (`0x239290`) and
  `ReleaseWaveform` (`0x2392A0`) forward to `Resource::AddRef` and
  `Resource::ReleaseRef`.

`DataNode::Evaluate`, `Int`, `Float`, `Sym` and `Array` are inline in the
map's build. The binary keeps weak copies at `0xEB40`, `0xEE30`, `0xE850` and
`0x5F0B0`. `Str` (`0x237570`) is out of line. It keeps a command's result in
the thread's `DataThread`.

## DataArray

A `DataArray` is 24 bytes: `mNodes`, the file `Symbol`, an atomic `mRefs`, and
16-bit `mSize` and `mLine`. It is pool-allocated, and `Release` deletes it
when the last reference drops; the binary also keeps an out-of-line copy at
`0x21FF70`.

The resizing operations (`Insert`, `InsertNodes`, `Resize`, `Remove`) always
reallocate the nodes. They copy the kept nodes and free the old storage.

`Evaluate` (inline, copy at `0xC7D30`) resolves variables, commands and
properties.

## Execution and variables

`DataExecute` (`0x21EAB0`) evaluates the first node and dispatches on it:
- a function is called;
- an object receives `MsgSink::Handle`;
- a component reference array goes to the component from `GetDataCom`;
- a symbol is matched against the class symbols of the thread's
  `GameObject`'s components, and then against `gDataFuncs`.

`DataExecuteBlock` (`0x21EDA0`) runs the nodes as commands, returns the last
one evaluated, and pops the variables they pushed.

Each thread has its own `DataThread` (`0x33C8` bytes). It holds:
- 500 variable values;
- a 200-entry stack of saved values;
- the context that commands run in.

The binary reaches it through the thread-local descriptor at `0x19B0378`. It
is modelled as `thread_local gDataThread`. The map has a
`TLSValue<DataThread>` in `utl/DataUtl.o`.

The variable indices are kept in an `eastl::map<Symbol, unsigned long>` under
`gVarIndexCrit`:
- `DataVarIndex(Symbol)` (`0x236750`) registers a name;
- `DataVarIndex(Symbol, DataNode)` (`0x236910`), which the map lacks, also
  writes the initial value into every thread's slot;
- `DataSetGlobal` (`0x236D50`) writes a value into every thread's slot.

## Script functions

`DataRegisterFunc` (`0x2221F0`) stores a `DataNode (*)(DataArray*)` under its
`Symbol` in `gDataFuncs` (`0x19E77A0`).

## Lookups

- `DataArray::FindArray(Symbol, bool)` (`0x21C970`) finds the child array
  whose first node is the key. The release build ignores `fail`. The two- and
  three-key forms (`0x21C9C0`, `0x21CA30`) assume each level exists.
- The `FindData` overloads read the node after the key:
  - `String&` at `0x21CE20`;
  - `const char*&` at `0x21CF20`;
  - `Symbol&` at `0x21CFC0`;
  - `int&` at `0x21D060`;
  - `unsigned long&` at `0x21D100`;
  - `float&` at `0x21D1B0`;
  - `bool&` at `0x21D260`, which uses `NotNull`.
- `SystemConfig` with one, two or three keys (`0x368B00`, `0x368CD0`,
  `0x369A90`) walks the root configuration with `FindArray` and failure
  off.
