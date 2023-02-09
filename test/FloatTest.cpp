
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <numeric>
#include <cassert>
#include <vector>
#include <bitset>
#include <limits>
#include <fstream>
#include <algorithm>
#include <random>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <future>
#include <set>
#include <map>
#include <list>
#include <iomanip>
#include <unordered_map>
#include <array>


#include "Test.h"
#include "STLExt.h"
#include "Tensor.hpp"

#define TO_STRING(c) #c

template <typename N>
struct A {
  template <typename T> static void func();

  template <typename T> void mfunc();
};

template<typename N>
template <typename T> void A<N>::func() {}

template<typename N>
template <typename T> void A<N>::mfunc() {}

template <typename ...ArgsT>
using VoidT = void;

struct OpBuilder {};
struct Value {};


class ElementwiseExprImpl {
public:
    explicit ElementwiseExprImpl(size_t value) :
            ElementwiseExprImpl([=](OpBuilder& builder) {
                return Value();
            }) {
    }

    explicit ElementwiseExprImpl(std::function<Value(OpBuilder&)>&& expr, const std::string& name = "") :
            nativeExpr_(std::move(expr)), name_(name) {

            }
    explicit ElementwiseExprImpl(Value value) : value_(value) {

    }

    Value call(OpBuilder& builder) {return Value();}

    void dump() {}

    std::function<Value(OpBuilder&)> nativeExpr_;
    Value value_;
    std::string name_;
};

struct ElementwiseExpr {
    using ImplType = std::shared_ptr<ElementwiseExprImpl>;

    ElementwiseExpr(ImplType impl) : impl_(std::move(impl)) {}

    ElementwiseExpr(const ElementwiseExpr&) = default;
    ElementwiseExpr(ElementwiseExpr&&) = default;
    ElementwiseExpr& operator=(const ElementwiseExpr&) = default;
    ElementwiseExpr& operator=(ElementwiseExpr&&) = default;

    // compiler maybe has some bug, recursive call ctor
    template </*typename F, */typename... Args/*, typename = typename std::enable_if<!std::is_same<typename std::remove_cv<F>::type, ElementwiseExpr>::value>::type*/>
    ElementwiseExpr(/*F &&f, */Args &&... args)
    {
      // std::cout << std::boolalpha << std::is_same<typename std::remove_cv<F>::type, ElementwiseExpr>::value << " ";
      std::cout << typeid(ElementwiseExpr).name() << " ";
      // std::cout << typeid(typename std::remove_cv<F>::type).name() << " ";
      (void)std::initializer_list<int>{((std::cout << typeid(Args).name() << " "), 0)...};
      std::cout << std::endl;
      auto ptr = new ElementwiseExprImpl(/*std::forward<F>(f), */std::forward<Args>(args)...);
      // impl_ = (std::make_shared<ElementwiseExprImpl>(std::forward<Args>(args)...));
      impl_.reset(ptr);
      // std::cout << typeid(F).name() << std::endl;
    }

    Value operator()(OpBuilder& builder) const {
        return impl_->call(builder);
    }

private:
    // avoid capture tempory variable, so we always construct expressions
    ImplType impl_;
};

ElementwiseExpr operator+(const ElementwiseExpr& lhs, const ElementwiseExpr& rhs) {
    return ElementwiseExpr([=](OpBuilder& builder) -> Value {
        auto newLhs = lhs(builder);
        auto newRhs = rhs(builder);
        return Value();
    }, "plus");
}

void TestOver(int a){}
void TestOver(const int& a){}


using int32x16_t = std::array<int32_t, 16>;

int32x16_t vload_lm_int32x16(int32_t* ptr)
{
  int32x16_t result;
  memcpy(result.data(), ptr, sizeof(int32x16_t));
  return result;
}
void vstore_lm_int32x16(int32_t* ptr, int32x16_t value)
{
  memcpy(ptr, value.data(), sizeof(int32x16_t));
}
int32x16_t vvadd_int32x16(int32x16_t lhs, int32x16_t rhs)
{
  int32x16_t result;
  for (size_t i = 0; i < 16; ++i)
  {
    result[i] = lhs[i] + rhs[i];
  }
  return result;
}

#define store(v, p) //(*(p) = (v))

template <typename T, size_t size>
using ArrayType = T[size];

#define GM2LM(src, dst, size) \
  do                          \
  {                           \
    memcpy(dst, src, size);   \
  } while (false);
