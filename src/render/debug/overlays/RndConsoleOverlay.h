#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "render/debug/overlays/ConsoleLineEditor.h"
#include "render/debug/overlays/RndOverlayTextBase.h"
#include "utl/text/Str.h"

// The "console" overlay: a script command line with history and tab
// completion over the last lines of debug output. The vtable is at
// 0x19393B0.
class RndConsoleOverlay : public RndOverlayTextBase {
public:
    // The command line: the script line editor specialised for the
    // console, whose command output the console's reflection collects. The
    // editor's methods replace the map's _History*, _TabCompletion* and
    // _ExecuteCommand members. The vtable is at 0x1939418. Name not in the
    // reference map.
    class ConsoleInput : public ConsoleLineEditor {
    public:
        // Edits every key (kEditKeys and kCursorKeys).
        explicit ConsoleInput(RndConsoleOverlay* owner);  // 0x6E18C0
        // Slots 0-1: 0x6E1210, 0x6E1CC0.
        ~ConsoleInput() override;
        // Slot 2 at 0x6E1D00.
        const char* _GetText() const override {
            return mText.c_str();
        }
        // Slot 3 at 0x6E1D10.
        void _SetText(const char* text) override {
            mText = text;
        }
        // Slot 4 at 0x6E1D90: the cursor's index in the input line.
        unsigned long _GetCursor() const override {
            return mCursor;
        }
        // Slot 5 at 0x6E1DA0.
        void _SetCursor(unsigned long cursor) override {
            mCursor = cursor;
        }
        // Slots 6 and 7: the console's reflection receives the debug
        // output while a command runs.
        void _BeginCommand() override;  // 0x6E1910
        void _EndCommand() override;    // 0x6E1930
        // Slot 8: forgets the kept output lines.
        void _ClearOutput() override;  // 0x6E1950
        // Slot 9: waits for a print in progress on another thread.
        void _FlushOutput() override;  // 0x6E1990

        // Field names are not in the reference map.
        RndConsoleOverlay* mOwner;
        String mText;  // The input line.
        unsigned long mCursor;
    };

    // Keeps the last lines of the debug output for _Print. The map's
    // constructor takes no argument. The vtable is at 0x1939478.
    class ConsoleDebugReflection : public TextStream {
    public:
        // Name not in the reference map.
        static constexpr unsigned long kNumLines = 30;

        // One output line. Name not in the reference map.
        struct Line {
            String mText;
            int mType = -1;  // The input's output type when it started.
        };

        explicit ConsoleDebugReflection(RndConsoleOverlay* owner);  // 0x6E19F0
        // Slots 0-1 at 0x6E1160 and 0x6E1DB0; the console's destructor
        // inlines the complete one.
        ~ConsoleDebugReflection() override {
            for (unsigned long i = 0; i < mNumStored; ++i) {
                mLines[i].~Line();
            }
        }

        void Print(const char* str) override;  // slot 2: 0x6E1B70

        // The lines, laid out as a FixedVector<Line, kNumLines>. They are
        // spelled out because FixedVector leaves its elements alive when
        // it is destroyed. Field names are not in the reference map.
        Line* mLines;
        unsigned long mNumStored;
        unsigned long mCapacity;
        union {
            Line mStorage[kNumLines];
        };
        RndConsoleOverlay* mOwner;
        unsigned long mCurrentLine;  // The line being written.
        unsigned long mNumLines;     // Lines written, at most kNumLines.
        bool mLineOpen;              // The current line has text.
        CritSec mCritSec;
    };

    RndConsoleOverlay();  // 0x6E0FE0
    // Slots 0-1: 0x6E1070, 0x6E1250.
    ~RndConsoleOverlay() override;

    // Draws the output, then the input line with a blinking cursor.
    int Draw(RndContext& context, int y) override;               // slot 2: 0x6E1270
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E1700
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E1710
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E1730
    Hmx::Color _GetTextColor() const override;                   // slot 9: 0x6E1810
    Hmx::Color _GetBackgroundColor() const override;             // slot 10: 0x6E18A0

    // Field names are not in the reference map.
    ConsoleInput mInput;
    ConsoleDebugReflection mReflection;
    // The type of the output line being drawn, or -1 outside _Print.
    int mPrintType;
};

static_assert(offsetof(RndConsoleOverlay::ConsoleInput, mOutputType) == 568);
static_assert(offsetof(RndConsoleOverlay::ConsoleInput, mOwner) == 576);
static_assert(offsetof(RndConsoleOverlay::ConsoleInput, mText) == 584);
static_assert(offsetof(RndConsoleOverlay::ConsoleInput, mCursor) == 600);
static_assert(sizeof(RndConsoleOverlay::ConsoleInput) == 608);
static_assert(sizeof(RndConsoleOverlay::ConsoleDebugReflection::Line) == 24);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mLines) == 8);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mOwner) == 752);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mCurrentLine) == 760);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mNumLines) == 768);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mLineOpen) == 776);
static_assert(offsetof(RndConsoleOverlay::ConsoleDebugReflection, mCritSec) == 784);
static_assert(sizeof(RndConsoleOverlay::ConsoleDebugReflection) == 800);
static_assert(offsetof(RndConsoleOverlay, mInput) == 64);
static_assert(offsetof(RndConsoleOverlay, mReflection) == 672);
static_assert(offsetof(RndConsoleOverlay, mPrintType) == 1472);
static_assert(sizeof(RndConsoleOverlay) == 1480);
