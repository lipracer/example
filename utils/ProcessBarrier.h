#ifndef UTILS_PROCESS_BARRIER
#define UTILS_PROCESS_BARRIER

#include <chrono>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <sstream>

namespace utils {

void SyncLog(const std::string &str);

class SimpleLogWrapper {
public:
  template <typename T> void put(T &&t) const { ss << std::forward<T>(t); }
  ~SimpleLogWrapper() { SyncLog(ss.str()); }

private:
  mutable std::stringstream ss;
};

template <typename T>
inline const SimpleLogWrapper &operator<<(const SimpleLogWrapper &w, T &&t) {
  w.put(std::forward<T>(t));
  return w;
}

#define SLOGW() SimpleLogWrapper()

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

const char *getUnitStr();

using TimeoutCallBack = std::function<void(const BarrierState *)>;
using duration_type = std::chrono::milliseconds;

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
        SLOGW() << "device" << id << " start wait since ctor duration:"
                << std::chrono::duration_cast<duration_type>(dur).count()
                << getUnitStr();
      };
  std::function<void(size_t id, duration_type dur)> onWaitEnd =
      [](size_t id, duration_type dur) {
        SLOGW() << "device" << id << " end wait since start wait duration:"
           << std::chrono::duration_cast<duration_type>(dur).count()
           << getUnitStr();
      };

protected:
  virtual void wait_impl() = 0;
  virtual int wait_for_impl(int64_t timeout, TimeoutCallBack callback) = 0;
};

std::unique_ptr<ProcessBarrierBase> createBarrier(size_t clientSize, size_t id);

} // namespace utils

#endif
