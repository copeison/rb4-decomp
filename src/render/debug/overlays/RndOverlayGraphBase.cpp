#include "render/debug/overlays/RndOverlayGraphBase.h"

#include <cstring>
#include <limits>
#include <new>

#include "render/context/RndContext.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/drawing/RndDrawUtl.h"

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

// The room an axis' labels need beyond the other axis' line, in pixels,
// without and with a label. Name not in the reference map.
const float kLabelSpace[2] = {10.0F, 21.0F};  // 0x12BADD0

// Tick and label placement, in pixels. Names not in the reference map.
constexpr float kTickLength = 5.0F;
constexpr float kLabelOffset = 8.0F;
constexpr float kQuarterTurn = 1.5707964F;

// The legend's border in pixels and its box color. Names not in the
// reference map.
constexpr float kLegendBorder = 5.0F;
const Hmx::Color kLegendColor(0.25F, 0.25F, 0.25F, 0.6F);

}  // namespace

// Reconstructed from eboot.elf at 0x6E33A0.
RndOverlayGraphBase::GraphOptions::GraphOptions()
    : mHeight(0.5F), mShowLegend(true), mLegendAlignX(2), mLegendAlignY(0) {}

// Reconstructed from eboot.elf at 0x6E33E0.
RndOverlayGraphBase::GraphAxes::GraphAxes()
    : mColor(Hmx::Color::GetWhite()), mOriginX(0.0F), mOriginY(0.0F) {}

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

