// render/RndMeshOptionsCom.o (0x5D0CE0 to 0x5D19FB).
#include "render/options/RndMeshOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x5D1930).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndMeshOptionsCom::sId("MeshOptions");
Symbol RndMeshOptionsCom::sClassName("MeshOptions");
PropRegistry RndMeshOptionsCom::sPropRegistry;
ComMetaData RndMeshOptionsCom::sMetaData;
RndMeshOptionsCom* theRndMeshOpts;

// Reconstructed from eboot.elf at 0x5D0CE0.
void RndMeshOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndMeshOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x5D0CF0.
bool RndMeshOptionsCom::_OnResourcesLoaded() {
    theRndMeshOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x5D0D00.
RndMeshOptionsCom::RndMeshOptionsCom()
    : mLabelSelectedMeshVertices(false),
      mSelectedMeshNormals(false),
      mSelectedMeshTangents(false),
      mMeshTangentSpaceScale(1.0F),
      mRestrictVertexRange(false),
      mStartVertex(0),
      mNumVertices(100) {}

// Reconstructed from eboot.elf at 0x5D0D50. The deleting destructor is at
// 0x5D0D60.
RndMeshOptionsCom::~RndMeshOptionsCom() {}

// Reconstructed from eboot.elf at 0x5D0D80. The vertex range is read-only until it is
// restricted.
void RndMeshOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options governing the appearance of meshes";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndMeshOptionsCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    BoolMetadata& labelVertices = TypeSpecificMetadata(
        registry.RegisterProp(
            "label_selected_mesh_vertices",
            offsetof(RndMeshOptionsCom, mLabelSelectedMeshVertices),
            kPropertyBool,
            1),
        prototype.mLabelSelectedMeshVertices);
    labelVertices.mDefault = prototype.mLabelSelectedMeshVertices;
    static_cast<void>(Symbol("prop"));
    labelVertices.mHelp = String("Should we label the vertices of selected meshes?");
    BoolMetadata& normals = TypeSpecificMetadata(
        registry.RegisterProp(
            "selected_mesh_normals",
            offsetof(RndMeshOptionsCom, mSelectedMeshNormals),
            kPropertyBool,
            1),
        prototype.mSelectedMeshNormals);
    normals.mDefault = prototype.mSelectedMeshNormals;
    static_cast<void>(Symbol("prop"));
    normals.mHelp = String("Should the vertex normals for selected meshes be drawn? (in blue)");
    BoolMetadata& tangents = TypeSpecificMetadata(
        registry.RegisterProp(
            "selected_mesh_tangents",
            offsetof(RndMeshOptionsCom, mSelectedMeshTangents),
            kPropertyBool,
            1),
        prototype.mSelectedMeshTangents);
    tangents.mDefault = prototype.mSelectedMeshTangents;
    static_cast<void>(Symbol("prop"));
    tangents.mHelp = String(
        "Should the vertex tangents/bitangents for selected meshes be drawn? (tangents in red, "
        "bitangents in green)");
    FloatMetadata& tangentScale = TypeSpecificMetadata(
        registry.RegisterProp(
            "mesh_tangent_space_scale",
            offsetof(RndMeshOptionsCom, mMeshTangentSpaceScale),
            kPropertyFloat,
            4),
        prototype.mMeshTangentSpaceScale);
    tangentScale.mDefault = prototype.mMeshTangentSpaceScale;
    static_cast<void>(Symbol("prop"));
    tangentScale.mHelp = String("Scale of the drawn arrows for mesh vertex normals and tangents");
    tangentScale.mMin = 0.01F;
    tangentScale.mMax = 1000.0F;
    tangentScale.mSlider = true;
    tangentScale.mSliderLogResolution = 2.0F;
    // A group of the component's own members: the struct sits at offset 0.
    PropRegistry& range = static_cast<GroupMetadata&>(
        *registry.RegisterProp("vertex_range", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& restrictRange = TypeSpecificMetadata(
        range.RegisterProp(
            "restrict_vertex_range",
            offsetof(RndMeshOptionsCom, mRestrictVertexRange),
            kPropertyBool,
            1),
        prototype.mRestrictVertexRange);
    restrictRange.mDefault = prototype.mRestrictVertexRange;
    static_cast<void>(Symbol("prop"));
    restrictRange.mHelp = String(
        "Should we limit the range of vertices we label, display tangent space vectors for, etc.?");
    IntMetadata<unsigned long>& startVertex = TypeSpecificMetadata(
        range.RegisterProp("start_vertex", offsetof(RndMeshOptionsCom, mStartVertex), kPropertyUInt64, 8),
        prototype.mStartVertex);
    startVertex.mDefault = prototype.mStartVertex;
    static_cast<void>(Symbol("prop"));
    startVertex.mHelp = String("Start index of the set of vertices to display");
    startVertex.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndMeshOptionsCom&>(args.mComponent).mRestrictVertexRange;
    });
    IntMetadata<unsigned long>& numVertices = TypeSpecificMetadata(
        range.RegisterProp("num_vertices", offsetof(RndMeshOptionsCom, mNumVertices), kPropertyUInt64, 8),
        prototype.mNumVertices);
    numVertices.mDefault = prototype.mNumVertices;
    static_cast<void>(Symbol("prop"));
    numVertices.mHelp = String("Number of vertices to display");
    numVertices.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndMeshOptionsCom&>(args.mComponent).mRestrictVertexRange;
    });
}

// Reconstructed from eboot.elf at 0x5D1540.
Symbol RndMeshOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x5D1550.
Symbol RndMeshOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x5D1560.
int RndMeshOptionsCom::CurrentRev() const {
    return const_cast<RndMeshOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x5D1580.
bool RndMeshOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5D15B0.
Component* RndMeshOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x5D15C0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndMeshOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndMeshOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndMeshOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x5D1710.
PropRegistry& RndMeshOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x5D1720.
ComMetaData& RndMeshOptionsCom::_GetMetaData() {
    return sMetaData;
}
