
// ================================================================================================
// -*- C++ -*-
// File: vectors.hpp
// Author: Guilherme R. Lampert
// Created on: 03/09/15
// Brief: Vector math types: Vec2, Vec3, Vec4.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#ifndef UTILS_VECTORS_HPP
#define UTILS_VECTORS_HPP

#include "utils/common.hpp"
#include <cmath>

namespace utils
{

//
// Since the vector types are header-only templates,
// this namespace hides a few implementation details
// that are not part of the public interface. The
// vector types are then imported into the utils
// namespace at the end of this file.
//
namespace internal
{

// ========================================================
// template struct Vec2Impl<T>:
// ========================================================

template<class T>
struct Vec2Impl final
{
	// Scalar components:
	T x;
	T y;

	// Default constructor leaves uninitialized.
	Vec2Impl();

	// Construct with explicit values.
	Vec2Impl(T xx, T yy);

	// Replicate `val` to all elements.
	explicit Vec2Impl(T val);

	// Construct with array of values. Size must match vector size.
	explicit Vec2Impl(const T array[]);

	// Set each element:
	void set(T xx, T yy);
	void setZero();

	// Pointer to the start of this vector.
	T * getData();
	const T * getData() const;

	// Operators:
	T &        operator [] (uint index);
	T          operator [] (uint index) const;
	Vec2Impl   operator  + (const Vec2Impl & other) const;
	Vec2Impl   operator  - (const Vec2Impl & other) const;
	Vec2Impl   operator  * (T scalar) const;
	Vec2Impl   operator  / (T scalar) const;
	Vec2Impl & operator += (const Vec2Impl & other);
	Vec2Impl & operator -= (const Vec2Impl & other);
	Vec2Impl & operator *= (T scalar);
	Vec2Impl & operator /= (T scalar);
	Vec2Impl   operator  - () const;

	// Vector length:
	T length()    const; // Length of vector.
	T sqrLength() const; // Squared length.
	T invLength() const; // Inverse length (1/length).

	// Normalization:
	Vec2Impl & normalize();        // Normalize this vector. Returns a reference to self.
	Vec2Impl   normalized() const; // Returns a normalized copy of this vector.

	// Distance (and squared distance) between this vector and another.
	T distance(const Vec2Impl & other) const;
	T sqrDistance(const Vec2Impl & other) const;

	// Dot product between this vector and another.
	T dot(const Vec2Impl & other) const;

	// Vector comparison:
	bool equals(const Vec2Impl & other) const;
	bool equals(const Vec2Impl & other, T tolerance) const;

	// Interpolation (`t` is not clamped to 0,1):
	Vec2Impl & lerp(const Vec2Impl & other, T t);

	// Built-in vectors:
	static Vec2Impl origin();
	static Vec2Impl unitX();
	static Vec2Impl unitY();
	static Vec2Impl positiveInf();
	static Vec2Impl negativeInf();

	// Dimensions of this vector (2).
	static constexpr uint getDimensions() { return 2; }
};

// ========================================================
// template struct Vec3Impl<T>:
// ========================================================

template<class T>
struct Vec3Impl final
{
	// Scalar components:
	T x;
	T y;
	T z;

	// Default constructor leaves uninitialized.
	Vec3Impl();

	// Construct with explicit values.
	Vec3Impl(T xx, T yy, T zz);

	// Replicate `val` to all elements.
	explicit Vec3Impl(T val);

	// Construct with array of values. Size must match vector size.
	explicit Vec3Impl(const T array[]);

	// Construct from Vec2.
	explicit Vec3Impl(const Vec2Impl<T> & v2, T zz = T(0));

	// Set each element:
	void set(T xx, T yy, T zz);
	void setZero();

	// Pointer to the start of this vector.
	T * getData();
	const T * getData() const;

	// Returns a 2D vector with the x,y elements of this 3D vector.
	Vec2Impl<T> getVec2() const;

	// Operators:
	T &        operator [] (uint index);
	T          operator [] (uint index) const;
	Vec3Impl   operator  + (const Vec3Impl & other) const;
	Vec3Impl   operator  - (const Vec3Impl & other) const;
	Vec3Impl   operator  * (T scalar) const;
	Vec3Impl   operator  / (T scalar) const;
	Vec3Impl & operator += (const Vec3Impl & other);
	Vec3Impl & operator -= (const Vec3Impl & other);
	Vec3Impl & operator *= (T scalar);
	Vec3Impl & operator /= (T scalar);
	Vec3Impl   operator  - () const;

