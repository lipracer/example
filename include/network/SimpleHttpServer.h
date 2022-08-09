#ifndef INCLUDE_NETWORK_SIMPLEHTTPSERVER_H
#define INCLUDE_NETWORK_SIMPLEHTTPSERVER_H

#include <string>

#include "../ThreadPool.h"
#include "MySocket.h"

namespace network {
class SimpleHttpServer {
 public:
  SimpleHttpServer(const std::string& ip, size_t port) : ip_(ip), port_(port) {}

  void run() {}

 private:
  std::string ip_;
  size_t port_;
};
}  // namespace network

#endif