# Scene-mask targets

`RndBufferCollection::_AllocSceneMaskBuffer` at `0x6B1760` creates three owned render
targets when target-resource flag `0x200` is set. `Mask Buffer` and
`Mask Scratch Buffer` use the full target extent. `Mask Tile Buffer` uses
`ceil(width / mask_tile_size)` by `ceil(height / mask_tile_size)`, where the
renderer setting at offset `0x80` defaults to 16.

The three resources occupy owner offsets `0x200`, `0x208`, and `0x210`.
Creation can reuse the corresponding target from a previous resource owner;
the new targets are also registered in the owner's resource list. The exact
render-target descriptor assembly and virtual factory call remain behind a
narrow adapter until the common target-resource owner is reconstructed.

The matching portion of `RndBufferCollection::Destroy` at `0x6AFFE0`
invokes each target's virtual deleting destructor in slot order and clears all
three pointers.
