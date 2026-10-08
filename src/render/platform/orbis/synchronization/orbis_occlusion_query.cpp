#include "render/platform/orbis/synchronization/orbis_occlusion_query.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/synchronization/orbis_occlusion_query_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOcclusionQueryResultSize = 256;
constexpr std::size_t kOcclusionQueryResultAlignment = 16;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8C30.
OrbisOcclusionQuery* orbis_create_occlusion_query(void* owner) {
    auto* storage = render_allocate(sizeof(OrbisOcclusionQuery));
    auto* query = reinterpret_cast<OrbisOcclusionQuery*>(storage);
    orbis_occlusion_query_construct(*query, owner);
    return query;
}

// Reconstructed from eboot.elf at 0x8E28C0.
void orbis_occlusion_query_construct(
    OrbisOcclusionQuery& query,
    void* owner) {
    render_occlusion_query_construct(query, owner);
    orbis_occlusion_query_install_vtable(query);
    query.result_address = nullptr;
}

// Reconstructed from eboot.elf at 0x8E28F0.
void orbis_occlusion_query_destruct(OrbisOcclusionQuery& query) {
    render_occlusion_query_destruct(query);
}

// Reconstructed from eboot.elf at 0x8E2900.
void orbis_occlusion_query_delete(OrbisOcclusionQuery& query) {
    orbis_occlusion_query_destruct(query);
    render_delete_occlusion_query_storage(query);
}

// Reconstructed from eboot.elf at 0x8E2920.
void orbis_occlusion_query_begin(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    auto* result_address = orbis_occlusion_query_allocate_result(
        context,
        kOcclusionQueryResultSize,
        kOcclusionQueryResultAlignment);
    query.result_address = result_address;
    orbis_command_begin_occlusion_query(context, result_address);
    orbis_command_set_occlusion_query_enabled(context, true);
}

// Reconstructed from eboot.elf at 0x8E29E0.
void orbis_occlusion_query_end(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    orbis_command_end_occlusion_query(
        context, query.result_address);
    orbis_command_set_occlusion_query_enabled(context, false);
}

// Reconstructed from eboot.elf at 0x8E2A30.
void orbis_occlusion_query_begin_conditional_render(
    const OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    orbis_command_begin_conditional_render(
        context, query.result_address);
}

// Reconstructed from eboot.elf at 0x8E2A60.
void orbis_occlusion_query_end_conditional_render(
    const OrbisOcclusionQuery&,
    OrbisRenderContext& context) {
    orbis_command_end_conditional_render(context);
}

}  // namespace rb4
