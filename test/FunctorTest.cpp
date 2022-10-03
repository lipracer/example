#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <numeric>
#include <cassert>
#include <vector>
#include <bitset>
#include <limits>

#include "Test.h"
#include "STLExt.h"
#include "Tensor.hpp"

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

struct A {
  A(){}
  A(const A& a) {
    std::cout << "copy constructor\n";
  }
  A(A&& a) {
    std::cout << "move constructor\n";
  }
  A &operator=(A &&a) {
    std::cout << "assign constructor\n";
    return *this;
  }
};


void print(const std::string& msg, float f) {
  std::cout << "float:" << f + 10.0 << std::endl;
  int a = 0;
  memcpy(&a, &f, 4);
  std::bitset<32> bs = a;
  std::cout << msg << bs << std::endl;
}


TEST(ConstTest, Test) {
  print("lowest:", std::numeric_limits<float>::max());
  print("-inf:", std::numeric_limits<float>::lowest());
  std::cout << std::numeric_limits<int>::epsilon() << std::endl;

  Tensor<int> tensor({1, 2, 3});

  std::cout << tensor << std::endl;
  size_t total = 3 * 4 * 8 * 5 * 2;
  auto ptr = ::malloc(total * sizeof(int));
  int *intPtr = static_cast<int *>(ptr);
  std::iota(intPtr, intPtr + total, 0);
  TensorBase tensorBase(ptr, total);
  auto tensor0 = tensorBase.cast<int>();
  std::cout << tensor0 << std::endl;

  tensor0.reshape({2, 4, 5, 8}, {480, 8, 96, 1});

  for (size_t i = 0; i < 2; i++) {
    std::cout << std::endl;
    for (size_t j = 0; j < 4; j++) {
      std::cout << std::endl;
      for (size_t k = 0; k < 5; k++) {
        std::cout << std::endl;
        for (size_t l = 0; l < 8; l++) {
          std::cout << "offset:" << tensor0.offset(i, j, k, l)
                    << " value:" << tensor0.at(i, j, k, l) << " ";
        }
      }
    }
  }

  std::cout << "offset:" << tensor0.offset(0, 1, 0, 0) << std::endl;
}