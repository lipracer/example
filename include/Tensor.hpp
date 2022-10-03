#include <memory>
#include <sstream>
#include <string>
#include <vector>

template <typename> class SimpleTensorStorage;

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

  static void freeFree(void *) {}

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
public:
  size_t totalElements() const {
    return std::accumulate(shape_.begin(), shape_.end(), 1,
                           std::multiplies<>());
  }

  template <typename T, typename... R> size_t offset(T t, R... r) {
    return t * strides_[strides_.size() - sizeof...(R) - 1] + offset(r...);
  }

  template <typename VecI64Ref>
  void reshape(VecI64Ref shape, VecI64Ref strides) {
    shape_.assign(shape.begin(), shape.end());
    strides_.assign(strides.begin(), strides.end());
  }

  void reshape(std::initializer_list<int64_t> shape,
               std::initializer_list<int64_t> strides) {
    shape_.assign(shape.begin(), shape.end());
    strides_.assign(strides.begin(), strides.end());
  }

  size_t offset() { return 0; }

protected:
  template <typename VecI64Ref>
  TensorVisitor(VecI64Ref shape)
      : shape_(shape.begin(), shape.end()), strides_(shape.size(), 0),
        offsets_(shape.size(), 0) {
    int64_t stride = 0;
    int64_t curIndex = shape.size() - 1;
    auto curIterator = shape.rend();
    for (; curIndex >= 0; --curIndex) {
      strides_[curIndex] = stride;
      stride *= *curIterator;
    }
  }
  template <typename VecI64Ref>
  TensorVisitor(VecI64Ref shape, VecI64Ref strides)
      : shape_(shape.begin(), shape.end()),
        strides_(strides.begin(), strides.end()), offsets_(shape.size(), 0) {}
  template <typename VecI64Ref>
  TensorVisitor(VecI64Ref shape, VecI64Ref strides, VecI64Ref offsets)
      : shape_(shape.begin(), shape.end()),
        strides_(strides.begin(), strides.end()),
        offsets_(offsets.begin(), offsets.end()) {}

  std::vector<int64_t> shape_;
  std::vector<int64_t> strides_;
  std::vector<int64_t> offsets_;
};

template <typename StorageT = SimpleTensorStorageBase<void>>
class TensorBaseImpl : public TensorVisitor {
  template <typename, typename> friend class Tensor;

public:
  TensorBaseImpl(void *data, size_t len)
      : TensorVisitor(std::vector<int64_t>{static_cast<int64_t>(len)}),
        storage_(data, len) {}

  template <typename T> auto cast() {
    return *reinterpret_cast<Tensor<T, SimpleTensorStorage<T>> *>(this);
  }

protected:
  TensorBaseImpl(StorageT storage) : storage_(storage) {}

private:
  StorageT storage_;
};

using TensorBase = TensorBaseImpl<>;

template <typename EleT, typename StorageT = SimpleTensorStorage<EleT>>
class Tensor : public TensorVisitor {
public:
  using element_type = EleT;
  enum DUMP_FMT { FMT_BIN, FMT_TXT };
  Tensor(StorageT storage)
      : TensorVisitor({storage.length}), storage_(storage) {}

  template <typename VecI64Ref>
  Tensor(VecI64Ref shape, VecI64Ref strides) : TensorVisitor(shape, strides) {}

  Tensor(const std::initializer_list<EleT> &list)
      : TensorVisitor(std::vector<int64_t>{static_cast<int64_t>(list.size())}) {
    storage_.alloc(list.size() * sizeof(EleT));
    auto ptr = storage_.getData();
    std::copy(list.begin(), list.end(), storage_.getData());
  }

  Tensor(const std::vector<EleT> &vectorData) {}

  template <typename... T> EleT &at(T... t) {
    size_t ofs = offset(t...);
    return *(storage_.getData() + ofs);
  }

  std::string toString() const {
    std::stringstream ss;
    for (size_t i = 0; i < totalElements(); ++i) {
      ss << storage_.getData()[i] << ",";
    }
    return ss.str();
  }

  void dumpToFile(const std::string &path, DUMP_FMT fmt) {
    std::ofstream ofs(path);
  }
  void loadFromFile() {}

private:
  StorageT storage_;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const Tensor<T> &tensor) {
  os << tensor.toString();
  return os;
}
