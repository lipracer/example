#include "Test.h"

TestCaseCollection& TestCaseCollection::instance() {
  static TestCaseCollection __instance;
  return __instance;
}

void TestCaseCollection::push(SuperTestClass* ptr) { data_.push(ptr); }

int main(int argc, char** args) {
  for (const auto& node : TestCaseCollection::instance().data()) {
    node.data->Testing();
  }
}