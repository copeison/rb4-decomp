#pragma once

#include <cstddef>
#include <cstring>

#include "render/meshes/RndMeshTyped.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/gnm_adapters.h"

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
        if (gPS4Device != nullptr) {
            gPS4Device->DeferredDelete(mVertexData[0]);
            gPS4Device->DeferredDelete(mVertexData[1]);
            gPS4Device->DeferredDelete(mIndexData);
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
        _SelectVertexBuffers(commands);
        _InlineInstanceBufferToCommandBuffer(static_cast<PS4Context&>(context), instances);
        GfxSetPrimitiveType(commands, GnmPrimitiveType::kTriangles);
        if (mIndexData != nullptr) {
            _DrawIndexed(commands, range);
        } else {
            GfxPrepareDraw(commands);
            GfxDrawIndexAuto(commands, static_cast<unsigned int>(this->mNumVerts));
        }
        GfxFinishDraw(commands);
        GfxSetNumInstances(commands, 1);
        this->mLastUseFrame = TheRndDevice()->mFrameCount;
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
            if (gPS4Device != nullptr) {
                gPS4Device->DeferredDelete(mVertexData[0]);
                gPS4Device->DeferredDelete(mVertexData[1]);
            }
            mVertexData[0] = static_cast<Vertex*>(MemAlloc(bytes, "VBuffer", 4));
            mVertexData[1] = (this->mVertexUsageFlags & 1U) != 0
                ? static_cast<Vertex*>(MemAlloc(bytes, "VBuffer", 4))
                : nullptr;
            mVertexDataCapacity = count;
        }

        std::memcpy(mVertexData[mActiveVertexData], this->mVerts.mpBegin, bytes);

        const auto* interpreter = RndVertexInterpreter::GetInstance(Vertex::kType);
        mBufferMask = 0;
        for (int bank = 0; bank < 2; ++bank) {
            if (mVertexData[bank] != nullptr) {
                PS4RenderUtl::InitializeVertexBuffers(
                    mVertexBuffers[bank],
                    mVertexData[bank],
                    mBufferMask,
                    static_cast<unsigned int>(count),
                    *interpreter);
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
        const auto format = wide ? GnmIndexSize::k32Bit : GnmIndexSize::k16Bit;
        const auto bytes = numIndices * (wide ? sizeof(unsigned int) : sizeof(unsigned short));

        if (mIndexCapacity != numIndices ||
            mIndexFormat != static_cast<unsigned int>(format)) {
            if (gPS4Device != nullptr) {
                gPS4Device->DeferredDelete(mIndexData);
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
    GnmBuffer mVertexBuffers[2][RndVertexInterpreter::kNumStreams] = {};
    Vertex* mVertexData[2] = {nullptr, nullptr};
    unsigned long mActiveVertexData = 0;
    unsigned long mVertexDataCapacity = 0;
    unsigned int mBufferMask = 0;
    unsigned int mIndexFormat = 0;
    void* mIndexData = nullptr;
    unsigned long mIndexCapacity = 0;

    // Copies the instance data into the command buffer, binds it after the
    // mesh streams and sets the instance count. The binary uses the context
    // as the command context.
    static void _InlineInstanceBufferToCommandBuffer(
        PS4Context& context,
        const VectorAdapter<RndInstanceData>& instances) {
        auto& commands = reinterpret_cast<rb4::OrbisRenderCommandContext&>(context);
        const auto bytes = sizeof(RndInstanceData) * instances.mSize;
        auto* uploaded = static_cast<RndInstanceData*>(
            GfxAllocateFromCommandBuffer(commands, bytes, 4));
        GnmBuffer buffers[PS4RenderUtl::kNumInstanceStreams] = {};
        PS4RenderUtl::InitializeInstanceBuffer(
            buffers, uploaded, static_cast<unsigned int>(instances.mSize));
        std::memcpy(uploaded, instances.mData, bytes);
        GfxSetVertexBuffers(
            commands,
            RndVertexInterpreter::kNumStreams,
            PS4RenderUtl::kNumInstanceStreams,
            buffers);
        GfxSetNumInstances(commands, static_cast<unsigned int>(instances.mSize));
    }

private:
    // Streams the mesh lacks fall back to the shared default buffers.
    // Inlined into _DrawBatchImpl; name not in the reference map.
    void _SelectVertexBuffers(rb4::OrbisRenderCommandContext& commands) const {
        const auto* defaults = gPS4Device->mDefaultVertexDescs;
        const auto* buffers = mVertexBuffers[mActiveVertexData];
        for (unsigned int stream = 0; stream < RndVertexInterpreter::kNumStreams; ++stream) {
            const auto* buffer = (mBufferMask & (1U << stream)) != 0
                ? &buffers[stream]
                : &defaults[stream];
            GfxSetVertexBuffers(commands, stream, 1, buffer);
        }
    }

    // Inlined into _DrawBatchImpl; name not in the reference map.
    void _DrawIndexed(
        rb4::OrbisRenderCommandContext& commands,
        const RndDrawable::DrawRange& range) const {
        const auto numFaces = range.mNumFaces == RndDrawable::DrawRange::kAllFaces
            ? this->mNumFaces
            : range.mNumFaces;
        const auto size = static_cast<GnmIndexSize>(mIndexFormat);
        const auto indexBytes = size == GnmIndexSize::k32Bit ? 4UL : 2UL;
        const auto* indices = static_cast<const unsigned char*>(mIndexData) +
            3 * indexBytes * range.mFirstFace;
        GfxSetIndexSize(commands, size, GnmCachePolicy::kBypass);
        GfxPrepareDraw(commands);
        GfxDrawIndex(commands, static_cast<unsigned int>(3 * numFaces), indices);
    }
};
