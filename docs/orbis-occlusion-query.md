# Orbis occlusion query

`orbis_create_occlusion_query` at `0x8D8C30` is virtual slot 34 of the Orbis
render system. It allocates a 72-byte object and calls the platform constructor
at `0x8E28C0`. The common constructor at `0x5F7D50` stores the owner, resets
the query flags and per-frame values, and initializes an intrusive-list node.

The Orbis constructor installs the platform vtable and clears the eight-byte
backend query address. The begin method at `0x8E2920` reserves a 256-byte block
aligned to 16 bytes from the active graphics command arena, stores its address
at object offset `+64`, begins the hardware query, and enables query
collection. The method at `0x8E29E0` ends the query at the same address and
disables collection.

The final two virtual methods control conditional rendering. The method at
`0x8E2A30` configures the active graphics command buffer to predicate draws on
the stored query result. The method at `0x8E2A60` clears that predicate. The
surrounding renderer code labels this subsystem `Occlusion Queries` and
`Occlusion Query Coverage`.

The platform destructor at `0x8E28F0` delegates to common teardown, which
unlinks the query's intrusive-list node. The deleting destructor at `0x8E2900`
then frees the object. The query result belongs to the per-frame command arena
and therefore needs no object-owned release.

IDA originally treated the five methods after the constructor as data inside
the next function. Their boundaries are now restored at `0x8E2900`,
`0x8E2920`, `0x8E29E0`, `0x8E2A30`, and `0x8E2A60`.

IDA evidence is preserved in
`analysis/exports/orbis-occlusion-query-backend.asm` and
`analysis/exports/orbis-occlusion-query-backend.c`.
