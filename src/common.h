#ifndef BLUR_COMMON_H
#define BLUR_COMMON_H

#include <array>
#include <cstdint>

namespace blur {

using int64 = std::int64_t;
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
constexpr FirstType Aggregate(Function f, FirstType value0,
                              RemainingTypes... values);

// Below are implementation details.
namespace {
template <typename T, typename Function, size_t... index>
constexpr std::array<T, sizeof...(index)> GenerateArrayHelper(
    Function f, std::index_sequence<index...>) {
  return {{f(index)...}};
}
}  // namespace

template <typename T, size_t N, typename Function>
constexpr std::array<T, N> GenerateArray(Function f) {
  return GenerateArrayHelper<T>(f, std::make_index_sequence<N>());
}

namespace {
template <typename Function, typename FirstType>
constexpr FirstType AggregateHelper(Function, FirstType value0) {
  return value0;
}

template <typename Function, typename FirstType, typename SecondType,
          typename... RemainingTypes>
constexpr FirstType AggregateHelper(Function f, FirstType value0,
                                    SecondType value1,
                                    RemainingTypes... values) {
  return AggregateHelper(f, f(value0, value1), values...);
}
}  // namespace

template <typename Function, typename FirstType, typename... RemainingTypes>
constexpr FirstType Aggregate(Function f, FirstType value0,
                              RemainingTypes... values) {
  return AggregateHelper(f, value0, values...);
}

}  // namespace blur

#endif  // BLUR_COMMON_H
