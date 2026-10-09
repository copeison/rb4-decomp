# Script data

The script values that console commands and configuration use are modelled in
`src/utl/data`, after the map's `utl/DataNode.o`, `DataArray.o` and
`DataFunc.o`.

## DataNode

A `DataNode` is 16 bytes: an 8-byte value and a `DataType`. The type values
follow the Milo engine:
- integer `0`, float `1`, variable `2`, function `3`, object `4`, symbol `5`;
- array `0x10`, command `0x11`, string `0x12`, property `0x13`;
- two waveform types, `0x26` and `0x27`, which are new in this engine.

Types with bit `0x10` hold a `DataArray`. A string is a `DataArray` whose node
pointer holds the characters and whose size is negative.

Copying and destroying a node is inline:
- **Arrays** are reference-counted at `+0x10` of the array.
- **Waveforms** use `DataNode::AddRefWaveform` (`0x239290`) and
  `ReleaseWaveform` (`0x2392A0`). These lock a global mutex, and the last
  reference destroys the waveform through its virtual slot 11.

## DataArray

A `DataArray` is 24 bytes: `mNodes`, the file `Symbol`, an atomic `mRefs`, and
16-bit `mSize` and `mLine`. It is pool-allocated, and `Release` deletes it
when the last reference drops; the binary also keeps an out-of-line copy at
`0x21FF70`.

`Size` and `Node` are inline. `Evaluate` (`0xC7D30`) resolves variables,
commands and properties.

## Script functions

`DataRegisterFunc` (`0x2221F0`) stores a `DataNode (*)(DataArray*)` under its
`Symbol` in a red-black tree.

## Lookups

These lookups are not inline:
- `DataArray::FindArray(Symbol, bool)` (`0x21C970`) finds the child array
  whose first node is the key.
- The `FindData` overloads read the node after the key: `Symbol&` at
  `0x21CFC0`, `int&` at `0x21D060` and `bool&` at `0x21D260`.
- `SystemConfig` with one, two or three keys (`0x368B00`, `0x368CD0`,
  `0x369A90`) walks the root configuration with `FindArray` and failure
  off.

`Int`, `Sym` and `Str` by index are inline.
