# Orbis occlusion-query factory

`orbis_create_occlusion_query` at `0x8D8C30` is virtual slot 34 of the Orbis
render system. It allocates a 72-byte object and calls the platform constructor
at `0x8E28C0`. The common constructor at `0x5F7D50` stores the owner, resets
the query flags and per-frame values, and initializes an intrusive-list node.

The Orbis constructor installs the platform vtable and clears the eight-byte
backend query address. The begin method at `0x8E2920` reserves an aligned
256-byte query block from the active frame's command arena. The methods at
`0x8E29E0` and `0x8E2A30` end the query and resolve its result through the
Orbis command context. The surrounding renderer code labels this subsystem
`Occlusion Queries` and `Occlusion Query Coverage`.

IDA originally treated the five methods after the constructor as data inside
the next function. Their boundaries are now restored at `0x8E2900`,
`0x8E2920`, `0x8E29E0`, `0x8E2A30`, and `0x8E2A60`.
