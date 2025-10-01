#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <type_traits>

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

struct TaggedLen {
private:
        size_t _len;
public:
        constexpr TaggedLen(): _len(0) {}
        constexpr TaggedLen(TaggedLen &&o) {
                _len = o._len;
                o._len = 0;
        }

        constexpr
        TaggedLen& operator = (TaggedLen &&o) {
                _len = o._len;
                o._len = 0;
                return *this;
        }

        auto operator=(auto n) = delete;

        constexpr size_t get() const noexcept {
                return _len >> 1;
        }

        constexpr size_t operator*() const { return get(); }

        void set_heap() {
                _len |= 0b1;
        }

        constexpr void inc() {
                _len += 0b10;
        }

        constexpr void set(size_t n) {
                _len &= 0b1;
                _len |= n << 1;
        }

        constexpr void dec() {
                size_t l = _len >> 1;
                l -= 1;
                _len &= 0b1;
                _len |= l << 1;
        }

        constexpr bool operator<=(size_t n) const {
                return this->get() <= n;
        }

        constexpr bool operator>=(size_t n) const {
                return this->get() >= n;
        }

        constexpr bool operator<(size_t n) const {
                return this->get() < n;
        }

        constexpr bool operator>(size_t n) const {
                return this->get() > n;
        }

        constexpr bool is_stack() const noexcept {
                return (_len & 0b1) == 0;
        }
};

template<typename T>
struct RawVec {
private:
        struct MallocDeleter {
                void operator()(T *ptr) {
                        std::free(ptr);
                }
        };
public:
        /* We want to be able to use vectors of types without a default
         * constructor.
         * This forces us to use malloc and free, since we don't want to
         * initialize the elements beforehand -which `new` always does.
         */
        std::unique_ptr<T, MallocDeleter> elems;
        size_t cap;

        constexpr RawVec(): elems(nullptr), cap(0) {}
};

template <typename T>
const size_t MAX_STACK_CAPACITY_FOR = sizeof(RawVec<T>) / sizeof(T);

/**
 *  A dynamic array that allows to store a fixed number of elements on the stack
 *  Just like std::string, it has a stack-based array for the first N elements; and
 *  when the length grows over that initial capacity, switches to a heap-based buffer.
 */
template <
        typename T,
        size_t N
> class smallvec;

template<
        typename T,
        size_t N,
        bool is_def_const = std::is_default_constructible_v<T>
>
union storage;

template<typename T, size_t N>
union storage<T, N, true> {
        RawVec<T> heap;
        T inlined[N];

        constexpr
        T* inlined_ptr() { return &inlined[0]; }

        constexpr
        const T* inlined_ptr() const { return &inlined[0]; }

        constexpr storage(): inlined() {}
        constexpr ~storage() {}
};

template<typename T, size_t N>
union storage<T, N, false> {
        RawVec<T> heap;
        /* If T can't be default constructed, we can't declare an array of T */
        alignas(T) char inlined[N * sizeof(T)];

        /* This means that we won't be able to use the vector in constexpr code,
         * since casting from char* to T* is not constexpr-safe :( */

        constexpr
        T* inlined_ptr() { return reinterpret_cast<T*>(&inlined[0]); }

        constexpr
        const T* inlined_ptr() const { return reinterpret_cast<T*>(&inlined[0]); }

        constexpr storage(): inlined() {}
        constexpr ~storage() {}
};


template<
        typename T,
        size_t N = MAX_STACK_CAPACITY_FOR<T>
>
class smallvec {
private:

        TaggedLen len;
        storage<T, N> store;

        static constexpr inline
        auto ptr_impl(auto *self, size_t index) noexcept {
                return self->len.is_stack() ?
                       &self->store.inlined_ptr()[index]:
                       &(self->store.heap.elems.get()[index]);
        }

        constexpr inline T* ptr(size_t index = 0) noexcept {
                return ptr_impl(this, index);
        }

        constexpr inline const T* ptr(size_t index = 0) const noexcept {
                return ptr_impl(this, index);
        }

        inline void __switch_heap(size_t _n) {
                T *old = ptr();
                T *_new_elems = static_cast<T*>(std::malloc(_n * sizeof(T)));
                std::move(old, &old[len.get()], _new_elems);
                len.set_heap();

                store.heap.elems.release();
                store.heap.elems.reset(_new_elems);
                store.heap.cap = _n;
        }

        inline void __grow(size_t _n) {
                if (len.is_stack()) {
                        __switch_heap(_n);
                } else {
                        T *_new_elems = static_cast<T*>(std::malloc(_n * sizeof(T)));
                        if (store.heap.elems) {
                                std::move(store.heap.elems.get(), &(store.heap.elems.get())[len.get()], _new_elems);
                        }

                        store.heap.elems.reset(_new_elems);
                        store.heap.cap = _n;
                }
        }

public:

