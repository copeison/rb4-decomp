# Render particle component

`RndParticleCom` is the particle system component of the map's `render`
module, a `RndDrawInstanceCom` subclass (see
[entity-components.md](entity-components.md)). Its sources are in
`src/render/particles`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndParticleCom.o` | `0x601880`-`0x61E1F5` | `0x192BD08` (46 slots) | 3440 |

The object starts with `StaticInit` (`0x601880`) and `StaticTerminate`
(`0x601910`), which create and delete the unit box (`sDefaultMesh`,
`0x1AA8E48`) a mesh system without a mesh path draws. `Rnd::Init` and
`Rnd::Terminate` call them; they were formerly attributed to an
`RndParticleBouncePlaneEditCom`.

## Class registration

The class id and the class symbol are both `"PSys"` (the editor names the
class `RndParticleCom`; the registry builder calls it `ParticleSystem`).

| Static | Address |
| --- | ---: |
| `sId` | `0x1AA8B68` |
| `sClassName` | `0x1AA8B70` |
| `sPropRegistry` | `0x1AA8B80` |
| `sMetaData` | `0x1AA8C20` |
| `sDefaultPitch` ... `sDefaultExtraData` | `0x1AA8DC8`-`0x1AA8E40` |
| `_Init` | `0x603850` |
| `Init` / `_Create` (with the other render components) | `0x3FA5F0` / `0x405B30` |

The initializer (`0x61DF50`) builds the symbols, the registry and the
metadata, then registers the destructors of 16 default waveforms
(`ResourcePtr<WaveformResource<float>>`, one `<Hmx::Color>`) that `_Init`
creates and the parameter constructors copy.

## Layout

| Offset | Member | Properties |
| ---: | --- | --- |
| `0x0C0` | `EmitterParams` | duration, emit_can_kill_oldest, never_kill, min/max_box_extents, pitch, yaw, birth_momentum(_amount), subsamples, local_space |
| `0x140` | `ParticleParams` | particle_type, mesh_resource_path, max_count, emit_rate(_multiplier), emit_distance_per_particle, lifespan(_multiplier), speed, speed_multiplier_over_life, drag, initial_size, x/y/z_scale_over_life, uniform_scale, x/y/z_pivot_over_life, offset_then_scale, force_direction |
| `0x340` | `ColorParams` | color, alpha |
| `0x390` | `SpinParams` | initial_rotation, initial_x/y/z_rotation_angle, x/y/z_orientation (a Matrix3), x/y/z_rpm_over_life, x/y/z_rpm_drag |
| `0x4C0` | `ExtraDataParams` | extra_data_type_0-2, extra_data_0-2 |
| `0x548` | `RenderParams` | particle_alignment, sort_method |
| `0x550` | | time_units, sim_when_hidden |
| `0x558` | `CollisionParams` | per_particle_collision_shape, elasticity, colliders |
| `0x588` | | affectors |
| `0x5B0` | `RuntimeData` | burst_count (min/max), num_active |

Waveform properties are `Waveform<float>` / `Waveform<Hmx::Color>` of
`entity/Waveform.o` (40 bytes: resource, eval data, `Rand*` at +16,
animation override). They are not modelled, so they are opaque storage
and the parameter constructors, copies and destructors are declared only.

`RuntimeData` holds the extension list (`RndParticleExtCom*`), the mesh
resource and mesh, the clock, `TransCom` and `RndDrawNodeCom`, the system
time, the last position and transform, the rate and distance remainders,
the `RndParticleCollection` (`0x658`), the `Rand` (seeded from its address),
the `RndParticleBuffer`, the draw transform, the velocity-align flag and the
queued birth transforms.

## Behaviour

- `_OnResourcesLoaded` (slot 29) caches the components and the clock, finds
  the mesh among the entity's inlined resources or loads it, allocates the
  collection and the sprite buffer and installs the generator.
- `_Poll` (slot 33): a hidden system without `sim_when_hidden` empties its
  sphere and particles. Otherwise the time advances; in local space the
  particles spawn at the identity and the affectors see the draw node's
  inverse world transform, in world space the reverse. `_UpdateParticles`
  kills expired particles (or ones born after the clock went back), updates
  the life fraction, force and affectors, positions and colliders, spin,
  pivot, color/size, extra data and spawning; then the bounding sphere is
  fitted, the buffer gets its collection, transform, alignment, sort and
  flags, and the extensions poll.
- Spawning queues births by distance travelled and by rate (with bursts and
  subsamples), kills the oldest particles when allowed, creates the particles
  and initializes them (`_InitParticles`, which places them on the
  `RndParticleEmitterCom` shape or the emission box).
- Slots 41-45: one sprite instance or one mesh instance per particle per
  level; the lit or particle default material; slot 43 forwards to
  `_SyncInstanceStateFlags`; slot 44 fills the instances; slot 45 is false.

`_Imprint` (`0x6181C0`) is reconstructed; it places a copy made by the copy
constructor (`0x619130`) and imprints the properties past it.

Not reconstructed: `_Init`, the copy constructor, the parameter
constructors and destructors, `_InstallRand`, `_UpdatePositions`,
`_UpdateRateEmission`, `_UpdateBirth`, `_UpdateSpin`, `_UpdatePivot`,
`_UpdateColorSize`, `_UpdateExtraData`, `_InitParticles` and
`_SyncDrawInstancesImpl`. They build, copy or evaluate the opaque
waveforms (`Waveform<float>::Evaluate` at `0x2D7C60`, `ValueRange` at
`0x6178D0`).

## Related components

Declared in their own headers for the calls the system makes; their layouts
are not decoded. None is in the reference map except the velocity aligner;
the names come from the editor strings.

| Class | Id | Vtable |
| --- | --- | ---: |
| `RndParticleAffectorCom` | `PSysAffector` | `0x192B2D0` (43) |
| `RndParticleColliderCom` | `PSysCollider` | `0x192B7E8` (41) |
| `RndParticleEmitterCom` | `PSysEmitter` | `0x192CDF8` (43) |
| `RndParticleExtCom` | `PSysExtension` | `0x192D4B8` (42) |
| `RndParticleVelocityAlignCom` | `PSysVelocityAlign` | `0x192DE80` (42) |

## Weak evidence

- `_UpdateBirth` for `0x617A40` and `_UpdateRateEmission` for `0x6122C0`;
  `_UpdateAffectors`, `_UpdateExtraData`, `_PlaceNewParticles` and the
  `Gather*` names.
- `RndParticleVelocityAlignCom` deriving from `RndParticleExtCom` (shared
  slot 20 and slot 41).
- `mParticleTypeChanged` (written by the particle_type handler, never read
  here).

## Particle collection

`render/RndParticleCollection.o` (`0x6ED3D0`-`0x6F292A`) is reconstructed in
`src/render/particles/RndParticleCollection.cpp`. It covers the constructor, the
destructor, `DeleteAllParticles`, `SetMaxCount`, `CreateParticle`,
`DeleteParticle`, `_CopyParticle` (`0x6F1170`), `_FreeAllChunks`
(`0x6F1550`), `FreeUnusedChunks` and `gNextParticleID` (`0x1AB1F68`). The
collection holds 41 `RndParticleAttr<T>` members. Each one is an
`eastl::vector<T*>` of 16-element chunks plus its pool. The attribute types
are int, `Hmx::Color`, float, `unsigned long` (the live-list links) and 15
`WaveformEvalData` attributes. The chunks come from the free-list pools of
`theParticleSystemMgr` (`RndParticleSystemMgr.h`, layout only), and every
pool operation locks `sParticleSystemCritSec`. The attribute names are
inferred from their readers. `mInitialSizes` (attribute 14) rests on weak
evidence. `ReorderParticle` from the map does not appear in this build.
`RndParticleSystemMgr.o` itself (constructor `0x628ED0`, destructor
`0x6292E0`, `Init` `0x628E60`, `Terminate` `0x628E90`,
`sParticleSystemCritSec` `0x1AAA460`, `theParticleSystemMgr` `0x1AAA470`)
is not reconstructed.
