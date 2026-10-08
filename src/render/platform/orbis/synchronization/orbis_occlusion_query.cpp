#include "render/platform/orbis/synchronization/orbis_occlusion_query.h"

#include <cstddef>

#include "render/platform/orbis/synchronization/orbis_occlusion_query_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisOcclusionQuerySize = 72;
constexpr std::size_t kOcclusionQueryResultSize = 256;
constexpr std::size_t kOcclusionQueryResultAlignment = 16;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8C30.
OrbisOcclusionQuery* orbis_create_occlusion_query(void* owner) {
    auto* storage = render_allocate(kOrbisOcclusionQuerySize);
    auto* query = reinterpret_cast<OrbisOcclusionQuery*>(storage);
    orbis_occlusion_query_construct(*query, owner);
    return query;
}

// Reconstructed from eboot.elf at 0x8E28C0.
void orbis_occlusion_query_construct(
    OrbisOcclusionQuery& query,
    void* owner) {
    occlusion_query_construct(query, owner);
    orbis_occlusion_query_clear_backend_state(query);
}

// Reconstructed from eboot.elf at 0x8E28F0.
void orbis_occlusion_query_destruct(OrbisOcclusionQuery& query) {
    occlusion_query_destruct(query);
}

// Reconstructed from eboot.elf at 0x8E2900.
void orbis_occlusion_query_delete(OrbisOcclusionQuery& query) {
    orbis_occlusion_query_destruct(query);
    render_delete_occlusion_query(query);
}

// Reconstructed from eboot.elf at 0x8E2920.
void orbis_occlusion_query_begin(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    auto* result_address = orbis_occlusion_query_allocate_result(
        context,
        kOcclusionQueryResultSize,
        kOcclusionQueryResultAlignment);
    orbis_occlusion_query_set_result_address(query, result_address);
    orbis_command_begin_occlusion_query(context, result_address);
    orbis_command_set_occlusion_query_enabled(context, true);
}

// Reconstructed from eboot.elf at 0x8E29E0.
void orbis_occlusion_query_end(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    orbis_command_end_occlusion_query(
        context, orbis_occlusion_query_result_address(query));
    orbis_command_set_occlusion_query_enabled(context, false);
}

// Reconstructed from eboot.elf at 0x8E2A30.
void orbis_occlusion_query_begin_conditional_render(
    const OrbisOcclusionQuery& query,
    OrbisRenderContext& context) {
    orbis_command_begin_conditional_render(
        context, orbis_occlusion_query_result_address(query));
}

// Reconstructed from eboot.elf at 0x8E2A60.
void orbis_occlusion_query_end_conditional_render(
    const OrbisOcclusionQuery&,
    OrbisRenderContext& context) {
    orbis_command_end_conditional_render(context);
}

}  // namespace rb4
