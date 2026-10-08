#include "orbis_occlusion_query.h"

#include <cstddef>

#include "orbis_occlusion_query_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisOcclusionQuerySize = 72;

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

}  // namespace rb4
