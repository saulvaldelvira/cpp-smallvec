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
        explicit iterator(T* _ref, size_t _num = 0) : ref(_ref), num(_num) {}

        iterator& operator++() {
            num++;
            return *this;
        }
        iterator operator++(int) {
            iterator retval = *this;
            ++(*this);
            return retval;
        }
        iterator& operator--() {
            num--;
            return *this;
        }
        iterator operator--(int) {
            iterator retval = *this;
            --(*this);
            return retval;
        }

        iterator operator+(int) {
                iterator retval = *this;
                ++retval;
                return retval;
        }

        bool operator==(iterator other) const { return num == other.num; }
        bool operator!=(iterator other) const { return !(*this == other); }

        reference operator*() const { return ref[num]; }

        pointer operator->() { return &ref[num]; }
    };
}
