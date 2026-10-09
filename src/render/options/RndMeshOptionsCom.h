#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's display of the selected mesh's vertices,
// normals and tangents (render/RndMeshOptionsCom.o, 0x5D0CE0-0x5D19FB). Its
// class id is "MeshOptions". The vtable at 0x1929A10 has 41 slots. The object
// is 56 bytes.
class RndMeshOptionsCom : public RndOptionsCom {
public:
    RndMeshOptionsCom();  // 0x5D0D00
    ~RndMeshOptionsCom() override;  // slots 0-1: 0x5D0D50, 0x5D0D60

    Symbol GetId() const override;         // slot 4: 0x5D1540
    Symbol GetClassName() const override;  // slot 5: 0x5D1550
    int CurrentRev() const override;       // slot 7: 0x5D1560
    bool IsA(Symbol type) const override;  // slot 8: 0x5D1580
    Component* AsComponent() override;     // slot 9: 0x5D15B0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x5D15C0
    // Slot 20: stops being theRndMeshOpts.
    void _PreDestroy(DestroyType type) override;  // 0x5D0CE0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x5D1710
    ComMetaData& _GetMetaData() override;       // slot 23: 0x5D1720
    // Slot 29: becomes theRndMeshOpts.
    bool _OnResourcesLoaded() override;  // 0x5D0CF0

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x5D0D80

    static Symbol sId;                  // 0x1AA6698, "MeshOptions"
    // Also "MeshOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AA66A0
    static PropRegistry sPropRegistry;  // 0x1AA66B0
    static ComMetaData sMetaData;       // 0x1AA6750

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mLabelSelectedMeshVertices;  // "label_selected_mesh_vertices"
    bool mSelectedMeshNormals;  // "selected_mesh_normals"
    bool mSelectedMeshTangents;  // "selected_mesh_tangents"
    float mMeshTangentSpaceScale;  // "mesh_tangent_space_scale"
    bool mRestrictVertexRange;  // "vertex_range/restrict_vertex_range"
    unsigned long mStartVertex;  // "vertex_range/start_vertex"
    unsigned long mNumVertices;  // "vertex_range/num_vertices"
};

static_assert(offsetof(RndMeshOptionsCom, mLabelSelectedMeshVertices) == 0x17);
static_assert(offsetof(RndMeshOptionsCom, mSelectedMeshNormals) == 0x18);
static_assert(offsetof(RndMeshOptionsCom, mSelectedMeshTangents) == 0x19);
static_assert(offsetof(RndMeshOptionsCom, mMeshTangentSpaceScale) == 0x1C);
static_assert(offsetof(RndMeshOptionsCom, mRestrictVertexRange) == 0x20);
static_assert(offsetof(RndMeshOptionsCom, mStartVertex) == 0x28);
static_assert(offsetof(RndMeshOptionsCom, mNumVertices) == 0x30);
static_assert(sizeof(RndMeshOptionsCom) == 0x38);

// The live component, or null while none has loaded its resources.
extern RndMeshOptionsCom* theRndMeshOpts;  // 0x1AA68F8
