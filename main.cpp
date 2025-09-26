#include "smallvec.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sys/cdefs.h>

using sv::smallvec;

template<typename T>
consteval T force_consteval(T val) {
        return val;
}

constexpr sv::smallvec<int> const_cap() {
        sv::smallvec<int> sv { 1, 2 };

        sv.push_back(1);
        sv.push_back(2);

        return sv;
}

int main(){

        constexpr auto sv = const_cap();

        for (auto i: sv) {

        }

        sv.for_each([](auto ii) {

        });


        std::cout << sv.capacity() << "\n";
}
