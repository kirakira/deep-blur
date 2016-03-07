#ifndef BLUR_COMMON_H
#define BLUR_COMMON_H

#include <array>
#include <cstdint>

namespace blur {

// using int64 = std::int64_t;
using uint64 = std::uint64_t;
using size_t = std::size_t;

// Return an constexpr array a initialized with a[i] = f(i) for 0 <= i < N.
template <typename T, size_t N, typename Function>
constexpr std::array<T, N> GenerateArray(Function f);

// Given a binary function f, return f(f(f(values[0], values[1]), values[2]),
// values[3]) ...
// For example, if f(a, b) returns a+b, then Aggregate(f, 1, 2, 3, 4, 5) will
// return 15.
template <typename Function, typename FirstType, typename... RemainingTypes>
constexpr auto Aggregate(Function f, FirstType value0,
                         RemainingTypes... values);

// Extract a given bit from an uint64.
constexpr int GetBit(uint64 x, int pos) { return (x >> pos) & 1; }

// Return an uint64 with specified bits set.
// For example, FillBits(1, 5, 17) gives (1 << 1) | (1 << 5) | (1 << 17).
template <typename... Ints>
constexpr uint64 FillBits(Ints... pos);

// Given a constexpr function f and a list of arguments, CurryFront returns a
// constexpr functor whose front parameters are bound with given arguments.
// Bound arguments are stored and passed by value; references are decayed. To
// pass by reference, use std::ref.
//
// For example,
//   constexpr int Sum(int x, int y) { return x + y; }
//   constexpr auto plus_five = CurryFront(Sum, 5);
//   constexpr int y = plus_five(9);   // y == 14.
//   constexpr int z = plus_five(-1);  // z == 4.
namespace impl {
template <typename Function, typename... CurryTypes>
class CurriedFunction {
 public:
  template <typename... UndecayedCurryTypes>
  constexpr CurriedFunction(Function f, UndecayedCurryTypes&&... curry)
      : f_(f), curry_(std::make_tuple(std::forward<CurryTypes>(curry)...)) {}

  template <typename... Params>
  constexpr auto operator()(Params&&... params) const {
    return InvokeHelper(std::make_index_sequence<sizeof...(CurryTypes)>(),
                        std::forward<Params>(params)...);
  }

  template <typename... Params>
  constexpr auto operator()(Params&&... params) {
    return InvokeHelper(std::make_index_sequence<sizeof...(CurryTypes)>(),
                        std::forward<Params>(params)...);
  }

 private:
  Function f_;
  std::tuple<CurryTypes...> curry_;

  template <size_t... curry_indices, typename... Params>
  constexpr auto InvokeHelper(std::index_sequence<curry_indices...>,
                              Params&&... params) const {
    return f_(std::get<curry_indices>(curry_)...,
              std::forward<Params>(params)...);
  }

  template <size_t... curry_indices, typename... Params>
  constexpr auto InvokeHelper(std::index_sequence<curry_indices...>,
                              Params&&... params) {
    return f_(std::get<curry_indices>(curry_)...,
              std::forward<Params>(params)...);
  }
};
}  // namespace impl

template <typename Function, typename... CurryTypes>
constexpr auto CurryFront(Function f, CurryTypes&&... curry) {
  return impl::CurriedFunction<Function, std::decay_t<CurryTypes>...>(
      f, std::forward<CurryTypes>(curry)...);
}

// Below are implementation details.
namespace impl {
template <typename T, typename Function, size_t... index>
constexpr std::array<T, sizeof...(index)> GenerateArrayHelper(
    Function f, std::index_sequence<index...>) {
  return {{f(index)...}};
}
}  // namespace impl

template <typename T, size_t N, typename Function>
constexpr std::array<T, N> GenerateArray(Function f) {
  return impl::GenerateArrayHelper<T>(f, std::make_index_sequence<N>());
}

namespace impl {
template <typename Function, typename FirstType>
constexpr auto AggregateHelper(Function, FirstType value0) {
  return value0;
}

template <typename Function, typename FirstType, typename SecondType,
          typename... RemainingTypes>
constexpr auto AggregateHelper(Function f, FirstType value0, SecondType value1,
                               RemainingTypes... values) {
  return AggregateHelper(f, f(value0, value1), values...);
}
}  // namespace impl

template <typename Function, typename FirstType, typename... RemainingTypes>
constexpr auto Aggregate(Function f, FirstType value0,
                         RemainingTypes... values) {
  return impl::AggregateHelper(f, value0, values...);
}

namespace impl {
constexpr uint64 BinaryOr(uint64 x, uint64 y) { return x | y; }
}  // namespace impl

template <typename... Ints>
constexpr uint64 FillBits(Ints... pos) {
  return impl::AggregateHelper(impl::BinaryOr, 0,
                               (static_cast<uint64>(1) << pos)...);
}

}  // namespace blur

#endif  // BLUR_COMMON_H
