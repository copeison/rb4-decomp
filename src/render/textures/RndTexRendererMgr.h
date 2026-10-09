#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

class PollDepBase;

// The texture renderers of a drawable entity, drawn under the "Tex
// Renderers" GPU statistic block before the entity's own draw. A drawable
// entity's root component creates one on demand
// (RndDrawableEntityCom::ObtainTexRendererMgr). The object is 360 bytes: a
// lock, the renderers and two jobs that bracket their threaded draws. Only
// the members the scene objects call are declared; the class (0x6A8150 on)
// is not reconstructed. Name not in the reference map; it is inferred from
// the statistic name, and the evidence is weak.
class RndTexRendererMgr {
public:
    RndTexRendererMgr();   // 0x6A8150
    ~RndTexRendererMgr();  // 0x6A8250

    // Draws the pending renderers on the immediate context. Name not in the
    // reference map.
    void DrawImmediate();  // 0x6A85C0
    // Queues the pending renderers' draw jobs between the two jobs and adds
    // them to `jobs`. Name not in the reference map.
    void AddDrawJobs(
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs);  // 0x6A8740
    // Clears the dependencies AddDrawJobs created. Name not in the reference
    // map.
    void ClearDrawJobs();  // 0x6A8900

    unsigned char mOpaque[360];  // Not modelled.
};

static_assert(sizeof(RndTexRendererMgr) == 360);