#define LM2GM(src, dst, size) \
  do                          \
  {                           \
    memcpy(dst, src, size);   \
  } while (false);

#define GM2LM_ASYNC GM2LM
#define LM2GM_ASYNC LM2GM

unsigned gClusterId = 0;
unsigned gThreadId = 0;

unsigned cluster_id() {
  return gClusterId;
}

unsigned core_id() {
  return gThreadId;
}

#define __global_ptr__
#define __global__
#define __simd__
#define size_t unsigned

#define PRINT(dst, src, size) std::cout << "dst:" << (dst) << std::setw(20) << "src:" << (src) << std::setw(20) << "size:" << (size) << std::endl;
void mfence() {}

__global__ void elementwise_fusion0(int32_t* v1, int32_t* v2, int32_t* v3) {
  __simd__ ArrayType<int32_t, 128> v4 = {};
  __simd__ ArrayType<int32_t, 256> v5 = {};
  __simd__ ArrayType<int32_t, 256> v6 = {};
  size_t v7 = 1;
  size_t v8 = 8192;
  size_t v9 = 128;
  size_t v10 = cluster_id();
  size_t v11 = core_id();
  size_t v12 = v10 * v8;
  size_t v13 = v11 * v9;
  size_t v14 = v12 + v13;
  size_t v15 = 0;
  __global_ptr__ int32_t* v16 = v1 + v14;
  int32_t* v17 = v6 + v15;
  size_t v18 = 512;
  GM2LM_ASYNC(v16, v17, v18);
  __global_ptr__ int32_t* v19 = v2 + v14;
  int32_t* v20 = v5 + v15;
  GM2LM_ASYNC(v19, v20, v18);
  mfence();
  mfence();
  size_t v21 = v14 + v9;
  std::cout << "cluster_id:" << cluster_id() << " core_id:" << core_id() << "\n";
  for (size_t v22 = v14; v22 < v21; v22 += v7) {
    std::cout << v22 << " ";
    int32_t v23 = *v5;
    int32_t v24 = *v6;
    int32_t v25 = v23 + v24;
    int32_t* v26 = v4 + v22;
    store(v25, v26);
  }
  std::cout << std::endl;
  __global_ptr__ int32_t* v27 = v3 + v14;
  LM2GM_ASYNC(v4, v27, v18);
  mfence();
  return;
}

std::vector<int> t0(128 * 512);
std::vector<int> t1(128 * 512);
std::vector<int> t2(128 * 512);
std::vector<int> t3(1025);



TEST(FloatTest, basic) {
  // float f = std::numeric_limits<uint32_t>::max();
  // std::vector<float> values;
  // for(size_t i = 0; i < 10000; ++i) {
  //   values.push_back(f);
  // }
  // float sum = std::accumulate(values.begin(), values.end(), 0.0f, std::plus<>());
  // std::cout << (int64_t)(sum) << std::endl;

  // A<int>::func<void>();

  // A<int> a;
  // a.mfunc<int>();

  std::iota(t0.begin(), t0.end(), 0);
  std::iota(t1.begin(), t1.end(), 0);
  std::iota(t2.begin(), t2.end(), 0);
  for (size_t i = 0; i < 8; ++i)
  {
    for (size_t j = 0; j < 64; ++j)
    {
      gClusterId = i;
      gThreadId = j;
      // elementwise_fusion0(t0.data(), t1.data(), t2.data());
    }
  }

  // std::string name = "elementwise_fusion";
  // static size_t index = 0;
  // std::unordered_map<const char*, void*> map;
  // std::vector<std::string> names;
  // std::vector<void*> lmap;
  // for (size_t i = 0; i < 10; ++i)
  // {
  //   names.emplace_back(name + std::to_string(i));
  //   map.emplace(names.back().c_str(), nullptr);
  //   lmap.push_back(nullptr);
  // }

  // auto start = std::chrono::high_resolution_clock::now().time_since_epoch();
  // size_t sindex = 0;
  // for (const auto &str : names)
  // {
  //   std::cout << "start point:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() - start).count() << std::endl;
  //   // auto iter = map.find(str.c_str());
  //   (void)lmap[sindex++];
  //   auto end = std::chrono::high_resolution_clock::now().time_since_epoch();
  //   std::cout << "end point:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() - start).count() << "\n" << std::endl;
  // }
}