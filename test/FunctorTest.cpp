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
  A &operator=(A &&a) { std::cout << "assign constructor\n"; }
};

struct B {
  B(A &&a) { this->a = std::move(a); }

private:
  A a;
};

A getA() {
  return A();
}

void print(const std::string& msg, float f) {
  std::cout << "float:" << f + 10.0 << std::endl;
  int a = 0;
  memcpy(&a, &f, 4);
  std::bitset<32> bs = a;
  std::cout << msg << bs << std::endl;
}

template <typename T>
struct xpu_numeric_limits;

template <> struct xpu_numeric_limits<float> {
  static constexpr float inf() { return 3.40282e+38; }
  static constexpr float max() { return 3.40282e+38; }
  static constexpr float lowest() { return -3.40282e+38; }
};

template <typename T> struct xpu_less {
  bool operator()(const T &lhs, const T &rhs) { return lhs < rhs; }
};

template <typename T> struct xpu_greater {
  bool operator()(const T &lhs, const T &rhs) { return lhs > rhs; }
};

template <typename T> struct InitValue;

template <typename T> struct InitValue<xpu_less<T>> {
  constexpr static auto value = xpu_numeric_limits<T>::max;
};

template <typename T> struct InitValue<xpu_greater<T>> {
  constexpr static auto value = xpu_numeric_limits<T>::lowest;
};

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

  tensor0.rehsape({2, 4, 5, 8}, {480, 8, 96, 1});

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