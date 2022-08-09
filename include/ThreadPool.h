#ifndef INCLUDE_THREADPOOL_H
#define INCLUDE_THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

namespace example {

class TreadPool {
 public:
  TreadPool(size_t size) {
    for (size_t i = 0; i < size; ++i) {
      pool_.emplace_back([]() {
std::lock


      });
    }
  };

  template <typename Func>
  void enque(Func&& func) {
    {
      std::lock_guard<std::mutex> g(mtx_);
      tasks_.push_back(std::forward<Func>(func));
    }
    cond_.notify_all();
  }

 private:
  void task() {
      
  }

 private:
  std::function<void(void)> tasks_;
  std::vector<std::thread> pool_;
  std::mutex mtx_;
  std::condition_variable cond_;
};
};  // namespace example

#endif