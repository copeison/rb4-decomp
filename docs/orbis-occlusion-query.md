# Orbis occlusion query

`orbis_create_occlusion_query` at `0x8D8C30` is virtual slot 34 of the Orbis
render system. It allocates a 72-byte object and calls the platform constructor
at `0x8E28C0`. The common constructor at `0x5F7D50` stores the name, clears
the enabled flag and the two per-view culled flags, copies `Sphere::sZero`
into the query sphere, sets the manager index to `-1`, clears the expected
pixel count, and creates a self-linked intrusive-list node. These fields
form the exact 64-byte `RndOcclusionQuery` base.

The Orbis constructor installs the platform vtable and clears the eight-byte
backend query address. The begin method at `0x8E2920` reserves a 256-byte block
aligned to 16 bytes from the active graphics command arena, stores its address
at object offset `+64` as a `sce::Gnm::OcclusionQueryResults*`, issues
`writeOcclusionQuery(kOcclusionQueryOpClearAndBegin)`, and enables perfect
Z-pass counts with `setDbCountControl`. The method at `0x8E29E0` writes
`kOcclusionQueryOpEnd` to the same results and disables the counts. All four
calls go through the active `GfxContext` (`PS4OcclusionQuery.cpp`).

The final two virtual methods control conditional rendering. The method at
`0x8E2A30` calls `setZPassPredicationEnable` with `kPredicationZPassHintWait` and
`kPredicationZPassActionDrawIfVisible`, so predicated draws wait for the
result and draw when any fragment passed. The method at `0x8E2A60` calls
`setZPassPredicationDisable`. The
surrounding renderer code labels this subsystem `Occlusion Queries` and
`Occlusion Query Coverage`.

The platform destructor at `0x8E28F0` delegates to common teardown, which
unlinks the query's intrusive-list node. The deleting destructor at `0x8E2900`
then frees the object. The query result belongs to the per-frame command arena
and therefore needs no object-owned release.

The shared lifecycle is reconstructed in
`src/render/queries/RndOcclusionQuery.cpp`. The Orbis type derives from that
base and adds only the eight-byte result address, preserving the original
72-byte platform object.

IDA originally treated the five methods after the constructor as data inside
the next function. Their boundaries are now restored at `0x8E2900`,
`0x8E2920`, `0x8E29E0`, `0x8E2A30`, and `0x8E2A60`.

IDA evidence is preserved in
`analysis/exports/render-occlusion-query.asm`,
`analysis/exports/render-occlusion-query.c`,
`analysis/exports/orbis-occlusion-query-backend.asm` and
`analysis/exports/orbis-occlusion-query-backend.c`.

## Shader constants

`RndOcclusionQuery::SetShaderConstants` (`0x5F7E00`) writes the query's
index and expected pixel count, as floats, into
`RndShaderMgr::mOcclusionQueryCoverageParams` of the context's occlusion
constant buffer (`mCBuffers[4]`). It then uploads and selects the buffer.
`SetNoOcclusionQueryShaderConstants` (`0x5F7E70`) writes `(0, 1)` into a
buffer and marks it dirty. `RndDevice::_InitBuiltinCBuffers` calls it for
the default buffer; the reconstruction inlines that call.

## Manager

`RndOcclusionQueryMgr` (`render/RndOcclusionQueryMgr.o`, `0x5F7EC0`-
`0x5F8D60`, 88 bytes) belongs to a scene drawer. It holds:

- the number of registered queries, which is also the next index;
- the coverage buffer (`+8`), one uint per query, named
  "Occlusion Query Coverage";
- the head of the query list, allocated by the constructor (`0x5F8010`);
- one vector of visible queries per view (`+24`, `+56`).

The destructor (`0x5F8080`) unlinks and frees the list head. The queries
stay linked to each other.

- `RegisterQuery` (`0x5F8110`) appends the query to the list and gives it
  the next index.
- `PostEnter` (`0x5F8140`) and `Poll` (`0x5F8270`) recreate the coverage
  buffer (`_AllocCoverageBuffer`, `0x5F81E0`). `PostEnter` does this when
  the size differs from the query count, and `Poll` when the buffer is
  smaller.
- `Submit` (`0x5F8310`) clears the camera's view list. It classifies each
  enabled query's sphere against the camera frustum
  (`ConvexHullBase::_ClassifySphereImpl`, `0x116E9A0`, through the inlined
  `Frustum::ClassifySphere`). A query outside the frustum is marked culled
  for the view. A visible query is listed, and its expected pixel count is
  set to `ceil(r * r * 2 * sqrt(2))`. Here `r` is half the projected
  height, and the factor is the area of the octagon that the 8-segment
  sphere projects to (`8 sin(pi/8) sin(5pi/8)`).
- `GetViewIndex` (`0x5F8590`) maps 2D and cube-face targets to view 0 and
  the eyes to views 0 and 1.
- `DrawQueries` (`0x5F85C0`, "Occlusion Queries") turns the coverage
  buffer into an unordered-access buffer and clears it with
  `RndCShaderClearBuffer`. It binds only the depth target, sets blend mode
  `kDestination`, depth mode 3 and back-face culling, and selects
  `RndShaderOcclusionQuery::Select` (`0x5F8FC0`, which binds the buffer
  read-write to the pixel stage). For each listed query, it writes the
  index into the smallest per-draw constant buffer that fits, then draws
  the sphere inside the query. Finally it hands the buffer back as a
  pixel-shader resource.
- `DrawOcclusionSphere` (`0x5F8BF0`) draws the 8 x 4 "Occlusion Query
  Sphere" over a sphere. The sphere is scaled by its radius and turned to
  the camera's rotation. `RndLightFlareEditCom` calls it.

The static initializer of `RndOcclusionQuery.o` (`0x5F7EA0`) sets three
ints of a shared header (`0x1AA7C04`), which are not modelled.
