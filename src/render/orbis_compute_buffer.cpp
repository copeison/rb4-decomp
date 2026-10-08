#include "orbis_compute_buffer.h"

#include <cstddef>

#include "orbis_compute_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisComputeBufferSize = 136;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8BC0.
OrbisComputeBuffer* orbis_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisComputeBufferSize);
    auto* buffer = reinterpret_cast<OrbisComputeBuffer*>(storage);
    orbis_compute_buffer_construct(*buffer, descriptor);
    return buffer;
}

// Reconstructed from eboot.elf at 0x8E3250.
void orbis_compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor) {
    compute_buffer_construct(buffer, descriptor);
    orbis_compute_buffer_clear_backend_state(buffer);
}

}  // namespace rb4
