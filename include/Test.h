#ifndef INCLUDE_TEST_H
#define INCLUDE_TEST_H

template <typename T>
class SimpleList {
  struct SimpleListNode {
    T data;
    SimpleListNode *next{nullptr};
    SimpleListNode(const T &t) : data(t), next(nullptr) {}
  };

  struct Iterator {
    Iterator(SimpleListNode *ptr) : ptr(ptr) {}
    SimpleListNode *operator->() { return ptr; }
    SimpleListNode *operator->() const { return ptr; }
    SimpleListNode &operator*() { return *ptr; }
    SimpleListNode &operator*() const { return *ptr; }
    Iterator operator++() {
      ptr = ptr->next;
      return ptr;
    }
    Iterator operator++(int) {
      auto tmp = ptr;
      ptr = ptr->next;
      return tmp;
    }
    bool operator==(Iterator other) { return ptr == other.ptr; }
    bool operator!=(Iterator other) { return !(*this == other.ptr); }

   private:
    SimpleListNode *ptr;
  };

 public:
  void push(const T &t) {
    auto tmp = tail;
    tail = new SimpleListNode(t);
    if (tmp) {
      tmp->next = tail;
    } else {
      head = tail;
    }
  }
  Iterator begin() { return head; }
  Iterator end() { return nullptr; }
  ~SimpleList() {
    while (head) {
      auto tmp = head->next;
      delete head;
      head = tmp;
    }
    tail = nullptr;
  }

 private:
  SimpleListNode *head = nullptr;
  SimpleListNode *tail = nullptr;
};

#define CONCAT_MACRO(name, c) name##c

#define UNIQUE_NAME(name) CONCAT_MACRO(name, __COUNTER__)

class SuperTestClass;

class TestCaseCollection {
 public:
  static TestCaseCollection &instance();
  void push(SuperTestClass *ptr);
  SimpleList<SuperTestClass *> &data() { return data_; }

 private:
  TestCaseCollection() = default;
  SimpleList<SuperTestClass *> data_;
};

class SuperTestClass {
 protected:
  SuperTestClass() { TestCaseCollection::instance().push(this); }

 public:
  virtual void Testing() const = 0;
};

#define TEST(class_, name)                            \
  static class class_##name : public SuperTestClass { \
    void Testing() const override;                    \
  } UNIQUE_NAME(class_##name);                        \
  void class_##name::Testing() const

#endif