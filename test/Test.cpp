#include "Test.h"
// #include "gperftools/profiler.h"

#include <iostream>

TestCaseCollection& TestCaseCollection::instance() {
  static TestCaseCollection __instance;
  return __instance;
}

void TestCaseCollection::push(SuperTestClass* ptr) { data_.push(ptr); }

int main(int argc, char** args) {
  // ProfilerStart("/home/workspace/example/profiler.prof");
  for (const auto &node : TestCaseCollection::instance().data()) {
    std::cout << "[begin]"
              << "[" << node.data->name() << "]:" << std::endl;
    node.data->Testing();
    std::cout << "[end]" << std::endl;
  }
  // ProfilerStop();
}