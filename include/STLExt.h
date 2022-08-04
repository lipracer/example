
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
    : funPtr(Invoker<Functor, R, Args...>::__Invoker), anyDataBuf(t) {
  ;
}

}  // namespace example

#endif