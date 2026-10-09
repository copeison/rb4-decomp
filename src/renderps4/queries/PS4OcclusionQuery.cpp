#include "renderps4/queries/PS4OcclusionQuery.h"

#include "renderps4/context/PS4Context.h"

static_assert(sizeof(sce::Gnm::OcclusionQueryResults) == 256);

// Reconstructed from eboot.elf at 0x8E28C0.
PS4OcclusionQuery::PS4OcclusionQuery(const char* name)
    : RndOcclusionQuery(name), mResults(nullptr) {}

// Reconstructed from eboot.elf at 0x8E2920. The results live in embedded
// command-buffer memory, so a query is read back in the frame it ran.
void PS4OcclusionQuery::_BeginQueryImpl(RndContext& context) {
    auto& gfx = static_cast<PS4Context&>(context)._ActiveGfxContext();
    mResults = static_cast<sce::Gnm::OcclusionQueryResults*>(gfx.allocateFromCommandBuffer(
        sizeof(sce::Gnm::OcclusionQueryResults), sce::Gnm::kEmbeddedDataAlignment16));
    gfx.writeOcclusionQuery(sce::Gnm::kOcclusionQueryOpClearAndBegin, mResults);
    gfx.setDbCountControl(sce::Gnm::kDbCountControlPerfectZPassCountsEnable, 0);
}

// Reconstructed from eboot.elf at 0x8E29E0.
void PS4OcclusionQuery::_EndQueryImpl(RndContext& context) {
    auto& gfx = static_cast<PS4Context&>(context)._ActiveGfxContext();
    gfx.writeOcclusionQuery(sce::Gnm::kOcclusionQueryOpEnd, mResults);
    gfx.setDbCountControl(sce::Gnm::kDbCountControlPerfectZPassCountsDisable, 0);
}

// Reconstructed from eboot.elf at 0x8E2A30. Predicated draws wait for the
// result and draw when any fragment passed.
void PS4OcclusionQuery::_BeginPredicationImpl(RndContext& context) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setZPassPredicationEnable(
        mResults,
        sce::Gnm::kPredicationZPassHintWait,
        sce::Gnm::kPredicationZPassActionDrawIfVisible);
}

// Reconstructed from eboot.elf at 0x8E2A60.
void PS4OcclusionQuery::_EndPredicationImpl(RndContext& context) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setZPassPredicationDisable();
}
