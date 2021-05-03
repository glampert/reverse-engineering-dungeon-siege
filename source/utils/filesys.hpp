#pragma once
// ================================================================================================
// -*- C++ -*-
// File: filesys.hpp
// Author: Guilherme R. Lampert
// Created on: 12/12/14
// Brief: Some File-System and file handling helpers.
//
// This project's source code is released under the MIT License.
// - http://opensource.org/licenses/MIT
//
// ================================================================================================

#include "utils/common.hpp"
#include <fstream>

namespace utils
{
namespace filesys
{

// Return the standard path separator for this platform. "/" for Unix, "\" for Windows.
const char * getPathSeparator() noexcept PURE_FUNC_DECL;

// Strips the extension of a filename, returning just the filename. Does nothing if no ext present.
std::string removeFilenameExtension(const std::string & filename);

// Returns the extension of a filename (everything after the last dot)
// or an empty string if no dot in the name. Can optionally exclude the dot from the output.
std::string getFilenameExtension(const std::string & filename, bool includeDot = true);

// Tries to get the size in bytes of a file, if `filename` exits and is a file.
bool queryFileSize(const std::string & filename, size_t & sizeInBytes);

// Creates a single directory at an existing path. Fails with no side-effects if the dir already exists.
bool createDirectory(const std::string & dirPath);

// Creates a full path of directories. Fails with no side-effects if path already exists.
bool createPath(const std::string & pathEndedWithSeparatorOrFilename);

// Attempts to open the file as a C++ stream, without throwing an exception if it fails.
// This will clear `errno` before attempting to open the file, so you can getLastFileError()
// if this function fails to get an error description string for debug printing.
bool tryOpen(std::ofstream & file, const std::string & filename, std::ofstream::open_mode mode = 0);
bool tryOpen(std::ifstream & file, const std::string & filename, std::ifstream::open_mode mode = 0);

// Same as 'std::strerror(error)'. Sets `errno` to zero.
std::string getLastFileError();

} // namespace filesys {}
} // namespace utils {}
