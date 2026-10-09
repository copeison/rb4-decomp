// The script console's line editor (0x11AECB0 to 0x11B0D20), emitted after
// JsonResource among code that is not in the reference map.
#include "render/debug/overlays/ConsoleLineEditor.h"

#include <cctype>
#include <cstring>
#include <new>

#include "os/joypads/Keyboard.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFile.h"
#include "utl/data/DataFunc.h"
#include "utl/data/DataNode.h"
#include "utl/data/DataUtl.h"

namespace {

// The key codes the editor handles. Names not in the reference map.
enum EditorKey : int {
    kKeyBackspace = 8,
    kKeyTab = 9,
    kKeyEnter = 10,
    kKeyDelete = 311,
    kKeyHome = 312,
    kKeyEnd = 313,
    kKeyLeft = 320,
    kKeyRight = 321,
    kKeyUp = 322,
    kKeyDown = 323,
};

// The history and completion sizes.
constexpr unsigned long kHistorySize = 50;
constexpr unsigned long kNumMatchStrings = 10;

// Letters, digits and underscores make up the words that the control
// jumps skip and tab completion completes.
bool IsWordChar(char c) {
    return c == '_' || static_cast<unsigned char>(c - '0') < 10 ||
           std::isalpha(static_cast<unsigned char>(c)) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x11AECB0. Each history string and each
// match string starts with room for 64 and 32 characters.
ConsoleLineEditor::ConsoleLineEditor(int flags)
    : mFlags(flags),
      mHistoryIndex(0),
      mNumMatches(0),
      mMatchIndex(static_cast<unsigned long>(-1)),
      mWordStart(static_cast<unsigned long>(-1)),
      mOutputType(-1) {
    mEditText.reserve(64);
    mCompletionText.reserve(64);
    mHistory.reserve(kHistorySize);
    mHistoryPool.mSize = kHistorySize;
    for (unsigned long i = 0; i < kHistorySize; ++i) {
        mHistoryPool[i] = new String();
        mHistoryPool[i]->reserve(64);
    }
    mHistory.push_back(mHistoryPool[0]);
    mMatches.resize(kNumMatchStrings);
    for (unsigned long i = 0; i < kNumMatchStrings; ++i) {
        mMatches[i].reserve(32);
    }
}

// Reconstructed from eboot.elf at 0x11AF060. The deleting destructor is at
// 0x11AF110.
ConsoleLineEditor::~ConsoleLineEditor() {}

// Reconstructed from eboot.elf at 0x11AF1C0. Keys with alt, or with control
// and a key it does not use, are not handled.
bool ConsoleLineEditor::HandleKeyboardMsg(const KeyboardKeyMsg& msg) {
    const int key = msg.GetKey();
    const bool ctrl = msg.mData->Int(4) != 0;
    const bool alt = msg.mData->Int(5) != 0;
    if (ctrl || alt) {
        if (!ctrl || alt) {
            return false;
        }
        if ((mFlags & kCursorKeys) != 0 && key == kKeyRight) {
            _SetCursor(_NextWordEnd());
        } else if ((mFlags & kCursorKeys) != 0 && key == kKeyLeft) {
            _SetCursor(_PrevWordStart());
        } else if ((key | 0x20) == 'l') {
            _ClearOutput();
            return true;
        } else if ((key | 0x20) == 'c') {
            _SetText("");
            _SetCursor(0);
        } else if (key == kKeyDown) {
            _HistoryNextMatch();
        } else if (key == kKeyUp) {
            _HistoryPrevMatch();
        } else {
            return false;
        }
        _TabCompletionClear();
        return true;
    }

    if ((mFlags & kEditKeys) != 0) {
        if (static_cast<unsigned int>(key - ' ') <= '~' - ' ') {
            const unsigned long cursor = _GetCursor();
            mEditText = _GetText();
            mEditText.insert(cursor, 1, static_cast<char>(key));
            _SetText(mEditText.c_str());
            _SetCursor(cursor + 1);
            _TabCompletionClear();
            return true;
        }
        if (key == kKeyDelete) {
            mEditText = _GetText();
            const unsigned long cursor = _GetCursor();
            if (cursor < std::strlen(mEditText.c_str())) {
                mEditText.erase(cursor, 1);
                _SetText(mEditText.c_str());
                _TabCompletionClear();
            }
            return true;
        }
        if (key == kKeyBackspace) {
            const unsigned long cursor = _GetCursor();
            if (cursor == 0) {
                return true;
            }
            mEditText = _GetText();
            mEditText.erase(cursor - 1, 1);
            _SetText(mEditText.c_str());
            _SetCursor(cursor - 1);
            _TabCompletionClear();
            return true;
        }
    }

    if ((mFlags & kCursorKeys) != 0) {
        switch (key) {
        case kKeyHome:
            _SetCursor(0);
            _TabCompletionClear();
            return true;
        case kKeyEnd:
            _SetCursor(std::strlen(_GetText()));
            _TabCompletionClear();
            return true;
        case 314:
        case 315:
        case 316:
        case 317:
        case 318:
        case 319:
            return false;
        case kKeyLeft: {
            const unsigned long cursor = _GetCursor();
            if (cursor != 0) {
                _SetCursor(cursor - 1);
                _TabCompletionClear();
            }
            return true;
        }
        case kKeyRight: {
            const unsigned long cursor = _GetCursor();
            if (cursor < std::strlen(_GetText())) {
                _SetCursor(cursor + 1);
                _TabCompletionClear();
            }
            return true;
        }
        default:
            break;
        }
    }
    if (key == kKeyUp) {
        _HistoryPrev();
        _TabCompletionClear();
        return true;
    }
    if (key == kKeyDown) {
        _HistoryNext();
        _TabCompletionClear();
        return true;
    }
    if (key == kKeyTab) {
        _TabCompletionStep();
        return true;
    }
    if (key != kKeyEnter) {
        return false;
    }
    _ExecuteCommand();
    _SetText("");
    _SetCursor(0);
    _TabCompletionClear();
    return true;
}

// Reconstructed from eboot.elf at 0x11AF710. The "Evaluates to" text is
// built but not shown.
void ConsoleLineEditor::_ExecuteCommand() {
    const String command(_GetText());
    const char* text = command.c_str();
    while (*text == '\t' || *text == ' ') {
        ++text;
    }
    if (*text == '\0') {
        return;
    }
    _HistoryAddCommand();
    _BeginCommand();
    mOutputType = 0;
    DataNode parsed;
    {
        const DataArrayPtr array = DataReadString(command.c_str());
        parsed = DataNode(array);
    }
    Entity* const entity = gDataThread.mDefaultEntity;
    mOutputType = 1;
    DataNode result;
    bool executed = false;
    {
        const DataArrayPtr array = parsed.Array(nullptr);
        if (array->Size() == 1) {
            const DataArrayPtr single = parsed.Array(nullptr);
            const DataType type = single->Node(0).Type();
            if (type == kDataCommand || type == kDataVar) {
                const DataArrayPtr evaluated = parsed.Array(nullptr);
                result = evaluated->Evaluate(0);
                executed = true;
            }
        }
    }
    if (!executed) {
        const DataArrayPtr array = parsed.Array(nullptr);
        result = DataExecute(array);
    }
    _FlushOutput();
    StackString<4096> message;
    message << "Evaluates to ";
    result.Print(message, false);
    DataSetDefaultEntity(entity);
    mOutputType = -1;
    _EndCommand();
}

// Reconstructed from eboot.elf at 0x11AFF10. A word after '$' completes
// only variables. With several matches the typed word becomes the last
// one, so that cycling returns to it.
void ConsoleLineEditor::_TabCompletionStep() {
    if (_GetCursor() == 0) {
        return;
    }
    const String text(_GetText());
    if (mNumMatches != 0 &&
        (mCompletionText != text ||
         _GetCursor() != std::strlen(mCompletionText.c_str()))) {
        _TabCompletionClear();
    }
    if (mNumMatches != 0) {
        mMatchIndex = (mMatchIndex + 1) % mNumMatches;
        mCompletionText = text.c_str();
        mCompletionText.erase(mWordStart);
        mCompletionText += mMatches[mMatchIndex].c_str();
        _SetText(mCompletionText.c_str());
        _SetCursor(std::strlen(mCompletionText.c_str()));
        return;
    }

    const unsigned long start = _PrevWordStart();
    mWordStart = start;
    const char* prefix = text.c_str() + start;
    unsigned long length = std::strlen(prefix);
    if (length == 0) {
        return;
    }
    if (start == 0 || text.c_str()[start - 1] != '$') {
        for (const auto& func : GetDataFuncs()) {
            if (std::strncmp(func.first.Str(), prefix, length) == 0) {
                _TabCompletionAddMatchString(func.first.Str());
            }
        }
    }
    DataForEachVariable([&prefix, &length, this](Symbol name) {
        if (std::strncmp(name.Str(), prefix, length) == 0) {
            _TabCompletionAddMatchString(name.Str());
        }
        return true;
    });
    if (mNumMatches == 0) {
        return;
    }
    if (mNumMatches != 1) {
        mMatchIndex = 0;
        _TabCompletionAddMatchString(prefix);
    }
    mCompletionText = text.c_str();
    mCompletionText.erase(mWordStart);
    mCompletionText += mMatches[0].c_str();
    _SetText(mCompletionText.c_str());
    _SetCursor(std::strlen(mCompletionText.c_str()));
    if (mNumMatches == 1) {
        _TabCompletionClear();
    }
}

// Inlined into HandleKeyboardMsg and _TabCompletionStep.
void ConsoleLineEditor::_TabCompletionClear() {
    mNumMatches = 0;
    mMatchIndex = static_cast<unsigned long>(-1);
    mWordStart = static_cast<unsigned long>(-1);
    mCompletionText.erase();
}

// Reconstructed from eboot.elf at 0x11B02B0.
void ConsoleLineEditor::_HistoryPrev() {
    if (mHistoryIndex == 0) {
        return;
    }
    _HistorySnapshotInput();
    const String* entry = mHistory[--mHistoryIndex];
    _SetText(entry->c_str());
    _SetCursor(std::strlen(entry->c_str()));
}

// Reconstructed from eboot.elf at 0x11B03A0.
void ConsoleLineEditor::_HistoryNext() {
    if (mHistoryIndex >= mHistory.size() - 1) {
        return;
    }
    const String* entry = mHistory[++mHistoryIndex];
    _SetText(entry->c_str());
    _SetCursor(std::strlen(entry->c_str()));
}

// Reconstructed from eboot.elf at 0x11B0420. Searches back from the
// character before the cursor, past any separators, to the start of the
// word.
unsigned long ConsoleLineEditor::_PrevWordStart() const {
    const String text(_GetText());
    long last = static_cast<long>(std::strlen(text.c_str())) - 1;
    const long cursor = static_cast<long>(_GetCursor()) - 1;
    if (last > cursor) {
        last = cursor;
    }
    bool inWord = false;
    for (long position = last; position >= 0; --position) {
        if (IsWordChar(text[static_cast<unsigned long>(position)])) {
            inWord = true;
        } else if (inWord) {
            return static_cast<unsigned long>(position + 1);
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x11B0510. Searches forward from the
// cursor, past the rest of the word and the separators after it, to the
// start of the next word; the last character when there is none.
unsigned long ConsoleLineEditor::_NextWordEnd() const {
    const String text(_GetText());
    const unsigned long last = std::strlen(text.c_str()) - 1;
    bool pastWord = false;
    for (unsigned long position = _GetCursor(); position < last; ++position) {
        if (!IsWordChar(text[position])) {
            pastWord = true;
        } else if (pastWord) {
            return position;
        }
    }
    return last;
}

// Reconstructed from eboot.elf at 0x11B0600. Finds the newest older entry
// that starts with the text before the cursor.
void ConsoleLineEditor::_HistoryPrevMatch() {
    const String text(_GetText());
    const unsigned long cursor = _GetCursor();
    for (unsigned long index = mHistoryIndex; index != 0;) {
        const String* entry = mHistory[--index];
        if (std::strncmp(text.c_str(), entry->c_str(), cursor) == 0) {
            _HistorySnapshotInput();
            mHistoryIndex = index;
            _SetText(entry->c_str());
            _SetCursor(cursor);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x11B0740.
void ConsoleLineEditor::_HistoryNextMatch() {
    const String text(_GetText());
    const unsigned long cursor = _GetCursor();
    for (unsigned long index = mHistoryIndex + 1; index < mHistory.size(); ++index) {
        const String* entry = mHistory[index];
        if (std::strncmp(text.c_str(), entry->c_str(), cursor) == 0) {
            _HistorySnapshotInput();
            mHistoryIndex = index;
            _SetText(entry->c_str());
            _SetCursor(cursor);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x11B08A0. A repeated command moves to
// the end instead of being added again.
void ConsoleLineEditor::_HistoryAddCommand() {
    const String text(_GetText());
    bool found = false;
    for (unsigned long i = 0; mHistory.size() != 1 && i < mHistory.size() - 1; ++i) {
        String* entry = mHistory[i];
        if (*entry == text) {
            mHistory.erase(mHistory.mpBegin + i);
            mHistory.insert(mHistory.mpEnd - 1, entry);
            found = true;
            break;
        }
    }
    if (!found) {
        *mHistory.back() = text.c_str();
        String* next;
        if (mHistory.size() == kHistorySize) {
            next = mHistory[0];
            mHistory.erase(mHistory.mpBegin);
        } else {
            next = mHistoryPool[mHistory.size()];
        }
        mHistory.push_back(next);
    }
    mHistoryIndex = mHistory.size() - 1;
}

// Reconstructed from eboot.elf at 0x11B0B20.
void ConsoleLineEditor::_HistorySnapshotInput() {
    if (mHistoryIndex == mHistory.size() - 1) {
        *mHistory.back() = _GetText();
    }
}

// Reconstructed from eboot.elf at 0x11B0BB0.
void ConsoleLineEditor::_TabCompletionAddMatchString(const char* str) {
    if (mNumMatches == mMatches.size()) {
        mMatches.push_back(String(str));
    } else {
        mMatches[mNumMatches] = str;
    }
    ++mNumMatches;
}
