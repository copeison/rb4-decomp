#pragma once

#include <new>

#include "utl/containers/Std.h"

namespace eastl {

// The subset of EASTL's deque used by reconstructed code, with its 88-byte
// layout: the pointer array and its size, the begin and end iterators, then
// the allocator. The elements live in fixed subarrays of
// kSubarraySize elements (EASTL's DEQUE_DEFAULT_SUBARRAY_SIZE); the pointer
// array starts with eight entries and the first subarray in its middle, as
// DequeBase::DoInit does. RBOvershellSlotCom's message queue inlines it
// (0xC05910, 0xC1C500, 0xC1D160), with DoReallocPtrArray emitted at
// 0xC24820.
template <typename T, typename Allocator = HmxAllocator::allocator>
class deque {
public:
    static constexpr unsigned long kSubarraySize =
        sizeof(T) <= 4 ? 64 : sizeof(T) <= 8 ? 32 : sizeof(T) <= 16 ? 16 : sizeof(T) <= 32 ? 8 : 4;
    static constexpr unsigned long kMinPtrArraySize = 8;

    // EASTL's DequeIterator: the element, its subarray's bounds and the
    // subarray's entry in the pointer array.
    struct iterator {
        T* mpCurrent;
        T* mpBegin;
        T* mpEnd;
        T** mpCurrentArrayPtr;

        void SetSubarray(T** arrayPtr) {
            mpCurrentArrayPtr = arrayPtr;
            mpBegin = *arrayPtr;
            mpEnd = mpBegin + kSubarraySize;
        }
    };

    deque() : mpPtrArray(nullptr), mnPtrArraySize(0), mItBegin(), mItEnd(), mAllocator("EASTL deque") {
        mnPtrArraySize = kMinPtrArraySize;
        mpPtrArray = static_cast<T**>(mAllocator.allocate(mnPtrArraySize * sizeof(T*)));
        T** const arrayBegin = mpPtrArray + (mnPtrArraySize - 1) / 2;
        *arrayBegin = static_cast<T*>(mAllocator.allocate(kSubarraySize * sizeof(T)));
        mItBegin.SetSubarray(arrayBegin);
        mItBegin.mpCurrent = mItBegin.mpBegin;
        mItEnd.SetSubarray(arrayBegin);
        mItEnd.mpCurrent = mItEnd.mpBegin;
    }
    deque(const deque&) = delete;
    deque& operator=(const deque&) = delete;
    ~deque() {
        for (iterator it = mItBegin; it.mpCurrent != mItEnd.mpCurrent;) {
            it.mpCurrent->~T();
            if (++it.mpCurrent == it.mpEnd) {
                it.SetSubarray(it.mpCurrentArrayPtr + 1);
                it.mpCurrent = it.mpBegin;
            }
        }
        if (mpPtrArray != nullptr) {
            for (T** array = mItBegin.mpCurrentArrayPtr; array < mItEnd.mpCurrentArrayPtr + 1; ++array) {
                if (*array != nullptr) {
                    mAllocator.deallocate(*array, kSubarraySize * sizeof(T));
                }
            }
            mAllocator.deallocate(mpPtrArray, mnPtrArraySize * sizeof(T*));
        }
    }

    bool empty() const {
        return mItBegin.mpCurrent == mItEnd.mpCurrent;
    }
    T& front() {
        return *mItBegin.mpCurrent;
    }

    void push_back(const T& value) {
        if (mItEnd.mpCurrent + 1 != mItEnd.mpEnd) {
            new (mItEnd.mpCurrent++) T(value);
            return;
        }
        if (static_cast<long>(mItEnd.mpCurrentArrayPtr - mpPtrArray) + 1 >= static_cast<long>(mnPtrArraySize)) {
            DoReallocPtrArray(1, kSideBack);
        }
        mItEnd.mpCurrentArrayPtr[1] = static_cast<T*>(mAllocator.allocate(kSubarraySize * sizeof(T)));
        new (mItEnd.mpCurrent) T(value);
        mItEnd.SetSubarray(mItEnd.mpCurrentArrayPtr + 1);
        mItEnd.mpCurrent = mItEnd.mpBegin;
    }

    void pop_front() {
        if (mItBegin.mpCurrent + 1 != mItBegin.mpEnd) {
            (mItBegin.mpCurrent++)->~T();
            return;
        }
        mItBegin.mpCurrent->~T();
        if (mItBegin.mpBegin != nullptr) {
            mAllocator.deallocate(mItBegin.mpBegin, kSubarraySize * sizeof(T));
        }
        mItBegin.SetSubarray(mItBegin.mpCurrentArrayPtr + 1);
        mItBegin.mpCurrent = mItBegin.mpBegin;
    }

    T** mpPtrArray;
    unsigned long mnPtrArraySize;
    iterator mItBegin;
    iterator mItEnd;
    Allocator mAllocator;

private:
    enum Side { kSideFront, kSideBack };

    // EASTL's DequeBase::DoReallocPtrArray: makes room for `additional`
    // subarray pointers on the side, recentering the used pointers when the
    // array is less than half full and otherwise growing it.
    void DoReallocPtrArray(unsigned long additional, Side side) {
        const unsigned long unusedFront = static_cast<unsigned long>(mItBegin.mpCurrentArrayPtr - mpPtrArray);
        const unsigned long usedPtrSpace = static_cast<unsigned long>(mItEnd.mpCurrentArrayPtr - mItBegin.mpCurrentArrayPtr) + 1;
        const unsigned long usedPtrSpaceBytes = usedPtrSpace * sizeof(T*);
        const unsigned long unusedBack = (mnPtrArraySize - unusedFront) - usedPtrSpace;
        T** newBegin;
        if (side == kSideBack && additional <= unusedFront) {
            if (additional < unusedFront / 2) {
                additional = unusedFront / 2;
            }
            newBegin = mpPtrArray + (unusedFront - additional);
            __builtin_memmove(newBegin, mItBegin.mpCurrentArrayPtr, usedPtrSpaceBytes);
        } else if (side == kSideFront && additional <= unusedBack) {
            if (additional < unusedBack / 2) {
                additional = unusedBack / 2;
            }
            newBegin = mItBegin.mpCurrentArrayPtr + additional;
            __builtin_memmove(newBegin, mItBegin.mpCurrentArrayPtr, usedPtrSpaceBytes);
        } else {
            const unsigned long newSize = mnPtrArraySize + (mnPtrArraySize > additional ? mnPtrArraySize : additional) + 2;
            T** const newArray = static_cast<T**>(mAllocator.allocate(newSize * sizeof(T*)));
            newBegin = newArray + (unusedFront + (side == kSideFront ? additional : 0));
            if (mpPtrArray != nullptr) {
                __builtin_memcpy(newBegin, mItBegin.mpCurrentArrayPtr, usedPtrSpaceBytes);
                mAllocator.deallocate(mpPtrArray, mnPtrArraySize * sizeof(T*));
            }
            mpPtrArray = newArray;
            mnPtrArraySize = newSize;
        }
        mItBegin.mpCurrentArrayPtr = newBegin;
        mItBegin.mpBegin = *newBegin;
        mItBegin.mpEnd = mItBegin.mpBegin + kSubarraySize;
        T** const newEnd = newBegin + usedPtrSpace - 1;
        mItEnd.mpCurrentArrayPtr = newEnd;
        mItEnd.mpBegin = *newEnd;
        mItEnd.mpEnd = mItEnd.mpBegin + kSubarraySize;
    }
};

}  // namespace eastl
