#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>

template <typename> class SimpleTensorStorage;

static void freeFree(void *) {
  std::cout << "freeFree" << std::endl;
}

template <typename DerivedT = void, typename EleT = void>
struct SimpleTensorStorageBase {
  using type = typename std::conditional<std::is_same<DerivedT, void>::value,
                                         void, EleT>::type;
  SimpleTensorStorageBase() = default;
  SimpleTensorStorageBase(void *data, size_t byteLen) : length_(byteLen) {
    storage_.reset(data, freeFree);
  }
  SimpleTensorStorageBase(std::shared_ptr<void> storage, size_t length)
      : storage_(storage), length_(length) {}
  void alloc(size_t byteLen) {
    length_ = byteLen;
    storage_.reset(::malloc(byteLen), ::free);
  }
  type *getData() const { return reinterpret_cast<type *>(storage_.get()); }
  type *getData() { return reinterpret_cast<type *>(storage_.get()); }

  template <typename T> SimpleTensorStorage<T> cast() {
    return SimpleTensorStorage<T>(storage_, length_);
  }

private:
  std::shared_ptr<void> storage_;
  size_t length_;
};

template <typename T>
struct SimpleTensorStorage
    : public SimpleTensorStorageBase<SimpleTensorStorage<T>, T> {
  using type = T;
  using SimpleTensorStorageBase<SimpleTensorStorage<T>,
                                T>::SimpleTensorStorageBase;
};

template <typename, typename> class Tensor;

class TensorVisitor {
private:
  std::vector<int64_t> shape_;
  std::vector<int64_t> strides_;
  std::vector<int64_t> offsets_;
};

template <typename StorageT = SimpleTensorStorageBase<void>>
class TensorBaseImpl {
  template <typename, typename> friend class Tensor;

public:
  TensorBaseImpl(void *data, size_t len) : storage_(data, len) {}

  template <typename T> auto cast() {
    return Tensor<T, SimpleTensorStorage<T>>(storage_.template cast<T>());
  }

protected:
  TensorBaseImpl(StorageT storage) : storage_(storage) {}

private:
  StorageT storage_;
};

using TensorBase = TensorBaseImpl<>;

template <typename EleT, typename StorageT = SimpleTensorStorage<EleT>>
class Tensor {
public:
  using element_type = EleT;
  Tensor(StorageT storage) : storage_(storage) {}

  Tensor(const std::vector<int64_t> &shape,
         const std::vector<int64_t> &strides) {}

  Tensor(const std::initializer_list<EleT> &list) {
    storage_.alloc(list.size() * sizeof(EleT));
    auto ptr = storage_.getData();
    std::copy(list.begin(), list.end(), storage_.getData());
    shape_.clear();
    shape_.push_back(list.size());
  }

  Tensor(const std::vector<EleT>& vectorData) {}

  size_t totalElements() const {
    return std::accumulate(shape_.begin(), shape_.end(), 1,
                           std::multiplies<>());
  }

  template <typename T, typename... R> size_t offset(T t, R... r) {
    return t * strides_[strides_.size() - sizeof...(R) - 1] + offset(r...);
  }

  size_t offset() { return 0; }

  template <typename... T> EleT &at(T... t) {
    size_t ofs = offset(t...);
    return *(storage_.getData() + ofs);
  }

  void rehsape(const std::vector<int64_t> &shape,
               const std::vector<int64_t> &strides) {
    shape_ = shape;
    strides_ = strides;
  }

  std::string toString() const {
    std::stringstream ss;
    for (size_t i = 0; i < totalElements(); ++i) {
      ss << storage_.getData()[i] << ",";
    }
    return ss.str();
  }

private:
  StorageT storage_;
  std::vector<int64_t> shape_;
  std::vector<int64_t> strides_;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const Tensor<T> &tensor) {
    os << tensor.toString();
    return os;
}
