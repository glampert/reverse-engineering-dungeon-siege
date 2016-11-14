
// ================================================================================================
// -*- C++ -*-
// File: filesys.cpp
// Author: Guilherme R. Lampert
// Created on: 12/12/14
// Brief: Some File-System and file handling helpers.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/filesys.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(WIN32) || defined(WIN64)
    #include <direct.h> // _mkdir
    // Copied from linux libc sys/stat.h:
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
    #define SIEGE_MAKE_DIR(dirname) _mkdir(dirname)
#else // !WINDOWS
    #include <unistd.h>
    #define SIEGE_MAKE_DIR(dirname) mkdir((dirname), 0777)
#endif // WINDOWS

namespace utils
{
namespace filesys
{

// ========================================================
// getPathSeparator():
// ========================================================

const char * getPathSeparator() noexcept
{
	return "/";
}

// ========================================================
// removeFilenameExtension():
// ========================================================

std::string removeFilenameExtension(const std::string & filename)
{
	const size_t lastDot = filename.find_last_of('.');
	if (lastDot == std::string::npos)
	{
		return filename;
	}
	return filename.substr(0, lastDot);
}

// ========================================================
// getFilenameExtension():
// ========================================================

std::string getFilenameExtension(const std::string & filename, const bool includeDot)
{
	std::string extension;
	const size_t lastDot = filename.find_last_of('.');
	if (lastDot != std::string::npos)
	{
		const long extensionChars = filename.length() - lastDot;
		if (extensionChars > 0)
		{
			extension.assign(filename, includeDot ? lastDot : (lastDot + 1), extensionChars);
		}
	}
	return extension;
}

// ========================================================
// queryFileSize():
// ========================================================

bool queryFileSize(const std::string & filename, size_t & sizeInBytes)
{
	assert(!filename.empty());

	errno = 0;
	struct stat statBuf = {};
	if (stat(filename.c_str(), &statBuf) == 0 && S_ISREG(statBuf.st_mode))
	{
		sizeInBytes = static_cast<size_t>(statBuf.st_size);
		return true;
	}

	sizeInBytes = 0;
	return false;
}

// ========================================================
// createDirectory():
// ========================================================

bool createDirectory(const std::string & dirPath)
{
	assert(!dirPath.empty());

	errno = 0;
	struct stat dirStat = {};
	if (stat(dirPath.c_str(), &dirStat) != 0)
	{
        if (SIEGE_MAKE_DIR(dirPath.c_str()) != 0)
		{
			return false;
		}
	}
	else // Path already exists:
	{
		if (!S_ISDIR(dirStat.st_mode))
		{
			// Looks like there is a file with the same name
			// as the directory we are trying to create!
			return false;
		}
	}

	return true;
}

// ========================================================
// createPath():
// ========================================================

bool createPath(const std::string & pathEndedWithSeparatorOrFilename)
{
	char dirPath[MaxTempStringLen];

	assert(!pathEndedWithSeparatorOrFilename.empty());
	assert(pathEndedWithSeparatorOrFilename.length() < arrayLength(dirPath) && "Pathname too long!");

	std::strncpy(dirPath, pathEndedWithSeparatorOrFilename.c_str(), arrayLength(dirPath) - 1);
	char * pPath = dirPath;

	while (*pPath != '\0')
	{
		// Works for both Win and Unix without the need for extra tweaks.
		if (*pPath == '/' || *pPath == '\\')
		{
			*pPath = '\0';
			if (!createDirectory(dirPath))
			{
				return false;
			}
			*pPath = *getPathSeparator();
		}
		++pPath;
	}

	return true;
}

// ========================================================
// tryOpen() for ofstream and ifstream:
// ========================================================

bool tryOpen(std::ofstream & file, const std::string & filename, const std::ofstream::open_mode mode)
{
	assert(!filename.empty());
	assert(!file.is_open()); // Close it before calling this!

	errno = 0;
	file.exceptions(0); // Don't throw on error.
	file.open(filename, mode | std::ofstream::out);

	return file.is_open() && file.good();
}

bool tryOpen(std::ifstream & file, const std::string & filename, const std::ifstream::open_mode mode)
{
	assert(!filename.empty());
	assert(!file.is_open()); // Close it before calling this!

	errno = 0;
	file.exceptions(0); // Don't throw on error.
	file.open(filename, mode | std::ifstream::in);

	return file.is_open() && file.good();
}

// ========================================================
// getLastFileError():
// ========================================================

std::string getLastFileError()
{
	std::string str{ std::strerror(errno) };
	errno = 0;
	return str;
}

} // namespace filesys {}
} // namespace utils {}
