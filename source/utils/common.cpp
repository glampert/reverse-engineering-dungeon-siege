
// ================================================================================================
// -*- C++ -*-
// File: common.cpp
// Author: Guilherme R. Lampert
// Created on: 03/09/15
// Brief: Common functions and helpers for LibUtils.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/common.hpp"

namespace utils
{

// ========================================================
// format();
// ========================================================

std::string format(const char * format, ...)
{
	assert(format != nullptr);

	va_list vaList;
	char buffer[MaxTempStringLen];

	va_start(vaList, format);
	const int result = std::vsnprintf(buffer, MaxTempStringLen, format, vaList);
	va_end(vaList);

	if (result < 0)
	{
		assert(false && "vsnprintf() failed!");
		clearArray(buffer); // Clear the string.
		return buffer;
	}
	if (result >= MaxTempStringLen)
	{
		assert(false && "vsnprintf() overflowed buffer!");
		buffer[MaxTempStringLen - 1] = '\0'; // Truncate the string.
		return buffer;
	}

	buffer[result] = '\0';
	return buffer;
}

// ========================================================
// vformat():
// ========================================================

std::string vformat(const char * format, va_list vaList)
{
	assert(format != nullptr);

	char buffer[MaxTempStringLen];
	const int result = std::vsnprintf(buffer, MaxTempStringLen, format, vaList);

	if (result < 0)
	{
		assert(false && "vsnprintf() failed!");
		clearArray(buffer); // Clear the string.
		return buffer;
	}
	if (result >= MaxTempStringLen)
	{
		assert(false && "vsnprintf() overflowed buffer!");
		buffer[MaxTempStringLen - 1] = '\0'; // Truncate the string.
		return buffer;
	}

	buffer[result] = '\0';
	return buffer;
}

// ========================================================
// removeTrailingFloatZeros():
// ========================================================

std::string removeTrailingFloatZeros(const std::string & floatStr)
{
	// Only process if the number is decimal (has a dot somewhere):
	if (floatStr.find_last_of('.') == std::string::npos)
	{
		return floatStr;
	}

	std::string trimmed(floatStr);

	// Remove trailing zeros:
	while (!trimmed.empty() && (trimmed.back() == '0'))
	{
		trimmed.pop_back();
	}

	// If the dot was left alone at the end, remove it too:
	if (!trimmed.empty() && (trimmed.back() == '.'))
	{
		trimmed.pop_back();
	}

	return trimmed;
}

// ========================================================
// formatMemoryUnit():
// ========================================================

std::string formatMemoryUnit(const uint64_t memorySizeInBytes, const bool abbreviated)
{
	const char * memUnitStr;
	double adjustedSize;
	char numStrBuf[128];

	if (memorySizeInBytes < 1024)
	{
		memUnitStr   = (abbreviated ? "B" : "Bytes");
		adjustedSize = static_cast<double>(memorySizeInBytes);
	}
	else if (memorySizeInBytes < (1024 * 1024))
	{
		memUnitStr   = (abbreviated ? "KB" : "Kilobytes");
		adjustedSize = (memorySizeInBytes / 1024.0);
	}
	else if (memorySizeInBytes < (1024 * 1024 * 1024))
	{
		memUnitStr   = (abbreviated ? "MB" : "Megabytes");
		adjustedSize = (memorySizeInBytes / 1024.0 / 1024.0);
	}
	else
	{
		memUnitStr   = (abbreviated ? "GB" : "Gigabytes");
		adjustedSize = (memorySizeInBytes / 1024.0 / 1024.0 / 1024.0);
	}

	// We only care about the first 2 decimal digits.
	std::snprintf(numStrBuf, sizeof(numStrBuf), "%.2f", adjustedSize);

	// Remove trailing zeros if no significant decimal digits:
	return removeTrailingFloatZeros(numStrBuf) + std::string(" ") + memUnitStr;
}

// ========================================================
// computeCrc32():
// ========================================================

uint32_t computeCrc32(const void * data, size_t sizeBytes) noexcept
{
	assert(data != nullptr);
	assert(sizeBytes != 0);

	//
	// This compact CRC 32 algo was adapted from miniz.c, which in turn was taken from
	// "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed"
	// By Karl Malbrain.
	//
	static const uint32_t crcTable[16] =
	{
		0,
		0x1DB71064,
		0x3B6E20C8,
		0x26D930AC,
		0x76DC4190,
		0x6B6B51F4,
		0x4DB26158,
		0x5005713C,
		0xEDB88320,
		0xF00F9344,
		0xD6D6A3E8,
		0xCB61B38C,
		0x9B64C2B0,
		0x86D3D2D4,
		0xA00AE278,
		0xBDBDF21C
	};

	const uint8_t * ptr = reinterpret_cast<const uint8_t *>(data);
	uint32_t crcu32 = 0;

	crcu32 = ~crcu32;
	while (sizeBytes--)
	{
		uint8_t b = *ptr++;
		crcu32 = (crcu32 >> 4) ^ crcTable[(crcu32 & 0xF) ^ (b & 0xF)];
		crcu32 = (crcu32 >> 4) ^ crcTable[(crcu32 & 0xF) ^ (b >> 4) ];
	}
	return ~crcu32;
}

} // namespace utils {}