	// Vector length:
	T length()    const; // Length of vector.
	T sqrLength() const; // Squared length.
	T invLength() const; // Inverse length (1/length).

	// Normalization:
	Vec3Impl & normalize();        // Normalize this vector. Returns a reference to self.
	Vec3Impl   normalized() const; // Returns a normalized copy of this vector.

	// Distance (and squared distance) between this vector and another.
	T distance(const Vec3Impl & other) const;
	T sqrDistance(const Vec3Impl & other) const;

	// Dot product between this vector and another.
	T dot(const Vec3Impl & other) const;

	// Cross product for 3D vectors:
	Vec3Impl   cross(const Vec3Impl & other) const;
	Vec3Impl & cross(const Vec3Impl & a, const Vec3Impl & b);

	// Compute orthogonal complements to this vector:
	// (`this` should be normalized first).
	void normalVectors(Vec3Impl & left, Vec3Impl & down) const;
	void orthogonalBasis(Vec3Impl & left, Vec3Impl & up) const;

	// Vector comparison:
	bool equals(const Vec3Impl & other) const;
	bool equals(const Vec3Impl & other, T tolerance) const;

	// Interpolation (`t` is not clamped to 0,1):
	Vec3Impl & lerp(const Vec3Impl & other, T t);

	// Built-in vectors:
	static Vec3Impl origin();
	static Vec3Impl unitX();
	static Vec3Impl unitY();
	static Vec3Impl unitZ();
	static Vec3Impl positiveInf();
	static Vec3Impl negativeInf();

	// Dimensions of this vector (3).
	static constexpr uint getDimensions() { return 3; }
};

// ========================================================
// template struct Vec4Impl<T>:
// ========================================================

template<class T>
struct Vec4Impl final
{
	// Scalar components:
	T x;
	T y;
	T z;
	T w;

	// Default constructor leaves uninitialized.
	Vec4Impl();

	// Construct with explicit values.
	Vec4Impl(T xx, T yy, T zz, T ww);

	// Replicate `val` to all elements.
	explicit Vec4Impl(T val);

	// Construct with array of values. Size must match vector size.
	explicit Vec4Impl(const T array[]);

	// Construct from Vec2.
	explicit Vec4Impl(const Vec2Impl<T> & v2, T zz = T(0), T ww = T(0));

	// Construct from Vec3:
	explicit Vec4Impl(const Vec3Impl<T> & v3, T ww = T(0));

	// Set each element:
	void set(T xx, T yy, T zz, T ww);
	void setZero();

	// Set the xyz components but leave w untouched.
	void setXYZ(const Vec3Impl<T> & xyz);

	// Set the xyz components and the w.
	void setXYZ(const Vec3Impl<T> & xyz, T ww);

	// Pointer to the start of this vector.
	T * getData();
	const T * getData() const;

	// Returns a 2D vector with the x,y elements of this 4D vector.
	Vec2Impl<T> getVec2() const;

	// Returns a 3D vector with the x,y,z elements of this 4D vector.
	Vec3Impl<T> getVec3() const;

	// Operators:
	T &        operator [] (uint index);
	T          operator [] (uint index) const;
	Vec4Impl   operator  + (const Vec4Impl & other) const;
	Vec4Impl   operator  - (const Vec4Impl & other) const;
	Vec4Impl   operator  * (T scalar) const;
	Vec4Impl   operator  / (T scalar) const;
	Vec4Impl & operator += (const Vec4Impl & other);
	Vec4Impl & operator -= (const Vec4Impl & other);
	Vec4Impl & operator *= (T scalar);
	Vec4Impl & operator /= (T scalar);
	Vec4Impl   operator  - () const;

	// Vector length:
	T length()    const; // Length of vector.
	T sqrLength() const; // Squared length.
	T invLength() const; // Inverse length (1/length).

	// Normalization:
	Vec4Impl & normalize();        // Normalize this vector. Returns a reference to self.
	Vec4Impl   normalized() const; // Returns a normalized copy of this vector.

	// Distance (and squared distance) between this vector and another.
	T distance(const Vec4Impl & other) const;
	T sqrDistance(const Vec4Impl & other) const;

	// Dot product between this vector and another (using XYZW).
	T dot(const Vec4Impl & other) const;

	// Dot product between this vector and another (using XYZ only).
	T dot3(const Vec3Impl<T> & other) const;

