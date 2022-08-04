#include "Test.h"

TestCaseCollection& TestCaseCollection::instance() {
  static TestCaseCollection __instance;
  return __instance;
}

void TestCaseCollection::push(SuperTestClass* ptr) { data_.push_back(ptr); }

int main(int argc, char** args) {
  for (auto ptr : TestCaseCollection::instance().data()) {
    ptr->Testing();
  }
}