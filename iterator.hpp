#pragma once

#include <iterator>

namespace sv {

    template<typename T>
    class iterator {
        using iterator_category = ::std::random_access_iterator_tag;
        using value_type = size_t;
        using difference_type = size_t;
        using pointer = T*;
        using reference = T&;
        long num = 0;
        T* ref;

    public:
        explicit constexpr iterator(T* _ref, size_t _num = 0) : num(_num), ref(_ref) {}

        constexpr iterator& operator++() {
            num++;
            return *this;
        }
        constexpr iterator operator++(int) {
            iterator retval = *this;
            ++(*this);
            return retval;
        }
        constexpr iterator& operator--() {
            num--;
            return *this;
        }
        constexpr iterator operator--(int) {
            iterator retval = *this;
            --(*this);
            return retval;
        }

        constexpr iterator operator+(int) {
                iterator retval = *this;
                ++retval;
                return retval;
        }

        constexpr bool operator==(iterator other) const { return num == other.num; }
        constexpr bool operator!=(iterator other) const { return !(*this == other); }

        constexpr reference operator*() const { return ref[num]; }

        constexpr pointer operator->() { return &ref[num]; }
    };
}
