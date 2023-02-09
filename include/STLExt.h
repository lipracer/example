
#ifndef INCLUDE_STLEXT_H
#define INCLUDE_STLEXT_H

#include <utility>

namespace example {

class MyAnyData {
 public:
  template <typename T>
  MyAnyData(T&& t) {
    buf = new char[sizeof(typename std::remove_reference<T>::type)];
    using type = typename std::remove_reference<T>::type;
    new (buf) type(std::forward<T>(t));
  }

  template <typename T>
  T cast_as() {
    return *reinterpret_cast<T*>(buf);
  }

  ~MyAnyData() { delete buf; }

 private:
  char* buf;
};

template <typename T>
struct MyFunction;

template <typename Functor, typename R, typename... Args>
struct Invoker {
  static R __Invoker(MyAnyData& obj, Args... args) {
    return obj.cast_as<Functor>()(args...);
  }
};

template <typename R, typename... Args>
struct MyFunction<R(Args...)> {
  template <typename Functor>
  MyFunction(Functor);

  R operator()(Args&&... args) {
    return (*funPtr)(anyDataBuf, std::forward<Args>(args)...);
  }

 private:
  R (*funPtr)(MyAnyData&, Args...) = nullptr;
  MyAnyData anyDataBuf;
};

template <typename R, typename... Args>
template <typename Functor>
MyFunction<R(Args...)>::MyFunction(Functor t)
    : funPtr(Invoker<Functor, R, Args...>::__Invoker), anyDataBuf(t) {}

template <typename T>
class ArrayRef {
 public:
  using value_type = T;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = size_t;

  ArrayRef(const T* data, size_type size)
      : data_(const_cast<T*>(data)), size_(size) {}
  // ArrayRef(iterator begin, iterator end) : data_(begin),
  // size_(std::distance(begin, end)) {}

  template <size_type N>
  ArrayRef(T (&array)[N]) : ArrayRef(array, N) {}

  template <typename Iter>
  ArrayRef(Iter begin, Iter end)
      : data_(&*begin), size_(std::distance(begin, end)) {}

  ArrayRef(std::vector<T>& vec) : ArrayRef(vec.data(), vec.size()) {}

  reference front() { return *data_; }
  reference back() { return *(data_ + size_); }

  iterator begin() { return data_; }
  iterator end() { return data_ + size_; }

  size_type size() const { return size_; }
  reference operator[](size_type idx) { return *(data_ + idx); }

  ArrayRef<T> slice(size_t start) const {
    return ArrayRef<T>(data_ + start, size() - start);
  }

  ArrayRef<T> slice(size_t start, size_t size) const {
    return ArrayRef<T>(data_ + start, size);
  }

  ArrayRef<T> drop_front(size_t S = 1) const {
    return ArrayRef<T>(data_ + S, size() - S);
  }

  ArrayRef<T> drop_back(size_t S) const {
    return ArrayRef<T>(data_, size() - S);
  }

  bool empty() { return !size_; }

private:
  T* data_;
  size_type size_;
};

template <typename T, typename... Argvs>
ArrayRef<T> makeArrayRef(Argvs&&... argvs) {
  return ArrayRef<T>(std::forward<Argvs>(argvs)...);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, ArrayRef<T> array) {
  //
  for (const auto& it : array) {
    os << it << ", ";
  }
  return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> array) {
  //
  for (const auto& it : array) {
    os << it << ", ";
  }
  return os;
}


}  // namespace example

#endif