// Reconstructed from eboot.elf at 0x6E3880. The ranges grow away from the
// label side, so that the labels of the other axis fit between its line
// and the band's edge.
void RndOverlayGraphBase::_FitAxes(
    const Vector2& viewportSize,
    int top,
    int bottom,
    GraphAxes& axes) {
    axes.mX.mLabelSide = axes.mX.mLabelSide > 0.0F ? 1.0F : -1.0F;
    axes.mY.mLabelSide = axes.mY.mLabelSide > 0.0F ? 1.0F : -1.0F;
    Segment2D xAxis = {};
    Segment2D yAxis = {};
    _GetAxisLines(viewportSize, top, bottom, axes, xAxis, yAxis);

    const bool xLabel = axes.mX.mLabel != Symbol();
    if (xLabel || axes.mX.mStep > 0.0F) {
        const float side = axes.mX.mLabelSide;
        const float overlap = (side < 0.0F ? yAxis.start.y - xAxis.start.y
                                           : xAxis.start.y - yAxis.end.y) +
                              kLabelSpace[xLabel];
        if (overlap > 0.0F) {
            const float height = yAxis.end.y - yAxis.start.y;
            const float range = (axes.mY.mMax - axes.mY.mMin) * height / (height - overlap);
            if (side < 0.0F) {
                axes.mY.mMin = axes.mY.mMax - range;
            } else {
                axes.mY.mMax = range + axes.mY.mMin;
            }
        }
    }

    const bool yLabel = axes.mY.mLabel != Symbol();
    if (yLabel || axes.mY.mStep > 0.0F) {
        const float side = axes.mY.mLabelSide;
        const float overlap = (side < 0.0F ? xAxis.start.x - yAxis.start.x
                                           : yAxis.start.x - xAxis.end.x) +
                              kLabelSpace[yLabel];
        if (overlap > 0.0F) {
            const float width = xAxis.end.x - xAxis.start.x;
            const float range = (axes.mX.mMax - axes.mX.mMin) * width / (width - overlap);
            if (side < 0.0F) {
                axes.mX.mMin = axes.mX.mMax - range;
            } else {
                axes.mX.mMax = range + axes.mX.mMin;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x6E3A70. Each line runs from the
// previous point to the next; a line leaving the band is cut at its edge,
// and a line wholly above or below it is dropped.
void RndOverlayGraphBase::_DrawSeries(
    RndContext& context,
    int top,
    int bottom,
    const GraphAxes& axes,
    const GraphSeries& series) {
    if (series.mNumPoints == 0) {
        return;
    }
    const unsigned long numLines = series.mNumPoints - 1;
    mLines.reserve(numLines);
    const Vector2 viewportSize = context.mViewportSize;
    const float topY = static_cast<float>(top);
    const float bottomY = static_cast<float>(bottom);
    Vector2 previous = _GraphToPixels(series.mPoints[0], viewportSize, top, bottom, axes);
    for (unsigned long i = 0; i < numLines; ++i) {
        const Vector2 next = _GraphToPixels(series.mPoints[i + 1], viewportSize, top, bottom, axes);
        const Vector2 from = previous;
        previous = next;
        if ((from.y < topY && next.y < topY) || (from.y > bottomY && next.y > bottomY)) {
            continue;
        }
        Vector2 start = from;
        Vector2 end = next;
        const bool fromOutside = from.y > bottomY || from.y < topY;
        const bool nextOutside = next.y > bottomY || next.y < topY;
        if (fromOutside || nextOutside) {
            const float slope = (next.y - from.y) / (next.x - from.x);
            if (fromOutside) {
                const float edge = static_cast<float>(from.y > bottomY ? bottom : top);
                start = {next.x + (edge - next.y) / slope, edge};
            }
            if (nextOutside) {
                const float edge = static_cast<float>(next.y > bottomY ? bottom : top);
                end = {(edge - start.y) / slope + start.x, edge};
            }
        }
        mLines.push_back(Segment2D{start, end});
    }

    RndDrawUtl::Line2DParams params;
    params.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    params.mColor = series.mColor;
    RndDrawUtl::DrawLines2D(
        context, VectorAdapter<Segment2D>{mLines.begin(), mLines.size()}, params);
    mLines.clear();
}

// Reconstructed from eboot.elf at 0x6E4190. The ticks are spaced from the
// origin both ways to the ends of each axis and point to the label side.
// The x label is centered on its axis; the y label is turned a quarter
// towards its side.
void RndOverlayGraphBase::_DrawAxes(
    RndContext& context,
    int top,
    int bottom,
    const GraphAxes& axes) {
    Segment2D xAxis = {};
    Segment2D yAxis = {};
    const Vector2 viewportSize = context.mViewportSize;
    _GetAxisLines(viewportSize, top, bottom, axes, xAxis, yAxis);

    unsigned long maxLines = 2;
    if (axes.mX.mStep > 0.0F) {
        maxLines += static_cast<unsigned long>(
            __builtin_ceilf((axes.mX.mMax - axes.mX.mMin) / axes.mX.mStep));
    }
    if (axes.mY.mStep > 0.0F) {
        maxLines += static_cast<unsigned long>(
            __builtin_ceilf((axes.mY.mMax - axes.mY.mMin) / axes.mY.mStep));
    }
    Segment2D* lines = nullptr;
    if (maxLines != 0) {
        lines = static_cast<Segment2D*>(__builtin_alloca(maxLines * sizeof(Segment2D)));
    }
    lines[0] = xAxis;
    lines[1] = yAxis;
    unsigned long numLines = 2;

    if (axes.mX.mStep > 0.0F) {
        const float step =
            (xAxis.end.x - xAxis.start.x) * (axes.mX.mStep / (axes.mX.mMax - axes.mX.mMin));
        const float y = xAxis.start.y;
        const float tickY = axes.mX.mLabelSide * kTickLength + y;
        for (float x = yAxis.start.x - step; x >= xAxis.start.x; x -= step) {
            lines[numLines++] = Segment2D{{x, y}, {x, tickY}};
        }
        for (float x = yAxis.start.x + step; x <= xAxis.end.x; x += step) {
            lines[numLines++] = Segment2D{{x, y}, {x, tickY}};
        }
    }
    if (axes.mY.mStep > 0.0F) {
        const float step =
            (yAxis.end.y - yAxis.start.y) * (axes.mY.mStep / (axes.mY.mMax - axes.mY.mMin));
        const float x = yAxis.start.x;
        const float tickX = axes.mY.mLabelSide * kTickLength + x;
        for (float y = xAxis.start.y - step; y >= yAxis.start.y; y -= step) {
            lines[numLines++] = Segment2D{{x, y}, {tickX, y}};
        }
        for (float y = xAxis.start.y + step; y <= yAxis.end.y; y += step) {
            lines[numLines++] = Segment2D{{x, y}, {tickX, y}};
        }
    }

    RndDrawUtl::Line2DParams lineParams;
    lineParams.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    lineParams.mColor = axes.mColor;
    RndDrawUtl::DrawLines2D(context, VectorAdapter<Segment2D>{lines, numLines}, lineParams);

    RndDrawUtl::Text2DParams textParams;
    textParams.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    textParams.mShadow = true;
    textParams.mUnknown84 = 1;
    if (axes.mX.mLabel != Symbol()) {
        textParams.mUnknown80 = axes.mX.mLabelSide >= 0.0F ? 2 : 0;
        const Vector2 position = {
            static_cast<float>(RoundToInt((xAxis.end.x + xAxis.start.x) * 0.5F)),
            axes.mX.mLabelSide * kLabelOffset + xAxis.start.y};
        RndDrawUtl::DrawText2D(
            context, axes.mX.mLabel.Str(), position, textParams, nullptr, nullptr);
    }
    if (axes.mY.mLabel != Symbol()) {
        textParams.mUnknown80 = 0;
        textParams.mRotation = axes.mY.mLabelSide * kQuarterTurn;
        const Vector2 position = {
            axes.mY.mLabelSide * kLabelOffset + yAxis.start.x,
            static_cast<float>(RoundToInt((yAxis.end.y + yAxis.start.y) * 0.5F))};
        RndDrawUtl::DrawText2D(
            context, axes.mY.mLabel.Str(), position, textParams, nullptr, nullptr);
    }
}

// Reconstructed from eboot.elf at 0x6E47B0. The box fits the longest name
// with a 5-pixel border and is outlined in white; the names are listed
// upwards from its bottom.
void RndOverlayGraphBase::_DrawLegend(
    RndContext& context,
    const GraphOptions& options,
    int top,
    int bottom,
    const VectorAdapter<Symbol>& names,
    const VectorAdapter<Hmx::Color>& colors) {
    Symbol longest;
    unsigned long longestLength = 0;
    for (unsigned long i = 0; i < names.mSize; ++i) {
        const unsigned long length = std::strlen(names.mData[i].Str());
        if (longestLength < length) {
            longestLength = length;
            longest = names.mData[i];
        }
    }

    RndDrawUtl::Text2DParams textParams;
    textParams.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    Hmx::Rect lineBounds(0.0F, 0.0F, 0.0F, 0.0F);
    const Vector2 viewportSize = context.mViewportSize;
    RndDrawUtl::MeasureText2D(
        longest.Str(), Vector2::sZero, viewportSize, textParams, &lineBounds, nullptr);

    Hmx::Rect box;
    box.x = 0.0F;
    box.y = 0.0F;
    box.w = kLegendBorder * 2.0F + lineBounds.w;
    box.h = static_cast<float>(names.mSize) * lineBounds.h + kLegendBorder * 2.0F;
    switch (options.mLegendAlignX) {
    case 0:
        box.x = static_cast<float>(RndOverlayMgr::GetMarginInPixels());
        break;
    case 1:
        box.x = (context.mViewportSize.x - box.w) * 0.5F;
        break;
    case 2:
        box.x = context.mViewportSize.x - box.w -
                static_cast<float>(RndOverlayMgr::GetMarginInPixels());
        break;
    default:
        break;
    }
    switch (options.mLegendAlignY) {
    case 0:
        box.y = static_cast<float>(bottom) + -kLegendBorder - box.h;
        break;
    case 1:
        box.y = (static_cast<float>(bottom - top) - box.h) * 0.5F;
        break;
    case 2:
        box.y = static_cast<float>(top) + -kLegendBorder;
        break;
    default:
        break;
    }

    RndDrawUtl::Quad2DParams background;
    background.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    background.mRect = box;
    background.mColor = kLegendColor;
    background.mBlendMode = RndBlendMode::kSourceAlpha;
    RndDrawUtl::DrawQuad2D(context, background);
    RndDrawUtl::Line2DParams outline;
    outline.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    RndDrawUtl::DrawQuadWireframe2D(context, box, outline);

    Vector2 position = {box.x + kLegendBorder, box.y + -kLegendBorder + box.h};
    for (unsigned long i = 0; i < names.mSize; ++i) {
        textParams.mColor = colors.mData[i];
        RndDrawUtl::DrawText2D(context, names.mData[i].Str(), position, textParams, nullptr, nullptr);
        position.y -= lineBounds.h;
    }
}

// Reconstructed from eboot.elf at 0x6E4D00.
Vector2 RndOverlayGraphBase::_GraphToPixels(
    const Vector2& point,
    const Vector2& viewportSize,
    int top,
    int bottom,
    const GraphAxes& axes) {
    const float u = (point.x - axes.mX.mMin) / (axes.mX.mMax - axes.mX.mMin);
    const float v = (point.y - axes.mY.mMin) / (axes.mY.mMax - axes.mY.mMin);
    const float margin = static_cast<float>(RndOverlayMgr::GetMarginInPixels());
    const float x =
        u * ((viewportSize.x + margin * -2.0F) / viewportSize.x) + margin / viewportSize.x;
    const float inverseHeight = 1.0F / viewportSize.y;
    const float y = static_cast<float>(bottom - top) * inverseHeight * v +
                    static_cast<float>(top) * inverseHeight;
    return {static_cast<float>(RoundToInt(x * viewportSize.x)),
            static_cast<float>(RoundToInt(y * viewportSize.y))};
}

// Reconstructed from eboot.elf at 0x6E4E80.
void RndOverlayGraphBase::_GetAxisLines(
    const Vector2& viewportSize,
    int top,
    int bottom,
    const GraphAxes& axes,
    Segment2D& xAxis,
    Segment2D& yAxis) {
    xAxis = Segment2D{{axes.mX.mMin, axes.mOriginY}, {axes.mX.mMax, axes.mOriginY}};
    yAxis = Segment2D{{axes.mOriginX, axes.mY.mMin}, {axes.mOriginX, axes.mY.mMax}};
    xAxis.start = _GraphToPixels(xAxis.start, viewportSize, top, bottom, axes);
    xAxis.end = _GraphToPixels(xAxis.end, viewportSize, top, bottom, axes);
    yAxis.start = _GraphToPixels(yAxis.start, viewportSize, top, bottom, axes);
    yAxis.end = _GraphToPixels(yAxis.end, viewportSize, top, bottom, axes);
}
