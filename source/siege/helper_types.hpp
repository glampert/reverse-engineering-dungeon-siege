#pragma once
// ================================================================================================
// -*- C++ -*-
// File: helper_types.hpp
// Author: Guilherme R. Lampert
// Created on: 10/11/14
// Brief: Helper types and structs used by Tank Files and other DS resources.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include <cstdint>
#include <ctime>
#include <ostream>
#include <string>

namespace siege
{

#pragma pack(push, 1)

// ========================================================
// FourCC:
// ========================================================

struct FourCC final
{
	uint8_t c0, c1, c2, c3;
};

bool operator == (FourCC a, FourCC b) noexcept;
bool operator != (FourCC a, FourCC b) noexcept;

bool operator == (FourCC a, const char fccStr[]) noexcept;
bool operator != (FourCC a, const char fccStr[]) noexcept;

std::ostream & operator << (std::ostream & s, FourCC fcc);

// ========================================================
// ProductVersion:
// ========================================================

struct ProductVersion final
{
	uint32_t version1;
	uint32_t version2;
	uint32_t version3;
};

std::string versionWordToStr(uint32_t version);
std::ostream & operator << (std::ostream & s, const ProductVersion & version);

// Pack three version bytes into a single 32bit word.
// First byte of the word is unused.
inline constexpr uint32_t makeVersionWord(const uint32_t major,
                                          const uint32_t minor,
                                          const uint32_t build)
{
	return static_cast<uint32_t>(((major & 0xFF) << 16) |
	                             ((minor & 0xFF) << 8)  |
	                              (build & 0xFF));
}

// ========================================================
// SystemTime:
// ========================================================

struct SystemTime final
{
	// Replaces the non-portable Windows SYSTEMTIME.
	uint16_t year;
	uint16_t month;
	uint16_t dayOfWeek;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	uint16_t milliseconds;
};

std::ostream & operator << (std::ostream & s, const SystemTime & time);

// ========================================================
// Guid:
// ========================================================

struct Guid final
{
	// Same as a Windows GUID.
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t  data4[8];
};

std::ostream & operator << (std::ostream & s, const Guid & guid);

// ========================================================
// FileTime:
// ========================================================

struct FileTime final
{
	// Replaces a Windows FILETIME.
	// This is actually an uint64_t broken in two parts.
	// Heritage from the olden 16bits era, I guess?
	uint32_t lowDateTime  = 0;
	uint32_t highDateTime = 0;

	uint64_t toU64() const;
	time_t toPortableTime() const;
};

std::ostream & operator << (std::ostream & s, FileTime ft);

#pragma pack(pop)

} // namespace siege {}
