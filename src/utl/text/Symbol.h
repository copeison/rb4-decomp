#pragma once

// Interned engine string. The constructor implementation has not been
// reconstructed yet, but the executable stores each Symbol in one pointer.
class Symbol {
public:
    explicit Symbol(const char* str);

    const char* Str() const {
        return mStr;
    }

    bool operator==(const Symbol& other) const {
        return mStr == other.mStr;
    }
    bool operator!=(const Symbol& other) const {
        return mStr != other.mStr;
    }

private:
    const char* mStr;
};

static_assert(sizeof(Symbol) == sizeof(void*));
