#pragma once
// ================================================================================================
// -*- C++ -*-
// File: common.hpp
// Author: Guilherme R. Lampert
// Created on: 03/09/15
// Brief: Common functions and helpers for LibUtils.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include <cassert>
#include <cstdint>
#include <cstdarg>
#include <cstring>

#include <ostream>
#include <sstream>

#include <type_traits>
#include <exception>
#include <utility>

#include <string>
#include <vector>

// ========================================================

#ifdef __GNUC__
	#define FORMAT_FUNC_DECL(baseFunc, stringIndex, firstToCheck) __attribute__((format(baseFunc, stringIndex, firstToCheck)))
	#define PURE_FUNC_DECL __attribute__((pure))
#else // !__GNUC__
	#define FORMAT_FUNC_DECL(baseFunc, stringIndex, firstToCheck)
	#define PURE_FUNC_DECL
#endif // __GNUC__

// ========================================================

namespace utils
{

// ========================================================
// Various helper functions:
// ========================================================

// Max length of temporary stack allocated
// strings and strings built with `format()`.
constexpr int MaxTempStringLen = 2048;

// Length of statically allocated C++ arrays.
template<class T, size_t N>
constexpr size_t arrayLength(const T (&)[N]) noexcept
{
	return N;
}

// Zero fills a POD type, such as a C struct or union.
template<class T>
void clearPodObject(T & s) noexcept
{
	static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
	std::memset(&s, 0, sizeof(T));
}

// Zero fills a statically allocated array of POD or built-in types. Array length inferred by the compiler.
template<class T, size_t N>
void clearArray(T (&arr)[N]) noexcept
{
	static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
	std::memset(arr, 0, sizeof(T) * N);
}

// Zero fills an array of POD or built-in types, with array length provided by the caller.
template<class T>
void clearArray(T * arrayPtr, const size_t arrayLength) noexcept
{
	static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
	assert(arrayPtr != nullptr && arrayLength != 0);
	std::memset(arrayPtr, 0, sizeof(T) * arrayLength);
}

// Clamp any value within min/max range, inclusive.
template<class T>
constexpr T clamp(const T x, const T minimum, const T maximum) noexcept
{
	return (x < minimum) ? minimum : (x > maximum) ? maximum : x;
}

// Test if an integer is a power of two. Always false if input is negative.
template<class T>
constexpr bool isPowerOfTwo(const T x) noexcept
{
	return (x > 0) && ((x & (x - 1)) == 0);
}

// Marginally safer string formatting helpers.
std::string format(const char * format, ...) FORMAT_FUNC_DECL(printf, 1, 2);
std::string vformat(const char * format, va_list vaList) FORMAT_FUNC_DECL(printf, 1, 0);

// Trims a string representing a floating-point number to remove unnecessary trailing zeros.
std::string removeTrailingFloatZeros(const std::string & floatStr);

// Memory unit/size to printable string. Example "1 GB" or "1 Gigabyte", depending on 'abbreviated'.
std::string formatMemoryUnit(uint64_t memorySizeInBytes, bool abbreviated = false);

// Computes a CRC 32 for the given byte array. Pointer must not be null. `sizeBytes` must be nonzero.
uint32_t computeCrc32(const void * data, size_t sizeBytes) noexcept;

// ========================================================
// NonCopyable:
// ========================================================

// Any class/struct inheriting from this little
// helper won't be copyable nor movable.
class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable & operator = (const NonCopyable &) = delete;
};

} // namespace utils {}
