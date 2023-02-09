#ifndef UTILS_PROCESS_BARRIER
#define UTILS_PROCESS_BARRIER

#include <memory>
#include <chrono>
#include <functional>
#include <iosfwd>
#include <iostream>

namespace example {

enum class BarrierState : int {
  initialize = 0,
  uninitialized,
  waiting,
  timeout,
  unknown,
};

inline std::ostream &operator<<(std::ostream &os, BarrierState state) {
  switch (state) {
  case BarrierState::waiting: {
    os << "waiting";
  } break;
  case BarrierState::timeout: {
    os << "timeout";
  } break;
  case BarrierState::unknown: {
    os << "unknown";
  } break;
  default:
    os << 'I' << static_cast<int>(state);
    break;
  }
  return os;
}

using TimeoutCallBack = std::function<void(const BarrierState *)>;
using duration_type = std::chrono::seconds;

class ProcessBarrierBase {
public:
  void wait() { wait_impl(); }

  template <typename DurT> int wait_for(DurT dur, TimeoutCallBack callback) {
    return wait_for_impl(std::chrono::duration_cast<duration_type>(dur).count(),
                         callback);
  }

  virtual ~ProcessBarrierBase() {}

  virtual void releaseResource() = 0;

  virtual std::string getStatisticsInfo() { return ""; }

  std::function<void(size_t id, duration_type dur)> onWaitStart =
      [](size_t id, duration_type dur) {
        std::cout << "device" << id << "since ctor duration:"
                  << std::chrono::duration_cast<duration_type>(dur).count()
                  << "ms";
      };
  std::function<void(size_t id, duration_type dur)> onWaitEnd =
      [](size_t id, duration_type dur) {
        std::cout << "device" << id << "since wait duration:"
                  << std::chrono::duration_cast<duration_type>(dur).count()
                  << "ms";
      };

protected:
  virtual void wait_impl() = 0;
  virtual int wait_for_impl(int64_t timeout, TimeoutCallBack callback) = 0;
};

std::unique_ptr<ProcessBarrierBase> createBarrier(size_t clientSize, size_t id);

} // namespace example

#endif
