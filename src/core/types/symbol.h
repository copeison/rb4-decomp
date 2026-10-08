#pragma once

#include <cstddef>

namespace rb4 {

// Interned engine symbol. The constructor implementation has not been
// reconstructed yet, but the executable stores each Symbol in one pointer.
class Symbol {
public:
    explicit Symbol(const char* text);

    const void* value() const {
        return value_;
    }

private:
    const void* value_;
};

static_assert(sizeof(Symbol) == sizeof(void*));

}  // namespace rb4
