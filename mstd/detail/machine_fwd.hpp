#pragma once

namespace mstd { namespace detail {

template<size_t size>
struct size_to_int;

template<> struct size_to_int<1> { typedef boost::uint8_t type; };
template<> struct size_to_int<2> { typedef boost::uint16_t type; };
template<> struct size_to_int<4> { typedef boost::uint32_t type; };
template<> struct size_to_int<8> { typedef boost::uint64_t type; };

template<size_t size>
typename size_to_int<size>::type atomic_add(volatile void *, typename size_to_int<size>::type);

template<size_t size>
typename size_to_int<size>::type atomic_read_write(volatile void *, typename size_to_int<size>::type);

template<size_t size>
typename size_to_int<size>::type atomic_cas(volatile void *, typename size_to_int<size>::type, typename size_to_int<size>::type);

template<size_t size>
typename size_to_int<size>::type atomic_read(const volatile void *);

template<size_t size>
void atomic_write(volatile void *, typename size_to_int<size>::type);

template<size_t S, class F>
typename size_to_int<S>::type generic_modify(volatile void *ptr, F f);

} }