	// Vector comparison:
	bool equals(const Vec4Impl & other) const;
	bool equals(const Vec4Impl & other, T tolerance) const;

	// Interpolation (`t` is not clamped to 0,1):
	Vec4Impl & lerp(const Vec4Impl & other, T t);

	// Built-in vectors:
	static Vec4Impl origin();
	static Vec4Impl unitX();
	static Vec4Impl unitY();
	static Vec4Impl unitZ();
	static Vec4Impl unitW();
	static Vec4Impl positiveInf();
	static Vec4Impl negativeInf();

	// Dimensions of this vector (4).
	static constexpr uint getDimensions() { return 4; }
};

// ========================================================
// Internal helpers:
// ========================================================

// Overloads for generic type T are left unimplemented.
// Will fail to link if we are missing a template specialization.
template<class T> inline T getPositiveInfinity();
template<class T> inline T getNegativeInfinity();

// Specializations for our sized vector types:
template<> inline float    getPositiveInfinity() { return +INFINITY;  } // VecX
template<> inline float    getNegativeInfinity() { return -INFINITY;  } // VecX
template<> inline int32_t  getPositiveInfinity() { return INT32_MAX;  } // VecXi
template<> inline int32_t  getNegativeInfinity() { return INT32_MIN;  } // VecXi
template<> inline uint32_t getPositiveInfinity() { return UINT32_MAX; } // VecXu
template<> inline uint32_t getNegativeInfinity() { return UINT32_MAX; } // VecXu
template<> inline uint8_t  getPositiveInfinity() { return UINT8_MAX;  } // VecXb
template<> inline uint8_t  getNegativeInfinity() { return UINT8_MAX;  } // VecXb

inline float abs(const float x) noexcept
{
	return std::fabs(x);
}

template<class T> inline T abs(const T x) noexcept
{
	return (x < T(0)) ? -x : x;
}

// ================================================================================================
// Vec2Impl inline methods:
// ================================================================================================

template<class T>
Vec2Impl<T>::Vec2Impl()
{
	// Uninitialized by default.
}

template<class T>
Vec2Impl<T>::Vec2Impl(const T xx, const T yy)
	: x(xx), y(yy)
{
}

template<class T>
Vec2Impl<T>::Vec2Impl(const T val)
	: x(val), y(val)
{
}

template<class T>
Vec2Impl<T>::Vec2Impl(const T array[])
	: x(array[0]), y(array[1])
{
}

template<class T>
void Vec2Impl<T>::set(const T xx, const T yy)
{
	x = xx;
	y = yy;
}

template<class T>
void Vec2Impl<T>::setZero()
{
	x = T(0);
	y = T(0);
}

template<class T>
T * Vec2Impl<T>::getData()
{
	return reinterpret_cast<T *>(this);
}

template<class T>
const T * Vec2Impl<T>::getData() const
{
	return reinterpret_cast<const T *>(this);
}

template<class T>
T & Vec2Impl<T>::operator [] (const uint index)
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<T *>(this)[index];
}

template<class T>
T Vec2Impl<T>::operator [] (const uint index) const
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<const T *>(this)[index];
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::operator + (const Vec2Impl<T> & other) const
{
	return Vec2Impl(x + other.x, y + other.y);
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::operator - (const Vec2Impl<T> & other) const
{
	return Vec2Impl(x - other.x, y - other.y);
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::operator * (const T scalar) const
{
	return Vec2Impl(x * scalar, y * scalar);
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::operator / (const T scalar) const
{
	return Vec2Impl(x / scalar, y / scalar);
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::operator += (const Vec2Impl<T> & other)
{
	x += other.x;
	y += other.y;
	return *this;
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::operator -= (const Vec2Impl<T> & other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::operator *= (const T scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::operator /= (const T scalar)
{
	x /= scalar;
	y /= scalar;
	return *this;
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::operator - () const
{
	return Vec2Impl(-x, -y);
}

template<class T>
Vec2Impl<T> operator * (const T scalar, const Vec2Impl<T> & vec)
{
	return Vec2Impl<T>(scalar * vec.x, scalar * vec.y);
}

template<class T>
Vec2Impl<T> operator / (const T scalar, const Vec2Impl<T> & vec)
{
	return Vec2Impl<T>(scalar / vec.x, scalar / vec.y);
}

template<class T>
T Vec2Impl<T>::length() const
{
	return T(std::sqrt((x * x) + (y * y)));
}

template<class T>
T Vec2Impl<T>::sqrLength() const
{
	return (x * x) + (y * y);
}

template<class T>
T Vec2Impl<T>::invLength() const
{
	return T(1.0 / std::sqrt((x * x) + (y * y)));
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::normalize()
{
	const T invLen = invLength();
	x *= invLen;
	y *= invLen;
	return *this;
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::normalized() const
{
	Vec2Impl v(*this);
	v.normalize();
	return v;
}

template<class T>
T Vec2Impl<T>::distance(const Vec2Impl<T> & other) const
{
	return ((*this) - other).length();
}

template<class T>
T Vec2Impl<T>::sqrDistance(const Vec2Impl<T> & other) const
{
	return ((*this) - other).sqrLength();
}

template<class T>
T Vec2Impl<T>::dot(const Vec2Impl<T> & other) const
{
	return (x * other.x) + (y * other.y);
}

template<class T>
bool Vec2Impl<T>::equals(const Vec2Impl<T> & other) const
{
	return x == other.x && y == other.y;
}

template<class T>
bool Vec2Impl<T>::equals(const Vec2Impl<T> & other, const T tolerance) const
{
	if (abs(x - other.x) > tolerance) { return false; }
	if (abs(y - other.y) > tolerance) { return false; }
	return true;
}

template<class T>
Vec2Impl<T> & Vec2Impl<T>::lerp(const Vec2Impl<T> & other, const T t)
{
	x = x + t * (other.x - x);
	y = y + t * (other.y - y);
	return *this;
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::origin()
{
	return Vec2Impl(T(0));
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::unitX()
{
	return Vec2Impl(T(1), T(0));
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::unitY()
{
	return Vec2Impl(T(0), T(1));
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::positiveInf()
{
	return Vec2Impl(getPositiveInfinity<T>());
}

template<class T>
Vec2Impl<T> Vec2Impl<T>::negativeInf()
{
	return Vec2Impl(getNegativeInfinity<T>());
}

//
// Overload min/max/clamp for Vec2Impl (element wise):
//

template<class T>
Vec2Impl<T> clamp(Vec2Impl<T> v, const Vec2Impl<T> & minimum, const Vec2Impl<T> & maximum)
{
	if      (v.x < minimum.x) { v.x = minimum.x; }
	else if (v.x > maximum.x) { v.x = maximum.x; }
	if      (v.y < minimum.y) { v.y = minimum.y; }
	else if (v.y > maximum.y) { v.y = maximum.y; }
	return v;
}

template<class T>
Vec2Impl<T> min(const Vec2Impl<T> & a, const Vec2Impl<T> & b)
{
	return Vec2Impl<T>(std::min(a.x, b.x), std::min(a.y, b.y));
}

template<class T>
Vec2Impl<T> max(const Vec2Impl<T> & a, const Vec2Impl<T> & b)
{
	return Vec2Impl<T>(std::max(a.x, b.x), std::max(a.y, b.y));
}

//
// Vector to string conversions:
//
template<class T>
inline std::string toString(const Vec2Impl<T> & v)
{
	return "[" +
		std::to_string(v.x) + ", " +
		std::to_string(v.y) + "]";
}

//
// Friendlier typedefs for Vec2Impl:
//

typedef Vec2Impl<float>    Vec2;
typedef Vec2Impl<int32_t>  Vec2i;
typedef Vec2Impl<uint32_t> Vec2u;
typedef Vec2Impl<uint8_t>  Vec2b;

static_assert(sizeof(Vec2)  == sizeof(float)    * 2, "Bad size for Vec2!");
static_assert(sizeof(Vec2i) == sizeof(int32_t)  * 2, "Bad size for Vec2i!");
static_assert(sizeof(Vec2u) == sizeof(uint32_t) * 2, "Bad size for Vec2u!");
static_assert(sizeof(Vec2b) == sizeof(uint8_t)  * 2, "Bad size for Vec2b!");

// ================================================================================================
// Vec3Impl inline methods:
// ================================================================================================

template<class T>
Vec3Impl<T>::Vec3Impl()
{
	// Uninitialized by default.
}

template<class T>
Vec3Impl<T>::Vec3Impl(const T xx, const T yy, const T zz)
	: x(xx), y(yy), z(zz)
{
}

template<class T>
Vec3Impl<T>::Vec3Impl(const T val)
	: x(val), y(val), z(val)
{
}

template<class T>
Vec3Impl<T>::Vec3Impl(const T array[])
	: x(array[0]), y(array[1]), z(array[2])
{
}

template<class T>
Vec3Impl<T>::Vec3Impl(const Vec2Impl<T> & v2, const T zz)
	: x(v2.x), y(v2.y), z(zz)
{
}

template<class T>
void Vec3Impl<T>::set(const T xx, const T yy, const T zz)
{
	x = xx;
	y = yy;
	z = zz;
}

template<class T>
void Vec3Impl<T>::setZero()
{
	x = T(0);
	y = T(0);
	z = T(0);
}

template<class T>
T * Vec3Impl<T>::getData()
{
	return reinterpret_cast<T *>(this);
}

template<class T>
const T * Vec3Impl<T>::getData() const
{
	return reinterpret_cast<const T *>(this);
}

template<class T>
Vec2Impl<T> Vec3Impl<T>::getVec2() const
{
	return Vec2Impl<T>(x, y);
}

template<class T>
T & Vec3Impl<T>::operator [] (const uint index)
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<T *>(this)[index];
}

template<class T>
T Vec3Impl<T>::operator [] (const uint index) const
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<const T *>(this)[index];
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::operator + (const Vec3Impl<T> & other) const
{
	return Vec3Impl(x + other.x, y + other.y, z + other.z);
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::operator - (const Vec3Impl<T> & other) const
{
	return Vec3Impl(x - other.x, y - other.y, z - other.z);
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::operator * (const T scalar) const
{
	return Vec3Impl(x * scalar, y * scalar, z * scalar);
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::operator / (const T scalar) const
{
	return Vec3Impl(x / scalar, y / scalar, z / scalar);
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::operator += (const Vec3Impl<T> & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::operator -= (const Vec3Impl<T> & other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::operator *= (const T scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::operator /= (const T scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::operator - () const
{
	return Vec3Impl(-x, -y, -z);
}

template<class T>
Vec3Impl<T> operator * (const T scalar, const Vec3Impl<T> & vec)
{
	return Vec3Impl<T>(scalar * vec.x, scalar * vec.y, scalar * vec.z);
}

template<class T>
Vec3Impl<T> operator / (const T scalar, const Vec3Impl<T> & vec)
{
	return Vec3Impl<T>(scalar / vec.x, scalar / vec.y, scalar / vec.z);
}

template<class T>
T Vec3Impl<T>::length() const
{
	return T(std::sqrt((x * x) + (y * y) + (z * z)));
}

template<class T>
T Vec3Impl<T>::sqrLength() const
{
	return (x * x) + (y * y) + (z * z);
}

template<class T>
T Vec3Impl<T>::invLength() const
{
	return T(1.0 / std::sqrt((x * x) + (y * y) + (z * z)));
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::normalize()
{
	const T invLen = invLength();
	x *= invLen;
	y *= invLen;
	z *= invLen;
	return *this;
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::normalized() const
{
	Vec3Impl v(*this);
	v.normalize();
	return v;
}

template<class T>
T Vec3Impl<T>::distance(const Vec3Impl<T> & other) const
{
	return ((*this) - other).length();
}

template<class T>
T Vec3Impl<T>::sqrDistance(const Vec3Impl<T> & other) const
{
	return ((*this) - other).sqrLength();
}

template<class T>
T Vec3Impl<T>::dot(const Vec3Impl<T> & other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z);
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::cross(const Vec3Impl<T> & other) const
{
	return Vec3Impl(
		(y * other.z) - (z * other.y),
		(z * other.x) - (x * other.z),
		(x * other.y) - (y * other.x));
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::cross(const Vec3Impl<T> & a, const Vec3Impl<T> & b)
{
	x = (a.y * b.z) - (a.z * b.y);
	y = (a.z * b.x) - (a.x * b.z);
	z = (a.x * b.y) - (a.y * b.x);
	return *this;
}

template<class T>
void Vec3Impl<T>::normalVectors(Vec3Impl<T> & left, Vec3Impl<T> & down) const
{
	T d = (x * x) + (y * y);
	if (d == T(0))
	{
		left.x = T(1);
		left.y = T(0);
		left.z = T(0);
	}
	else
	{
		d = T(1.0 / std::sqrt(d));
		left.x = -y * d;
		left.y =  x * d;
		left.z =  T(0);
	}
	down = left.cross(*this);
}

template<class T>
void Vec3Impl<T>::orthogonalBasis(Vec3Impl<T> & left, Vec3Impl<T> & up) const
{
	float l, s;
	if (abs(z) > 0.7)
	{
		l = (y * y) + (z * z);
		s = 1.0 / std::sqrt(l);

		up.x = T(0);
		up.y = T( z * s);
		up.z = T(-y * s);

		left.x = T( l * s);
		left.y = T(-x * up.z);
		left.z = T( x * up.y);
	}
	else
	{
		l = (x * x) + (y * y);
		s = 1.0 / std::sqrt(l);

		left.x = T(-y * s);
		left.y = T( x * s);
		left.z = T(0);

		up.x = T(-z * left.y);
		up.y = T( z * left.x);
		up.z = T( l * s);
	}
}

template<class T>
bool Vec3Impl<T>::equals(const Vec3Impl<T> & other) const
{
	return x == other.x && y == other.y && z == other.z;
}

template<class T>
bool Vec3Impl<T>::equals(const Vec3Impl<T> & other, const T tolerance) const
{
	if (abs(x - other.x) > tolerance) { return false; }
	if (abs(y - other.y) > tolerance) { return false; }
	if (abs(z - other.z) > tolerance) { return false; }
	return true;
}

template<class T>
Vec3Impl<T> & Vec3Impl<T>::lerp(const Vec3Impl<T> & other, const T t)
{
	x = x + t * (other.x - x);
	y = y + t * (other.y - y);
	z = z + t * (other.z - z);
	return *this;
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::origin()
{
	return Vec3Impl(T(0));
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::unitX()
{
	return Vec3Impl(T(1), T(0), T(0));
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::unitY()
{
	return Vec3Impl(T(0), T(1), T(0));
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::unitZ()
{
	return Vec3Impl(T(0), T(0), T(1));
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::positiveInf()
{
	return Vec3Impl(getPositiveInfinity<T>());
}

template<class T>
Vec3Impl<T> Vec3Impl<T>::negativeInf()
{
	return Vec3Impl(getNegativeInfinity<T>());
}

//
// Overload min/max/clamp for Vec3Impl (element wise):
//

template<class T>
Vec3Impl<T> clamp(Vec3Impl<T> v, const Vec3Impl<T> & minimum, const Vec3Impl<T> & maximum)
{
	if      (v.x < minimum.x) { v.x = minimum.x; }
	else if (v.x > maximum.x) { v.x = maximum.x; }
	if      (v.y < minimum.y) { v.y = minimum.y; }
	else if (v.y > maximum.y) { v.y = maximum.y; }
	if      (v.z < minimum.z) { v.z = minimum.z; }
	else if (v.z > maximum.z) { v.z = maximum.z; }
	return v;
}

template<class T>
Vec3Impl<T> min(const Vec3Impl<T> & a, const Vec3Impl<T> & b)
{
	return Vec3Impl<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

template<class T>
Vec3Impl<T> max(const Vec3Impl<T> & a, const Vec3Impl<T> & b)
{
	return Vec3Impl<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

//
// Vector to string conversions:
//
template<class T>
inline std::string toString(const Vec3Impl<T> & v)
{
	return "[" +
		std::to_string(v.x) + ", " +
		std::to_string(v.z) + ", " +
		std::to_string(v.y) + "]";
}

//
// Friendlier typedefs for Vec3Impl:
//

typedef Vec3Impl<float>    Vec3;
typedef Vec3Impl<int32_t>  Vec3i;
typedef Vec3Impl<uint32_t> Vec3u;
typedef Vec3Impl<uint8_t>  Vec3b;

static_assert(sizeof(Vec3)  == sizeof(float)    * 3, "Bad size for Vec3!");
static_assert(sizeof(Vec3i) == sizeof(int32_t)  * 3, "Bad size for Vec3i!");
static_assert(sizeof(Vec3u) == sizeof(uint32_t) * 3, "Bad size for Vec3u!");
static_assert(sizeof(Vec3b) == sizeof(uint8_t)  * 3, "Bad size for Vec3b!");

// ================================================================================================
// Vec4Impl inline methods:
// ================================================================================================

template<class T>
Vec4Impl<T>::Vec4Impl()
{
	// Uninitialized by default.
}

template<class T>
Vec4Impl<T>::Vec4Impl(const T xx, const T yy, const T zz, const T ww)
	: x(xx), y(yy), z(zz), w(ww)
{
}

template<class T>
Vec4Impl<T>::Vec4Impl(const T val)
	: x(val), y(val), z(val), w(val)
{
}

template<class T>
Vec4Impl<T>::Vec4Impl(const T array[])
	: x(array[0]), y(array[1]), z(array[2]), w(array[3])
{
}

template<class T>
Vec4Impl<T>::Vec4Impl(const Vec2Impl<T> & v2, const T zz, const T ww)
	: x(v2.x), y(v2.y), z(zz), w(ww)
{
}

template<class T>
Vec4Impl<T>::Vec4Impl(const Vec3Impl<T> & v3, const T ww)
	: x(v3.x), y(v3.y), z(v3.z), w(ww)
{
}

template<class T>
void Vec4Impl<T>::set(const T xx, const T yy, const T zz, const T ww)
{
	x = xx;
	y = yy;
	z = zz;
	w = ww;
}

template<class T>
void Vec4Impl<T>::setZero()
{
	x = T(0);
	y = T(0);
	z = T(0);
	w = T(0);
}

template<class T>
void Vec4Impl<T>::setXYZ(const Vec3Impl<T> & xyz)
{
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
}

template<class T>
void Vec4Impl<T>::setXYZ(const Vec3Impl<T> & xyz, const T ww)
{
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
	w = ww;
}

template<class T>
T * Vec4Impl<T>::getData()
{
	return reinterpret_cast<T *>(this);
}

template<class T>
const T * Vec4Impl<T>::getData() const
{
	return reinterpret_cast<const T *>(this);
}

template<class T>
Vec2Impl<T> Vec4Impl<T>::getVec2() const
{
	return Vec2Impl<T>(x, y);
}

template<class T>
Vec3Impl<T> Vec4Impl<T>::getVec3() const
{
	return Vec3Impl<T>(x, y, z);
}

template<class T>
T & Vec4Impl<T>::operator [] (const uint index)
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<T *>(this)[index];
}

template<class T>
T Vec4Impl<T>::operator [] (const uint index) const
{
	assert(index < getDimensions() && "Index out of range!");
	return reinterpret_cast<const T *>(this)[index];
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::operator + (const Vec4Impl<T> & other) const
{
	return Vec4Impl(x + other.x, y + other.y, z + other.z, w + other.w);
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::operator - (const Vec4Impl<T> & other) const
{
	return Vec4Impl(x - other.x, y - other.y, z - other.z, w - other.w);
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::operator * (const T scalar) const
{
	return Vec4Impl(x * scalar, y * scalar, z * scalar, w * scalar);
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::operator / (const T scalar) const
{
	return Vec4Impl(x / scalar, y / scalar, z / scalar, w / scalar);
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::operator += (const Vec4Impl<T> & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::operator -= (const Vec4Impl<T> & other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::operator *= (const T scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::operator /= (const T scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
	return *this;
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::operator - () const
{
	return Vec4Impl(-x, -y, -z, -w);
}

template<class T>
Vec4Impl<T> operator * (const T scalar, const Vec4Impl<T> & vec)
{
	return Vec4Impl<T>(scalar * vec.x, scalar * vec.y, scalar * vec.z, scalar * vec.w);
}

template<class T>
Vec4Impl<T> operator / (const T scalar, const Vec4Impl<T> & vec)
{
	return Vec4Impl<T>(scalar / vec.x, scalar / vec.y, scalar / vec.z, scalar / vec.w);
}

template<class T>
T Vec4Impl<T>::length() const
{
	return T(std::sqrt((x * x) + (y * y) + (z * z) + (w * w)));
}

template<class T>
T Vec4Impl<T>::sqrLength() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

template<class T>
T Vec4Impl<T>::invLength() const
{
	return T(1.0 / std::sqrt((x * x) + (y * y) + (z * z) + (w * w)));
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::normalize()
{
	const T invLen = invLength();
	x *= invLen;
	y *= invLen;
	z *= invLen;
	w *= invLen;
	return *this;
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::normalized() const
{
	Vec4Impl v(*this);
	v.normalize();
	return v;
}

template<class T>
T Vec4Impl<T>::distance(const Vec4Impl<T> & other) const
{
	return ((*this) - other).length();
}

template<class T>
T Vec4Impl<T>::sqrDistance(const Vec4Impl<T> & other) const
{
	return ((*this) - other).sqrLength();
}

template<class T>
T Vec4Impl<T>::dot(const Vec4Impl<T> & other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

template<class T>
T Vec4Impl<T>::dot3(const Vec3Impl<T> & other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z);
}

template<class T>
bool Vec4Impl<T>::equals(const Vec4Impl<T> & other) const
{
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

template<class T>
bool Vec4Impl<T>::equals(const Vec4Impl<T> & other, const T tolerance) const
{
	if (abs(x - other.x) > tolerance) { return false; }
	if (abs(y - other.y) > tolerance) { return false; }
	if (abs(z - other.z) > tolerance) { return false; }
	if (abs(w - other.w) > tolerance) { return false; }
	return true;
}

template<class T>
Vec4Impl<T> & Vec4Impl<T>::lerp(const Vec4Impl<T> & other, const T t)
{
	x = x + t * (other.x - x);
	y = y + t * (other.y - y);
	z = z + t * (other.z - z);
	w = w + t * (other.w - w);
	return *this;
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::origin()
{
	return Vec4Impl(T(0));
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::unitX()
{
	return Vec4Impl(T(1), T(0), T(0), T(0));
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::unitY()
{
	return Vec4Impl(T(0), T(1), T(0), T(0));
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::unitZ()
{
	return Vec4Impl(T(0), T(0), T(1), T(0));
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::unitW()
{
	return Vec4Impl(T(0), T(0), T(0), T(1));
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::positiveInf()
{
	return Vec4Impl(getPositiveInfinity<T>());
}

template<class T>
Vec4Impl<T> Vec4Impl<T>::negativeInf()
{
	return Vec4Impl(getNegativeInfinity<T>());
}

//
// Overload min/max/clamp for Vec4Impl (element wise):
//

template<class T>
Vec4Impl<T> clamp(Vec4Impl<T> v, const Vec4Impl<T> & minimum, const Vec4Impl<T> & maximum)
{
	if      (v.x < minimum.x) { v.x = minimum.x; }
	else if (v.x > maximum.x) { v.x = maximum.x; }
	if      (v.y < minimum.y) { v.y = minimum.y; }
	else if (v.y > maximum.y) { v.y = maximum.y; }
	if      (v.z < minimum.z) { v.z = minimum.z; }
	else if (v.z > maximum.z) { v.z = maximum.z; }
	if      (v.w < minimum.w) { v.w = minimum.w; }
	else if (v.w > maximum.w) { v.w = maximum.w; }
	return v;
}

template<class T>
Vec4Impl<T> min(const Vec4Impl<T> & a, const Vec4Impl<T> & b)
{
	return Vec4Impl<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));
}

template<class T>
Vec4Impl<T> max(const Vec4Impl<T> & a, const Vec4Impl<T> & b)
{
	return Vec4Impl<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w));
}

//
// Vector to string conversions:
//
template<class T>
inline std::string toString(const Vec4Impl<T> & v)
{
	return "[" +
		std::to_string(v.x) + ", " +
		std::to_string(v.y) + ", " +
		std::to_string(v.z) + ", " +
		std::to_string(v.w) + "]";
}

//
// Friendlier typedefs for Vec4Impl:
//

typedef Vec4Impl<float>    Vec4;
typedef Vec4Impl<int32_t>  Vec4i;
typedef Vec4Impl<uint32_t> Vec4u;
typedef Vec4Impl<uint8_t>  Vec4b;

static_assert(sizeof(Vec4)  == sizeof(float)    * 4, "Bad size for Vec4!");
static_assert(sizeof(Vec4i) == sizeof(int32_t)  * 4, "Bad size for Vec4i!");
static_assert(sizeof(Vec4u) == sizeof(uint32_t) * 4, "Bad size for Vec4u!");
static_assert(sizeof(Vec4b) == sizeof(uint8_t)  * 4, "Bad size for Vec4b!");

} // namespace internal {}

// ========================================================
// Public vector types declared by the library:
// ========================================================

// 2D generic vector:
using internal::Vec2;
using internal::Vec2i;
using internal::Vec2u;
using internal::Vec2b;

// 3D generic vector:
using internal::Vec3;
using internal::Vec3i;
using internal::Vec3u;
using internal::Vec3b;

// 4D generic vector:
using internal::Vec4;
using internal::Vec4i;
using internal::Vec4u;
using internal::Vec4b;

// min()/max()/clamp() overloads:
using internal::min;   // Element-wise min value
using internal::max;   // Element-wise max value
using internal::clamp; // Element-wise clamping

// Vector => string conversions:
using internal::toString;

} // namespace utils {}

#endif // UTILS_VECTORS_HPP
