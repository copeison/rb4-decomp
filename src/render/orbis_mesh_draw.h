#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "orbis_mesh.h"
#include "orbis_vertex_descriptors.h"

namespace rb4 {

struct OrbisRenderCommandContext;

enum class OrbisIndexSize : std::uint32_t {
    k16Bit = 0,
    k32Bit = 1,
};

enum class MeshPrimitiveType : std::uint32_t {
    kTriangles = 3,
};

struct MeshInstanceBatch {
    const OrbisMeshInstanceData* instances;
    std::size_t count;
};

struct MeshDrawRange {
    static constexpr std::size_t kAllTriangles =
        std::numeric_limits<std::size_t>::max();

    std::size_t first_triangle;
    std::size_t triangle_count;
};

void orbis_mesh_draw(
    OrbisMesh& mesh,
    OrbisRenderCommandContext& context,
    const MeshInstanceBatch& instances,
    const MeshDrawRange& range);

}  // namespace rb4
