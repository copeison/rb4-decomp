#pragma once

namespace eastl {

// EASTL's introsort, in the subset reconstructed code uses: quick-sort
// partitions down to kQuickSortLimit elements or the recursion limit (a
// heap sort then takes over), followed by an insertion sort. Each user
// instantiates its own copy, for example MemHeap::ProcessBatchedFreeList
// (quick_sort_impl at 0x37ECA0).
namespace Internal {

constexpr long kQuickSortLimit = 28;

// The floor of the base-2 logarithm.
inline long Log2(long n) {
    long result = -1;
    for (; n != 0; n >>= 1) {
        ++result;
    }
    return result;
}

template <typename T, typename Compare>
const T& median(const T& a, const T& b, const T& c, Compare compare) {
    if (compare(a, b)) {
        if (compare(b, c)) {
            return b;
        }
        if (compare(a, c)) {
            return c;
        }
        return a;
    }
    if (compare(a, c)) {
        return a;
    }
    if (compare(b, c)) {
        return c;
    }
    return b;
}

template <typename T, typename Compare>
T* get_partition(T* first, T* last, T pivot, Compare compare) {
    for (;; ++first) {
        while (compare(*first, pivot)) {
            ++first;
        }
        --last;
        while (compare(pivot, *last)) {
            --last;
        }
        if (first >= last) {
            return first;
        }
        T temp = *first;
        *first = *last;
        *last = temp;
    }
}

template <typename T, typename Compare>
void adjust_heap(T* first, long top, long size, long position, T value, Compare compare) {
    long child = (2 * position) + 2;
    for (; child < size; child = (2 * child) + 2) {
        if (compare(first[child], first[child - 1])) {
            --child;
        }
        first[position] = first[child];
        position = child;
    }
    if (child == size) {
        first[position] = first[child - 1];
        position = child - 1;
    }
    for (long parent = (position - 1) >> 1; position > top && compare(first[parent], value);
         parent = (position - 1) >> 1) {
        first[position] = first[parent];
        position = parent;
    }
    first[position] = value;
}

// Heap-sorts [first, last); EASTL's partial_sort(first, last, last).
template <typename T, typename Compare>
void partial_sort(T* first, T* last, Compare compare) {
    const long size = last - first;
    if (size >= 2) {
        for (long parent = (size - 2) >> 1;; --parent) {
            adjust_heap(first, parent, size, parent, first[parent], compare);
            if (parent == 0) {
                break;
            }
        }
    }
    for (T* end = last; end - first > 1;) {
        --end;
        T value = *end;
        *end = *first;
        adjust_heap(first, 0L, end - first, 0L, value, compare);
    }
}

template <typename T, typename Compare>
void quick_sort_impl(T* first, T* last, long recursionCount, Compare compare) {
    while ((last - first) > kQuickSortLimit && recursionCount > 0) {
        T* position = get_partition(first, last,
                                    median(*first, *(first + (last - first) / 2), *(last - 1), compare),
                                    compare);
        quick_sort_impl(position, last, --recursionCount, compare);
        last = position;
    }
    if (recursionCount == 0) {
        partial_sort(first, last, compare);
    }
}

template <typename T, typename Compare>
void insertion_sort(T* first, T* last, Compare compare) {
    if (first == last) {
        return;
    }
    for (T* i = first + 1; i != last; ++i) {
        T value = *i;
        T* hole = i;
        while (hole != first && compare(value, *(hole - 1))) {
            *hole = *(hole - 1);
            --hole;
        }
        *hole = value;
    }
}

// The insertion sort for the elements after the first kQuickSortLimit,
// which are known to have a smaller element before them.
template <typename T, typename Compare>
void insertion_sort_simple(T* first, T* last, Compare compare) {
    for (T* i = first; i != last; ++i) {
        T value = *i;
        T* hole = i;
        while (compare(value, *(hole - 1))) {
            *hole = *(hole - 1);
            --hole;
        }
        *hole = value;
    }
}

}  // namespace Internal

template <typename T, typename Compare>
void sort(T* first, T* last, Compare compare) {
    if (first != last) {
        Internal::quick_sort_impl(first, last, 2 * Internal::Log2(last - first), compare);
        if ((last - first) > Internal::kQuickSortLimit) {
            Internal::insertion_sort(first, first + Internal::kQuickSortLimit, compare);
            Internal::insertion_sort_simple(first + Internal::kQuickSortLimit, last, compare);
        } else {
            Internal::insertion_sort(first, last, compare);
        }
    }
}

}  // namespace eastl
