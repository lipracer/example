
#include <fcntl.h> /* For O_RDWR */
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> /* For open(), creat() */
// #include <link.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <vector>


#include "../utils/ProcessBarrier.h"
#include "STLExt.h"
#include "Test.h"

using namespace utils;
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

void print(const std::string &msg, float f) {
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

  constexpr size_t kThreadCount = 8;
  std::vector<std::future<std::string>> futures;

  auto barrerThreadBody = [](size_t n, size_t id) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g1(seed);
    std::uniform_int_distribution<int> distribution(0, 2000);

    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    for (size_t i = 0; i < 10; ++i) {
      SLOGW() << "=================task=================";
      int dice_roll = distribution(g1);

      SLOGW() << "id:" << id << " sleep " << dice_roll << "ms...";

      std::this_thread::sleep_for(std::chrono::milliseconds(dice_roll));

      SLOGW() << "id:" << id << " run into sync point.";

      barrier->wait();
    }
    return barrier->getStatisticsInfo();
  };
  for (size_t i = 0; i < kThreadCount; ++i) {
    futures.emplace_back(std::async(barrerThreadBody, kThreadCount, i));
  }
  for (auto &f : futures) {
    SLOGW() << f.get();
  }
}

TEST(BarrerTest, barrer2) {

  constexpr size_t kThreadCount = 2;
  std::vector<std::future<std::string>> futures;

  auto barrerThreadBody = [](size_t n, size_t id) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g1(seed);
    std::uniform_int_distribution<int> distribution(0, 30);

    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    for (size_t i = 0; i < 10; ++i) {
      SyncLog("=================task=================");
      int dice_roll = distribution(g1);
      SLOGW() << "id:" << id << " sleep " << dice_roll << "ms...";
      std::this_thread::sleep_for(std::chrono::seconds(dice_roll));
      SLOGW() << "id:" << id << " run into sync point.";
      barrier->wait();
    }
    return barrier->getStatisticsInfo();
  };
  for (size_t i = 0; i < kThreadCount; ++i) {
    futures.emplace_back(std::async(barrerThreadBody, kThreadCount, i));
  }
  for (auto &f : futures) {
    SLOGW() << f.get();
  }
}

TEST(BarrerTest, barrer_timeout0) {

  constexpr size_t kThreadCount = 2;
  std::vector<std::future<std::string>> futures;

  auto barrerThreadBody0 = [](size_t n, size_t id) {
    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    int dice_roll = 30;
    SLOGW() << "id:" << id << " sleep " << dice_roll << "ms...";
    std::this_thread::sleep_for(std::chrono::seconds(dice_roll));
    SLOGW() << "id:" << id << " run into sync point.";
    barrier->wait_for(std::chrono::milliseconds(15000), [](auto states) {
      SLOGW() << "states0:" << states[0] << " states1:" << states[1];
    });
    return barrier->getStatisticsInfo();
  };

  auto barrerThreadBody1 = [](size_t n, size_t id) {
    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    int dice_roll = 60;

    SLOGW() << "id:" << id << " sleep " << dice_roll << "s...";

    std::this_thread::sleep_for(std::chrono::seconds(dice_roll));

    SLOGW() << "id:" << id << " run into sync point.";

    barrier->wait_for(std::chrono::milliseconds(30000), [](auto states) {
      std::stringstream ss;
      ss << "states0:" << states[0] << " states1:" << states[1];
      SyncLog(ss.str());
    });

    return barrier->getStatisticsInfo();
  };
  futures.emplace_back(std::async(barrerThreadBody0, kThreadCount, 0));
  futures.emplace_back(std::async(barrerThreadBody1, kThreadCount, 1));
  for (auto &f : futures) {
    SLOGW() << f.get();
  }
}

TEST(BarrerTest, barrer_timeout1) {

  constexpr size_t kThreadCount = 2;
  std::vector<std::future<std::string>> futures;

  auto barrerThreadBody0 = [](size_t n, size_t id) {
    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    int dice_roll = 10;

    SLOGW() << "id:" << id << " sleep " << dice_roll << "s...";
    std::this_thread::sleep_for(std::chrono::seconds(dice_roll));
    SLOGW() << "id:" << id << " run into sync point.";

    barrier->wait_for(std::chrono::milliseconds(60000), [](auto states) {
      SLOGW() << "states0:" << states[0] << " states1:" << states[1];
    });
    return barrier->getStatisticsInfo();
  };

  auto barrerThreadBody1 = [](size_t n, size_t id) {
    thread_local static std::unique_ptr<ProcessBarrierBase> barrier;
    barrier = createBarrier(n, id);

    int dice_roll = 30;

    SLOGW() << "id:" << id << " sleep " << dice_roll << "s...";
    std::this_thread::sleep_for(std::chrono::seconds(dice_roll));
    SLOGW() << "id:" << id << " run into sync point.";
    barrier->wait_for(std::chrono::milliseconds(60000), [](auto states) {
      SLOGW() << "states0:" << states[0] << " states1:" << states[1];
    });

    return barrier->getStatisticsInfo();
  };
  futures.emplace_back(std::async(barrerThreadBody0, kThreadCount, 0));
  futures.emplace_back(std::async(barrerThreadBody1, kThreadCount, 1));
  for (auto &f : futures) {
    SLOGW() << f.get();
  }
}
