#include "renderps4/meshes/PS4MeshTyped.h"

#include <cstring>

#include "renderps4/system/PS4Factory.h"

// Instantiation addresses in eboot.elf. Each layout's primary vtable is
// followed by the RndDynamicGpuData thunks and RndMeshTyped's own vtable.
//
// | Layout | vtable | dtor | draw | count | resize | free | vertex | copy | static | dynamic | grow | vertices | faces |
// |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
// | Color | 0x195EFB8 | 0x8D9E30 | 0x8D9FB0 | 0x8DA2B0 | 0x8DA2E0 | 0x8DA330 | 0x8DA3C0 | 0x8DA3D0 | 0x8DA430 | 0x8DA470 | 0x8DA790 | 0x8DAC80 | 0x8DAE90 |
// | ColorTex | 0x195F0E8 | 0x8DB190 | 0x8DB310 | 0x8DB610 | 0x8DB640 | 0x8DB690 | 0x8DB720 | 0x8DB730 | 0x8DB790 | 0x8DB7D0 | 0x44B2F0 | 0x8DBE60 | 0x8DC070 |
// | Unskinned | 0x195F218 | 0x8DC370 | 0x8DC4F0 | 0x8DC7F0 | 0x8DC820 | 0x8DC870 | 0x8DC900 | 0x8DC910 | 0x8DC970 | 0x8DC9B0 | 0x8DCCD0 | 0x8DD280 | 0x8DD490 |
// | Skinned | 0x195F348 | 0x8DD790 | 0x8DD910 | 0x8DDC10 | 0x8DDC40 | 0x8DDC90 | 0x8DDD20 | 0x8DDD30 | 0x8DDD90 | 0x8DDDD0 | 0x8DE0F0 | 0x8DE6D0 | 0x8DE8E0 |
// | UnskinnedCompressed | 0x195F478 | 0x8DEBE0 | 0x8DED60 | 0x8DF060 | 0x8DF090 | 0x8DF0E0 | 0x8DF170 | 0x8DF180 | 0x8DF1E0 | 0x8DF220 | 0x8DF540 | 0x8DFB70 | 0x8DFD80 |
// | SkinnedCompressed | 0x195F5A8 | 0x8E0080 | 0x8E0200 | 0x8E0500 | 0x8E0520 | 0x8E0560 | 0x8E05F0 | 0x8E0600 | 0x8E0660 | 0x8E06A0 | 0x8E09A0 | 0x8E1070 | 0x8E1260 |
// | PosOnly | 0x195EE88 | 0x8D8C80 | 0x8D8E00 | 0x8D9100 | 0x8D9130 | 0x8D9180 | 0x8D9210 | 0x8D9220 | 0x8D9280 | 0x8D92C0 | 0x6B3ED0 | 0x8D9920 | 0x8D9B30 |

template class RndMeshTyped<RndVertexColor>;
template class RndMeshTyped<RndVertexColorTex>;
template class RndMeshTyped<RndVertexUnskinned>;
template class RndMeshTyped<RndVertexSkinned>;
template class RndMeshTyped<RndVertexPosOnly>;
template class RndMeshTyped<RndVertexUnskinnedCompressed>;
template class RndMeshTyped<RndVertexSkinnedCompressed>;

template class PS4MeshTyped<RndVertexColor>;
template class PS4MeshTyped<RndVertexColorTex>;
template class PS4MeshTyped<RndVertexUnskinned>;
template class PS4MeshTyped<RndVertexSkinned>;
template class PS4MeshTyped<RndVertexPosOnly>;
template class PS4MeshTyped<RndVertexUnskinnedCompressed>;
template class PS4MeshTyped<RndVertexSkinnedCompressed>;

static_assert(offsetof(PS4MeshTyped<RndVertexColor>, mVerts) == 128);
static_assert(offsetof(PS4MeshTyped<RndVertexColor>, mVertexBuffers) == 160);
static_assert(offsetof(PS4MeshTyped<RndVertexColor>, mActiveVertexData) == 432);
static_assert(offsetof(PS4MeshTyped<RndVertexColor>, mBufferMask) == 448);
static_assert(offsetof(PS4MeshTyped<RndVertexColor>, mIndexData) == 456);
static_assert(sizeof(PS4MeshTyped<RndVertexColor>) == 472);

namespace {

// The factory zero-fills the platform part of the object before
// constructing it.
template <typename Vertex>
RndMesh* NewMesh(const char* name) {
    auto* storage = operator new(sizeof(PS4MeshTyped<Vertex>));
    std::memset(storage, 0, sizeof(PS4MeshTyped<Vertex>));
    return new (storage) PS4MeshTyped<Vertex>(name);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D85F0. The map has
// RndMeshFactory::NewImpl<PS4MeshTyped>; particle meshes are not created
// here.
RndMesh* PS4Factory::CreateMesh(RndVertexType type, const char* name) {
    switch (type) {
    case kVertexColor:
        return NewMesh<RndVertexColor>(name);
    case kVertexColorTex:
        return NewMesh<RndVertexColorTex>(name);
    case kVertexUnskinned:
        return NewMesh<RndVertexUnskinned>(name);
    case kVertexSkinned:
        return NewMesh<RndVertexSkinned>(name);
    case kVertexPosOnly:
        return NewMesh<RndVertexPosOnly>(name);
    case kVertexUnskinnedCompressed:
        return NewMesh<RndVertexUnskinnedCompressed>(name);
    case kVertexSkinnedCompressed:
        return NewMesh<RndVertexSkinnedCompressed>(name);
    default:
        return nullptr;
    }
}