        static const inline size_t STACK_CAP = N;

        using iterator = sv::iterator<T>;
        using iterator_const = sv::iterator<const T>;

        constexpr smallvec() noexcept : len(), store() {}

        constexpr ~smallvec() {
                if (len.get() > 0) {
                        for (T &e : *this) {
                                e.~T();
                        }
                }
                if (!lives_on_stack()) {
                        store.heap.elems.reset();
                }
        }

        constexpr smallvec(smallvec<T, N> &&other) noexcept : len(), store() {
                len = std::move(other.len);
                if (other.lives_on_stack()) {
                        std::move(
                                &other.store.inlined_ptr()[0],
                                &other.store.inlined_ptr()[other.size()],
                                &store.inlined_ptr()[0]
                        );
                }
                else
                        store.heap = std::move(other.store.heap);

                other.len = TaggedLen();
        }

        constexpr smallvec(::std::initializer_list<T> l) : smallvec() {
                reserve_exact(l.size());
                std::move(l.begin(), l.end(), ptr());
                len.set(l.size());
        }

        /**
         * Pushes the given element to the back of the vector.
         */
        constexpr inline void push_back(T elem) {
                reserve(1);
                *ptr(*len) = std::move(elem);
                len.inc();
        }

        template<class... Args>
        constexpr inline void emplace_back(Args&&... args) {
                push_back(std::move(T(args...)));
        }

        /**
         * Pushes the given element to the start of the vector.
         */
        constexpr inline void push_front(T elem) {
                reserve(1);
                T *elems = ptr();
                if (len.get() > 0) {
                        std::move_backward(
                                elems,
                                ptr(size()),
                                ptr(size() + 1)
                        );
                }
                elems[0] = std::move(elem);
                len.inc();
        }

        template<class... Args>
        constexpr inline void emplace_front(Args&&... args) {
                push_front(std::move(T(args...)));
        }

        constexpr std::optional<T> remove(size_t index) {
                std::optional<T> ret;
                if (index <= size()) {
                        ret = std::move(*ptr(index));
                        std::move(ptr(index + 1), ptr(size()), ptr(index));
                        len.dec();
                }
                return ret;
        }

        constexpr void pop_back() {
                if (len <= 0)
                        return;
                remove(len.get() - 1);
        }

        constexpr inline void pop_front() {
                remove(0);
        }

        constexpr void reserve(size_t n) {
                if (len.get() + n > capacity())
                        __grow((len.get() + n) * 2);
        }

        constexpr void reserve_exact(size_t n) {
                if (len.get() + n > capacity())
                        __grow(len.get() + n);
        }

        constexpr void resize(size_t n, const T& value)
        requires
                std::is_copy_constructible_v<T>
        {
                reserve(n);
                T *elems = ptr();
                size_t cap = capacity();
                for (size_t i = len.get(); i < cap; i++)
                        elems[i] = value;
                len.set(n);
        }

        constexpr void resize(size_t n)
        requires
                std::is_default_constructible_v<T>
        {
                resize(n, T());
        }

        inline void shrink_to_fit() {
                __grow(len.get());
        }

        constexpr
        const T& operator [] (size_t i) const noexcept {
                return *ptr(i);
        }

        constexpr T& operator [] (size_t i) noexcept {
                return *ptr(i);
        }

        constexpr iterator begin() {
                return iterator(ptr(), 0);
        }

        constexpr iterator end() {
                return iterator(ptr(), len.get());
        }

        constexpr iterator_const begin() const {
                return iterator_const(ptr(), 0);
        }

        constexpr iterator_const end() const {
                return iterator_const(ptr(), len.get());
        }

        template <typename F>
        requires std::invocable<F, T&>
        constexpr inline void for_each(F f) {
                for (T &elem : *this) {
                        f(elem);
                }
        }

        template <typename F>
        requires std::invocable<F, const T&>
        constexpr inline void for_each(F f) const {
                for (const T &elem : *this) {
                        f(elem);
                }
        }

        constexpr inline size_t size() const noexcept { return len.get(); }

        constexpr inline bool is_empty() const noexcept { return len.get() == 0; }

        constexpr size_t capacity() const noexcept {
                return len.get() <= N ? N : store.heap.cap;
        }

        constexpr inline
        bool lives_on_stack() const noexcept {
                return len.is_stack();
        }

        constexpr inline T* get_buffer(size_t i = 0) noexcept {
                return ptr(i);
        }

        constexpr inline const T* get_buffer(size_t i = 0) const noexcept {
                return ptr(i);
        }
};

}
