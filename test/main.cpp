#include <iostream>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <regex>

template <typename... T>
struct TypeSequence {
  constexpr static size_t size = sizeof...(T);
};

template <typename V, typename F, typename... R>
struct Find {
  constexpr static size_t value =
      Find<V, std::integral_constant<bool, std::is_same<V, F>::value>,
           R...>::value;
  using type = std::integral_constant<size_t, sizeof...(R) - value>;
};

template <typename V, typename... R>
struct Find<V, std::true_type, R...> {
  constexpr static size_t value = sizeof...(R);
};

template <typename V, typename... R>
struct Find<V, std::false_type, R...> {
  constexpr static size_t value = Find<V, R...>::value;
};

template <typename V>
struct Find<V, std::false_type> {
  constexpr static size_t value = 0;
};

template <typename... LHS, typename... RHS,
          template <typename... T> typename Seq>
constexpr static auto ConcatSeq(Seq<LHS...>, Seq<RHS...>) {
  return Seq<LHS..., RHS...>();
}

template <template <typename L, typename R> typename CMP, typename S>
struct Partial;

template <template <typename L, typename R> typename CMP,
          template <typename... T> typename S, typename... T>
struct Partial<CMP, S<T...>> {
  using last_type =
      typename std::tuple_element<sizeof...(T) - 1, std::tuple<T...>>::type;

  template <typename SS, typename L = S<>, typename R = S<>>
  struct PartialImpl;

  template <typename F, typename... N, typename L, typename R>
  struct PartialImpl<S<F, N...>, L, R> {
    using true_type =
        typename std::conditional<CMP<F, last_type>::value && sizeof...(N) != 0,
                                  S<F>, S<>>::type;
    using false_type = typename std::conditional<
        !CMP<F, last_type>::value && sizeof...(N) != 0, S<F>, S<>>::type;
    using current_lhs_type = decltype(ConcatSeq(L(), true_type()));
    using current_rhs_type = decltype(ConcatSeq(R(), false_type()));

    using lhs_type = typename PartialImpl<S<N...>, current_lhs_type,
                                          current_rhs_type>::lhs_type;
    using rhs_type = typename PartialImpl<S<N...>, current_lhs_type,
                                          current_rhs_type>::rhs_type;
  };

  template <typename L, typename R>
  struct PartialImpl<S<>, L, R> {
    using lhs_type = L;
    using rhs_type = R;
  };

  using lhs_type = typename PartialImpl<S<T...>>::lhs_type;
  using rhs_type = typename PartialImpl<S<T...>>::rhs_type;
  using mid_type = S<last_type>;
};

template <template <typename L, typename R> typename CMP,
          template <typename... T> typename S, typename T>
struct Partial<CMP, S<T>> {
  using lhs_type = S<>;
  using rhs_type = S<>;
  using mid_type = S<T>;
};

template <template <typename L, typename R> typename CMP,
          template <typename... T> typename S>
struct Partial<CMP, S<>> {
  using lhs_type = S<>;
  using rhs_type = S<>;
  using mid_type = S<>;
};

template <template <typename L, typename R> typename CMP, typename S>
struct QuickSort {
  using lhs_type =
      typename QuickSort<CMP, typename Partial<CMP, S>::lhs_type>::type;
  using rhs_type =
      typename QuickSort<CMP, typename Partial<CMP, S>::rhs_type>::type;
  using mid_type = typename Partial<CMP, S>::mid_type;
  using type =
      decltype(ConcatSeq(ConcatSeq(lhs_type(), mid_type()), rhs_type()));
};

template <template <typename L, typename R> typename CMP,
          template <typename T> typename S, typename T>
struct QuickSort<CMP, S<T>> {
  using type = S<T>;
};

template <template <typename L, typename R> typename CMP,
          template <typename... T> typename S>
struct QuickSort<CMP, S<>> {
  using type = S<>;
};
template <typename Lhs, typename Rhs>
struct Less {
  constexpr static bool value = Lhs::value < Rhs::value;
};
template <typename... T>
void print(std::tuple<T...>) {
  std::initializer_list<int> aa = {(std::cout << T::value << ", ", 0)...};
  std::cout << std::endl;
}

int main(int argc, char** args) {
  using result = QuickSort<
      Less,
      std::tuple<
          std::integral_constant<size_t, 7>, std::integral_constant<size_t, 2>,
          std::integral_constant<size_t, 1>, std::integral_constant<size_t, 3>,
          std::integral_constant<size_t, 9>, std::integral_constant<size_t, 8>,
          std::integral_constant<size_t, 5>, std::integral_constant<size_t, 4>,
          std::integral_constant<size_t, 6>>>::type;
  print(result());
  return 0;
}
