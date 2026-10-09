#include "render/debug/overlays/RndOverlayGraphBase.h"

#include "render/debug/RndOverlayMgr.h"

// Reconstructed from eboot.elf at 0x6E6280. Without a budget the graph
// tops out at a 30 fps frame. The ten series colors are built on first
// use.
RndTimerGraphOverlay::RndTimerGraphOverlay(
    const char* name,
    unsigned int flags,
    const char* timersName,
    float budget)
    : RndOverlayGraphBase(name, flags),
      mTimersOverlay(RndOverlayMgr::GetOverlay(Symbol(timersName))),
      mTimeWindow(5.0F) {
    mMaxMs = budget > 0.0F ? budget + budget : 1000.0F / 30.0F;
    mUnknown248.reserve(20);
    if (budget > 0.0F) {
        mBudgetLine.resize(2);
        mBudgetLine[0].y = budget;
        mBudgetLine[1].y = budget;
    }

    static const Hmx::Color sPalette[] = {
        Hmx::Color::GetGreen(),
        Hmx::Color::GetCyan(),
        Hmx::Color::GetBlue(),
        Hmx::Color::GetViolet(),
        Hmx::Color::GetYellow(),
        Hmx::Color::GetOrange(),
        Hmx::Color::GetRed(),
        Hmx::Color::GetMagenta(),
        Hmx::Color::GetGrey(),
        Hmx::Color(1.0F, 0.5F, 0.5F, 1.0F),
    };
    mPalette.resize(sizeof(sPalette) / sizeof(sPalette[0]));
    for (unsigned long i = 0; i < mPalette.size(); ++i) {
        mPalette[i].mColor = sPalette[i];
        mPalette[i].mUnknown16 = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x6E6880 (deleting variant at 0x6E6930).
RndTimerGraphOverlay::~RndTimerGraphOverlay() {}

// Reconstructed from eboot.elf at 0x6E72F0.
RndOverlayGraphBase::GraphOptions RndTimerGraphOverlay::_GetOptions() {
    GraphOptions options;
    options.mUnknown4 = true;
    options.mUnknown8 = 0;
    return options;
}
