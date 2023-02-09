
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h> 

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

#include "network/SimpleHttpServer.h"

#include "Test.h"
#include "STLExt.h"
#include "Tensor.hpp"
#include "utils/ProcessBarrier.h"

using namespace example;

#define random(n) (::rand() % n)

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

void print(const std::string& msg, float f) {
  std::cout << "float:" << f + 10.0 << std::endl;
  int a = 0;
  memcpy(&a, &f, 4);
  std::bitset<32> bs = a;
  std::cout << msg << bs << std::endl;
}

template <typename T>
std::tuple<std::shared_ptr<T>, size_t> loadFromFile(const std::string &name) {
  std::tuple<std::shared_ptr<T>, size_t> result = {nullptr, 0};
  std::ifstream ifs(name);
  ifs.seekg(0, std::ios::end);
  size_t length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  auto data = new float[length];
  ifs.read(reinterpret_cast<char *>(data), length);
  ifs.close();
  std::get<0>(result).reset(data);
  std::get<1>(result) = length / sizeof(T);
  return result;
}

TEST(BarrerTest, barrer) {

  // constexpr size_t kThreadCount = 8;
  // std::vector<std::future<std::string>> futures;

  // auto barrerThreadBody = [](size_t n, size_t id) {
  //   unsigned seed =
  //       std::chrono::system_clock::now().time_since_epoch().count();
  //   std::mt19937 g1(seed);
  //   std::uniform_int_distribution<int> distribution(0, 2000);

  //   thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
  //   barrier = createBarrier(n, id);

  //   for (size_t i = 0; i < 10; ++i) {
  //     int dice_roll = distribution(g1);
  //     std::cout << "id:" << id << " sleep " << dice_roll << "..." << std::endl;
  //     std::this_thread::sleep_for(std::chrono::milliseconds(dice_roll));
  //     std::cout << "id:" << id << " run into sync point." << std::endl;
  //     barrier->wait();
  //   }
  //   return barrier->getStatisticsInfo();
  // };
  // for (size_t i = 0; i < kThreadCount; ++i) {
  //   futures.emplace_back(std::async(barrerThreadBody, kThreadCount, i));
  // }
  // for (auto &f : futures) {
  //   std::cout << f.get();
  // }
}
