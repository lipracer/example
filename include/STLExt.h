
#ifndef STLEXT_H
#define STLEXT_H

#include <utility>


namespace example {

template <typename T>
struct MyFunction;

template <typename Functor, typename R, typename... Args>
struct Invoker {
  static R __Invoker(void* data, Args... args) {
    return Invoker::getObject(data)(args...);
  }

  static Functor& getObject(void* data) {
    return *reinterpret_cast<Functor*>(data);
  }
};

template <typename R, typename... Args>
struct MyFunction<R(Args...)> {

  template <typename Functor>
  MyFunction(Functor);

  ~MyFunction() { free(anyDataBuf); }

  R operator()(Args&&... args) {
    return (*funPtr)(anyDataBuf, std::forward<Args>(args)...);
  }

 private:
  R (*funPtr)(void*, Args...) = nullptr;
  void* anyDataBuf = nullptr;
};

template <typename R, typename... Args>
template <typename Functor>
MyFunction<R(Args...)>::MyFunction(Functor t) {
  funPtr = Invoker<Functor, R, Args...>::__Invoker;
  anyDataBuf = malloc(sizeof(Functor));
  new (anyDataBuf) Functor(t);
}

}  // namespace example

#endif