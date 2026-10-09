#pragma once

#include <cstddef>
#include <cstring>

#include "render/core/system/render_epoch.h"
#include "render/meshes/RndMeshTyped.h"
#include "render/platform/orbis/meshes/orbis_builtin_buffers.h"
#include "render/platform/orbis/meshes/orbis_gnm_mesh_api.h"
#include "render/platform/orbis/meshes/orbis_mesh_draw.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

// Mesh with double-buffered GPU vertices and a 16- or 32-bit index buffer.
// The factory zero-fills the object before constructing it. Every member is
// instantiated per vertex layout; the addresses are in PS4MeshTyped.cpp.
template <typename Vertex>
class PS4MeshTyped : public RndMeshTyped<Vertex> {
public:
    using Base = RndMeshTyped<Vertex>;

    explicit PS4MeshTyped(const char* name) : Base(name) {}

    // Returns the GPU buffers to the device for deferred release.
    ~PS4MeshTyped() override {
        if (rb4::g_orbis_render_system != nullptr) {
            PS4DeferredDelete(mVertexData[0]);
            PS4DeferredDelete(mVertexData[1]);
            PS4DeferredDelete(mIndexData);
        }
        mVertexData[0] = nullptr;
        mVertexData[1] = nullptr;
        mIndexData = nullptr;
    }

    // The binary uses the context argument as the command context.
    void _DrawBatchImpl(
        RndContext& context,
        const VectorAdapter<RndInstanceData>& instances,
        const RndDrawable::DrawRange& range) override {
        auto& commands = reinterpret_cast<rb4::OrbisRenderCommandContext&>(context);
        SelectVertexBuffers(commands);
        SelectInstanceBuffer(commands, instances);
        rb4::orbis_set_primitive_type(commands, rb4::MeshPrimitiveType::kTriangles);
        if (mIndexData != nullptr) {
            DrawIndexed(commands, range);
        } else {
            rb4::gnmx_prepare_draw(commands);
            rb4::gnm_draw_command_buffer_draw_index_auto(
                commands, static_cast<unsigned int>(this->mNumVerts));
        }
        rb4::gnmx_finish_draw(commands);
        rb4::gnm_draw_command_buffer_set_num_instances(commands, 1);
        this->mLastUseFrame = rb4::current_render_epoch();
    }

    void _SyncStaticImpl() override {
        Base::_SyncStaticImpl();
        _SyncStaticVertices();
        _SyncStaticFaces();
    }

    // Only a vertex update (flag 1) flips and refills the vertex data.
    void _SyncDynamicImpl(RndContext& context, unsigned int flags) override {
        if ((flags & 1U) == 0) {
            return;
        }
        Base::_SyncDynamicImpl(context, flags);
        mActiveVertexData ^= 1;
        std::memcpy(
            mVertexData[mActiveVertexData],
            this->mVerts.mpBegin,
            sizeof(Vertex) * this->mNumVerts);
    }

    // Reallocates the vertex data when the count changes (a second copy only
    // for dynamic meshes), copies the vertices, and rebuilds the descriptors.
    void _SyncStaticVertices() {
        const auto count = this->_GetNumVerticesImpl();
        if (count == 0) {
            return;
        }
        const auto bytes = count * sizeof(Vertex);
        if (mVertexDataCapacity != count) {
            if (rb4::g_orbis_render_system != nullptr) {
                PS4DeferredDelete(mVertexData[0]);
                PS4DeferredDelete(mVertexData[1]);
            }
            mVertexData[0] = static_cast<Vertex*>(MemAlloc(bytes, "VBuffer", 4));
            mVertexData[1] = (this->mVertexUsageFlags & 1U) != 0
                ? static_cast<Vertex*>(MemAlloc(bytes, "VBuffer", 4))
                : nullptr;
            mVertexDataCapacity = count;
        }

        std::memcpy(mVertexData[mActiveVertexData], this->mVerts.mpBegin, bytes);

        const auto* format = rb4::render_mesh_format_descriptor(Vertex::kType);
        mBufferMask = 0;
        for (int bank = 0; bank < 2; ++bank) {
            if (mVertexData[bank] != nullptr) {
                rb4::orbis_build_mesh_vertex_descriptors(
                    mVertexBuffers[bank],
                    mVertexData[bank],
                    mBufferMask,
                    static_cast<unsigned int>(count),
                    *format);
            }
        }
    }

