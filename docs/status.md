# Decompilation status

## Milestones

- [x] Recover the ELF payload from `eboot.bin`.
- [x] Identify the executable format and compiler SDK baseline.
- [x] Prepare an IDA-compatible analysis copy without modifying program data.
- [x] Create the persistent IDA database and export the initial function map.
- [x] Recover and name the startup path.
- [x] Resolve Orbis imports against the PS4 SDK 5.008 stub libraries.
- [x] Classify major subsystems and source translation units.
- [x] Begin function-by-function C++ reconstruction.
- [x] Reconstruct the top-level game initialization sequence.
- [x] Reconstruct the main per-frame update and render schedule.
- [x] Recover the complete UI layout ID and primary asset-path map.
- [x] Resolve the private pad imports and reconstruct the special-controller calibration sample path.
- [x] Recover all FMOD 1.10.04 import names and apply them to IDA.
- [x] Reconstruct FMOD module setup and the primary audio initialization path.
- [x] Reconstruct the synchronous and asynchronous FMOD file I/O bridge.
- [x] Reconstruct the engine-to-FMOD listener transform bridge.
- [ ] Establish a PS4 SDK 5.008 build and comparison loop.

## Naming conventions

- Confirmed names use their original spelling when recovered from RTTI,
  assertions, logging strings, imports, or source paths.
- Inferred names use a descriptive `subsystem_action` form until stronger
  evidence becomes available.
- Unidentified functions keep IDA's address-based name.
- Every reconstructed function records its source address in a nearby comment.

## Evidence policy

Function names and types should be tied to at least one concrete signal:
callers/callees, referenced strings, RTTI, vtable position, imported APIs,
structure offsets, or behavior visible in pseudocode. Guesses are marked as
such rather than silently promoted to confirmed names.
