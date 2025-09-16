#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include "iterator.hpp"

namespace sv {

struct TaggedLen {
private:
        size_t __len;
public:
        TaggedLen(): __len(0) {}

        auto operator=(auto n) = delete;

        size_t get() const noexcept {
                return __len >> 1;
        }

        size_t operator*() const { return get(); }

        void set_heap() {
                __len |= 0b1;
        }

        void inc() {
                __len += 0b10;
        }

        void set(size_t n) {
                __len &= 0b1;
                __len |= n << 1;
        }

        void dec() {
                size_t l = __len >> 1;
                l -= 1;
                __len &= 0b1;
                __len |= l << 1;
        }

        bool operator<=(size_t n) const {
                return this->get() <= n;
        }

        bool operator>=(size_t n) const {
                return this->get() >= n;
        }

        bool operator<(size_t n) const {
                return this->get() < n;
        }

        bool operator>(size_t n) const {
                return this->get() > n;
        }

        bool is_stack() const noexcept {
                return (__len & 0b1) == 0;
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
        /* We need to use malloc and free, since we don't want to initialize
         * any elements beforehand -which `new` always does.
         * We want to be able to use vectors of types without a default
         * constructor. */
        std::unique_ptr<T, MallocDeleter> elems;
        size_t cap;
};

template <typename T>
const size_t MAX_STACK_CAPACITY_FOR = sizeof(RawVec<T>) / sizeof(T);

template <
        typename T,
        size_t N
> class smallvec;

template<
        typename T,
        size_t N = MAX_STACK_CAPACITY_FOR<T>
>
class smallvec {
private:

        TaggedLen len;
        union _annon {
                RawVec<T> heap;
                T inlined[N];

                _annon() {}
                ~_annon() {}
        } u;

        inline T* ptr(size_t index = 0) noexcept {
                return len.is_stack() ? &u.inlined[index] : &(u.heap.elems.get()[index]);
        }

        inline const T* ptr(size_t index = 0) const noexcept {
                return len.is_stack() ? &u.inlined[index] : &(u.heap.elems.get()[index]);
        }

        inline void __switch_heap(size_t _n) {
                T *old = ptr();
                T *_new_elems = static_cast<T*>(std::malloc(_n * sizeof(T)));
                std::move(old, &old[len.get()], _new_elems);
                len.set_heap();

                u.heap.elems.release();
                u.heap.elems.reset(_new_elems);
                u.heap.cap = _n;
        }

        inline void __grow(size_t _n) {
                if (len.is_stack()) {
                        __switch_heap(_n);
                } else {
                        T *_new_elems = static_cast<T*>(std::malloc(_n * sizeof(T)));
                        if (u.heap.elems) {
                                std::move(u.heap.elems.get(), &(u.heap.elems.get())[len.get()], _new_elems);
                        }

                        u.heap.elems.reset(_new_elems);
                        u.heap.cap = _n;
                }
        }

public:

        static const inline size_t STACK_CAP = N;

        using iterator = sv::iterator<T>;
        using iterator_const = sv::iterator<const T>;

        smallvec() noexcept : len() {}

        ~smallvec() {
                for (T &e : *this) {
                        e.~T();
                }
                if (!lives_on_stack()) {
                        u.heap.elems.reset();
                }
        }

        smallvec(smallvec<T, N> &&other) noexcept {
                len = other.len;
                u = other.u;
                other.len = TaggedLen();
        }

        smallvec(::std::initializer_list<T> l) : smallvec() {
                __grow(l.size());
                for (auto it = l.begin(); it != l.end(); it++){
                        *ptr(*len) = *it;
                        len.inc();
                }
        }

        [[gnu::always_inline]]
        inline void push_back(T elem) {
                reserve(1);
                *ptr(*len) = std::move(elem);
                len.inc();
        }

        template<class... Args>
        void emplace_back(Args&&... args) {
                push_back(std::move(T(args...)));
        }

        inline void push_front(T elem) {
                reserve(1);
                T *elems = ptr();
                if (len.get() > 0) {
                        std::move_backward(elems, &elems[len.get()], &elems[len.get() + 1]);
                }
                elems[0] = std::move(elem);
                len.inc();
        }

        template<class... Args>
        void emplace_front(Args&&... args) {
                push_front(std::move(T(args...)));
        }

        void pop_back() {
                if (len <= 0)
                        return;
                len.dec();
        }

        void pop_front() {
                if (len <= 0)
                        return;
                T *elems = ptr();
                T ret = elems[0];
                std::move_backward(&elems[1], &elems[len.get()], elems);
                len.dec();
        }

        void reserve(size_t n) {
                if (len.get() + n > capacity())
                        __grow((len.get() + n) * 2);
        }

        void reserve_exact(size_t n) {
                if (len.get() + n > capacity())
                        __grow(len.get() + n);
        }

        void resize(size_t n, const T& value = T()) {
                reserve(n);
                T *elems = ptr();
                size_t cap = capacity();
                for (size_t i = len.get(); i < cap; i++)
                        elems[i] = value;
                len.set(n);
        }

        inline void shrink_to_fit() {
                __grow(len.get());
        }

        const T& operator [] (size_t i) const noexcept {
                return ptr(i);
        }

        T& operator [] (size_t i) noexcept {
                return ptr(i);
        }

        iterator begin() {
                return iterator(ptr(), 0);
        }

        iterator end() {
                return iterator(ptr(), len.get());
        }

        iterator_const begin() const {
                return iterator_const(ptr(), 0);
        }

        iterator_const end() const {
                return iterator_const(ptr(), len.get());
        }

        template <typename F>
        requires std::invocable<F, T&>
        inline void for_each(F f) {
                for (T &elem : *this) {
                        f(elem);
                }
        }

        template <typename F>
        requires std::invocable<F, const T&>
        inline void for_each(F f) const {
                for (const T &elem : *this) {
                        f(elem);
                }
        }

        inline size_t size() const noexcept { return len.get(); }

        size_t capacity() const noexcept {
                return len.get() <= N ? N : u.heap.cap;
        }

        inline bool lives_on_stack() const noexcept {
                return len.is_stack();
        }

        inline T* get_buffer() noexcept {
                return ptr();
        }

        inline const T* get_buffer() const noexcept {
                return ptr();
        }
};

}
