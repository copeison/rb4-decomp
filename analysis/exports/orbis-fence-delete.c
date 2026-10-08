// Releases an Orbis fence's GPU value and then deletes the fence object.
// Reconstructed from eboot.elf at 0x8E1680.
void orbis_fence_delete(OrbisFence* fence) {
    fence->vtable = &orbis_fence_vtable;
    if (g_orbis_render_system != 0) {
        orbis_defer_allocation_release(
            g_orbis_render_system, fence->value);
    } else {
        render_release(fence->value);
    }
    render_delete(fence);
}
