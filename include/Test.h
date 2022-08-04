#ifndef INCLUDE_TEST_H
#define INCLUDE_TEST_H

#include <vector>

#define CONCAT_MACRO(name, c) name##c

#define UNIQUE_NAME(name) CONCAT_MACRO(name, __COUNTER__)

class SuperTestClass;

class TestCaseCollection {
 public:
  static TestCaseCollection& instance();
  void push(SuperTestClass* ptr);
  std::vector<SuperTestClass*>& data() { return data_; }

 private:
  TestCaseCollection() = default;
  std::vector<SuperTestClass*> data_;
};

class SuperTestClass {
 protected:
  SuperTestClass() {
    TestCaseCollection::instance().push(this);
  }

 public:
  virtual void Testing() = 0;
};

#define TEST(class_, name)                            \
  static class class_##name : public SuperTestClass { \
    void Testing() override;                          \
  } UNIQUE_NAME(class_##name);                        \
  void class_##name::Testing()

#endif