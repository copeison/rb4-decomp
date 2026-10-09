#pragma once

class RndCameraContext;
class RndContext;
class RndOcclusionQuery;
class RndTextureBase;
enum RndTargetMode : int;
class RndShaderDisplayOcclusionQueryCoverage;

// The registry that submits the occlusion queries. Only the startup and
// shutdown entry points that Rnd::Init and Rnd::Terminate call and the
// coverage shader accessor are declared.
class RndOcclusionQueryMgr {
public:
    // Builds the occlusion query sphere and its two shaders.
    static void Init();  // 0x5F7ED0
    static void Terminate();  // 0x5F7FB0
    static RndShaderDisplayOcclusionQueryCoverage* GetDisplayCoverageShader();  // 0x5F7EC0

    // A scene drawer's queries (88 bytes): the drawer creates one, steps it
    // after its scene enters and every post-poll, and submits each camera's
    // queries. The map's Submit takes a RndContext& first.
    RndOcclusionQueryMgr();   // 0x5F8010
    ~RndOcclusionQueryMgr();  // 0x5F8080
    void PostEnter();         // 0x5F8140
    void Poll();              // 0x5F8270
    // Gives the query the next index and links it into the list.
    void RegisterQuery(RndOcclusionQuery& query);  // 0x5F8110
    // The view a target mode draws: 0 for 2D and cube faces, the eye for
    // one eye of a stereo pair, -1 otherwise. Inlined into DrawQueries;
    // RndLightFlareCom calls it. Name not in the reference map.
    static long GetViewIndex(RndTargetMode mode);  // 0x5F8590
    void Submit(const RndCameraContext& camera);  // 0x5F8310
    // Draws the queries of the depth target's view against it after the
    // opaque passes. Name not in the reference map.
    void DrawQueries(RndContext& context, RndTextureBase& depth);  // 0x5F85C0

    unsigned char mOpaque0[8];  // Not modelled.
    // What the scene drawer's batches receive (+8). Name not in the
    // reference map; the type is not recovered.
    void* mQueries;
    unsigned char mOpaque16[72];  // Not modelled.
};
