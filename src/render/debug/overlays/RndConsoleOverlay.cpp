#include "render/debug/overlays/RndConsoleOverlay.h"

#include <cmath>
#include <cstring>
#include <new>

#include "render/context/RndContext.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/drawing/RndDrawUtl.h"
#include "utl/time/TimeMgr.h"

namespace {

// The console's colors. Names not in the reference map.
const Hmx::Color sInputBackgroundColor(0.0F, 0.4F, 0.4F, 0.8F);  // 0x1AB1D30
const Hmx::Color sBackgroundColor(0.0F, 0.3F, 0.3F, 0.8F);       // 0x1AB1D40
// Output of type 0, and the input line.
const Hmx::Color sInputColor(1.0F, 1.0F, 0.0F, 1.0F);  // 0x1AB1D50
// Output of any type but 0 and 1.
const Hmx::Color sOtherOutputColor(1.0F, 0.5F, 0.0F, 1.0F);  // 0x1AB1D60

}  // namespace

// Reconstructed from eboot.elf at 0x6E19F0. Fills the line storage and
// reserves 128 characters per line.
RndConsoleOverlay::ConsoleDebugReflection::ConsoleDebugReflection(RndConsoleOverlay* owner)
    : mLines(mStorage),
      mNumStored(0),
      mCapacity(kNumLines),
      mOwner(owner),
      mCurrentLine(0),
      mNumLines(0),
      mLineOpen(false) {
    while (mNumStored < kNumLines) {
        new (&mLines[mNumStored++]) Line();
    }
    while (mNumStored > kNumLines) {
        mLines[--mNumStored].~Line();
    }
    for (unsigned long i = 0; i < mNumStored; ++i) {
        mLines[i].mText.reserve(128);
    }
}

// Reconstructed from eboot.elf at 0x6E1B70. A line takes the input's output
// type when its first character arrives and ends after its newline; the
// oldest line is reused once all are written.
void RndConsoleOverlay::ConsoleDebugReflection::Print(const char* str) {
    ScopedCritSec lock(mCritSec);
    for (char c = *str; c != '\0'; c = *++str) {
        if (!mLineOpen) {
            mLineOpen = true;
            Line& line = mLines[mCurrentLine];
            line.mText.erase();
            line.mType = mOwner->mInput.mOutputType;
            const unsigned long count = mNumLines + 1;
            mNumLines = count >= kNumLines ? kNumLines : count;
            c = *str;
        }
        mLines[mCurrentLine].mText += c;
        if (*str == '\n') {
            mCurrentLine = (mCurrentLine + 1) % kNumLines;
            mLineOpen = false;
        }
    }
}

// Reconstructed from eboot.elf at 0x6E0FE0.
RndConsoleOverlay::RndConsoleOverlay()
    : RndOverlayTextBase(
          "console",
          kFlagKeyboard | kFlagHasHelp | kFlagUnknown8 | kFlagWrapText),
      mInput(this),
      mReflection(this),
      mPrintType(-1) {}

// Reconstructed from eboot.elf at 0x6E1070 (deleting variant at 0x6E1250).
RndConsoleOverlay::~RndConsoleOverlay() {}

