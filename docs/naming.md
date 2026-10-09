# Naming and class conventions

The reconstruction is written as the original C++ source, using names
recovered from `files/rockband_ps4_r.map`. That map comes from an older release
build. It is not committed, and its addresses do not match the analyzed
binary. Behaviour, signatures and layouts always come from the binary; the map
supplies only names and module placement.

## Names

- Classes, methods, free functions and globals use their original spelling in
  the global namespace: `RndShaderBlur::Select`, `BinStream::ReadEndian`,
  `GammaToLinear_sRGB`, `MemAlloc`.
- Data members use the engine's `mName` style. Static members and file-scope
  globals use `gName` or `sName`, as the original does.
- When the map has no name for something, choose one in the same style and add
  `// Name not in the reference map.` at its declaration. Common reasons:
  - the code was added after the map's build;
  - the function was inlined;
  - the member is a data field.
- When a signature in the binary differs from the map, follow the binary and
  add a comment giving the map's signature.

## Classes and vtables

- Polymorphic engine types are real C++ classes. Declare their virtual methods
  in the binary's slot order; Itanium places the complete and deleting
  destructors in the first two slots for a virtual destructor declared first.
  Annotate each virtual with its slot and implementation address.
- Assert layouts with `static_assert(offsetof(...))` and `sizeof`. The build
  passes `-Wno-invalid-offsetof` so these asserts work on polymorphic classes.
- Engine functions not yet reconstructed are declared in their owning class
  header with the original signature. The relocatable link's undefined-symbol
  list then names real engine symbols. Reserve `*_adapters.h` files for code
  that has no original home.

## Files

- Each translation unit is named after its object file in the map, such as
  `RndShaderBlur.cpp` or `PS4Context.cpp`. A class reconstructed across several
  object files keeps those object names (`Thread.cpp`, `Thread_PS4.cpp`).
- A file's folder is `src/<module>/<domain>/`, where `<module>` is the map
  directory (`render`, `renderps4`, `utl`, `os`, `math`, `audio`, …).
- Reconstruction comments keep the form
  `// Reconstructed from eboot.elf at 0xADDRESS.`
