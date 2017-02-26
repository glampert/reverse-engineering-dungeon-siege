
// ================================================================================================
// -*- C++ -*-
// File: common.cpp
// Author: Guilherme R. Lampert
// Created on: 22/07/14
// Brief: Common functions and helpers for LibSiege.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "siege/common.hpp"
#include <fstream>

namespace siege
{

// ========================================================
// wideStringToStdString():
// ========================================================

std::string wideStringToStdString(const WideString & wStr)
{
	static_assert(sizeof(WideChar) == 2, "This will only work if we are dealing with 2-byte wchars!");

	//
	// Convert 2-byte long Windows wchar_t string to a C string.
	//
	// Currently not doing a proper conversion, just grabbing
	// the lower byte of each WideChar.
	//
	// Also uses a fixed size buffer, so string length is
	// limited to MaxTempStringLen!
	//

	if (wStr.empty())
	{
		return std::string();
	}

	int i;
	char temBuf[utils::MaxTempStringLen];

	for (i = 0; i < (utils::MaxTempStringLen - 1); ++i)
	{
		const char c = static_cast<char>(wStr[i] & 0x00FF);
		temBuf[i] = c;
		if (c == 0)
		{
			break;
		}
	}

	if (i == (utils::MaxTempStringLen - 1))
	{
		SiegeError("String overflow at wideStringToStdString()!");
	}

	temBuf[i] = '\0';
	return temBuf;
}

// ========================================================
// Default log stream:
// ========================================================

LogVerbosity defaultLogVerbosity  = LogVerbosity::All;
static const char   * logFilename = "lib_siege.log";
static std::ostream * logStream   = nullptr;

std::string getDefaultLogFileName()
{
	return logFilename;
}

std::ostream & getDefaultLogStream()
{
	if (logStream != nullptr)
	{
		return *logStream;
	}

	static std::ofstream logFile(getDefaultLogFileName(), std::ofstream::out | std::ofstream::trunc);
	logStream = &logFile;

	return *logStream;
}

void setDefaultLogFileName(const char * filename) noexcept
{
	logFilename = filename;
}

void setDefaultLogStream(std::ostream & ostr) noexcept
{
	logStream = &ostr;
}

// ========================================================
// Siege Exception:
// ========================================================

Exception::Exception() noexcept
{
	clearString();
}

Exception::Exception(const std::string & error) noexcept
	: Exception(error.c_str())
{
}

Exception::Exception(const char * error) noexcept
{
	if (error != nullptr)
	{
#ifdef _MSC_VER
        strcpy_s(errorMessage, error);
#else // _MSC_VER
		std::strncpy(errorMessage, error, sizeof(errorMessage));
#endif // _MSC_VER
	}
	else
	{
		clearString();
	}

	SiegeError("**** Raising exception: " << error << " ****");
}

Exception::~Exception()
{
	// No-op
}

const char * Exception::what() const noexcept
{
	return errorMessage;
}

void Exception::clearString() noexcept
{
	utils::clearArray(errorMessage);
}

} // namespace siege {}
