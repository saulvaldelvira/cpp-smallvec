#include "smallvec.hpp"
#include <iostream>

using sv::smallvec;

int main(){
        smallvec<int> vec {1, 2, 3, 4, 5};

        std::cout << sv::MAX_STACK_CAPACITY_FOR<int> << "\n";

        for (int const &n : vec) {
                std::cout << n << ", ";
        }
        std::cout << "\n";
}