    // Meshes with more than 0xFFFF vertices use 32-bit indices.
    void _SyncStaticFaces() {
        const auto& faces = this->mFaces;
        if (faces.mpBegin == faces.mpEnd) {
            return;
        }

        const auto* source = reinterpret_cast<const unsigned int*>(faces.mpBegin);
        const auto numIndices = static_cast<unsigned long>(
            reinterpret_cast<const unsigned int*>(faces.mpEnd) - source);
        const bool wide = this->_GetNumVerticesImpl() > 0xFFFF;
        const auto format = wide ? rb4::OrbisIndexSize::k32Bit : rb4::OrbisIndexSize::k16Bit;
        const auto bytes = numIndices * (wide ? sizeof(unsigned int) : sizeof(unsigned short));

        if (mIndexCapacity != numIndices ||
            mIndexFormat != static_cast<unsigned int>(format)) {
            if (rb4::g_orbis_render_system != nullptr) {
                PS4DeferredDelete(mIndexData);
            }
            mIndexData = MemAlloc(bytes, "IBuffer", 4);
            mIndexCapacity = numIndices;
            mIndexFormat = static_cast<unsigned int>(format);
        }

        if (wide) {
            std::memcpy(mIndexData, source, bytes);
            return;
        }
        auto* destination = static_cast<unsigned short*>(mIndexData);
        for (unsigned long index = 0; index < numIndices; ++index) {
            destination[index] = static_cast<unsigned short>(source[index]);
        }
    }

    // Field names are not in the reference map.
    rb4::OrbisBufferDescriptor mVertexBuffers[2][rb4::kMeshVertexStreamCount] = {};
    Vertex* mVertexData[2] = {nullptr, nullptr};
    unsigned long mActiveVertexData = 0;
    unsigned long mVertexDataCapacity = 0;
    unsigned int mBufferMask = 0;
    unsigned int mIndexFormat = 0;
    void* mIndexData = nullptr;
    unsigned long mIndexCapacity = 0;

private:
    // Streams the mesh lacks fall back to the shared default buffers.
    void SelectVertexBuffers(rb4::OrbisRenderCommandContext& commands) const {
        const auto* defaults = rb4::orbis_default_vertex_descriptors();
        const auto* buffers = mVertexBuffers[mActiveVertexData];
        for (unsigned int stream = 0; stream < rb4::kMeshVertexStreamCount; ++stream) {
            const auto* buffer = (mBufferMask & (1U << stream)) != 0
                ? &buffers[stream]
                : &defaults[stream];
            rb4::orbis_bind_vertex_buffers(commands, stream, 1, buffer);
        }
    }

    // Copies the instance data into the command buffer and binds it.
    static void SelectInstanceBuffer(
        rb4::OrbisRenderCommandContext& commands,
        const VectorAdapter<RndInstanceData>& instances) {
        const auto bytes = sizeof(RndInstanceData) * instances.mSize;
        auto* uploaded = static_cast<RndInstanceData*>(
            rb4::orbis_allocate_embedded_data(commands, bytes, 4));
        rb4::OrbisBufferDescriptor buffers[rb4::kInstanceVertexStreamCount] = {};
        rb4::orbis_build_instance_vertex_descriptors(
            buffers, uploaded, static_cast<unsigned int>(instances.mSize));
        std::memcpy(uploaded, instances.mData, bytes);
        rb4::orbis_bind_vertex_buffers(
            commands,
            static_cast<unsigned int>(rb4::kMeshVertexStreamCount),
            static_cast<unsigned int>(rb4::kInstanceVertexStreamCount),
            buffers);
        rb4::gnm_draw_command_buffer_set_num_instances(
            commands, static_cast<unsigned int>(instances.mSize));
    }

    void DrawIndexed(
        rb4::OrbisRenderCommandContext& commands,
        const RndDrawable::DrawRange& range) const {
        const auto numFaces = range.mNumFaces == RndDrawable::DrawRange::kAllFaces
            ? this->mNumFaces
            : range.mNumFaces;
        const auto size = static_cast<rb4::OrbisIndexSize>(mIndexFormat);
        const auto indexBytes = size == rb4::OrbisIndexSize::k32Bit ? 4UL : 2UL;
        const auto* indices = static_cast<const unsigned char*>(mIndexData) +
            3 * indexBytes * range.mFirstFace;
        rb4::gnm_draw_command_buffer_set_index_size(
            commands, size, rb4::OrbisCachePolicy::kBypass);
        rb4::gnmx_prepare_draw(commands);
        rb4::gnm_draw_command_buffer_draw_index(
            commands, static_cast<unsigned int>(3 * numFaces), indices);
    }
};
