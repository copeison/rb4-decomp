# Render platforms and APIs

The renderer uses a 13-value platform ID and a separate seven-value graphics
API ID. `render_platform_name` at `0x363030` recovers the known platform names:
`pc` at 3, `xb1` at 5, `ps4` at 7, `android` through `tvos` at 8 through 11,
and `nx` at 12. The remaining slots use the empty symbol in this build.

`render_api_name` at `0x1AE4D0` maps the API values in order to `null`, `dx11`,
`ps4`, `mtl`, `vlk`, `nx`, and `gles3`. The shortened spellings are retained
exactly as stored in the executable.

`render_api_for_platform` at `0x4414A0` reads a configured API name for a
platform and converts it to the API enum, falling back to `null` when no known
name matches. The Orbis-specific getter at `0x8D5DE0` always returns API value
2, `ps4`.
