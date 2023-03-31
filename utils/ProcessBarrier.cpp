
#include "ProcessBarrier.h"
#include <fcntl.h> /* For O_RDWR */
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> /* For open(), creat() */

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <limits>
#include <mutex>
#include <numeric>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

namespace utils {

class SyncLogImpl {
public:
  SyncLogImpl() : th_(&SyncLogImpl::print, this), exit_(false) {}
  void putLog(const std::string &str) {
    {
      std::lock_guard<std::mutex> guard{mtx_};
      buf_.push_back(str);
    }
  }

  void print() {
    while (!exit_) {
      {
        mtx_.lock();
        if (buf_.empty()) {
          mtx_.unlock();
          std::this_thread::yield();
          continue;
        }
        mtx_.unlock();
      }
      std::cout << buf_.front() << std::endl;
      buf_.pop_front();
      std::this_thread::yield();
    }
  }

  ~SyncLogImpl() {
    exit_.store(true);
    th_.join();
  }

private:
  std::mutex mtx_;
  std::deque<std::string> buf_;
  std::thread th_;
  std::atomic<bool> exit_;
};

void SyncLog(const std::string &str) {
  static SyncLogImpl syncLog;
  syncLog.putLog(str);
}

struct SemGuard {
  SemGuard(sem_t *sem) : sem_(sem) { sem_wait(sem_); }
  ~SemGuard() { sem_post(sem_); }
  sem_t *sem_;
};

constexpr static const size_t kMaxMsgBuf = 64;

using TimeoutCallBack = std::function<void(const BarrierState *)>;

const char *getUnitStr() {
  if (std::is_same<duration_type, std::chrono::seconds>::value) {
    return "s";
  } else if (std::is_same<duration_type, std::chrono::milliseconds>::value) {
    return "ms";
  } else if (std::is_same<duration_type, std::chrono::microseconds>::value) {
    return "us";
  } else {
    throw std::runtime_error("unknown unit type!");
  }
}

template <typename DerivedT> class ProcessBarrier : public ProcessBarrierBase {
protected:
  struct SharedHandle {
    SharedHandle() {
      shmid_ = shmget((key_t)kSharedMemId(), sizeof(ShareMemoryType),
                      0640 | IPC_CREAT);
      share_mem_ = static_cast<ShareMemoryType *>(shmat(shmid_, 0, 0));
      assert(share_mem_ && "shared memory null!");
    }
    ~SharedHandle() { shmdt(share_mem_); }

    size_t getWaitingCount() { return share_mem_->wait_count; }
    void setWaitingCount(size_t count) { share_mem_->wait_count = count; }
    void increase() { ++(share_mem_->wait_count); }

    void clearMemory() {
      setWaitingCount(0);
      for (size_t i = 0;
           i < sizeof(share_mem_->msg) / sizeof(share_mem_->msg[0]); ++i) {
        share_mem_->msg[i] = BarrierState::initialize;
      }
    }

    void setStartTP(int64_t pt) { share_mem_->time_point = pt; }
    int64_t getStartTP() { return share_mem_->time_point; }

    void setState(size_t id, BarrierState state) {
      assert(id < kMaxMsgBuf && "client id over bound of max!");
      share_mem_->msg[id] = state;
    }

    BarrierState getState(size_t id) {
      assert(id < kMaxMsgBuf && "client id over bound of max!");
      return static_cast<BarrierState>(share_mem_->msg[id]);
    }

    const BarrierState *getStates() { return share_mem_->msg; }

    std::string toString() {
      std::string str = "wait_count:";
      str += std::to_string(share_mem_->wait_count);
      str += " cur_id:";
      str += std::to_string(share_mem_->cur_id);
      return str;
    }

  private:
    struct ShareMemoryType {
      volatile int64_t time_point;
      volatile size_t wait_count;
      volatile size_t cur_id;
      BarrierState msg[kMaxMsgBuf];
    };
    int shmid_ = 0;
    ShareMemoryType *share_mem_;
  };

  sem_t *mem_sem_ = nullptr;
  std::unique_ptr<SharedHandle> share_handle_;
  int shmid_ = 0;
  sem_t *sem_count_;
  sem_t *sem_init_;
  std::stringstream log_;

  static std::string getUniqueStr(const std::string &flag) {
    return flag;
  }

  static const std::string &kAllReduceSemFlag() {
    static std::string str = getUniqueStr("__kAllReduceSemFlagNIII__");
    return str;
  }

  static const std::string &kAllReduceSemInitFlag() {
    static std::string str = getUniqueStr("__kAllReduceSemInitFlagNIII_X__");
    return str;
  }

  static int kSharedMemId() { return ftok("/dev/null", 2); }

public:
  std::string getStatisticsInfo() override {
    log().flush();
    auto info = log().str();
    log().str() = "";
    return info;
  }

  ProcessBarrier() : ProcessBarrierBase() {
    sem_init_ = static_cast<DerivedT *>(this)->getInitializeSem();
    sem_count_ = getCountSem();
    clearWaitCount(sem_count_, 1);
    share_handle_ = std::make_unique<SharedHandle>();
  }

  class BarrierServer;
  class BarrierClient;

  sem_t *getInitializeSem() {
    throw std::runtime_error("unreached getInitializeSem!");
  }

  ~ProcessBarrier() override { sem_unlink(getName(0).c_str()); }

  void releaseResource() final {
    sem_unlink(getName(0).c_str());
    static_cast<DerivedT *>(this)->releaseResourceImpl();
  }

  int64_t durationSinceCtor() {
    return std::chrono::duration_cast<duration_type>(
               std::chrono::steady_clock::now().time_since_epoch())
               .count() -
           share_handle_->getStartTP();
  }

  sem_t *getSemPtr(const std::string &name, size_t initValue) {
    sem_t *semptr = nullptr;
    semptr = sem_open(name.c_str(), O_EXCL, S_IROTH | S_IWOTH, initValue);
    if (!semptr) {
      semptr = sem_open(name.c_str(), O_CREAT, S_IROTH | S_IWOTH, initValue);
    }
    return semptr;
  }
  std::string getName(size_t id) {
    return kAllReduceSemFlag() + std::to_string(id);
  }
  sem_t *getCountSem() { return getSemPtr(getName(0), 1); }

  std::string strWaitCount(sem_t *sem) {
    int value = -1;
    sem_getvalue(sem, &value);
    return std::to_string(value);
  }

  void clearWaitCount(sem_t *sem, size_t count = 0) {
    int waitCount = -1;
    while (true) {
      sem_getvalue(sem, &waitCount);
      if (waitCount > count) {
        sem_wait(sem);
      } else if (waitCount < count) {
        sem_post(sem);
      } else {
        break;
      }
    }
  }

  bool checkHasTimeout(size_t id) {
    if (share_handle_->getState(id) == BarrierState::timeout) {
      return true;
    }
    return false;
  }

  std::stringstream &log() { return log_; }
};

#define DEBUG_LOG
class BarrierServer : public ProcessBarrier<BarrierServer> {
public:
  BarrierServer(size_t nclient)
      : ProcessBarrier<BarrierServer>(), nclient_(nclient) {
    clearState();
    for (size_t i = 0; i < nclient_ - 1; ++i) {
      auto name = getName(i + 1);
      sem_names_.push_back(name);
      auto sem = getSemPtr(name, 0);
      sems_.push_back(sem);
      clearWaitCount(sems_[i], 0);
    }
    share_handle_->setStartTP(
        std::chrono::duration_cast<duration_type>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    for (size_t i = 0; i < nclient_ - 1; ++i) {
      sem_post(sem_init_);
    }
  }
  ~BarrierServer() override { releaseResourceImpl(); }

  void releaseResourceImpl() {
    for (const auto &name : sem_names_) {
      sem_unlink(name.c_str());
    }
  }

  void notifyAll() {
    for (auto sem : sems_) {
      sem_post(sem);
    }
  }

  void wait_impl() final {
    auto waiting_tp = durationSinceCtor();
    if (onWaitStart)
      onWaitStart(0, duration_type(waiting_tp));

    while (share_handle_->getWaitingCount() < nclient_ - 1) {
      std::this_thread::yield();
    }
    clearState();
    notifyAll();
    if (onWaitEnd)
      onWaitEnd(0, duration_type(durationSinceCtor() - waiting_tp));
  }

  // duration us
  int wait_for_impl(int64_t timeout, TimeoutCallBack callback) override {
    auto waiting_tp = durationSinceCtor();
    if (onWaitStart)
      onWaitStart(0, duration_type(waiting_tp));
    int ret = 0;
    auto spt = std::chrono::steady_clock::now();
    while (share_handle_->getWaitingCount() < nclient_ - 1) {
      auto dur = std::chrono::duration_cast<duration_type>(
                     std::chrono::steady_clock::now() - spt)
                     .count();
      bool client_timeout = false;
      for (size_t i = 1; i < nclient_; ++i) {
        if (checkHasTimeout(i)) {
          client_timeout = true;
        }
      }
      if (client_timeout) {
        ret = -1;
        break;
      }
      if (dur > timeout) {
        share_handle_->setState(0, BarrierState::timeout);
        ret = -1;
        break;
      }
      std::this_thread::yield();
    }
    if (onWaitEnd)
      onWaitEnd(0, duration_type(durationSinceCtor() - waiting_tp));
    notifyAll();
    if (ret && callback) {
      callback(share_handle_->getStates());
    }
    return ret;
  }

  void clearState() {
    SemGuard semGuard(sem_count_);
    share_handle_->clearMemory();
  }

  sem_t *getInitializeSem() {
    return sem_open(kAllReduceSemInitFlag().c_str(), O_CREAT, S_IROTH | S_IWOTH,
                    0);
  }

private:
  const size_t nclient_;
  std::vector<sem_t *> sems_;
  std::vector<std::string> sem_names_;
};

class BarrierClient : public ProcessBarrier<BarrierClient> {
public:
  BarrierClient(size_t id) : ProcessBarrier<BarrierClient>(), id_(id) {
    sem_wait(sem_init_);
    sem_wait_ = getSemPtr(getName(id), 0);
  }

