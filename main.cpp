#include "smallvec.hpp"
#include <iostream>
#include <sys/cdefs.h>

using sv::smallvec;

constexpr int constexpr_example() {
        sv::smallvec<int, 5> sv {1, 2, 3};

        sv.push_back(4);
        sv.push_back(5);

        return sv.size();
}


int main(){
        constexpr int length_5 = constexpr_example();
        static_assert(length_5 == 5);

        struct NoDefConstructor {
                NoDefConstructor() = delete;
        };

        // Can't use constexpr on non-default constructible types :(
        // constexpr smallvec<Destro> cant { 1, 2, 3};
}
