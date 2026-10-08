# Render platform configuration

The render-system constructor owns 13 fixed platform-configuration slots.
`render_platform_config_construct` at `0x6B9940` resets each 128-byte slot to
its empty defaults. `render_platform_config_initialize` at `0x6B99B0` then
applies the built-in capability profile for a selected platform ID.

The slot is now typed directly. It contains a 32-byte resolution vector,
resource tier and feature flags, five capability values, the enabled flag, an
`0x2000` default resource budget, option flags, and a 128-bit capability mask.
Construction owns all defaults, and render-system destruction invokes the
resolution vector destructor for each of the 13 slots in reverse order.

Resolution strings are read from that platform's `resolutions` list and parsed
with `render_parse_resolution`. Invalid entries are discarded. An empty result
receives the 1,920 × 1,080 fallback, and the final list is sorted by width and
then height.

The supported IDs come from `platform_mgr.supported_platforms` through
`render_supported_platform_ids` at `0x3641B0`. This explains why the renderer
constructs every fixed slot but populates only the platform configurations
selected by data.

The exact meanings of several packed capability masks remain unresolved. Their
per-platform constants stay behind the capability adapter while the confirmed
platform selection and resolution behavior are expressed directly.

The platform-seven boot predicate at `0x6B9FE0` is also direct: the resource
tier must be at least four and feature bits zero and three must both be set.
The constructor intentionally ignores this predicate's result, as in the
binary.
