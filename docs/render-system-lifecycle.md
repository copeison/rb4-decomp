# Render-system construction

`render_system_construct` at `0x3DD410` builds the shared base renderer. The
clean control-flow reconstruction is in
`src/render/core/system/render_system_lifecycle.cpp`; backend object layouts remain behind
adapters until their individual types are identified.

Construction proceeds in this order:

1. Initialize the core frame state and its recursive mutex.
2. Construct 13 fixed, 128-byte platform-configuration slots.
3. Construct the default resources and the remaining backend state.
4. Initialize the callback queue and its recursive mutex.
5. Publish `g_render_system`.
6. Read `platform_mgr.supported_platforms` and initialize the corresponding
   slots, including each platform's sorted resolution list.
7. Invoke a capability predicate for slot seven. The return value is ignored
   in this build, so its precise source-level purpose remains open.
8. Allocate the 232-byte renderer settings block and initialize it.

`render_system_destruct` at `0x3DD790` releases the settings block, callback
state, backend objects, default resources, platform configurations, core
vectors, and mutexes in reverse ownership order.

The fixed platform array is separate from the supported-platform list. Every
slot receives its empty constructor, while only IDs named by configuration are
populated with capability flags and resolutions.
