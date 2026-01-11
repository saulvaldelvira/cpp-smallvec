#include "smallvec.hpp"
#include <iostream>
#include <print>
#include <sys/cdefs.h>
#include <type_traits>

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

        struct MyS {
                int a;

                MyS(): a(0) {};
                MyS(int a): a(a) {};

                MyS(const MyS &o): a(o.a) {
                        std::println("{} MyS(const MyS &s)", a);
                }
                MyS(MyS &&o): a(o.a) {
                        o.a = {};
                        std::println("{} MyS(MyS &&)", a);
                }
                MyS& operator=(const MyS &o) {
                        a = o.a;
                        std::println("{} MyS& operator=(const MyS &)", a);
                        return *this;
                }
                MyS& operator=(MyS &&o) {
                        a = o.a;
                        o.a = {};
                        std::println("{} MyS& operator=(MyS &&)", a);
                        return *this;
                }
                ~MyS() { std::println("{} ~MyS()", a); }
        };

        sv::smallvec<MyS> sv;
        // static_assert(std::is_trivially_destructible_v<MyS>, "NO");

        sv.resize(5, MyS{1});
        sv.resize(10, [](){
                std::println("Lambda");
                return MyS{1};
        });

        std::println("Hey");

        // Can't use constexpr on non-default constructible types :(
        // constexpr smallvec<Destro> cant { 1, 2, 3};
}
