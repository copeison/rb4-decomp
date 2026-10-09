#pragma once

#include <cstddef>

#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"

class KeyboardKeyMsg;

// A script command line: keyboard editing, a 50-entry command history and
// tab completion of script functions and variables (0x11AECB0-0x11B0D20,
// vtable 0x19A5818). The map's build had these as RndConsoleOverlay's
// _History*, _TabCompletion* and _ExecuteCommand members; this build moves
// them into a base class that RndConsoleOverlay::ConsoleInput specialises,
// emitted among code that is not in the map. Name not in the reference map;
// the method names follow the map's console members where they match.
class ConsoleLineEditor {
public:
    // The keys the editor handles; the console sets both. Names not in the
    // reference map.
    enum Flags {
        // Typing, backspace and delete.
        kEditKeys = 1,
        // Home, end, left and right, and the word jumps with control.
        kCursorKeys = 2,
    };

    explicit ConsoleLineEditor(int flags);  // 0x11AECB0
    // Slots 0-1: 0x11AF060, 0x11AF110. The history strings are not freed.
    virtual ~ConsoleLineEditor();

    // Slots 2-5: the input line and the cursor, which the subclass stores.
    // Names not in the reference map.
    virtual const char* _GetText() const = 0;
    virtual void _SetText(const char* text) = 0;
    virtual unsigned long _GetCursor() const = 0;
    virtual void _SetCursor(unsigned long cursor) = 0;
    // Slots 6-9, empty here: around a command's execution, clearing the
    // output (control+L), and waiting for the output after the command.
    // Names not in the reference map.
    virtual void _BeginCommand() {}  // 0x11B0CD0
    virtual void _EndCommand() {}    // 0x11B0CE0
    virtual void _ClearOutput() {}   // 0x11B0CF0
    virtual void _FlushOutput() {}   // 0x11B0D00

    // Edits the line for a key. Without modifiers: printable characters,
    // backspace, delete, home, end, left, right, up and down (history),
    // tab (completion) and enter (execute). With control: left and right
    // jump words, up and down search the history for the text before the
    // cursor, L clears the output and C clears the line.
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg);  // 0x11AF1C0

    // Runs the line as a script: a single command or variable is
    // evaluated, anything else executed, with the default entity restored
    // afterwards. The line joins the history.
    void _ExecuteCommand();  // 0x11AF710
    // Completes the word before the cursor; repeated tabs cycle through the
    // matches and back to the typed text.
    void _TabCompletionStep();  // 0x11AFF10
    // Inlined into its callers.
    void _TabCompletionClear();
    // Stores a match, reusing the strings of earlier completions.
    void _TabCompletionAddMatchString(const char* str);  // 0x11B0BB0
    void _HistoryPrev();       // 0x11B02B0
    void _HistoryNext();       // 0x11B03A0
    void _HistoryPrevMatch();  // 0x11B0600
    void _HistoryNextMatch();  // 0x11B0740
    // Moves the line to the end of the history, dropping the oldest entry
    // when it is full, and points the history at the new empty entry.
    void _HistoryAddCommand();  // 0x11B08A0
    // Keeps the edited line in the last history entry while browsing.
    void _HistorySnapshotInput();  // 0x11B0B20
    // The start of the word before the cursor, and the end of the word
    // after it. Names not in the reference map.
    unsigned long _PrevWordStart() const;  // 0x11B0420
    unsigned long _NextWordEnd() const;    // 0x11B0510

    // Field names are not in the reference map.
    int mFlags;
    // The line being edited, as a copy of _GetText.
    String mEditText;
    // The history, oldest first; the last entry is the line being typed.
    eastl::vector<String*> mHistory;
    // The 50 history strings, created once.
    FixedVector<String*, 50> mHistoryPool;
    // The history entry shown.
    unsigned long mHistoryIndex;
    // The completion matches; their strings are reused.
    eastl::vector<String> mMatches;
    unsigned long mNumMatches;
    unsigned long mMatchIndex;
    // Where the completed word starts in the line.
    unsigned long mWordStart;
    // The line with the current match, to see whether the user typed since.
    String mCompletionText;
    // What the executing command prints: -1 outside a command, 0 while it
    // is parsed and 1 while it runs.
    int mOutputType;
};

static_assert(offsetof(ConsoleLineEditor, mFlags) == 8);
static_assert(offsetof(ConsoleLineEditor, mEditText) == 16);
static_assert(offsetof(ConsoleLineEditor, mHistory) == 32);
static_assert(offsetof(ConsoleLineEditor, mHistoryPool) == 64);
static_assert(offsetof(ConsoleLineEditor, mHistoryIndex) == 488);
static_assert(offsetof(ConsoleLineEditor, mMatches) == 496);
static_assert(offsetof(ConsoleLineEditor, mNumMatches) == 528);
static_assert(offsetof(ConsoleLineEditor, mWordStart) == 544);
static_assert(offsetof(ConsoleLineEditor, mCompletionText) == 552);
static_assert(offsetof(ConsoleLineEditor, mOutputType) == 568);
static_assert(sizeof(ConsoleLineEditor) == 576);
