// Base-object variant of the Orbis fence destructor.
// Reconstructed from eboot.elf at 0x8E1640.
void orbis_fence_base_destruct(OrbisFence* fence) {
    if (g_orbis_render_system != 0) {
        orbis_defer_allocation_release(
            g_orbis_render_system, fence->value);
    } else {
        render_release(fence->value);
    }
    fence->value = 0;
}