  ~BarrierClient() override { releaseResourceImpl(); }

  void releaseResourceImpl() { sem_unlink(getName(id_).c_str()); }

  void wait_impl() override {
    auto waiting_tp = durationSinceCtor();
    if (onWaitStart)
      onWaitStart(id_, duration_type(waiting_tp));
    {
      SemGuard semGuard(sem_count_);
      share_handle_->increase();
    }
    share_handle_->setState(id_, BarrierState::waiting);
    sem_wait(sem_wait_);
    if (onWaitEnd)
      onWaitEnd(id_, duration_type(durationSinceCtor() - waiting_tp));
  }

  int wait_for_impl(int64_t timeout, TimeoutCallBack callback) override {
    auto waiting_tp = durationSinceCtor();
    if (onWaitStart)
      onWaitStart(id_, duration_type(waiting_tp));
    int ret = 0;
    {
      SemGuard semGuard(sem_count_);
      share_handle_->increase();
    }
    share_handle_->setState(id_, BarrierState::waiting);
    auto spt = std::chrono::steady_clock::now();
    while (sem_trywait(sem_wait_)) {
      auto dur = std::chrono::duration_cast<duration_type>(
                     std::chrono::steady_clock::now() - spt)
                     .count();
      if (checkHasTimeout(0)) {
        ret = -1;
        break;
      }
      if (dur > timeout) {
        share_handle_->setState(id_, BarrierState::timeout);
        ret = -1;
        break;
      }
    }
    if (ret && callback) {
      callback(share_handle_->getStates());
    }
    if (onWaitEnd)
      onWaitEnd(id_, duration_type(durationSinceCtor() - waiting_tp));
    return ret;
  }

  sem_t *getInitializeSem() {
    sem_t *ret = nullptr;
    while (!ret) {
      ret = sem_open(kAllReduceSemInitFlag().c_str(), O_EXCL, S_IROTH | S_IWOTH,
                     0);
      std::this_thread::yield();
    }
    return ret;
  }

private:
  const size_t id_;
  sem_t *sem_wait_ = nullptr;
  std::string sem_name_;
};

std::unique_ptr<ProcessBarrierBase> createBarrier(size_t worker_size,
                                                  size_t id) {
  if (id == 0) {
    return std::make_unique<BarrierServer>(worker_size);
  } else {
    return std::make_unique<BarrierClient>(id);
  }
}
} // namespace utils