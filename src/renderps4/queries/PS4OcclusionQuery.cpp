#include "renderps4/queries/PS4OcclusionQuery.h"

namespace {

constexpr unsigned long kResultSize = 256;
constexpr unsigned long kResultAlignment = 16;

}  // namespace

// Reconstructed from eboot.elf at 0x8E28C0.
PS4OcclusionQuery::PS4OcclusionQuery(const char* name)
    : RndOcclusionQuery(name), mResults(nullptr) {}

// Reconstructed from eboot.elf at 0x8E2920.
void PS4OcclusionQuery::_BeginQueryImpl(RndContext& context) {
    mResults = AllocateResults(context, kResultSize, kResultAlignment);
    BeginQueryCommand(context, mResults);
    SetQueryEnabled(context, true);
}

// Reconstructed from eboot.elf at 0x8E29E0.
void PS4OcclusionQuery::_EndQueryImpl(RndContext& context) {
    EndQueryCommand(context, mResults);
    SetQueryEnabled(context, false);
}

// Reconstructed from eboot.elf at 0x8E2A30.
void PS4OcclusionQuery::_BeginPredicationImpl(RndContext& context) {
    BeginPredicationCommand(context, mResults);
}

// Reconstructed from eboot.elf at 0x8E2A60.
void PS4OcclusionQuery::_EndPredicationImpl(RndContext& context) {
    EndPredicationCommand(context);
}
