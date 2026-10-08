#pragma once

namespace rb4 {

struct OrbisComputeBuffer;
struct RenderComputeBufferDescriptor;

OrbisComputeBuffer* orbis_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor);
void orbis_compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor);

}  // namespace rb4
