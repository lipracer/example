#include "Test.h"
// #include "gperftools/profiler.h"

#include <iostream>
#include <regex>

TestCaseCollection& TestCaseCollection::instance() {
  static TestCaseCollection __instance;
  return __instance;
}

void TestCaseCollection::push(SuperTestClass* ptr) { data_.push(ptr); }

int main(int argc, char** args) {
  // ProfilerStart("/home/workspace/example/profiler.prof");
  const char *filter_str = nullptr;
  for (int i = 0; i < argc; ++i) {
    if (strstr(args[i], "--filter")) {
      filter_str = args[i] + strlen("--filter=");
      break;
    }
  }
  auto filter_func = [filter_str](const std::string &test_case) {
    if (!filter_str)
      return true;
    std::regex pattern(filter_str);
    if (std::regex_search(test_case, pattern)) {
      return true;
    }
    return false;
  };
  for (const auto &node : TestCaseCollection::instance().data()) {
    if (!filter_func(node.data->name()))
      continue;
    std::cout << "\033[0;32m[begin]"
              << "[" << node.data->name() << "]\033[0m" << std::endl;

    node.data->Testing();
    std::cout << "\033[0;32m[end]\033[0m" << std::endl;
  }
  // ProfilerStop();
}