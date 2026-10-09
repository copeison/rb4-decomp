# Render lighting resources

The render system embeds a 304-byte lighting-resource block at offset `0xCB8`.
`RndLightGlobals::RndLightGlobals` reconstructs its constructor at
`0x47EEE0`: every resource owner begins null, the scalar at offset `0x10`
starts at `-1.0`, the two fixed group sizes are 16 and 8, and both dynamic
pointer arrays begin empty.

Runtime initialization at `0x47F030` creates the lighting sphere, derives the
scalar used by the lighting passes, prepares the remaining targets, and creates
the tiled-light count compute buffer. That creation sequence remains behind a
typed adapter while its subordinate resource types are recovered.

Shutdown at `0x47F580` is source-owned. It dynamically releases the two leading
resources and the fixed owner slots in the binary's observed order, releases
every object in both pointer arrays, resets their logical ends, and clears the
three separately owned state handles. The destructor at `0x47EFA0` then frees
the two arrays by their capacities and releases any of those three state
handles left by an incomplete shutdown. The special handle's internal release
operation remains a narrow adapter.
