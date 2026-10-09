#pragma once

// Interned engine string. The constructor implementation has not been
// reconstructed yet, but the executable stores each Symbol in one pointer.
class Symbol {
public:
    // The empty symbol. Its string is the empty literal, which the old map
    // names gNullStr.
    Symbol() : mStr("") {}
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
    // Orders symbols by their interned addresses, as the engine's
    // eastl::map<Symbol, ...> lookups compare them (for example
    // AudioRenderTargetRegistry::Find at 0xC14B0).
    bool operator<(const Symbol& other) const {
        return mStr < other.mStr;
    }

private:
    const char* mStr;
};

static_assert(sizeof(Symbol) == sizeof(void*));
