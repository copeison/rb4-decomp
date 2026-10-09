#pragma once

#include <cstring>

#include "render/meshes/RndMesh.h"

// Mesh with CPU vertices of one layout. The platform subclass adds the GPU
// buffers; the per-instantiation addresses are listed in PS4MeshTyped.cpp.
template <typename Vertex>
class RndMeshTyped : public RndMesh {
public:
    explicit RndMeshTyped(const char* name) : RndMesh(name) {}
    ~RndMeshTyped() override {}

    RndVertexType _GetVertexTypeImpl() const override {
        return Vertex::kType;
    }

    unsigned long _GetNumVerticesImpl() const override {
        return mVerts.mpBegin == nullptr ? 0 : mVerts.size();
    }

    // New vertices take the layout's default values.
    void _SetNumVerticesImpl(unsigned long count) override {
        mVerts.resize(count);
    }

    void _FreeVerticesImpl() override {
        mVerts.set_capacity(0);
    }

    void* _GetVertexVoidImpl(unsigned long index) override {
        return mVerts.mpBegin + index;
    }

    // Copies the vertices into a tracked allocation labeled "VerticesCopy";
    // empty meshes return null.
    void* _MakeVerticesCopyImpl() const override {
        const auto count = _GetNumVerticesImpl();
        if (count == 0) {
            return nullptr;
        }
        const auto bytes = sizeof(Vertex) * count;
        auto* copy = MemAlloc(bytes, "VerticesCopy", 4);
        std::memcpy(copy, mVerts.mpBegin, bytes);
        return copy;
    }

    // The base static and dynamic syncs record the vertex count; the
    // platform overrides call them first.
    void _SyncStaticImpl() override {
        mNumVerts = _GetNumVerticesImpl();
    }

    void _SyncDynamicImpl(RndContext&, unsigned int) override {
        mNumVerts = _GetNumVerticesImpl();
    }

    eastl::vector<Vertex> mVerts;  // Name not in the reference map.
};
