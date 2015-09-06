
// ================================================================================================
// -*- C++ -*-
// File: helper_types.cpp
// Author: Guilherme R. Lampert
// Created on: 10/11/14
// Brief: Implementation for the functions declared on helper_types.hpp.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "helper_types.hpp"
#include <cstdio> // For std::snprintf()

namespace siege
{

// ========================================================
// FourCC:
// ========================================================

bool operator == (const FourCC a, const FourCC b) noexcept
{
	return (a.c0 == b.c0) && (a.c1 == b.c1) &&
	       (a.c2 == b.c2) && (a.c3 == b.c3);
}

bool operator != (const FourCC a, const FourCC b) noexcept
{
	return (a.c0 != b.c0) || (a.c1 != b.c1) ||
	       (a.c2 != b.c2) || (a.c3 != b.c3);
}

bool operator == (const FourCC a, const char fccStr[]) noexcept
{
	return (a.c0 == fccStr[0]) && (a.c1 == fccStr[1]) &&
	       (a.c2 == fccStr[2]) && (a.c3 == fccStr[3]);
}

bool operator != (const FourCC a, const char fccStr[]) noexcept
{
	return (a.c0 != fccStr[0]) || (a.c1 != fccStr[1]) ||
	       (a.c2 != fccStr[2]) || (a.c3 != fccStr[3]);
}

std::ostream & operator << (std::ostream & s, const FourCC fcc)
{
	s << static_cast<char>(fcc.c0) << static_cast<char>(fcc.c1)
	  << static_cast<char>(fcc.c2) << static_cast<char>(fcc.c3);
	return s;
}

// ========================================================
// ProductVersion:
// ========================================================

std::string versionWordToStr(const uint32_t version)
{
	const uint32_t major = (version & 0x00FFA000) >> 16;
	const uint32_t minor = (version & 0x0000FF00) >> 8;
	const uint32_t build = (version & 0x000000FF);

	char verStr[256];

	// Print as: major.minor.build
	std::sprintf(verStr, "%u.%u.%u", major, minor, build);
	verStr[sizeof(verStr) - 1] = '\0';

	return verStr;
}

std::ostream & operator << (std::ostream & s, const ProductVersion & version)
{
	s << versionWordToStr(version.version1) << "  "
	  << versionWordToStr(version.version2) << "  "
	  << versionWordToStr(version.version3);
	return s;
}

// ========================================================
// SystemTime:
// ========================================================

std::ostream & operator << (std::ostream & s, const SystemTime & time)
{
	char timeStr[512];

	// Print as: dd/mm/yyy  hh:mm:ss
	std::sprintf(timeStr, "%02d/%02d/%04d  %02d:%02d:%02d",
		time.day,  time.month,  time.year,
		time.hour, time.minute, time.second);

	timeStr[sizeof(timeStr) - 1] = '\0';
	s << timeStr;
	return s;
}

// ========================================================
// Guid:
// ========================================================

std::ostream & operator << (std::ostream & s, const Guid & guid)
{
	char guidStr[1024];

	//
	// Sample: a42790e0-7810-11cf-8f52-0040333594a3
	//
	std::sprintf(guidStr,
		"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		guid.data1,
		guid.data2,
		guid.data3,
		guid.data4[0],
		guid.data4[1],
		guid.data4[2],
		guid.data4[3],
		guid.data4[4],
		guid.data4[5],
		guid.data4[6],
		guid.data4[7]);

	guidStr[sizeof(guidStr) - 1] = '\0';
	s << guidStr;
	return s;
}

// ========================================================
// FileTime:
// ========================================================

uint64_t FileTime::toU64() const
{
	return static_cast<uint64_t>(highDateTime) << 32 | lowDateTime;
}

time_t FileTime::toPortableTime() const
{
	constexpr uint64_t TicksPerSecond  = 10000000;
	constexpr uint64_t EpochDifference = 11644473600UL;

	uint64_t input = toU64();
	uint64_t temp  = input / TicksPerSecond; // Convert from 100ns intervals to seconds
	temp = temp - EpochDifference;           // Subtract number of seconds between epochs

	return static_cast<time_t>(temp);
}

std::ostream & operator << (std::ostream & s, const FileTime ft)
{
	// Detect a null FileTime:
	if (ft.toU64() == 0)
	{
		return s << "<NULL>";
	}

	const time_t t = ft.toPortableTime();
	std::string ftStr = std::ctime(&t);

	// Remove the '\n' always added by ctime():
	if (!ftStr.empty())
	{
		ftStr.pop_back();
	}
	return s << ftStr;
}

} // namespace siege {}
