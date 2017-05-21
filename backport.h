#pragma once

#include <functional>
#include <assert.h>

namespace nonstd
{

template<class T, class Compare>
constexpr const T &clamp (const T &v, const T &lo, const T &hi, Compare comp)
{
  return assert (!comp (hi, lo) ),
         comp (v, lo) ? lo : comp (hi, v) ? hi : v;
}

template<class T>
constexpr const T &clamp (const T &v, const T &lo, const T &hi)
{
  return clamp (v, lo, hi, std::less<T>() );
}

template< class C >
constexpr auto cbegin (const C &c)->decltype(std::begin (c))
{
  return std::begin (c);
}

template< class C >
constexpr auto cend (const C &c)->decltype(std::end (c))
{
  return std::end (c);
}

template <class T>
constexpr typename std::add_const<T>::type & as_const (T & t) noexcept
{
  return t;
}

template <class T>
void as_const (const T &&) = delete;

}
