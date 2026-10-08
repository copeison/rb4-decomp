# PS4 reconstruction build

The installed PS4 SDK 5.500 provides a working native Orbis toolchain:

- `orbis-clang++`: Clang 5.0.1, PS4 version 5.50.0.425
- `orbis-ld`: Orbis linker 5.50.0.4727
- Target: `x86_64-scei-ps4`
- Runtime headers: release 05.508.071

The reconstructed source uses C++17 language support where the compiler
provides it, while avoiding library facilities missing from this SDK's libc++.

Run the object build from the repository root:

```powershell
.\tools\build_ps4.cmd
```

The script uses `SCE_ORBIS_SDK_DIR` when set. Otherwise it selects the newest
directory under `C:\Program Files (x86)\SCE\ORBIS SDKs`. An explicit SDK path
can be supplied when needed:

```powershell
.\tools\build_ps4.cmd `
  -SdkDir "C:\Program Files (x86)\SCE\ORBIS SDKs\5.500" `
  -Configuration Release
```

Products are written under the ignored `build/orbis/<configuration>/`
directory. The script compiles every reconstructed translation unit, creates
`librb4_reconstruction.a`, combines the objects into the relocatable
`rb4_reconstruction.o`, records object sizes and hashes in `objects.csv`, and
records the combined object's unresolved external dependencies in
`undefined-symbols.txt`. Pass `-EmitDisassembly` to produce an assembly listing
beside each object for structural comparison with IDA.

This build validates source syntax, target ABI compatibility, and recovered
object structure. Producing a complete executable still requires reconstruction
or adapter implementations for the game's external engine functions and the
matching FMOD libraries. The SDK 5.500 compiler is also newer than the game's
5.000-generation toolchain, so byte-identical code generation is not expected.
