#include "render/debug/overlays/RndConsoleOverlay.h"

#include <new>

namespace {

// The console's colors. Draw's input-line background (0, 0.4, 0.4, 0.8) at
// 0x1AB1D30 is not declared until Draw is reconstructed. Names not in the
// reference map.
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
