#include <iostream>

#include "Test.h"
#include "STLExt.h"

using namespace example;

struct Functor {
  int operator()(int n) {
    std::cout << "n:" << n << std::endl;
    return n;
  }
};

TEST(FunctorTest, object) {
  MyFunction<int(int)> myFunc = Functor();
  myFunc(0);
}