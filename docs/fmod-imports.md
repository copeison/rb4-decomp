# FMOD import recovery

The executable imports 76 functions from `libfmod` and 59 from
`libfmodstudio`. All 135 names are now recovered in
`tools/ps4_symbol_names.csv` and applied to the IDA database.

## Version evidence

The audio initialization path at `0x2773C0` compares its compile-time FMOD
header version with `0x00011004`. This identifies FMOD Studio 1.10.04. The
public 1.10.01 headers provide nearly the same API surface; two recording
signatures and the listener-attribute signature changed within the 1.10 line
and were recovered from their call sites.

## Recovery method

PS4 imports store an 11-character NID instead of the original linker symbol.
The NID is deterministic: SHA-1 is calculated over the original symbol name
followed by Sony's fixed 16-byte suffix, the first eight digest bytes are
reversed, and the result is encoded with the PS4 Base64 variant.

The low-level and Studio C++ APIs use Itanium ABI mangled names. Public FMOD
headers supplied the class methods and parameter types; compiling member
pointer declarations with GCC produced the canonical mangled forms. Hashing
those forms matched every FMOD NID in the executable.

Call-site checks confirmed the version-sensitive results:

- `FMOD_Orbis_SetThreadAffinity` receives the thread-affinity table built at
  `0x261F60` before either FMOD module is initialized.
- `FMOD::System::getRecordNumDrivers(int *, int *)` and
  `getRecordDriverInfo(..., unsigned int *)` match the microphone enumeration
  code at `0x27B880` and `0x8ED0D0`.
- `FMOD::Studio::System::setListenerAttributes(int,
  const FMOD_3D_ATTRIBUTES *)` is called at `0x262358`. The helper at
  `0x27ACB0` builds the attribute block from the engine transform, clears its
  velocity, and changes handedness by flipping the X components.

## Reproducibility

`tools/resolve_ps4_imports.py` calculates the NID for every reviewed original
name in `tools/ps4_symbol_names.csv`. A name is accepted only when its hash and
library both match an executable import. The generated import table marks these
rows with `name_source=hashed-symbol`.

