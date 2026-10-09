#include "render/debug/overlays/RndOverlayGraphBase.h"

#include <limits>
#include <new>

#include "render/context/RndContext.h"

namespace {

// Rounds half away from zero, saturating at the int range. Name not in the
// reference map.
int RoundToInt(float value) {
    if (value > 0.0F) {
        value += 0.5F;
        return value < 2147483648.0F ? static_cast<int>(value)
                                     : std::numeric_limits<int>::max();
    }
    value -= 0.5F;
    return value > -2147483648.0F ? static_cast<int>(value)
                                  : std::numeric_limits<int>::min();
}

}  // namespace

// Reconstructed from eboot.elf at 0x6E33A0.
RndOverlayGraphBase::GraphOptions::GraphOptions()
    : mHeight(0.5F), mShowLegend(true), mUnknown8(2) {}

// Reconstructed from eboot.elf at 0x6E33E0.
RndOverlayGraphBase::GraphAxes::GraphAxes()
    : mColor(Hmx::Color::GetWhite()), mUnknown64(0.0F), mUnknown68(0) {}

// Reconstructed from eboot.elf at 0x6E3470.
RndOverlayGraphBase::GraphSeries::GraphSeries()
    : mColor(Hmx::Color::GetWhite()), mPoints(nullptr), mNumPoints(0) {}

// Reconstructed from eboot.elf at 0x6E32E0. Reserves room for 200 lines.
RndOverlayGraphBase::RndOverlayGraphBase(const char* name, unsigned int flags)
    : RndOverlay(name, flags) {
    mLines.reserve(200);
}

// Reconstructed from eboot.elf at 0x6E34F0 (deleting variant at 0x6E3530).
RndOverlayGraphBase::~RndOverlayGraphBase() {}

// Reconstructed from eboot.elf at 0x6E3580. The band's height is rounded
// to whole pixels; the legend lists the series in order when the options
// ask for one.
int RndOverlayGraphBase::Draw(RndContext& context, int y) {
    const GraphOptions options = _GetOptions();
    const int top = y - RoundToInt(options.mHeight * context.mViewportSize.y);
    GraphAxes axes = _GetAxes();
    const Vector2 viewportSize = context.mViewportSize;
    _FitAxes(viewportSize, top, y, axes);

    const unsigned long numSeries = _GetNumSeries();
    Symbol* names = nullptr;
    Hmx::Color* colors = nullptr;
    if (numSeries != 0 && options.mShowLegend) {
        names = static_cast<Symbol*>(__builtin_alloca(numSeries * sizeof(Symbol)));
        colors = static_cast<Hmx::Color*>(__builtin_alloca(numSeries * sizeof(Hmx::Color)));
    }
    unsigned long numNames = 0;
    unsigned long numColors = 0;
    for (unsigned long i = 0; i < numSeries; ++i) {
        const GraphSeries series = _GetSeries(i);
        _DrawSeries(context, top, y, axes, series);
        if (options.mShowLegend) {
            new (&names[numNames++]) Symbol(series.mName);
            new (&colors[numColors++]) Hmx::Color(series.mColor);
        }
    }

    _DrawAxes(context, top, y, axes);
    if (options.mShowLegend) {
        _DrawLegend(
            context,
            options,
            top,
            y,
            VectorAdapter<Symbol>{names, numNames},
            VectorAdapter<Hmx::Color>{colors, numColors});
    }
    return top;
}
