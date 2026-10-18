#pragma once

#include <cstddef>

#include "render/queries/RndOcclusionQuery.h"
#include "utl/containers/Vector.h"

class RndCameraContext;
class RndComputeBuffer;
class RndContext;
class RndTextureBase;
enum RndTargetMode : int;
class RndShaderDisplayOcclusionQueryCoverage;
class Sphere;

// A scene drawer's occlusion queries (render/RndOcclusionQueryMgr.o, 88
// bytes): the drawer creates one, steps it after its scene enters and every
// post-poll, culls the registered queries against each camera and draws
// the visible ones against the depth target after the opaque passes. Each
// query counts its pixels into its slot of the coverage buffer, which the
// scene drawer's batches receive.
class RndOcclusionQueryMgr {
public:
    // Builds the occlusion query sphere and its two shaders.
    static void Init();  // 0x5F7ED0
    static void Terminate();  // 0x5F7FB0
    static RndShaderDisplayOcclusionQueryCoverage* GetDisplayCoverageShader();  // 0x5F7EC0

    RndOcclusionQueryMgr();   // 0x5F8010
    ~RndOcclusionQueryMgr();  // 0x5F8080
    // Gives the query the next index and links it into the list.
    void RegisterQuery(RndOcclusionQuery& query);  // 0x5F8110
    // Recreates the coverage buffer when it does not hold one slot per
    // registered query.
    void PostEnter();  // 0x5F8140
    // (Re)creates the coverage buffer ("Occlusion Query Coverage"), one
    // uint per registered query.
    void _AllocCoverageBuffer();  // 0x5F81E0
    // Grows the coverage buffer when queries were registered since.
    void Poll();  // 0x5F8270
    // Culls the enabled queries against the camera's frustum, sets their
    // expected coverage and lists the visible ones for the camera's view.
    // The map's Submit takes a RndContext& first.
    void Submit(const RndCameraContext& camera);  // 0x5F8310
    // The view a target mode draws: 0 for 2D and cube faces, the eye for
    // one eye of a stereo pair, -1 otherwise. Inlined into Submit and
    // DrawQueries; RndLightFlareCom calls it. Name not in the reference
    // map.
    static long GetViewIndex(RndTargetMode mode);  // 0x5F8590
    // Clears the coverage buffer and draws the listed queries of the depth
    // target's view against it, without color or depth writes. Name not in
    // the reference map.
    void DrawQueries(RndContext& context, RndTextureBase& depth);  // 0x5F85C0
    // Draws the occlusion sphere over the world sphere, facing the
    // context's camera, with the bound shader. RndLightFlareEditCom calls
    // it (0x47EBF7).
    static void DrawOcclusionSphere(
        RndContext& context,
        const Sphere& sphere);  // 0x5F8BF0

    // Field names are not in the reference map.
    // The number of registered queries, and the next index.
    unsigned int mNumQueries;
    // One uint of coverage per registered query; the scene drawer's
    // batches receive it.
    RndComputeBuffer* mQueries;
    // The head of the registered queries' list, allocated by the
    // constructor.
    RndOcclusionQuery::Link* mQueryList;
    // The visible queries of each view, filled by Submit.
    eastl::vector<RndOcclusionQuery*> mViewQueries[2];
};

static_assert(offsetof(RndOcclusionQueryMgr, mQueries) == 8);
static_assert(offsetof(RndOcclusionQueryMgr, mQueryList) == 16);
static_assert(offsetof(RndOcclusionQueryMgr, mViewQueries) == 24);
static_assert(sizeof(RndOcclusionQueryMgr) == 88);
