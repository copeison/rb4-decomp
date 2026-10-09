#include "render/debug/overlays/RndOverlayTextBase.h"

#include "render/context/RndContext.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/drawing/RndDrawUtl.h"

namespace {

// The default background, at 0x1AB1E10. Name not in the reference map.
const Hmx::Color sBackgroundColor(0.4F, 0.4F, 0.4F, 0.4F);

// Darkens every other line of a striped overlay. Name not in the reference
// map.
void DarkenStripe(Hmx::Color& color) {
    color.red *= 0.5F;
    color.green *= 0.5F;
    color.blue *= 0.5F;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6E5DB0. Draw inlines it.
RndOverlayTextBase::OverlayTextStream::OverlayTextStream(
    RndOverlayTextBase* overlay,
    RndContext& context,
    int y)
    : mOverlay(overlay),
      mContext(&context),
      mX(RndOverlayMgr::GetMarginInPixels()),
      mY(y),
      mCursor(mBuffer),
      mLineCount(0) {}

// Reconstructed from eboot.elf at 0x6E5E10. A line ends at a newline or
// when the buffer has room only for the terminator.
void RndOverlayTextBase::OverlayTextStream::Print(const char* str) {
    for (char c = *str; c != '\0'; c = *++str) {
        *mCursor++ = c;
        if (c == '\n') {
            Flush();
            ++mLineCount;
        } else if (mCursor - mBuffer == sizeof(mBuffer) - 1) {
            Flush();
        }
    }
}

// Reconstructed from eboot.elf at 0x6E5EA0. The first line keeps a gap of
// the vertical spacing below it, and striped overlays leave two pixels
// between lines.
void RndOverlayTextBase::OverlayTextStream::Flush() {
    *mCursor = '\0';

    RndDrawUtl::Text2DParams params;
    params.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    params.mColor = mOverlay->_GetTextColor();
    if ((mOverlay->mFlags & kFlagWrapText) != 0) {
        params.mFitMode = kTextFitModeWordWrap;
        params.mWrapWidth = mContext->mViewportSize.x +
                            static_cast<float>(RndOverlayMgr::GetMarginInPixels()) * -2.0F;
    }

    Vector2 position = {static_cast<float>(mX), static_cast<float>(mY)};
    if (mLineCount == 0) {
        position.y = static_cast<float>(mY) -
                     static_cast<float>(RndOverlayMgr::GetVerticalSpacingInPixels());
    } else if ((mOverlay->mFlags & kFlagStripedLines) != 0) {
        position.y = static_cast<float>(mY) - 2.0F;
    }
    Vector2 end = {0.0F, 0.0F};
    const Vector2 viewportSize = mContext->mViewportSize;
    RndDrawUtl::MeasureText2D(mBuffer, position, viewportSize, params, nullptr, &end);

    RndDrawUtl::Quad2DParams background;
    background.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    background.mRect.x = 0.0F;
    background.mRect.y = end.y;
    background.mRect.w = mContext->mViewportSize.x;
    background.mRect.h = static_cast<float>(mY) - end.y;
    background.mColor = mOverlay->_GetBackgroundColor();
    if ((mLineCount & 1) != 0 && (mOverlay->mFlags & kFlagStripedLines) != 0) {
        DarkenStripe(background.mColor);
    }
    background.mBlendMode = RndBlendMode::kSourceAlpha;
    RndDrawUtl::DrawQuad2D(*mContext, background);

    RndDrawUtl::DrawText2D(*mContext, mBuffer, position, params, nullptr, nullptr);
    mY = static_cast<int>(end.y);
    mCursor = mBuffer;
}

// Reconstructed from eboot.elf at 0x6E5B00.
RndOverlayTextBase::RndOverlayTextBase(const char* name, unsigned int flags)
    : RndOverlay(name, flags) {}

// Reconstructed from eboot.elf at 0x6E5B30 (deleting variant at 0x6E5B40).
RndOverlayTextBase::~RndOverlayTextBase() {}

// Reconstructed from eboot.elf at 0x6E5B60. An unterminated last line is
// finished with a newline; the closing band continues the stripes.
int RndOverlayTextBase::Draw(RndContext& context, int y) {
    OverlayTextStream stream(this, context, y);
    _Print(stream);
    if (stream.mCursor > stream.mBuffer) {
        stream << "\n";
    }

    RndDrawUtl::Quad2DParams params;
    params.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    params.mRect.x = 0.0F;
    params.mRect.y = static_cast<float>(stream.mY) + -5.0F;
    params.mRect.w = context.mViewportSize.x;
    params.mRect.h = static_cast<float>(RndOverlayMgr::GetVerticalSpacingInPixels());
    params.mColor = _GetBackgroundColor();
    if ((stream.mLineCount & 1) == 0 && (mFlags & kFlagStripedLines) != 0) {
        DarkenStripe(params.mColor);
    }
    params.mBlendMode = RndBlendMode::kSourceAlpha;
    RndDrawUtl::DrawQuad2D(context, params);
    return static_cast<int>(params.mRect.y);
}

// Reconstructed from eboot.elf at 0x6E5D90.
Hmx::Color RndOverlayTextBase::_GetBackgroundColor() const {
    return sBackgroundColor;
}
