#pragma once

#include <cstdint>

#include "render/core/textures/render_data_format.h"

namespace rb4 {

std::int32_t render_data_format_resolve(
    const RenderDataFormatDescriptor& descriptor,
    std::uint32_t resource_class);

}  // namespace rb4
