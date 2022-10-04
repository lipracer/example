#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

template <typename> struct SimpleTensorStorage;

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
  type *data() const { return reinterpret_cast<type *>(storage_.get()); }
  type *data() { return reinterpret_cast<type *>(storage_.get()); }
  size_t size() { return length_; }

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

  size_t offset() { return 0; }

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

protected:
  template <typename VecI64Ref>
  TensorVisitor(VecI64Ref shape)
      : shape_(shape.begin(), shape.end()), strides_(shape.size(), 0),
        offsets_(shape.size(), 0) {
    int64_t stride = 1;
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

  template <typename U> class FlattenIterator {
  public:
    using value_type = U;
    // std::bidirectional_iterator_tag
    FlattenIterator(Tensor<U> &tensor, const std::vector<int64_t> &offsets,
                    int64_t dim = -1)
        : tensor_(tensor), offsets_(offsets), curDim_(dim) {}
    FlattenIterator(const FlattenIterator<U> &) = default;
    FlattenIterator(FlattenIterator<U> &&) = default;
    FlattenIterator &operator=(const FlattenIterator<U> &) = default;
    FlattenIterator &operator=(FlattenIterator<U> &&) = default;

    FlattenIterator &operator++() {
      increase();
      return *this;
    }

    FlattenIterator operator++(int) {
      auto cur = *this;
      ++*this;
      return cur;
    }
    U &operator*() const { return *(this->operator->()); }

    U *operator->() const {
      size_t offset = std::inner_product(offsets_.begin(), offsets_.end(),
                                         tensor_.strides_.begin(), 0);
      return tensor_.storage_.data() + offset;
    }

    bool operator==(const FlattenIterator &other) const {
      return &tensor_ == &other.tensor_ && offsets_ == other.offsets_ &&
             curDim_ == other.curDim_;
    }

    bool operator!=(const FlattenIterator &other) const {
      return !(*this == other);
    }

  public:
    void increase() {
      while (curDim_>=0 && ++offsets_[curDim_] == tensor_.shape_[curDim_]) {
        assert(curDim_ >= 0 && curDim_ < offsets_.size());
        offsets_[curDim_] = 0;
        --curDim_;
      }
      if (curDim_ >= 0) {
        curDim_ = offsets_.size() - 1;
      }
    }

    Tensor<U> &tensor_;
    std::vector<int64_t> offsets_;
    int64_t curDim_;
  };

  Tensor(StorageT storage)
      : TensorVisitor({storage.length}), storage_(storage) {}

  Tensor(const std::vector<int64_t> &shape, const std::vector<int64_t> &strides,
         const std::vector<int64_t> &offsets)
      : TensorVisitor(shape, strides, offsets) {}

  Tensor(const std::initializer_list<EleT> &list)
      : TensorVisitor(std::vector<int64_t>{static_cast<int64_t>(list.size())}) {
    storage_.alloc(list.size() * sizeof(EleT));
    auto ptr = storage_.data();
    std::copy(list.begin(), list.end(), storage_.data());
  }

  explicit Tensor(const std::vector<EleT> &vectorData) {}

  Tensor(const Tensor<EleT, StorageT> &) = default;
  Tensor(Tensor<EleT, StorageT> &&) = default;
  Tensor<EleT, StorageT> &operator=(const Tensor<EleT, StorageT> &) = default;
  Tensor<EleT, StorageT> &operator=(Tensor<EleT, StorageT> &&) = default;

  template <typename... T> EleT &at(T... t) {
    size_t ofs = offset(t...);
    return *(storage_.data() + ofs);
  }

  template <typename VecI64Ref>
  Tensor<EleT> as_stride(VecI64Ref shape, VecI64Ref strides,
                         VecI64Ref offsets) {
    Tensor<EleT> result(shape, strides, offsets);
    result.storage_ = storage_;
    return result;
  }

  Tensor<EleT> flatten() {
    Tensor<EleT> result(shape_, {}, {});
    result.storage_.alloc(result.totalElements() * sizeof(EleT));
    auto ptr = result.storage_.data();
    for_each([&](EleT &data) { *ptr++ = data; });
    return result;
  }

  void for_each(std::function<void(EleT &)> func) {
    auto first = flatten_begin();
    auto last = flatten_end();
    while (first != last) {
      func(*first);
      ++first;
    }
  }

  FlattenIterator<EleT> flatten_begin() {
    return FlattenIterator<EleT>(*this, offsets_, offsets_.size() - 1);
  }
  FlattenIterator<EleT> flatten_end() {
    std::vector<int64_t> endOffsets(offsets_.size(), 0);
    return FlattenIterator<EleT>(*this, endOffsets, -1);
  }

  EleT *data() { return storage_.data(); }
  size_t storage_size() { return storage_.size(); }

  std::string toString() const {
    std::stringstream ss;
    auto printNewLine = [&](std::ostream &os, size_t index) {
      for (auto it = shape_.rbegin(); it != shape_.rend(); ++it) {
        if (!index) {
          return;
        }
        if (index % *it == 0) {
          index /= *it;
          os << "\n";
        } else {
          return;
        }
      }
    };
    for (size_t i = 0; i < totalElements(); ++i) {
      printNewLine(ss, i);
      ss << storage_.data()[i] << ",";
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
