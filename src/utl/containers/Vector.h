#pragma once

#include <new>
#include <utility>

#include "utl/containers/Std.h"

namespace eastl {

// The subset of EASTL's vector used by reconstructed code, with its 32-byte
// layout: begin, end, capacity, then the allocator. Growth doubles the
// current size (one when empty), as EASTL's GetNewCapacity does.
template <typename T, typename Allocator = HmxAllocator::allocator>
class vector {
public:
    vector()
        : mpBegin(nullptr),
          mpEnd(nullptr),
          mpCapacity(nullptr),
          mAllocator("EASTL vector") {}
    // Allocates exactly count elements and default-constructs them, as
    // EASTL's vector(size_type, const allocator_type&) does; FusionSampler's
    // constructor at 0x95C40 inlines it.
    explicit vector(unsigned long count)
        : mpBegin(nullptr),
          mpEnd(nullptr),
          mpCapacity(nullptr),
          mAllocator("EASTL vector") {
        if (count != 0) {
            mpBegin = static_cast<T*>(mAllocator.allocate(count * sizeof(T)));
        }
        mpEnd = mpBegin;
        mpCapacity = mpBegin + count;
        for (; mpEnd != mpCapacity; ++mpEnd) {
            new (mpEnd) T();
        }
    }
    // Copies the allocator, allocates exactly the other vector's size and
    // copy-constructs its elements, as EASTL's copy constructor does.
    vector(const vector& other)
        : mpBegin(nullptr),
          mpEnd(nullptr),
          mpCapacity(nullptr),
          mAllocator(other.mAllocator) {
        const auto count = other.size();
        if (count != 0) {
            mpBegin = static_cast<T*>(mAllocator.allocate(count * sizeof(T)));
        }
        mpEnd = mpBegin;
        mpCapacity = mpBegin + count;
        for (const auto* input = other.mpBegin; input != other.mpEnd; ++input, ++mpEnd) {
            new (mpEnd) T(*input);
        }
    }
    vector(vector&& other)
        : mpBegin(other.mpBegin),
          mpEnd(other.mpEnd),
          mpCapacity(other.mpCapacity),
          mAllocator(other.mAllocator) {
        other.mpBegin = other.mpEnd = other.mpCapacity = nullptr;
    }
    // Copies the other vector's elements, reallocating exactly its size
    // only when the capacity is too small, as EASTL's operator= does. Its
    // users inline it, for example AddGameWideEmitterName at 0x35450.
    vector& operator=(const vector& other) {
        if (this == &other) {
            return *this;
        }
        const auto count = other.size();
        if (count > capacity()) {
            auto* storage = static_cast<T*>(mAllocator.allocate(count * sizeof(T)));
            auto* output = storage;
            for (const auto* input = other.mpBegin; input != other.mpEnd; ++input, ++output) {
                new (output) T(*input);
            }
            DestroyElements();
            Free();
            mpBegin = storage;
            mpEnd = storage + count;
            mpCapacity = storage + count;
        } else if (count > size()) {
            const auto* input = other.mpBegin;
            for (auto* output = mpBegin; output != mpEnd; ++input, ++output) {
                *output = *input;
            }
            for (; input != other.mpEnd; ++input, ++mpEnd) {
                new (mpEnd) T(*input);
            }
        } else {
            auto* output = mpBegin;
            for (const auto* input = other.mpBegin; input != other.mpEnd; ++input, ++output) {
                *output = *input;
            }
            for (auto* element = output; element != mpEnd; ++element) {
                element->~T();
            }
            mpEnd = output;
        }
        return *this;
    }
    ~vector() {
        DestroyElements();
        Free();
    }

    unsigned long size() const {
        return static_cast<unsigned long>(mpEnd - mpBegin);
    }
    unsigned long capacity() const {
        return static_cast<unsigned long>(mpCapacity - mpBegin);
    }
    bool empty() const {
        return mpBegin == mpEnd;
    }