// Reconstructed from eboot.elf at 0x6E1270. The output is drawn only once
// a line exists. The input line follows a "> " prompt on a band of twice
// the vertical spacing, and the cursor is an underscore shown for the
// first half of every 0.6 seconds.
int RndConsoleOverlay::Draw(RndContext& context, int y) {
    if (mReflection.mNumLines != 0 || mReflection.mLineOpen) {
        y = RndOverlayTextBase::Draw(context, y);
    }

    static const unsigned long sPromptLength = std::strlen("> ");
    const char* input = mInput._GetText();
    auto* line = static_cast<char*>(
        __builtin_alloca(sPromptLength + std::strlen(input) + 1));
    std::strcpy(line, "> ");
    std::strcpy(line + sPromptLength, input);

    RndDrawUtl::Text2DParams params;
    params.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    params.mColor = sInputColor;
    params.mWrapMode = 1;
    params.mWrapWidth = context.mViewportSize.x +
                        static_cast<float>(RndOverlayMgr::GetMarginInPixels()) * -2.0F;
    Vector2 position = {
        static_cast<float>(RndOverlayMgr::GetMarginInPixels()),
        static_cast<float>(y - RndOverlayMgr::GetVerticalSpacingInPixels())};
    Hmx::Rect bounds(0.0F, 0.0F, 0.0F, 0.0F);
    const Vector2 viewportSize = context.mViewportSize;
    RndDrawUtl::MeasureText2D(line, position, viewportSize, params, &bounds, nullptr);

    RndDrawUtl::Quad2DParams background;
    background.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
    background.mRect.x = 0.0F;
    background.mRect.w = context.mViewportSize.x;
    background.mRect.h =
        static_cast<float>(RndOverlayMgr::GetVerticalSpacingInPixels()) * 2.0F + bounds.h;
    background.mRect.y = static_cast<float>(y) - background.mRect.h;
    background.mColor = sInputBackgroundColor;
    background.mBlendMode = RndBlendMode::kSourceAlpha;
    RndDrawUtl::DrawQuad2D(context, background);
    RndDrawUtl::DrawText2D(context, line, position, params, nullptr, nullptr);

    if (std::fmod(TheTimeMgr->mClock.Seconds(), 0.6F) < 0.3F) {
        line[sPromptLength + mInput._GetCursor()] = '\0';
        Vector2 cursor = {0.0F, 0.0F};
        const Vector2 size = context.mViewportSize;
        RndDrawUtl::MeasureText2D(line, position, size, params, nullptr, &cursor);
        cursor.y += -2.0F;
        RndDrawUtl::DrawText2D(context, "_", cursor, params, nullptr, nullptr);
    }
    return static_cast<int>(background.mRect.y);
}

// Reconstructed from eboot.elf at 0x6E1700.
bool RndConsoleOverlay::HandleKeyboardMsg(const KeyboardKeyMsg& msg) {
    return mInput.HandleKeyboardMsg(msg);
}

// Reconstructed from eboot.elf at 0x6E1710.
void RndConsoleOverlay::PrintHelp(TextStream& stream) {
    stream << "editing:\n"
              "  tab:        cycle through possible completions of current word\n"
              "              (only implemented for data-funcs and data-vars)\n"
              "  ctrl-C:     clear current line\n"
              "  ctrl-L:     clear console output\n"
              "cursor navigation:\n"
              "  left:       move cursor back one character\n"
              "  right:      move cursor back one character\n"
              "  home:       move cursor to the start of the line\n"
              "  end:        move cursor to the end of the line\n"
              "  ctrl-left:  move cursor to prev word\n"
              "  ctrl-right: move cursor to next word\n"
              "history:\n"
              "  up:         move to previous command\n"
              "  down:       move to next command\n"
              "  ctrl-up:    move to previous command which matches input up to cursor\n"
              "  ctrl-down:  move to next command which matches input up to cursor\n";
}

// Reconstructed from eboot.elf at 0x6E1730. Prints the kept lines oldest
// first, ending with the last complete line or the open one, and sets
// mPrintType for _GetTextColor while each is drawn.
void RndConsoleOverlay::_Print(TextStream& stream) {
    constexpr unsigned long kNumLines = ConsoleDebugReflection::kNumLines;
    unsigned long last = mReflection.mCurrentLine;
    if (!mReflection.mLineOpen) {
        last = (last + kNumLines - 1) % kNumLines;
    }
    const unsigned long first = (last + kNumLines + 1 - mReflection.mNumLines) % kNumLines;
    for (unsigned long i = 0; i < mReflection.mNumLines; ++i) {
        const auto& line = mReflection.mLines[(first + i) % kNumLines];
        mPrintType = line.mType;
        stream.Print(line.mText.c_str());
    }
    mPrintType = -1;
}

// Reconstructed from eboot.elf at 0x6E1810.
Hmx::Color RndConsoleOverlay::_GetTextColor() const {
    switch (mPrintType) {
    case 0:
        return sInputColor;
    case 1:
        return Hmx::Color::GetWhite();
    default:
        return sOtherOutputColor;
    }
}

// Reconstructed from eboot.elf at 0x6E18A0.
Hmx::Color RndConsoleOverlay::_GetBackgroundColor() const {
    return sBackgroundColor;
}
