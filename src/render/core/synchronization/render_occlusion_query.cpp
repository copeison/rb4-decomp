#include "render/core/synchronization/render_occlusion_query.h"

#include "render/core/synchronization/render_occlusion_query_adapters.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x5F7D30.
RenderOcclusionQuery* render_create_occlusion_query(void* owner) {
    auto& factory = *render_system_factory(*render_system_instance());
    return render_factory_create_occlusion_query(factory, owner);
}

// Reconstructed from eboot.elf at 0x5F7D50.
void render_occlusion_query_construct(
    RenderOcclusionQuery& query,
    void* owner) {
    render_occlusion_query_set_base_dispatch(query);
    query.owner = owner;
    query.state_flags[0] = 0;
    query.state_flags[1] = 0;
    query.state_flags[2] = 0;
    for (auto& value : query.query_values) {
        value = -1;
    }
    query.frame_index = -1;
    query.result = 0;
    query.link.next = &query.link;
    query.link.previous = &query.link;
}

// Reconstructed from eboot.elf at 0x5F7DA0.
void render_occlusion_query_destruct(RenderOcclusionQuery& query) {
    render_occlusion_query_set_base_dispatch(query);
    query.link.next->previous = query.link.previous;
    query.link.previous->next = query.link.next;
}

// Reconstructed from eboot.elf at 0x5F7DD0.
void render_occlusion_query_delete(RenderOcclusionQuery& query) {
    render_occlusion_query_destruct(query);
    render_delete_occlusion_query_storage(query);
}

}  // namespace rb4