    T* begin() {
        return mpBegin;
    }
    T* end() {
        return mpEnd;
    }
    const T* begin() const {
        return mpBegin;
    }
    const T* end() const {
        return mpEnd;
    }
    T& front() {
        return *mpBegin;
    }
    const T& front() const {
        return *mpBegin;
    }
    T& back() {
        return *(mpEnd - 1);
    }
    const T& back() const {
        return *(mpEnd - 1);
    }
    void pop_back() {
        --mpEnd;
        mpEnd->~T();
    }
    T& operator[](unsigned long index) {
        return mpBegin[index];
    }
    const T& operator[](unsigned long index) const {
        return mpBegin[index];
    }

    // Relocates the elements by move-construction into new storage, as
    // EASTL's uninitialized_move does.
    void reserve(unsigned long count) {
        if (count <= capacity()) {
            return;
        }
        auto* storage =
            static_cast<T*>(mAllocator.allocate(count * sizeof(T)));
        auto* output = storage;
        for (auto* input = mpBegin; input != mpEnd; ++input, ++output) {
            new (output) T(std::move(*input));
        }
        DestroyElements();
        Free();
        mpBegin = storage;
        mpEnd = output;
        mpCapacity = storage + count;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (mpEnd == mpCapacity) {
            const auto count = size();
            reserve(count == 0 ? 1 : count * 2);
        }
        new (mpEnd) T(std::forward<Args>(args)...);
        return *mpEnd++;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    // Inserts a copy of the value before the position, moving the later
    // elements up, as EASTL's insert (DoInsertValue) does; the value may
    // live in the vector. RndLightProbeCom::StateInserted (0x4984C0) inlines
    // it.
    T* insert(T* position, const T& value) {
        const auto index = static_cast<unsigned long>(position - mpBegin);
        T copy(value);
        if (mpEnd == mpCapacity) {
            const auto count = size();
            reserve(count == 0 ? 1 : count * 2);
        }
        position = mpBegin + index;
        if (position == mpEnd) {
            new (mpEnd) T(std::move(copy));
        } else {
            new (mpEnd) T(std::move(*(mpEnd - 1)));
            for (auto* element = mpEnd - 1; element != position; --element) {
                *element = std::move(*(element - 1));
            }
            *position = std::move(copy);
        }
        ++mpEnd;
        return position;
    }

    void clear() {
        DestroyElements();
        mpEnd = mpBegin;
    }

    // Moves the later elements down over the range.
    T* erase(T* first, T* last) {
        auto* output = first;
        for (auto* input = last; input != mpEnd; ++input, ++output) {
            *output = std::move(*input);
        }
        for (auto* element = output; element != mpEnd; ++element) {
            element->~T();
        }
        mpEnd = output;
        return first;
    }
    T* erase(T* position) {
        return erase(position, position + 1);
    }

    // Grows by default-constructing new elements, or shrinks by destroying
    // trailing ones.
    void resize(unsigned long count) {
        const auto current = size();
        if (count <= current) {
            for (auto* element = mpBegin + count; element != mpEnd; ++element) {
                element->~T();
            }
            mpEnd = mpBegin + count;
            return;
        }
        if (count > capacity()) {
            const auto grown = current == 0 ? 1UL : current * 2;
            reserve(grown > count ? grown : count);
        }
        while (size() < count) {
            new (mpEnd) T();
            ++mpEnd;
        }
    }

    // Shrinking to zero releases the storage.
    void set_capacity(unsigned long count) {
        if (count == 0) {
            DestroyElements();
            Free();
            mpBegin = mpEnd = mpCapacity = nullptr;
            return;
        }
        reserve(count);
    }

    T* data() {
        return mpBegin;
    }

    T* mpBegin;
    T* mpEnd;
    T* mpCapacity;
    Allocator mAllocator;

private:
    void DestroyElements() {
        for (auto* element = mpBegin; element != mpEnd; ++element) {
            element->~T();
        }
    }

    void Free() {
        if (mpBegin != nullptr) {
            mAllocator.deallocate(
                mpBegin,
                static_cast<unsigned long>(
                    reinterpret_cast<char*>(mpCapacity) -
                    reinterpret_cast<char*>(mpBegin)));
        }
    }
};

}  // namespace eastl
