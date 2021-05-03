#pragma once
// ================================================================================================
// -*- C++ -*-
// File: common.hpp
// Author: Guilherme R. Lampert
// Created on: 22/07/14
// Brief: Common functions and helpers for LibSiege.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/utils.hpp"

// ========================================================

#ifndef SiegeLogStream
	#if SIEGE_LOG_FORCE_STDOUT
		#include <iostream>
		#define SiegeLogStream std::cout
	#else // !SIEGE_LOG_FORCE_STDOUT
		#define SiegeLogStream ::siege::getDefaultLogStream()
	#endif // SIEGE_LOG_FORCE_STDOUT
#endif // SiegeLogStream

// ========================================================

#if SIEGE_ENABLE_LOGGING
	#define SiegeLog(message) \
		if (int(::siege::defaultLogVerbosity) >= int(::siege::LogVerbosity::All)) \
		{ \
			SiegeLogStream << "LOG...: " << message << "\n"; \
		}

	#define SiegeWarn(message) \
		if (int(::siege::defaultLogVerbosity) >= int(::siege::LogVerbosity::Warnings)) \
		{ \
			SiegeLogStream << "WARN..: " << message << "\n"; \
		}

	#define SiegeError(message) \
		if (int(::siege::defaultLogVerbosity) >= int(::siege::LogVerbosity::Errors)) \
		{ \
			SiegeLogStream << "ERROR.: " << message << "\n"; \
		}
#else // !SIEGE_ENABLE_LOGGING
	#define SiegeLog(message)
	#define SiegeWarn(message)
	#define SiegeError(message)
#endif // SIEGE_ENABLE_LOGGING

// ========================================================

namespace siege
{

// Handy type alias for a frequently used vector of bytes.
using ByteArray = std::vector<uint8_t>;

// ========================================================
// 16bit Wide Character:
// ========================================================

// Dungeon Siege used WCHAR or wchar_t for some text strings,
// which is 16bits wide on Windows. They are saved to file
// as a pair of bytes so we need to use a properly sized type,
// since the assumption that wchar_t is 16bits everywhere is
// false. It is 32bits wide on Mac and Linux.
using WideChar = char16_t;

// Redefine std::string for the WideChar type.
using WideString = std::basic_string<WideChar>;

// Convert a 2-bytes WideChar string to a std::string.
std::string wideStringToStdString(const WideString & wStr);

// ========================================================
// Default log stream:
// ========================================================

// The default log is a file. It is opened on the first reference to it.
// Closed when the C++ destructors are run.
std::string getDefaultLogFileName();
std::ostream & getDefaultLogStream();

// Set the default log filename returned by `getDefaultLogFileName()`
// and the default stream reference returned by `getDefaultLogStream()`.
// Make sure the stream provided stays alive until the application shuts down!
void setDefaultLogFileName(const char * filename) noexcept;
void setDefaultLogStream(std::ostream & ostr) noexcept;

// Verbosity levels of the default Siege log.
// NOTE: Not thread safe! Default value is `LogVerbosity::All`.
enum class LogVerbosity { Silent, Errors, Warnings, All };
extern LogVerbosity defaultLogVerbosity;

// ========================================================
// Exception:
// ========================================================

// Base exception type used by the Siege Library.
class Exception
	: public std::exception
{
public:

	// Max length in chars of an error/exception message.
	// Including a null terminator at the end.
	static constexpr int MaxMessageLen = 1024;

	Exception() noexcept;
	Exception(const char * error) noexcept;
	Exception(const std::string & error) noexcept;

	const char * what() const noexcept;
	virtual ~Exception();

protected:

	void clearString() noexcept;

private:

	char errorMessage[MaxMessageLen];
};

// ========================================================
// SiegeThrow() macro:
// ========================================================

#define SiegeThrow(exceptionType, message) \
	do { \
		std::ostringstream ostr; \
		ostr << message; \
		throw exceptionType(ostr.str()); \
	} while (0)

} // namespace siege {}
