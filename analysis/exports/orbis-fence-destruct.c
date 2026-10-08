// Releases an Orbis fence's GPU-visible value.
// Reconstructed from eboot.elf at 0x8E15F0.
void orbis_fence_destruct(OrbisFence* fence) {
    fence->vtable = &orbis_fence_vtable;
    if (g_orbis_render_system != 0) {
        orbis_defer_allocation_release(
            g_orbis_render_system, fence->value);
    } else {
        render_release(fence->value);
    }
    fence->value = 0;
}
